#include <QApplication>
#include <QTranslator>

#include "Forms\mainwindow.h"

int main(int argc, char *argv[])
{
    // So it works well on eg. laptop.
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QApplication a(argc, argv);

    MainWindow w;
    w.show();

    return a.exec();
}
