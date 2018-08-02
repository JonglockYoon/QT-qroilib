//
// mainwindow.h
//
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#define SIMULATION

// Qt
#include <QMainWindow>
#include <QAction>
#include <QToolBar>
#include <QMutex>

#include "viewmainpage.h"
#include "picapture.h"
#include "leftdock.h"
#include "outwidget.h"

class QModelIndex;
class QUrl;

using namespace Qroilib;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow();
    ~MainWindow();

public:

    /**
     * Defines the url to display when the window is shown for the first time.
     */
    void setInitialUrl(const QUrl&);

    ViewMainPage* viewMainPage() const;
    DocumentView* currentView() const;
    LeftDock *mLeftDock;

    int camWidth = 640;
    int camHeight = 480;
    int nCamExposure = 1;
    int fps = 10;

protected:
    void closeEvent(QCloseEvent *event);

private:
    struct Private;
    MainWindow::Private* const d;
    CamCapture* myCamCapture;

public  Q_SLOTS:
    void setSaveImage();
public:
    QVector<OutWidget*> vecOutWidget;
    void outWidget(QString title, IplImage* iplImg);
};

extern MainWindow* theMainWindow;

#endif /* MAINWINDOW_H */
