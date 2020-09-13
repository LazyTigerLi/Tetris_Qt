#include "gui/homepage.h"
#include <QApplication>

int main(int argc, char *argv[])
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    // QApplication::setQuitOnLastWindowClosed(false);
    // not use this
    /* if not set this, when the child widget is closed, the app will
     * exit because there is no visible widget now. (When close
     * SinglePalyerWidget, HomePage is hidden.) */
    QApplication a(argc, argv);
    HomePage w;
    w.show();
    return a.exec();
}
