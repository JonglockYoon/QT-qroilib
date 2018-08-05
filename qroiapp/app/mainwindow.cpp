/*

QROILIB : QT Vision ROI Library
Copyright 2018, Modifed by Yoon Jong-lock <jerry1455@gmail.com,jlyoon@yj-hitech.com>

이 Program은 Machine Vision Program을 QT 환경에서 쉽게 구현할수 있게 하기 위해 작성되었다.

Gwenview 의 Multi view 기능과 메모리보다 큰 이미지를 로드 할 수 있도록 작성된 이미지 viewer기능을 이용하고,
Tiled의 Object drawing 기능을 가져와서 camera의 이미지를 실시간 display하면서 vision ROI를 작성하여,
내가 원하는 결과를 OpenCV를 통하여 쉽게 구현할수 있도록 한다.

이 Program은 ARM계열 linux  와 X86계열 linux에서 test되었다.
*/

#include <roilib_export.h>
// Qt
#include <QApplication>
#include <QDateTime>
#include <QPushButton>
#include <QShortcut>
#include <QSplitter>
#include <QStackedWidget>
#include <QTimer>
#include <QUndoGroup>
#include <QVBoxLayout>
#include <QMenuBar>
#include <QUrl>
#include <QFileDialog>
#include <QFileOpenEvent>

#include <QMediaMetaData>
#include <QCamera>
#include <QCameraInfo>
#include <QMessageBox>
#include <QObject>
#include <QRegExp>


// Local

#include <qroilib/document/documentfactory.h>
#include <qroilib/gvdebug.h>
#include <qroilib/mimetypeutils.h>
#include <qroilib/documentview/documentviewcontroller.h>
#include <qroilib/zoommode.h>
#include <qroilib/mousewheelbehavior.h>
#include <qroilib/documentview/documentview.h>
#include <qroilib/documentview/rasterimageview.h>

#include "mainwindow.h"
//#include "ui_mainwindow.h"

#include "recipedata.h"
#include "imgprocengine.h"

#include "editpolygontool.h"
#include "qextserialenumerator.h"

#include "toolmanager.h"
#include "orthogonalrenderer.h"
#include "roiobject.h"
#include "addremoveroiobject.h"
#include "common.h"
#include "roireader.h"
#include "dialogconfig.h"
#include "logviewdock.h"
#include "dialogmodel.h"
#include "mtrsbase.h"

using namespace Qroilib;
using namespace cv;

struct MainWindow::Private
{
    MainWindow* q;
    QWidget* mContentWidget;
    ViewMainPage* mViewMainPage;
    QStackedWidget* mViewStackedWidget;
    QActionGroup* mViewModeActionGroup;

    QString mCaption;


    QMenu *fileMenu;
    QMenu *viewMenu;
    //QMenu *deviceMenu;
    //QMenu *portsMenu;
    QMenu *channelMenu;
    QMenu *inspectMenu;


    QAction *selectToolAct;
    QAction *createPointToolAct;
    QAction *createRectangleToolAct;
    QAction *createPatternToolAct;
    QAction *loadRoi;
    QAction *writeRoi;
    QAction *saveImage;
    QAction *readImage;
    QAction *dialogConfig;
    //QAction *dialogLog;
    QAction *dialogModel;
    QAction *exitAct;
    QAction * mDeleteAction;
    QAction *chAllAction;
    QAction *ch1Action;
    QAction *ch2Action;
    QAction *inspectAll;
    QAction *previewAll;

    void setupWidgets()
    {
        mContentWidget = new QWidget(q);
        q->setCentralWidget(mContentWidget);

        mViewStackedWidget = new QStackedWidget(mContentWidget);
        QVBoxLayout* layout = new QVBoxLayout(mContentWidget);
        layout->addWidget(mViewStackedWidget);
        layout->setMargin(0);
        layout->setSpacing(0);

        mViewMainPage = new ViewMainPage(mViewStackedWidget);
        mViewStackedWidget->addWidget(mViewMainPage);

    }

