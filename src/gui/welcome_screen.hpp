#pragma once
#include <QPushButton>
#include <QWidget>

class WelcomeScreenProjectWidget : public QPushButton {
    Q_OBJECT
  public:
    explicit WelcomeScreenProjectWidget(const QString &title,
                                        QWidget *parent = nullptr);
};

class WelcomeScreenWidget : public QWidget {
    Q_OBJECT
  public:
    explicit WelcomeScreenWidget(QWidget *parent = nullptr);

  signals:
    void newProjectClicked();
};
