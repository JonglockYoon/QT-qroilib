/*
qroiapp: qroilib sample application
Copyright 2018 jerry1455@gmail.com
*/
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

#include "viewmainpage.h"
#include "logviewdock.h"
#include "config.h"
#include "mlogthread.h"

class QModelIndex;
class QUrl;
class CImgProcEngine;

using namespace QtCharts;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

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

    Qroilib::RoiObject *ManualInspection(int ch);
    void SetCameraPause(int viewNumber, int bPause);

protected:
    void closeEvent(QCloseEvent *event);

private:
    struct Private;
    MainWindow::Private* const d;

public:
    QLineSeries* redSeries;

    CImgProcEngine* pImgProcEngine;

    MLogThread m_LogThread; // class가 생성되면서 Thread가 자동 시작된다.
    QMutex	m_logsync;
    QMutex	m_alignlogsync;
    void DevLogSave(string strMsg, ...);

    int m_iActiveView; // -1=all,0-first, 1-second
    LogViewDock *pLogViewDock;

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

    void setChannel1();
    void setChannel2();
    void setChannelAll();

    void setInspectAll();
    void setPreviewAll();

    void finishNewRoiObject();

};

extern MainWindow* theMainWindow;

#endif /* MAINWINDOW_H */