     void setupActions()
    {
        //DocumentView* pDocumentView = mViewMainPage->currentView();

        selectToolAct = new QAction(QIcon(":/resources/select.png"), tr("&Select ..."), q);
        connect(selectToolAct, SIGNAL(triggered(bool)), q, SLOT(setActSelectedTool()));


        createRectangleToolAct = new QAction(QIcon(":/resources/rect-roi.png"), tr("Create &Rectangle..."), q);
        connect(createRectangleToolAct, SIGNAL(triggered(bool)), q, SLOT(setCreateRectangleTool()));

        createPointToolAct = new QAction(QIcon(":/resources/point-roi.png"), tr("Create p&Oint..."), q);
        connect(createPointToolAct, SIGNAL(triggered(bool)), q, SLOT(setCreatePointTool()));

        createPatternToolAct = new QAction(QIcon(":/resources/pattern-roi.png"), tr("Create &Pattern ROI..."), q);
        connect(createPatternToolAct, SIGNAL(triggered(bool)), q, SLOT(setCreatePatternTool()));

        loadRoi = new QAction(QIcon(), tr("&load RoiMap ..."), q);
        connect(loadRoi, SIGNAL(triggered(bool)), q, SLOT(setLoadRoi()));
        writeRoi = new QAction(QIcon(), tr("&write RoiMap ..."), q);
        connect(writeRoi, SIGNAL(triggered(bool)), q, SLOT(setWriteRoi()));
        saveImage = new QAction(QIcon(), tr("&Save Image ..."), q);
        connect(saveImage, SIGNAL(triggered(bool)), q, SLOT(setSaveImage()));
        readImage = new QAction(QIcon(), tr("&Read Image ..."), q);
        connect(readImage, SIGNAL(triggered(bool)), q, SLOT(setReadImage()));
        dialogConfig = new QAction(QIcon(":/resources/settings.png"), tr("&Config ..."), q);
        connect(dialogConfig, SIGNAL(triggered(bool)), q, SLOT(setDialogconfig()));
        //dialogLog = new QAction(QIcon(":/resources/notes.png"), tr("&Log ..."), q);
        //connect(dialogLog, SIGNAL(triggered(bool)), q, SLOT(setDialoglog()));
        dialogModel = new QAction(QIcon(":/resources/model.png"), tr("&Model ..."), q);
        connect(dialogModel, SIGNAL(triggered(bool)), q, SLOT(setDialogmodel()));

        exitAct = new QAction(QIcon(":/resources/exit.png"), tr("E&xit"), q);
        exitAct->setShortcuts(QKeySequence::Quit);
        connect(exitAct, SIGNAL(triggered()), q, SLOT(close()));

        chAllAction = new QAction(QIcon(), tr("channel&All ..."), q);
        connect(chAllAction, SIGNAL(triggered(bool)), q, SLOT(setChannelAll()));
        ch1Action = new QAction(QIcon(), tr("Channel&1 ..."), q);
        connect(ch1Action, SIGNAL(triggered(bool)), q, SLOT(setChannel1()));
        ch2Action = new QAction(QIcon(), tr("Channel&2 ..."), q);
        connect(ch2Action, SIGNAL(triggered(bool)), q, SLOT(setChannel2()));

        inspectAll = new QAction(QIcon(":/resources/search.png"), tr("Inspect&All ..."), q);
        connect(inspectAll, SIGNAL(triggered(bool)), q, SLOT(setInspectAll()));
        previewAll = new QAction(QIcon(":/resources/camera.png"), tr("&Preview All ..."), q);
        connect(previewAll, SIGNAL(triggered(bool)), q, SLOT(setPreviewAll()));

        fileMenu = new QMenu(tr("&File"), q);

        fileMenu->addAction(selectToolAct);
        fileMenu->addAction(createPointToolAct);
        fileMenu->addAction(createRectangleToolAct);
        fileMenu->addAction(createPatternToolAct);
        fileMenu->addSeparator();
        fileMenu->addAction(loadRoi);
        fileMenu->addAction(writeRoi);
        fileMenu->addAction(saveImage);
        fileMenu->addAction(readImage);
        fileMenu->addSeparator();
        fileMenu->addAction(dialogConfig);
        //fileMenu->addAction(dialogLog);
        fileMenu->addAction(dialogModel);
        fileMenu->addSeparator();
        fileMenu->addAction(exitAct);

        viewMenu = new QMenu(tr("&View"), q);

//        deviceMenu = new QMenu(tr("&Device"), q);

//        QActionGroup *videoDevicesGroup = new QActionGroup(q);
//        videoDevicesGroup->setExclusive(true);
//        foreach (const QCameraInfo &cameraInfo, QCameraInfo::availableCameras()) {
//            QAction *videoDeviceAction = new QAction(cameraInfo.description(), videoDevicesGroup);
//            videoDeviceAction->setCheckable(true);
//            videoDeviceAction->setData(QVariant::fromValue(cameraInfo.description()));
//            if (cameraInfo == QCameraInfo::defaultCamera())
//                videoDeviceAction->setChecked(true);

//            deviceMenu->addAction(videoDeviceAction);
//        }

//        QActionGroup *portsGroup = new QActionGroup(q);
//        portsGroup->setExclusive(true);
//        portsMenu = new QMenu(tr("&Ports"), q);
//        //QStringList List;
//        //List << "None";
//        foreach (QextPortInfo info, QextSerialEnumerator::getPorts()) {
//            //List << info.portName;
//            QAction *portAction = new QAction(info.portName, portsGroup);
//            portAction->setCheckable(true);
//            portAction->setData(QVariant::fromValue(info.portName));
//            //connect(portAction, SIGNAL(triggered(bool)), q, SLOT(setPort()));
//            portsMenu->addAction(portAction);
//        }
//        connect(portsGroup, SIGNAL(triggered(QAction *)), q, SLOT(clickedPortsGroup(QAction *)));

        channelMenu = new QMenu(tr("&Channel"), q);
        channelMenu->addAction(chAllAction);
        channelMenu->addAction(ch1Action);
        channelMenu->addAction(ch2Action);


        inspectMenu = new QMenu(tr("&Inspect"), q);
        inspectMenu->addAction(inspectAll);
        inspectMenu->addAction(previewAll);

        q->menuBar()->addMenu(fileMenu);
        q->menuBar()->addMenu(viewMenu);
        //q->menuBar()->addMenu(deviceMenu);
        //q->menuBar()->addMenu(portsMenu);
        if (gCfg.m_nCamNumber > 1)
        {
            q->menuBar()->addMenu(channelMenu);
        }
        q->menuBar()->addMenu(inspectMenu);

//        mDeleteAction = new QAction(q);
//        QList<QKeySequence> deleteKeys = QKeySequence::keyBindings(QKeySequence::Delete);
//        deleteKeys.removeAll(Qt::Key_D | Qt::ControlModifier);  // used as "duplicate" shortcut
//        deleteKeys.prepend(QKeySequence(Qt::Key_Backspace));
//        mDeleteAction->setShortcuts(deleteKeys);
        //connect(d->mDeleteAction, &QAction::triggered, this, &MainWindow::delete_);
    }

