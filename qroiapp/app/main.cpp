// STL
#include <memory>

// Qt
#include <QDebug>
#include <QScopedPointer>
#include <QUrl>
#include <QTemporaryDir>
#include <QApplication>
#include <QStringList>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>

// Local
#include "mainwindow.h"
#include "qtsingleapplication.h"

int main(int argc, char *argv[])
{
    QtSingleApplication app(argc, argv);
    //QApplication app(argc, argv);
    if (app.isRunning()) {
        qDebug() << "main msg: "
              << "Another instance is running, so I will exit.";
        return 0;
    }
    QTranslator* translator = new QTranslator();

    QString strLocaleName = QLocale::system().name();		// language_country (예:ko_KR)
    strLocaleName = strLocaleName.left(2);			// language 부분만 남기기.
    translator->load("qroiapp_ko.qm");
    QApplication::installTranslator(translator);


    MainWindow* window = new MainWindow();
    window->show();
    return app.exec();
}
