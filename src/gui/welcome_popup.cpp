#include "welcome_popup.hpp"
#include "gui.hpp"
#include <QSettings>
#include <QVBoxLayout>

WelcomePopup::WelcomePopup(NewMainWindow *parent) : QDialog(parent) {
    setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose);
    setFixedSize(500, 400);

    QVBoxLayout *mainLay = new QVBoxLayout(this);
    mainLay->setSpacing(0);
    mainLay->setContentsMargins(0, 0, 0, 0);

    QLabel *preview = new QLabel(this);
    QPixmap pixmap(parent->dataPath + "/assets/banner.png");
    pixmap.setDevicePixelRatio(2);
    preview->setPixmap(std::move(pixmap));
    mainLay->addWidget(preview);

    QVBoxLayout *lay = new QVBoxLayout();
    lay->setContentsMargins(16, 16, 16, 16);
    mainLay->addLayout(lay);

    QLabel *descriptionText = new QLabel(this);
    descriptionText->setText(
        "<h3>Welcome to Graphics Creator!</h3>\n\nThis program automatically "
        "checks for "
        "updates for new features and bug fixes including crash fixes. Note "
        "that this may make a request to api.github.com on a maximum of once "
        "per 24 hours.");
    descriptionText->setWordWrap(true);
    lay->addWidget(descriptionText);

    updateCheck = new QCheckBox("Check for updates (recommended)");
    updateCheck->setChecked(true);

    lay->addSpacing(12);
    lay->addWidget(updateCheck);

    lay->addStretch();

    QPushButton *okButton = new QPushButton("OK", this);
    connect(okButton, &QPushButton::clicked, this, &WelcomePopup::accept);
    lay->addWidget(okButton);

    connect(this, &WelcomePopup::accepted, this, &WelcomePopup::acceptedSlot);
}

void WelcomePopup::acceptedSlot() {
    QSettings settings;
    settings.setValue("welcome/shown", true);
    settings.setValue("updates/enabled", updateCheck->isChecked());
}