    void createToolBars()
    {
        QToolBar* toolBar;
        toolBar = q->addToolBar(tr("&Tool"));
        toolBar->addAction(exitAct);
        //toolBar->addAction(dialogLog);
        toolBar->addAction(dialogModel);
        toolBar->addAction(dialogConfig);
        toolBar->addAction(inspectAll);
        toolBar->addAction(previewAll);

        toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        toolBar->setIconSize(QSize(32,32));

        toolBar->installEventFilter(q);
    }

};

MainWindow* theMainWindow = 0;
CRecipeData * g_cRecipeData = NULL;

MainWindow::MainWindow()
://ui(new Ui::MainWindow),
d(new MainWindow::Private)
{
    theMainWindow = this;
    //ui->setupUi(this);
    pLogViewDock = nullptr;
    //pDlgModel = nullptr;

    gCfg.ReadConfig();
    QString strRootDir = gCfg.RootPath;//QApplication::applicationDirPath();
    QString dirName;
    dirName.sprintf("%s/TeachingData", strRootDir.toStdString().c_str());
    QDir dir;
    dir.mkdir(dirName);

    m_pInfoTeachManager = new MInfoTeachManager;
    m_pInfoTeachManager->ReadData();

    g_cRecipeData = new CRecipeData(this);
    pImgProcEngine = new CImgProcEngine;

    dirName += "/";
    dirName += gCfg.m_sLastRecipeName;
    dir.mkdir(dirName);

    setMinimumSize(800, 480);

    d->q = this;
    d->setupWidgets();
    d->setupActions();
    d->createToolBars();

    setWindowTitle(gCfg.m_sLastRecipeName);

    extern Qroilib::ParamTable ROIDSHARED_EXPORT paramTable[];
    for (int i=0; i<2; i++)
    {
        DocumentView* view = d->mViewMainPage->view(i);
        if (view)
        {
            view->setParamTable(paramTable);
            view->bMultiView = true;
            connect(view, &Qroilib::DocumentView::finishNewRoiObject, this, &MainWindow::finishNewRoiObject);
        }
    }

    setActSelectedTool();

    theMainWindow->kbdNum.Instance(this)->Init("black", 10);
    theMainWindow->kbdAlpha.Instance(this)->Init("control", "silvery", 12, 12);

    itfport.Init(gCfg.m_sInterfacePort);
    lightport.Init(gCfg.m_sLightPort);

    QTimer *timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, [=]() {

        if (gCfg.m_nCamNumber <= 1)
            setChannel1();
        setLoadRoi();

        timer->stop();
        timer->deleteLater();
    } );
    timer->start(2000); // 프로그램 시작 2초후 setLoadRoi() 실행.

    int n = 0;
    for (int i=0; i<2; i++)
    {
        DocumentView* view = d->mViewMainPage->view(i);
        if (view) {
            m_iActiveView = i;
            n++;
        }
    }
    if (n > 1)
        m_iActiveView = -1;


    m_pTrsAlign[0] = new CMTrsAlign("left");
    m_pTrsAlign[1] = new CMTrsAlign("right");


    pLogViewDock = new LogViewDock(tr("Logview"), this);
    pLogViewDock->setFeatures(QDockWidget::DockWidgetClosable);
    addDockWidget(Qt::RightDockWidgetArea, pLogViewDock);
    d->viewMenu->addAction(pLogViewDock->toggleViewAction());

    mPositionsDock = new PositionsDock(tr("Positions"), this);
    mPositionsDock->setFeatures(QDockWidget::DockWidgetClosable);
    addDockWidget(Qt::RightDockWidgetArea, mPositionsDock);
    d->viewMenu->addAction(mPositionsDock->toggleViewAction());

    mTeachDock = new TeachDock(tr("Teach"), this);
    mTeachDock->setFeatures(QDockWidget::DockWidgetClosable);
    addDockWidget(Qt::RightDockWidgetArea, mTeachDock);
    d->viewMenu->addAction(mTeachDock->toggleViewAction());

    mAlignGraphDock = new AlignGraphDock(tr("AlignGraph"), this);
    mAlignGraphDock->setFeatures(QDockWidget::DockWidgetClosable);
    addDockWidget(Qt::BottomDockWidgetArea, mAlignGraphDock);
    d->viewMenu->addAction(mAlignGraphDock->toggleViewAction());

    if (m_iActiveView == -1)
        mTeachDock->hide();

    QTimer *timer1 = new QTimer(this);
    timer1->setSingleShot(true);
    connect(timer1, &QTimer::timeout, [=]() {
        setChannelAll();
    } );
    timer1->start(1000);

    // test
