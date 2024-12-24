#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QDialog>
#include <QLabel>
#include <QVBoxLayout>
#include <QImage>

class ImageViewer:public QDialog
{
    Q_OBJECT
public:
    explicit ImageViewer(const QImage &image, QWidget *parent = nullptr);

private:
    QLabel *imageLabel;

};

#endif // IMAGEVIEWER_H
