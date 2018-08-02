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
#include "camcapture.h"
#include "leftdock.h"

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

protected:
    void closeEvent(QCloseEvent *event);

private:
    struct Private;
    MainWindow::Private* const d;
    CamCapture* myCamCapture;

public  Q_SLOTS:
    void setSaveImage();
};

extern MainWindow* theMainWindow;

#endif /* MAINWINDOW_H */