#if 0
    gCfg.m_bSaveEngineImg = false;
    QTimer *timer2 = new QTimer(this);
    timer2->setSingleShot(false);
    connect(timer2, &QTimer::timeout, [=]() {
    } );
    timer2->start(1000);
#endif
     m_pTrsAlign[0]->ThreadRun();
     m_pTrsAlign[1]->ThreadRun();

     m_pTrsAlign[0]->SetOPStatus(START_RUN);
     m_pTrsAlign[1]->SetOPStatus(START_RUN);
}


MainWindow::~MainWindow()
{
}

void MainWindow::LightOn(int ch)
{
    Qroilib::DocumentView* v = d->mViewMainPage->view(ch);
    if (v == nullptr)
        return;

    for (const Layer *layer : v->mRoi->objectGroups()) {
        const ObjectGroup &objectGroup = *static_cast<const ObjectGroup*>(layer);
        for (const Qroilib::RoiObject *roiObject : objectGroup) {
            Qroilib::RoiObject *mObject = (Qroilib::RoiObject *)roiObject;
            CParam *pParam = mObject->getParam(("Light"));
            if (pParam)	{
                int val = (int)pParam->Value.toDouble();
                lightport.SetBrightness(ch, val);
                break;
            }
        }
    }
}
void MainWindow::LightOff(int ch)
{
    lightport.SetBrightness(ch, 0);
}

