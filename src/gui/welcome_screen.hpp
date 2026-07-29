#pragma once
#include <QPushButton>
#include <QWidget>

constexpr int PREVIEW_IMAGE_ORIGINAL_WIDTH = 160;
constexpr int PREVIEW_IMAGE_WIDTH = PREVIEW_IMAGE_ORIGINAL_WIDTH * 2;
constexpr int PREVIEW_IMAGE_ORIGINAL_HEIGHT = 90;
constexpr int PREVIEW_IMAGE_HEIGHT = PREVIEW_IMAGE_ORIGINAL_HEIGHT * 2;

class NewMainWindow;

class WelcomeScreenProjectWidget : public QPushButton {
    Q_OBJECT
  public:
    explicit WelcomeScreenProjectWidget(const QString &title, bool isNew,
                                        QImage img, QWidget *parent = nullptr);
};

class WelcomeScreenWidget : public QWidget {
    Q_OBJECT
  public:
    explicit WelcomeScreenWidget(NewMainWindow *mainWindow,
                                 QWidget *parent = nullptr);

  signals:
    void newProjectClicked();
    void openClicked(const QString &path, bool asTemplate);
};
