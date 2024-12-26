#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QFile file(":/Ubuntu.qss");
    if (file.open(QFile::ReadOnly)) {
        QTextStream ts(&file);
        QString style = ts.readAll();
        a.setStyleSheet(style);  // 将 QSS 应用到整个应用
    }
    MainWindow w;
    w.show();
    return a.exec();
}
