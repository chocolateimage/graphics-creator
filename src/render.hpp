#pragma once
#include "variant.hpp"
#include <QMutex>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftstroke.h>
#include <hb-ft.h>
#include <hb.h>
#include <string>
#include <unordered_map>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/frame.h>
#include <libswscale/swscale.h>
}

class FontManager;
class ElementRender;
class RenderedElement;
class FrameTask;

void convertToAvFrame(AVFrame *frame, uint32_t *source, int w, int h);

class ImageData {
  public:
    ~ImageData();
    int width;
    int height;
    uint32_t *data;
};

class VideoData {
  public:
    VideoData(const std::string &path);
    ~VideoData();

    QMutex mutex{};
    AVFrame *getFrame(double seconds, int w, int h);

    bool error{false};
    double lastSecond{-1};

    int streamIndex;
    AVStream *stream{nullptr};
    AVFormatContext *fmtCtx{nullptr};
    AVCodecContext *decodeCtx{nullptr};
    AVPacket *packet{nullptr};
    AVFrame *frame{nullptr};
};

class Loader {
  public:
    std::unordered_map<std::string, std::shared_ptr<ImageData>> imageDatas{};
    std::unordered_map<std::string, std::shared_ptr<VideoData>> videoDatas{};
    QMutex imageDatasMutex{};
    QMutex videoDatasMutex{};

    std::shared_ptr<ImageData> loadImage(const std::string &path);
    std::shared_ptr<VideoData> loadVideo(const std::string &path);
};

extern Loader globalLoader;

class StrokeInfo {
  public:
    StrokeInfo() : strokeWidth(0), lineJoin(FT_STROKER_LINEJOIN_ROUND) {}
    StrokeInfo(double w, FT_Stroker_LineJoin lj)
        : strokeWidth(w), lineJoin(lj) {}
    std::string hash() const {
        if (strokeWidth > 0) {
            return std::to_string(strokeWidth) + "\n" +
                   std::to_string(lineJoin);
        } else {
            return "0\n0";
        }
    }
    double strokeWidth;
    FT_Stroker_LineJoin lineJoin;
};

std::string getFontHash(const Font &font, int fontSize, bool antialiased);

class FontInfo {
  public:
    FontInfo(FT_Face face, hb_font_t *hb, int pixelHeight, Font font);
    ~FontInfo();

    FT_BitmapGlyph getGlyph(hb_codepoint_t codepoint);

    std::unordered_map<hb_codepoint_t, FT_BitmapGlyph> glyphs;

    FontManager *fontManager;
    Font font;
    FT_Face face;
    hb_font_t *hb;
    int pixelHeight;
    bool antialiased{true};
    StrokeInfo strokeInfo;
    int framesUnused{0};
};

class FontManager {
  public:
    FontManager();
    ~FontManager();
    FT_Library ftLibrary{nullptr};
    FontInfo *getFont(const Font &font, int fontSize, bool antialiased,
                      const StrokeInfo &strokeInfo);

    void garbageCollect();

  private:
    std::unordered_map<std::string, FontInfo *> loadedFonts;
};

// Please excuse the names for this, I don't know what else I should call it.
struct ElementSelectionSnippet {
    Rect rect;
    const uint32_t *__restrict__ values;
};

class RenderThread {
  public:
    void init();
    void garbageCollect();
    void close();

    ElementSelectionSnippet getSnippet(const ElementSelection &selection);

    FontManager *fontManager{nullptr};
    FrameTask *currentFrameTask{nullptr};
};

class RenderedElement {
  public:
    RenderedElement(const RenderedElement &) = default;
    RenderedElement(RenderedElement &&) = delete;
    RenderedElement &operator=(const RenderedElement &) = default;
    RenderedElement &operator=(RenderedElement &&) = delete;

    RenderedElement(ElementRender *element);
    ~RenderedElement();
    ElementRender *element;

    Rect elementRect;
    uint32_t *__restrict__ elementValues;
    uint32_t *__restrict__ finalValues;
    Rect finalRect;
    bool hasError{false};
};

class FrameTask {
  public:
    ~FrameTask();
    std::vector<ElementRender *> renderElements;
    std::unordered_map<QString, RenderedElement *> renderedElements;
    // used for detecting recursion on getSnippet
    QList<QString> currentElementStack;

    int width;
    int height;
    int frame;
    double seconds;

    uint64_t id;
    uint32_t *values;

    void render(RenderThread &renderThread);
};
