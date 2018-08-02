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
#include "imgprocbase.h"
#include "align.h"

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
    CImgAlign align;

protected:
    void closeEvent(QCloseEvent *event);

private:
    struct Private;
    MainWindow::Private* const d;
    CamCapture* myCamCapture;
    CImgProcBase base;

public  Q_SLOTS:
    void finishNewRoiObject();

public:
    int InspectOneItem(IplImage* img, Qroilib::RoiObject *pData);
    void DrawResultCrossMark(IplImage *iplImage, Qroilib::RoiObject *pData);
    double SinglePattFind(IplImage* croppedImage, Qroilib::RoiObject *pData, QRectF rect);

};

extern MainWindow* theMainWindow;

#endif /* MAINWINDOW_H */