Qroilib::RoiObject * MainWindow::FindScrewHole(int ch)
{
    cv::Mat frame;
    IplImage riplImg;
    IplImage *iplImg;

    Qroilib::DocumentView* v = d->mViewMainPage->view(ch);
    if (v == nullptr)
        return nullptr;

    SetCameraPause(ch, true);

    QImage img;
    const QImage *camimg = v->image();
    qimage_to_mat(camimg, frame);
    riplImg = frame;
    iplImg = &riplImg;

    for (const Layer *layer : v->mRoi->objectGroups()) {
        const ObjectGroup &objectGroup = *static_cast<const ObjectGroup*>(layer);
        for (const Qroilib::RoiObject *roiObject : objectGroup) {
            Qroilib::RoiObject *mObject = (Qroilib::RoiObject *)roiObject;
            mObject->m_vecDetectResult.clear();
            pImgProcEngine->InspectOneItem(iplImg, mObject);
            if (mObject->m_vecDetectResult.size() > 0) {

                pImgProcEngine->DrawResultCrossMark(iplImg, mObject);
                Mat mat = cvarrToMat(iplImg);

                mat_to_qimage(mat, img);

                if (v->document()) {
                    v->document()->setImageInternal(img);
                    v->imageView()->updateBuffer();
                }


                return mObject;
            }
        }
    }
    return nullptr;
}

ViewMainPage* MainWindow::viewMainPage() const
{
    return d->mViewMainPage;
}

DocumentView* MainWindow::currentView() const
{
    DocumentView* v = d->mViewMainPage->currentView();
    return v;
}

void MainWindow::closeEvent(QCloseEvent *event)
{

    event->ignore();
    QString str = tr("Close application");
    qDebug() << str;

    int ret = QMessageBox::warning(this, str,
                                   tr("Do you really want to quit?"),
                                   QMessageBox::Ok| QMessageBox::Cancel,
                                   QMessageBox::Ok); //종료시 메세지 박스 출력...
    switch(ret)
    {
        case QMessageBox::Ok:
            //여기에 OK 눌렀을 시 할 내용 추가.
            event->accept();

            if (pLogViewDock) {
                pLogViewDock->close();
            }
            if (m_pInfoTeachManager)
                delete m_pInfoTeachManager;

            itfport.Close();
            delete pImgProcEngine;
            delete g_cRecipeData;
            delete d->mViewMainPage;
            delete d;

            qApp->quit(); // terminates application.
            break;
    }
}

void MainWindow::SetCameraPause(int ch, int bPause)
{
    if (ch < 0) return;
    if (ch > 1) return;
    qDebug() << "SetCameraPause" << ch << bPause;
    bCamPause[ch] = bPause;
//    ViewMainPage* pView = viewMainPage();
//    CamCapture* pCam = pView->myCamCapture[ch];
//    if (pCam)
//        pCam->bCamPause = bPause;

    if (bPause == 0)
    {
        Qroilib::DocumentView* v = d->mViewMainPage->view(ch);
        if (v) {
            v->mRoi->setWidth(MAX_CAMERA_WIDTH);
            v->mRoi->setHeight(MAX_CAMERA_HEIGHT);
        }
    }
}

void MainWindow::setChannelAll()
{
    Qroilib::DocumentView* v = d->mViewMainPage->view(1);
    if (v == nullptr)
        return;

    m_iActiveView = -1;
    int h1 = pLogViewDock->height();
    int h2 = mPositionsDock->height();
    int h3 = mTeachDock->height();
    int h4 = mAlignGraphDock->height();
    if (h1 < h2) {
        h1++; h2--;
    } else {
        h1--; h2++;
    }
    QList<QDockWidget*> docks = { pLogViewDock, mPositionsDock, mTeachDock, mAlignGraphDock };
    QList<int> dockSizes1 = { h1,h2,h3,h4+1 };
    resizeDocks(docks, dockSizes1, Qt::Vertical);
    mTeachDock->hide();

    int seq = 0;
    while (true) {
        Qroilib::DocumentView* v = d->mViewMainPage->view(seq);
        if (v == nullptr)
            break;
        qDebug() << "setChannelAll" << seq;
        v->selectNone();
        v->show();
        v->setCompareMode(true);
        v->bMultiView = true;
        SetCameraPause(seq, false);

        v->zoomActualSize();
        v->setZoomToFit(true);
        seq++;
    }
    d->mViewMainPage->updateLayout();
}
void MainWindow::setChannel1()
{
    Qroilib::DocumentView* v = d->mViewMainPage->view(0);
    if (v == nullptr)
        return;
    setChannelAll();

    m_iActiveView = 0;
    int h1 = pLogViewDock->height();
    int h2 = mPositionsDock->height();
    int h3 = mTeachDock->height();
    int h4 = mAlignGraphDock->height();
    if (h1 < h2) {
        h1++; h2--;
    } else {
        h1--; h2++;
    }
    QList<QDockWidget*> docks = { pLogViewDock, mPositionsDock, mTeachDock, mAlignGraphDock };
    QList<int> dockSizes1 = { h1,h2,h3,h4-1 };
    resizeDocks(docks, dockSizes1, Qt::Vertical);
    mTeachDock->show();

    v = d->mViewMainPage->view(1);
    if (v != nullptr) {
        v->setCurrent(false);
        v->hide();
        v->bMultiView = false;
        SetCameraPause(1, true);
    }
    v = d->mViewMainPage->view(0);
    if (v != nullptr) {
        v->setCurrent(true);
        v->mRoiScene->setMapDocument(v);
        d->mViewMainPage->setCurrentView(v);
        v->mSelectedTool = nullptr;
        v->ToolsSelect();
        v->show();
        v->bMultiView = false;
        SetCameraPause(0, false);

        v->zoomActualSize();
        v->setZoomToFit(true);
    }
    d->mViewMainPage->updateLayout();
}

