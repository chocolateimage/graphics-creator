#include "video_file_button.hpp"
#include <QApplication>
#include <QDesktopServices>
#include <QDrag>
#include <QFileInfo>
#include <QMimeData>
#include <QMouseEvent>
#include <QUrl>

VideoFileButton::VideoFileButton(QWidget *parent) : QPushButton(parent) {
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setIcon(QIcon::fromTheme("video-symbolic"));
    setIconSize(QSize(32, 32));

    setToolTip("Click to open. Hold to drag file.");

    setStyleSheet("padding: 8px 12px;");
    QFont font;
    font.setPointSize(font.pointSize() * 1.1);
    setFont(font);

    connect(this, &QPushButton::clicked, this, &VideoFileButton::openFile);
}

void VideoFileButton::setFile(QString filePath) {
    this->filePath = filePath;
    setText(QFileInfo(filePath).fileName());
}

void VideoFileButton::openFile() {
    QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
}

void VideoFileButton::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        dragStartPosition = event->pos();
    }

    QPushButton::mousePressEvent(event);
}

void VideoFileButton::mouseMoveEvent(QMouseEvent *event) {

    if (event->buttons() & Qt::LeftButton) {
        if ((event->pos() - dragStartPosition).manhattanLength() >=
            QApplication::startDragDistance()) {

            QDrag *drag = new QDrag(this);
            QMimeData *mimeData = new QMimeData();

            mimeData->setUrls({QUrl::fromLocalFile(filePath)});
            drag->setMimeData(mimeData);
            drag->setPixmap(QIcon::fromTheme("video-symbolic").pixmap(64, 64));

            Qt::DropAction dropAction = drag->exec(Qt::DropAction::CopyAction |
                                                   Qt::DropAction::MoveAction);
        }
    }

    QPushButton::mouseMoveEvent(event);
}
