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

#include <QLogValueAxis>
#include <QLineSeries>
#include <QValueAxis>
#include <QChart>
#include <QChartView>

#include "positionsdock.h"
#include "viewmainpage.h"
#include "interfaceport.h"
#include "lightplustek.h"
#include "mlogthread.h"
#include "logviewdock.h"
#include "teachdock.h"
#include "aligngraphdock.h"
#include "config.h"
#include "minfoteachmanager.h"

#include "frminput.h"
#include "frmnum.h"
#include "mtrsalign.h"

class QModelIndex;
class QUrl;
class CImgProcEngine;

using namespace QtCharts;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

using namespace Qroilib;

//class ViewMainPage;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow();
    ~MainWindow();

public:
    frmInput kbdAlpha;
    frmNum kbdNum;
    bool bCamPause[2];

    /**
     * Defines the url to display when the window is shown for the first time.
     */
    void setInitialUrl(const QUrl&);

    ViewMainPage* viewMainPage() const;
    DocumentView* currentView() const;

    void LightOn(int ch);
    void LightOff(int ch);
    Qroilib::RoiObject *FindScrewHole(int ch);
    void SetCameraPause(int ch, int bPause);

protected:
    //virtual bool queryClose() Q_DECL_OVERRIDE;
    //virtual QSize sizeHint() const Q_DECL_OVERRIDE;
    //virtual void showEvent(QShowEvent*) Q_DECL_OVERRIDE;
    //virtual void resizeEvent(QResizeEvent*) Q_DECL_OVERRIDE;

protected:
    void closeEvent(QCloseEvent *event);

private:
    //Ui::MainWindow *ui;

    struct Private;
    MainWindow::Private* const d;

public:
    QLineSeries* redSeries;

    //PropertiesDock *mPropertiesDock;
    CImgProcEngine* pImgProcEngine;

    CInterfacePort itfport;
    CLightPlusTek lightport;

    MLogThread m_LogThread; // class가 생성되면서 Thread가 자동 시작된다.
    QMutex	m_logsync;
    QMutex	m_alignlogsync;
    void DevLogSave(string strMsg, ...);
    void AlignLogSave(int ch, int point, double x, double y);

    int m_iActiveView; // -1=all,0-first, 1-second
    LogViewDock *pLogViewDock;
    PositionsDock *mPositionsDock;
    TeachDock *mTeachDock;
    AlignGraphDock *mAlignGraphDock;
    MInfoTeachManager*	m_pInfoTeachManager;

public  Q_SLOTS:
    void setActSelectedTool();
    void setCreatePointTool();
    void setCreateRectangleTool();
    void setCreatePatternTool();

    void setLoadRoi();
    void setWriteRoi();
    void setSaveImage();
    void setReadImage();
    void setDialogconfig();
    //void setDialoglog();
    void setDialogmodel();

    void setChannel1();
    void setChannel2();
    void setChannelAll();

    void setInspectAll();
    void setPreviewAll();
    //void setPort();

    void finishNewRoiObject();

public:
    CMTrsAlign* m_pTrsAlign[2];
    bool bWorking[2];
};

extern MainWindow* theMainWindow;

#endif /* MAINWINDOW_H */