void MainWindow::setChannel2()
{
    Qroilib::DocumentView* v = d->mViewMainPage->view(1);
    if (v == nullptr)
        return;
    setChannelAll();

    m_iActiveView = 1;
    int h1 = pLogViewDock->height();
    int h2 = mPositionsDock->height();
    int h3 = mTeachDock->height();
    int h4 = mAlignGraphDock->height();
    if (h1 < h2) {
        h1++; h2=h2-2;
    } else {
        h1--; h2=h2+2;
    }
    QList<QDockWidget*> docks = { pLogViewDock, mPositionsDock, mTeachDock, mAlignGraphDock };
    QList<int> dockSizes1 = { h1,h2,h3,h4+1 };
    resizeDocks(docks, dockSizes1, Qt::Vertical);
    mTeachDock->show();

    v = d->mViewMainPage->view(0);
    if (v != nullptr) {
        v->setCurrent(false);
        v->hide();
        v->bMultiView = false;
        SetCameraPause(0, true);
    }
    v = d->mViewMainPage->view(1);
    if (v != nullptr) {
        v->setCurrent(true);
        v->mRoiScene->setMapDocument(v);
        d->mViewMainPage->setCurrentView(v);
        v->mSelectedTool = nullptr;
        v->ToolsSelect();
        v->show();
        v->bMultiView = false;
        SetCameraPause(1, false);

        v->zoomActualSize();
        v->setZoomToFit(true);
    }
    d->mViewMainPage->updateLayout();
}

void MainWindow::setInspectAll()
{
    cv::Mat frame;
    IplImage riplImg;
    IplImage *iplImg;
    QImage img;
    ViewMainPage* pView = viewMainPage();

    int seq = 0;
    while (true) {
        Qroilib::DocumentView* v = d->mViewMainPage->view(seq);
        if (v == nullptr)
            break;

        SetCameraPause(seq, true);

        const QImage *camimg = v->image();
        qimage_to_mat(camimg, frame);
        riplImg = frame;
        iplImg = &riplImg;

        for (const Layer *layer : v->mRoi->objectGroups()) {
            const ObjectGroup &objectGroup = *static_cast<const ObjectGroup*>(layer);
            for (const Qroilib::RoiObject *roiObject : objectGroup) {
                Qroilib::RoiObject *mObject = (Qroilib::RoiObject *)roiObject;
                mObject->m_vecDetectResult.clear();

                const QRectF bounds = v->boundingRect().intersected(mObject->bounds());
                mObject->setBounds(bounds);

                if (gCfg.m_nAlignMode == 1) {
                    if (mObject->mInspectType == _Inspect_Roi_Screw) {
                        theMainWindow->pImgProcEngine->InspectOneItem(iplImg, mObject);
                        theMainWindow->pImgProcEngine->DrawResultCrossMark(iplImg, mObject);
                    }
                } else if (gCfg.m_nAlignMode == 2) {
                    if (mObject->mInspectType == _Inspect_Roi_CenterOfPlusMark) {
                        theMainWindow->pImgProcEngine->InspectOneItem(iplImg, mObject);
                        theMainWindow->pImgProcEngine->DrawResultCrossMark(iplImg, mObject);
                    }
                }

                int size = mObject->m_vecDetectResult.size();
                for (int i = 0; i < size; i++) {
                    //DetectResult *prst = &mObject->m_vecDetectResult[i];

                }

                Mat mat = cvarrToMat(iplImg);
                mat_to_qimage(mat, img);

                if (v->document()) {
                    v->document()->setImageInternal(img);
                    v->imageView()->updateBuffer();
                }
            }
        }
        seq++;
    }
}

