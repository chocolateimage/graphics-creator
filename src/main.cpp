#include "gui/gui.hpp"
#include "gui/render_window.hpp"
#include "plugin.hpp"
#include "shared.hpp"
#include <KIconTheme>
#include <KStyleManager>
#include <QApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QFileInfo>

int main(int argc, char **argv) {
#ifdef Q_OS_WIN
    CreateMutexA(nullptr, false, "GraphicsCreatorOpen");
    if (qEnvironmentVariable("MSYSTEM").isEmpty()) {
        if (AttachConsole(ATTACH_PARENT_PROCESS)) {
            freopen("CON", "w", stdout);
            freopen("CON", "w", stderr);
            freopen("CON", "r", stdin);
        }
    }
#endif

    KIconTheme::initTheme();
    QApplication application(argc, argv);
    application.setOrganizationName(QStringLiteral("graphics-creator"));
    application.setApplicationName(QStringLiteral("graphics-creator"));
    application.setDesktopFileName("me.chocolateimage.graphics-creator");
    application.setApplicationDisplayName(QStringLiteral("Graphics Creator"));
    application.setApplicationVersion(VERSION);
    KStyleManager::initStyle();

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addPositionalArgument("filepath", "Project to open", "[filepath]");

    QCommandLineOption newProjectOption(
        "new",
        "Create a new project without showing the welcome screen. Format: "
        "width:height:durationFrames:fps. Example: 1920:1080:300:60",
        "info");
    parser.addOption(newProjectOption);

    QCommandLineOption renderOption("render",
                                    "Render project into file. Will not "
                                    "overwrite unless --overwrite is set. An "
                                    "encoder with --encoder must be set.",
                                    "file");
    parser.addOption(renderOption);

    QCommandLineOption overwriteOption(QStringList() << "y" << "overwrite",
                                       "Overwrite file when rendering");
    parser.addOption(overwriteOption);

    QCommandLineOption encoderOption(
        "encoder",
        "The FFmepg encoder to use when rendering. For mov with transparency: "
        "prores (or prores_ks which may be faster but can cause issues). For "
        "mp4/H264: libx264. For mp4/H264 with NVDIA: h264_nvenc. "
        "For webm/VP9: libvpx-vp9. List of encoders can be viewed with ffmpeg "
        "-encoders",
        "encoder");
    parser.addOption(encoderOption);

    QCommandLineOption pluginOption(
        "load-plugin",
        "Location of a plugin .dll or .so file to additionally load. Can be a "
        "directory, then it loads all plugins in that directory.",
        "pluginPath");
    parser.addOption(pluginOption);

    QCommandLineOption pluginCwdOption(
        "load-plugins-from-cwd",
        "Load all .dlls from the current working directory.");
    parser.addOption(pluginCwdOption);

    QCommandLineOption thumbnailOption(
        "thumbnail",
        "While rendering creates a thumbnail to the file specified. Will not "
        "overwrite unless --overwrite is set.",
        "file");
    parser.addOption(thumbnailOption);

    parser.process(application);

    const QStringList args = parser.positionalArguments();
    QString newProject = parser.value(newProjectOption);
    QString renderFile = parser.value(renderOption);
    QString thumbnailFile = parser.value(thumbnailOption);
    QStringList additionalPlugins = parser.values(pluginOption);
    bool overwrite = parser.isSet(overwriteOption);
    bool hasThumbnail = parser.isSet(thumbnailOption);

    pluginManager = new PluginManager();
    pluginManager->loadDefaultPlugins();
    for (const auto &path : additionalPlugins) {
        if (QFileInfo(path).isDir()) {
            for (auto file : QDir(path).entryInfoList(QDir::Files)) {
                if (file.filePath().endsWith(".dll") ||
                    file.filePath().endsWith(".so")) {
                    pluginManager->loadPlugin(file.filePath());
                }
            }
        } else {
            pluginManager->loadPlugin(path);
        }
    }
    if (parser.isSet(pluginCwdOption)) {
        QDir dir = QDir::current();
        for (auto file : dir.entryInfoList(QDir::Files)) {
            if (file.filePath().endsWith(".dll")) {
                pluginManager->loadPlugin(file.filePath());
            }
        }
    }

    NewMainWindow widget;
    if (!args.isEmpty()) {
        widget.loadFile(args[0], false);
    } else if (!newProject.isEmpty()) {
        QStringList splitted = newProject.split(":");
        if (splitted.length() != 4) {
            qCritical() << "Invalid format for --new";
            return 1;
        }
        int width = splitted[0].toInt();
        int height = splitted[1].toInt();
        int durationFrames = splitted[2].toInt();
        int fps = splitted[3].toInt();
        widget.newProjectNew(width, height, fps, durationFrames);
    }

    if (!renderFile.isEmpty()) {
        qInfo() << "";
        QString encoder = parser.value(encoderOption);
        QFileInfo info(renderFile);
        if (info.exists()) {
            if (!overwrite) {
                qCritical()
                    << "Not overwriting file" << info.absoluteFilePath();
                qCritical() << "Use --overwrite to overwrite the file";
                return 1;
            }
        }

        QFileInfo thumbnailInfo(thumbnailFile);
        if (hasThumbnail) {
            if (thumbnailInfo.exists()) {
                if (!overwrite) {
                    qCritical() << "Not overwriting thumbnail file"
                                << thumbnailInfo.absoluteFilePath();
                    qCritical() << "Use --overwrite to overwrite the file";
                    return 1;
                }
            }
        }

        // TODO: template/placeholder text

        RenderWindow *renderWindow = new RenderWindow(&widget);
        renderWindow->renderFilePathInput->setText(info.absoluteFilePath());
        if (hasThumbnail) {
            renderWindow->thumbnailFile = thumbnailInfo.absoluteFilePath();
        }
        renderWindow->render(info, encoder);
        if (renderWindow->thread) {
            renderWindow->thread->wait();
            if (renderWindow->thread->hasErrored) {
                qCritical() << "An error occured while rendering:"
                            << qPrintable(renderWindow->thread->errorMsg);
            }
            return 0;
        } else {
            return 1;
        }
    } else {
        widget.show();
    }

    return application.exec();
}
