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
    CImgProcBase base;

public  Q_SLOTS:
    void finishNewRoiObject();

public:
    int InspectOneItem(IplImage* img, Qroilib::RoiObject *pData);
    int NoiseOut(Qroilib::RoiObject *pData, IplImage* grayImg, int t = -1, int nDbg = 100, int h = -1);
    int SingleROIFindShape(IplImage* croppedImage, Qroilib::RoiObject *pData, QRectF rectIn);
    double SinglePattFind(IplImage* croppedImage, Qroilib::RoiObject *pData, QRectF rect);
    void DrawResultCrossMark(IplImage *iplImage, Qroilib::RoiObject *pData);
    void SaveOutImage(IplImage* pImgOut, Qroilib::RoiObject *pData, QString strMsg, bool bClear = false);
};

extern MainWindow* theMainWindow;

#endif /* MAINWINDOW_H */