void MainWindow::setPreviewAll()
{
    //ViewMainPage* pView = viewMainPage();
    int seq = 0;
    while (true) {
        Qroilib::DocumentView* v = d->mViewMainPage->view(seq);
        if (v == nullptr)
            break;
        v->mRoi->setWidth(MAX_CAMERA_WIDTH);
        v->mRoi->setHeight(MAX_CAMERA_HEIGHT);

        SetCameraPause(seq, false);
        seq++;
    }
}

void MainWindow::setActSelectedTool()
{
    Qroilib::DocumentView* v = currentView();
    if (!v)
        return;
    v->selectTool(v->actSelectTool);
}

void MainWindow::setCreatePointTool()
{
    Qroilib::DocumentView* v = currentView();
    if (!v)
        return;
    v->selectTool(v->actCreatePoint);
}

void MainWindow::setCreateRectangleTool()
{
    Qroilib::DocumentView* v = currentView();
    if (!v)
        return;
    v->selectTool(v->actCreateRectangle);
}

void MainWindow::setCreatePatternTool()
{
    Qroilib::DocumentView* v = currentView();
    if (!v)
        return;
    v->selectTool(v->actCreatePattern);
}

void MainWindow::setLoadRoi()
{
    Qroilib::DocumentView* v;

    int seq = 0;
    QList<const Qroilib::RoiObject*> selectedObjects;
    while (true) {
        v = d->mViewMainPage->view(seq);
        if (v == nullptr)
            break;

        for (const Layer *layer : v->mRoi->objectGroups()) {
            const ObjectGroup &objectGroup = *static_cast<const ObjectGroup*>(layer);
            for (const Qroilib::RoiObject *roiObject : objectGroup) {
                selectedObjects.append(roiObject);
            }
        }
        if (selectedObjects.size() > 0) {
            v->setSelectedObjects((const QList<RoiObject *> &)selectedObjects);
            v->delete_();
        }
        selectedObjects.clear();
        seq++;
    }

    g_cRecipeData->LoadRecipeData();

    seq = 0;
    while (true) {
        v = d->mViewMainPage->view(seq);
        if (v == nullptr)
            break;
        v->zoomActualSize();
        v->setZoomToFit(true);
        seq++;
    }
    d->mViewMainPage->updateLayout();

    return;
}

void MainWindow::setWriteRoi()
{
    g_cRecipeData->SaveRecipeData();

    return;
}

void MainWindow::setSaveImage()
{
    DocumentView *v = (DocumentView *)viewMainPage()->currentView();
    if (v) {
        const QImage *pimg = v->image();
        if (!pimg)
            return;

        QApplication::setOverrideCursor(Qt::WaitCursor);
        QString fileName = QFileDialog::getSaveFileName(this,
                tr("Save Image"), "",
                tr("Images (*.png *.bmp *.jpg *.tif *.ppm *.GIF);;All Files (*)"));
        if (fileName.isEmpty()) {
            QApplication::restoreOverrideCursor();
            return;
        }
        QApplication::setOverrideCursor(Qt::WaitCursor);
        QTimer::singleShot(1, [=] {
            pimg->save(fileName);
            QApplication::restoreOverrideCursor();
        });
    }
}

