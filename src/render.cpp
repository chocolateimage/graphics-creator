#include "render.hpp"
#include "animatable/element/element.hpp"
#include "math.hpp"
#include <QDebug>
#include <QImage>
#include <QMutexLocker>
#include <freetype/ftoutln.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
}

ImageData::~ImageData() { delete[] data; }

VideoData::VideoData(const std::string &path) {
    qDebug() << "Creating video";

    if (avformat_open_input(&fmtCtx, path.c_str(), nullptr, nullptr) < 0) {
        error = true;
        return;
    }
    if (avformat_find_stream_info(fmtCtx, nullptr) < 0) {
        error = true;
        return;
    }

    streamIndex =
        av_find_best_stream(fmtCtx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (streamIndex < 0) {
        error = true;
        return;
    }

    stream = fmtCtx->streams[streamIndex];

    const AVCodec *codec = avcodec_find_decoder(stream->codecpar->codec_id);
    if (!codec) {
        error = true;
        return;
    }

    decodeCtx = avcodec_alloc_context3(codec);
    if (!decodeCtx) {
        error = true;
        return;
    }

    if (avcodec_parameters_to_context(decodeCtx, stream->codecpar) < 0) {
        error = true;
        return;
    }

    decodeCtx->thread_count = 0;
    decodeCtx->thread_type = FF_THREAD_SLICE;

    if (avcodec_open2(decodeCtx, codec, nullptr) < 0) {
        error = true;
        return;
    }

    packet = av_packet_alloc();
    frame = av_frame_alloc();

    streamDuration = stream->duration;
    if (streamDuration == AV_NOPTS_VALUE) {
        qWarning() << "Duration not found from stream";
        streamDuration = av_rescale_q(fmtCtx->duration, {1, AV_TIME_BASE},
                                      stream->time_base);
    }

    durationSeconds = (double)fmtCtx->duration / AV_TIME_BASE;

    qDebug() << "Created video. Size:" << decodeCtx->width << "x"
             << decodeCtx->height << "Duration:" << streamDuration;
}

AVFrame *VideoData::getFrame(double seconds, int w, int h, int scaleFlags) {
    int64_t timestamp = av_rescale_q(seconds * AV_TIME_BASE, {1, AV_TIME_BASE},
                                     stream->time_base);
    if (timestamp >= streamDuration) {
        return nullptr;
    }

    PriorityMutexLocker locker(&mutex, seconds);

    swsCtx = sws_getCachedContext(swsCtx, decodeCtx->width, decodeCtx->height,
                                  decodeCtx->pix_fmt, w, h, AV_PIX_FMT_BGRA,
                                  scaleFlags, nullptr, nullptr, nullptr);
    if (!swsCtx) {
        return nullptr;
    }

    if (lastSecond == seconds) {
        return scaleCurrentFrame();
    }
    av_frame_unref(frame);

    bool seek =
        lastSecond == -1 || seconds <= lastSecond || seconds - lastSecond > 1;

    if (seek) {
        if (av_seek_frame(fmtCtx, streamIndex, timestamp,
                          AVSEEK_FLAG_BACKWARD) < 0) {
            return nullptr;
        }

        avcodec_flush_buffers(decodeCtx);
    }

    AVFrame *vdFrame{nullptr};

    while (av_read_frame(fmtCtx, packet) >= 0) {
        if (packet->stream_index != streamIndex) {
            av_packet_unref(packet);
            continue;
        }

        int ret = avcodec_send_packet(decodeCtx, packet);
        av_packet_unref(packet);
        if (ret < 0) {
            qWarning() << "Error submitting a packet for decoding."
                       << av_err2str(ret);
            continue;
        }

        while (true) {
            ret = avcodec_receive_frame(decodeCtx, frame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                break;
            } else if (ret < 0) {
                break;
            }

            if (frame->pts != AV_NOPTS_VALUE && frame->pts < timestamp) {
                av_frame_unref(frame);
                continue;
            }

            if (decodeCtx->codec->type == AVMEDIA_TYPE_VIDEO) {
                lastSecond = seconds;
                vdFrame = scaleCurrentFrame();
            } else {
                qWarning() << "Non video frame";
            }

            break;
        }
        if (vdFrame) {
            break;
        }
    }

    return vdFrame;
}

AVFrame *VideoData::scaleCurrentFrame() {
    AVFrame *vdFrame = av_frame_alloc();
    vdFrame->width = swsCtx->dst_w;
    vdFrame->height = swsCtx->dst_h;
    vdFrame->format = AV_PIX_FMT_BGRA;
    av_frame_get_buffer(vdFrame, 0);
    sws_scale_frame(swsCtx, vdFrame, frame);
    return vdFrame;
}

VideoData::~VideoData() {
    qDebug() << "Freeing video";
    if (swsCtx) {
        sws_freeContext(swsCtx);
    }
    if (decodeCtx) {
        avcodec_free_context(&decodeCtx);
    }
    if (fmtCtx) {
        avformat_close_input(&fmtCtx);
    }
    if (packet) {
        av_packet_free(&packet);
    }
    if (frame) {
        av_frame_free(&frame);
    }
}

std::shared_ptr<ImageData> Loader::loadImage(const std::string &path) {
    QMutexLocker locker(&imageDatasMutex);
    auto it = imageDatas.find(path);
    if (it != imageDatas.end()) {
        return it->second;
    }

    QImage img(QString::fromStdString(path));
    img = img.convertToFormat(QImage::Format_ARGB32);

    std::shared_ptr<ImageData> data = std::make_shared<ImageData>();
    data->width = img.width();
    data->height = img.height();
    data->data = new uint32_t[data->width * data->height];
    memcpy(data->data, img.bits(), data->width * data->height * 4);
    imageDatas.emplace(path, data);
    return data;
}

std::shared_ptr<VideoData> Loader::loadVideo(const std::string &path) {
    QMutexLocker locker(&videoDatasMutex);
    auto it = videoDatas.find(path);
    if (it != videoDatas.end()) {
        return it->second;
    }

    std::shared_ptr<VideoData> data = std::make_shared<VideoData>(path);
    videoDatas.emplace(path, data);
    return data;
}

Loader globalLoader;

std::string getFontHash(const Font &font, int fontSize, bool antialiased,
                        const StrokeInfo &strokeInfo) {
    return font.path + "\n" + std::to_string(fontSize) + "\n" +
           (antialiased ? "1" : "0") + "\n" + std::to_string(font.index) +
           "\n" + strokeInfo.hash();
}

FontInfo::FontInfo(FT_Face face, hb_font_t *hb, int pixelHeight, Font font)
    : font(font), face(face), hb(hb), pixelHeight(pixelHeight) {};

FontInfo::~FontInfo() {
    static int deleteCount = 0;
    for (auto &glyph : glyphs) {
        FT_Done_Glyph((FT_Glyph)(glyph.second));
    }
    hb_font_destroy(hb);
    FT_Done_Face(face);
}

FT_BitmapGlyph FontInfo::getGlyph(hb_codepoint_t codepoint) {
    auto it = glyphs.find(codepoint);
    if (it != glyphs.end()) {
        return it->second;
    }

    FT_Glyph _glyph;
    FT_Load_Glyph(face, codepoint, FT_LOAD_COLOR | FT_LOAD_NO_HINTING);

    FT_Render_Mode renderMode =
        antialiased ? FT_RENDER_MODE_NORMAL : FT_RENDER_MODE_MONO;

    if (strokeInfo.strokeWidth > 0) {
        FT_Stroker stroker;
        FT_Stroker_New(fontManager->ftLibrary, &stroker);
        FT_Stroker_Set(stroker, pixelHeight * strokeInfo.strokeWidth,
                       FT_STROKER_LINECAP_SQUARE, strokeInfo.lineJoin, 0);
        FT_Get_Glyph(face->glyph, &_glyph);
        FT_Glyph_Stroke(&_glyph, stroker, true);
        FT_Glyph_To_Bitmap(&_glyph, renderMode, 0, true);
        FT_Stroker_Done(stroker);
    } else {
        FT_Render_Glyph(face->glyph, renderMode);
        FT_Get_Glyph(face->glyph, &_glyph);
    }

    FT_BitmapGlyph glyph = (FT_BitmapGlyph)_glyph;
    glyphs.emplace(codepoint, glyph);
    return glyph;
}

FontManager::FontManager() { FT_Init_FreeType(&ftLibrary); }

FontInfo *FontManager::getFont(const Font &font, int fontSize, bool antialiased,
                               const StrokeInfo &strokeInfo) {
    std::string hash = getFontHash(font, fontSize, antialiased, strokeInfo);
    auto it = loadedFonts.find(hash);
    if (it != loadedFonts.end()) {
        FontInfo *fontInfo = it->second;
        fontInfo->framesUnused = 0;
        return fontInfo;
    }

    int pixelHeight = fontSize;

    FT_Face ftFace;
    FT_Error error =
        FT_New_Face(ftLibrary, font.path.c_str(), font.index, &ftFace);
    if (error != 0) {
        qWarning() << "Font could not be loaded!" << font.path << "index"
                   << font.index;
        auto font = Variant::defaultFont;
        FT_New_Face(ftLibrary, font.path.c_str(), font.index, &ftFace);
    }
    FT_Set_Pixel_Sizes(ftFace, 0, pixelHeight);

    hb_font_t *hbFont = hb_ft_font_create(ftFace, nullptr);
    FontInfo *fontInfo = new FontInfo(ftFace, hbFont, pixelHeight, font);
    fontInfo->antialiased = antialiased;
    fontInfo->strokeInfo = strokeInfo;
    fontInfo->fontManager = this;
    loadedFonts.emplace(hash, fontInfo);
    return fontInfo;
}

void FontManager::garbageCollect() {
    std::vector<std::string> toRemove;
    for (auto &fontInfo : loadedFonts) {
        if (++fontInfo.second->framesUnused > 10) {
            delete fontInfo.second;
            toRemove.push_back(fontInfo.first);
        }
    }
    for (const auto &key : toRemove) {
        loadedFonts.erase(key);
    }
}

FontManager::~FontManager() {
    for (auto loadedFont : loadedFonts) {
        delete loadedFont.second;
    }
    loadedFonts.clear();
    FT_Done_FreeType(ftLibrary);
}

void RenderThread::init() { fontManager = new FontManager(); }

void RenderThread::garbageCollect() { fontManager->garbageCollect(); }

ElementSelectionSnippet
RenderThread::getSnippet(const ElementSelection &selection) {
    if (currentFrameTask->currentElementStack.contains(selection.elementId)) {
        return {{0, 0, 0, 0}, nullptr};
    }

    currentFrameTask->currentElementStack.append(selection.elementId);

    RenderedElement *renderedElement;

    auto it = currentFrameTask->renderedElements.find(selection.elementId);
    if (it == currentFrameTask->renderedElements.end()) {
        ElementRender *element{nullptr};
        for (auto renderElement : currentFrameTask->renderElements) {
            if (renderElement->id == selection.elementId) {
                element = renderElement;
                break;
            }
        }
        if (!element) {
            return {{0, 0, 0, 0}, nullptr};
        }
        renderedElement = new RenderedElement(element);
        if (renderedElement->hasError) {
            delete renderedElement;
            return {{0, 0, 0, 0}, nullptr};
        }
        currentFrameTask->renderedElements.emplace(element->id,
                                                   renderedElement);
    } else {
        renderedElement = it->second;
    }

    if (selection.frameType == ElementSelection::Source) {
        return {renderedElement->elementRect, renderedElement->elementValues};
    } else {
        return {renderedElement->finalRect, renderedElement->finalValues};
    }
}

void RenderThread::close() {
    delete fontManager;
    fontManager = nullptr;
}

RenderedElement::RenderedElement(ElementRender *element) : element(element) {
    elementRect = element->getRenderBox();
    elementValues = new uint32_t[elementRect.w * elementRect.h];
    memset(elementValues, 0, elementRect.w * elementRect.h * 4);

    finalRect = elementRect;
    finalValues = elementValues;

    bool success = element->render(elementValues);

    if (!success) {
        delete[] elementValues;
        hasError = true;
        return;
    }

    for (auto effect : element->effects) {
        effect->renderThread = element->renderThread;
        effect->currentFrame = element->currentFrame;
        effect->currentSeconds = element->currentSeconds;
        effect->originalBox = elementRect;
        effect->originalValues = elementValues;
        Rect effectBox = effect->getRenderBox(finalRect);
        effect->renderBox = effectBox;
        uint32_t *__restrict__ effectValues =
            new uint32_t[effectBox.w * effectBox.h];
        memset(effectValues, 0, effectBox.w * effectBox.h * 4);

        bool success = effect->render(finalValues, finalRect, effectValues);
        if (!success) {
            delete[] effectValues;
            continue;
        }

        if (elementValues != finalValues) {
            delete[] finalValues;
        }
        finalValues = effectValues;
        finalRect = effectBox;
    }
}

RenderedElement::~RenderedElement() {
    if (elementValues != finalValues) {
        delete[] finalValues;
    }
    delete[] elementValues;
}

void FrameTask::render(RenderThread &renderThread) {
    uint32_t *__restrict__ frameValues = new uint32_t[width * height];
    memset(frameValues, 0, width * height * 4);

    renderThread.currentFrameTask = this;

    for (auto element : renderElements) {
        element->renderThread = &renderThread;
        element->currentFrame = frame;
        element->currentSeconds = seconds;
        element->prepare();
    }

    for (auto element : renderElements) {
        if (!element->visible)
            continue;

        if (frame < element->startFrame ||
            frame >= element->startFrame + element->durationFrames) {
            continue;
        }

        if (element->hasParent())
            continue;

        RenderedElement *renderedElement;

        currentElementStack.clear();
        currentElementStack.append(element->id);
        auto it = renderedElements.find(element->id);
        if (it == renderedElements.end()) {
            renderedElement = new RenderedElement(element);
            if (renderedElement->hasError) {
                delete renderedElement;
                continue;
            }
            renderedElements.emplace(element->id, renderedElement);
        } else {
            renderedElement = it->second;
        }

        Rect &finalRect = renderedElement->finalRect;
        uint32_t *__restrict__ finalValues = renderedElement->finalValues;

        int maxY = std::min(height, finalRect.y + finalRect.h) - finalRect.y;
        int maxX = std::min(width, finalRect.x + finalRect.w) - finalRect.x;

        for (int y = std::max(0, -finalRect.y); y < maxY; y++) {
            for (int x = std::max(0, -finalRect.x); x < maxX; x++) {
                auto index =
                    pixelIndex(x + finalRect.x, y + finalRect.y, width);
                frameValues[index] =
                    over(frameValues[index],
                         finalValues[pixelIndex(x, y, finalRect.w)]);
            }
        }
    }

    for (auto element : renderedElements) {
        delete element.second;
    }
    renderedElements.clear();

    values = frameValues;

    for (auto element : renderElements) {
        delete element;
    }
    renderElements.clear();

    renderThread.currentFrameTask = nullptr;
}

FrameTask::~FrameTask() {
    for (auto element : renderElements) {
        delete element;
    }
    renderElements.clear();
}
