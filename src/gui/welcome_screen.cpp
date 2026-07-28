#include "welcome_screen.hpp"
#include "flowlayout.hpp"
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

WelcomeScreenProjectWidget::WelcomeScreenProjectWidget(const QString &title,
                                                       QWidget *parent)
    : QPushButton(parent) {
    setFlat(true);
    setFixedSize(180, 150);
    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->setAlignment(Qt::AlignCenter);

    int imgW = 160;
    int imgH = 90;
    QPixmap pixmap{imgW * 2, imgH * 2};
    pixmap.setDevicePixelRatio(2);
    pixmap.fill(Qt::transparent);
    {
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(128, 128, 140, 50));
        painter.drawRoundedRect(0, 0, imgW, imgH, 4, 4);

        constexpr int pw = 6;

        painter.setBrush(palette().text());
        painter.drawRect(imgW / 2 - pw / 2, imgH / 2 - pw * 2, pw, pw * 4);
        painter.drawRect(imgW / 2 - pw * 2, imgH / 2 - pw / 2, pw * 4, pw);
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

WelcomeScreenWidget::WelcomeScreenWidget(QWidget *parent) : QWidget(parent) {
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

    lay->addSpacing(12);

    FlowLayout *flowLayout = new FlowLayout(0, 0, 0);
    WelcomeScreenProjectWidget *newBtn =
        new WelcomeScreenProjectWidget("Blank", this);
    connect(newBtn, &WelcomeScreenProjectWidget::clicked, this,
            &WelcomeScreenWidget::newProjectClicked);
    flowLayout->addWidget(newBtn);
    lay->addLayout(flowLayout);
    lay->addSpacing(16);

    lay->addStretch();
}