void MainWindow::setReadImage()
{
    theMainWindow->kbdNum.DeleteInstance();
    theMainWindow->kbdAlpha.DeleteInstance();

    ViewMainPage* pView = viewMainPage();
    if (!pView)
        return;

    DocumentView* v = currentView();
    if (!v)
        return;
    SetCameraPause(m_iActiveView, true);


    QString fileName = QFileDialog::getOpenFileName(this,
            tr("Open Image"), QDir::currentPath(),
            tr("Images (*.png *.bmp *.jpg *.tif *.ppm *.GIF);;All Files (*)"));
    if (fileName.isEmpty()) {
        return;
    }


    QApplication::setOverrideCursor(Qt::WaitCursor);
    QTimer::singleShot(1, [=] {
        QImage img;
        cv::Mat m = cv::imread(fileName.toLocal8Bit().toStdString().c_str(), cv::IMREAD_COLOR);
        mat_to_qimage(m, img);
        if (v) {
            v->mRoi->setWidth(img.width());
            v->mRoi->setHeight(img.height());

            v->document()->setImageInternal(img);
            v->imageView()->updateBuffer();

            v->updateLayout();
            pView->updateLayout();

        }
        QApplication::restoreOverrideCursor();
    });

}

void MainWindow::setDialogconfig()
{
    DialogConfig *pDlg = new DialogConfig(this);
    pDlg->setModal(true);
    pDlg->exec();
    delete pDlg;
}

void MainWindow::finishNewRoiObject()
{
    Qroilib::DocumentView* v = currentView();
    v->clearSelectedObjectItems();
    v->selectTool(v->actSelectTool);
}

//void MainWindow::clickedPortsGroup(QAction *act)
//{

//#ifdef Q_OS_LINUX
//        QString name = "/dev/" + act->data().toString();
//#else
//        QString name = act->data().toString();
//#endif

//        light.Close();
//        light.Init(name);

//        light.SetBrightness(0, 100);
//}

//void MainWindow::setDialoglog()
//{
//    //QDateTime time = QDateTime::currentDateTime();
//    //log.logtime = time.toString("yyyyMMdd").toLatin1().data();
//    //DevLogSave("test1");
//    //DevLogSave("test2");

//    if (pLogViewDock == nullptr)
//        pLogViewDock = new LogViewDock();
//    pLogViewDock->clear();
//    pLogViewDock->show();
//    pLogViewDock->raise();
//}

void MainWindow::setDialogmodel()
{
    DialogModel *pDlg = new DialogModel(this);
    pDlg->setModal(true);
    pDlg->exec();
    delete pDlg;

//    if (pDlgModel == nullptr)
//        pDlgModel = new DialogModel();
//    pDlgModel->show();
//    pDlgModel->raise();

}

void MainWindow::DevLogSave(string strMsg, ...)
{
    QMutexLocker ml(&m_logsync);

    {
        char str[4096] = { 0, };

        va_list va;
        va_start(va, strMsg);
        int n = vsnprintf((char *)str, 4096, strMsg.c_str(), va);
        va_end(va);


        int len;
        while ((len = strlen(str)) > 0
            && (str[len - 1] == '\r' || str[len - 1] == '\n'))
        {
            str[len - 1] = 0;
        }

        //TRACE1(_T("DevLog: %s\n"), str);

        QString strMsg;
        string m_strFileName;

        strMsg = str;

        QDateTime time = QDateTime::currentDateTime();
        m_strFileName = string_format("%s/Log/%s_devlog.dat", gCfg.RootPath.toLatin1().data(),
            time.toString("yyyyMMdd").toLatin1().data());

        //string ts = QTime::currentTime().toString().toLatin1().data();
        string ts = QTime::currentTime().toString("hh:mm:ss.zzz").toLatin1().data();
        m_LogThread.Add(m_strFileName, ts, strMsg.toLatin1().data());
        //lastLogTime = time;

        if (pLogViewDock) {
            pLogViewDock->Add(QString::fromStdString(ts), strMsg);
        }
    }
}

// ch : 0(left) or 1(right)
// point : 1-1point aling, 2-two point, 3-all point align data
void MainWindow::AlignLogSave(int ch, int point, double x, double y)
{
    QMutexLocker ml(&m_alignlogsync);

    QString strMsg;
    string m_strFileName;

    strMsg.sprintf(" %2d %2d %7.2g %7.2g", ch, point, x, y);

    QDateTime time = QDateTime::currentDateTime();
    m_strFileName = string_format("%s/Log/%s_alignlog.dat", gCfg.RootPath.toLatin1().data(),
        time.toString("yyyyMM").toLatin1().data());

    string ts = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz").toLatin1().data();
    m_LogThread.Add(m_strFileName, ts, strMsg.toLatin1().data());

    if (mAlignGraphDock) {
        mAlignGraphDock->alignValue(ch, point, x, y); //  realtime graph data
    }
}
