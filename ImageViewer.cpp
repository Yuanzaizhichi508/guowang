#include "ImageViewer.h"

ImageViewer::ImageViewer(const QImage &image, QWidget *parent) : QDialog(parent) {
    imageLabel = new QLabel(this);
    imageLabel->setPixmap(QPixmap::fromImage(image).scaled(800, 600, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(imageLabel);
    setLayout(layout);

    // 设置放大窗口的标题和大小
    setWindowTitle("Zoomed Image");
    resize(800, 600);
}
