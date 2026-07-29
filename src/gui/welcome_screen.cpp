#include "welcome_screen.hpp"
#include "flowlayout.hpp"
#include "gui.hpp"
#include <QCryptographicHash>
#include <QDir>
#include <QFileInfo>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QScrollArea>
#include <QSettings>
#include <QStandardPaths>
#include <QVBoxLayout>

WelcomeScreenProjectWidget::WelcomeScreenProjectWidget(const QString &title,
                                                       bool isNew, QImage img,
                                                       QWidget *parent)
    : QPushButton(parent) {
    setFlat(true);
    setFixedSize(180, 150);
    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->setAlignment(Qt::AlignCenter);

    constexpr int imgW = PREVIEW_IMAGE_ORIGINAL_WIDTH;
    constexpr int imgH = PREVIEW_IMAGE_ORIGINAL_HEIGHT;
    QPixmap pixmap{PREVIEW_IMAGE_WIDTH, PREVIEW_IMAGE_HEIGHT};
    pixmap.setDevicePixelRatio(2);
    pixmap.fill(Qt::transparent);
    {
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(128, 128, 140, 50));
        painter.drawRoundedRect(0, 0, imgW, imgH, 4, 4);

        if (isNew) {
            constexpr int pw = 6;

            painter.setBrush(palette().text());
            painter.drawRect(imgW / 2 - pw / 2, imgH / 2 - pw * 2, pw, pw * 4);
            painter.drawRect(imgW / 2 - pw * 2, imgH / 2 - pw / 2, pw * 4, pw);
        } else if (img.isNull()) {
            QIcon icon = QIcon::fromTheme("graphics");
            QPixmap pixmap = icon.pixmap(QSize(64, 64), 2);
            painter.drawPixmap(imgW / 2 - 32, imgH / 2 - 32, pixmap);
        } else {
            img.setDevicePixelRatio(2);
            painter.drawImage(0, 0, img);
        }
    }

    QLabel *image = new QLabel();
    image->setPixmap(pixmap);
    image->setAlignment(Qt::AlignCenter);
    lay->addWidget(image);

    QLabel *label = new QLabel();
    QFont font;
    font.setWeight(QFont::Weight::Medium);
    label->setFont(font);
    label->setText(title);
    label->setAlignment(Qt::AlignCenter);
    lay->addWidget(label);
}

WelcomeScreenWidget::WelcomeScreenWidget(NewMainWindow *mainWindow,
                                         QWidget *parent)
    : QWidget(parent) {
    QVBoxLayout *mainLay = new QVBoxLayout(this);
    mainLay->setContentsMargins(0, 0, 0, 0);
    QScrollArea *area = new QScrollArea();
    mainLay->addWidget(area);
    QWidget *areaWidget = new QWidget(area);
    area->setWidget(areaWidget);
    area->setWidgetResizable(true);
    QVBoxLayout *lay = new QVBoxLayout(areaWidget);
    lay->setContentsMargins(40, 30, 40, 30);

    QLabel *newTitle = new QLabel("New project");
    QFont titleFont;
    titleFont.setPointSize(18);
    titleFont.setWeight(QFont::Weight::DemiBold);
    newTitle->setFont(titleFont);
    lay->addWidget(newTitle);

    FlowLayout *flowLayout = new FlowLayout(0, 0, 0);
    WelcomeScreenProjectWidget *newBtn =
        new WelcomeScreenProjectWidget("Blank", true, QImage(), this);
    connect(newBtn, &WelcomeScreenProjectWidget::clicked, this,
            &WelcomeScreenWidget::newProjectClicked);
    flowLayout->addWidget(newBtn);

    QFile templatesFile(mainWindow->dataPath +
                        "/project-templates/templates.json");
    if (templatesFile.open(QIODevice::ReadOnly)) {
        QByteArray templatesData = templatesFile.readAll();
        QJsonDocument templatesDoc = QJsonDocument::fromJson(templatesData);
        templatesFile.close();

        for (const auto &templateJson : templatesDoc.array()) {
            auto templateObj = templateJson.toObject();
            QString fileName = mainWindow->dataPath + "/project-templates/" +
                               templateObj["file"].toString();

            QImage img = QImage(fileName + ".png");

            WelcomeScreenProjectWidget *btn = new WelcomeScreenProjectWidget(
                templateObj["name"].toString(), false, img, this);
            connect(btn, &WelcomeScreenProjectWidget::clicked, this,
                    [this, fileName]() {
                        emit openClicked(fileName + ".gcp", true);
                    });
            flowLayout->addWidget(btn);
        }
    }

    lay->addLayout(flowLayout);
    lay->addSpacing(16);

    QSettings settings;
    QStringList recentList = settings.value("recent/list").toStringList();

    if (!recentList.isEmpty()) {
        QDir appData(QStandardPaths::writableLocation(
            QStandardPaths::AppLocalDataLocation));

        QLabel *recentTitle = new QLabel("Recents");
        recentTitle->setFont(titleFont);
        lay->addWidget(recentTitle);

        FlowLayout *flowLayout = new FlowLayout(0, 0, 0);
        for (int i = recentList.length() - 1; i >= 0; i--) {
            const auto &path = recentList[i];

            QCryptographicHash hash(QCryptographicHash::Sha256);
            hash.addData(path.toUtf8());
            QString imagePath = appData.absoluteFilePath(
                "previews/" + hash.result().toHex() + ".png");

            QFileInfo fileInfo(path);
            if (!fileInfo.exists()) {
                qInfo() << "Project" << path << "does not exist anymore";
                QFile imagePathInfo(imagePath);
                if (imagePathInfo.exists()) {
                    qInfo() << "Found preview image, removing";
                    imagePathInfo.remove();
                }
                continue;
            }

            QImage img = QImage(imagePath);

            WelcomeScreenProjectWidget *btn = new WelcomeScreenProjectWidget(
                fileInfo.completeBaseName(), false, img, this);
            connect(btn, &WelcomeScreenProjectWidget::clicked, this,
                    [this, path]() { emit openClicked(path, false); });
            flowLayout->addWidget(btn);
        }
        lay->addLayout(flowLayout);
        lay->addSpacing(16);
    }

    lay->addStretch();
}
