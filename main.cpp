#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator* translator = new QTranslator();
    QString transName = QApplication::applicationDirPath() + "/language/english.qm";

    if(!translator->load(transName))
        delete translator;
    else
        qApp->installTranslator(translator);

    MainWindow w;
    w.show();

    return a.exec();
}
