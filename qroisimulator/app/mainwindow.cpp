/*

QROILIB : QT Vision ROI Library
Copyright 2018, Created by Yoon Jong-lock <jerry1455@gmail.com,jlyoon@yj-hitech.com>

이 Program은 Machine Vision Program을 QT 환경에서 쉽게 구현할수 있게 하기 위해 작성되었다.

Gwenview 의 Multi view 기능과,
Tiled의 Object drawing 기능을 가져와서 camera의 이미지를 실시간 display하면서 vision ROI를 작성하여,
내가 원하는 결과를 OpenCV를 통하여 쉽게 구현할수 있도록 한다.

이 Program은 ARM계열 linux  와 X86계열 linux 및 windows에서 test되었다.

------

qroisimulator:
    OpenCV simulator program

*/

#include <roilib_export.h>
// Qt
#include <QApplication>
#include <QPushButton>
#include <QStackedWidget>
#include <QTimer>
#include <QMenuBar>
#include <QUrl>
#include <QFileDialog>
#include <QMessageBox>

// Local
#include <qroilib/gvdebug.h>
#include "mainwindow.h"
#include "common.h"
#include "recipedata.h"
#include "dialogcamera.h"
#include "dialogedge.h"
#include "dialogthreshold.h"
#include "dialogmorphology.h"
#include "dialogarithmeticlogic.h"
#include "dialogcontour.h"
#include "dialogblob.h"
#include "dialoglinefit.h"
#include "dialogcircle.h"
#include "dialogapplication.h"

struct MainWindow::Private
{
    MainWindow* q;
    QWidget* mContentWidget;
    ViewMainPage* mViewMainPage;
    QStackedWidget* mViewStackedWidget;
    QActionGroup* mViewModeActionGroup;

    QMenu *fileMenu;
    QMenu *simulatorMenu;
    QMenu *applicationMenu;

    QAction *saveImage;
    QAction *readImage;
    QAction *dialogCamera;
    QAction *exitAct;

    QAction *thresholdImage;
    QAction *mopologyImage;
    QAction *edgeImage;
    QAction *blobImage;
    QAction *contourImage;
    QAction *arithmeticlogicImage;
    QAction *linefitImage;
    QAction *ransaccircleImage;

    QAction *applicationImage;

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
        saveImage = new QAction(QIcon(), tr("&Save Image ..."), q);
        connect(saveImage, SIGNAL(triggered(bool)), q, SLOT(setSaveImage()));
        readImage = new QAction(QIcon(), tr("&Read Image ..."), q);
        connect(readImage, SIGNAL(triggered(bool)), q, SLOT(setReadImage()));
        dialogCamera = new QAction(QIcon(":/resources/camera.png"), tr("&Camera ..."), q);
        connect(dialogCamera, SIGNAL(triggered(bool)), q, SLOT(setDialogcamera()));
        exitAct = new QAction(QIcon(), tr("E&xit"), q);
        exitAct->setShortcuts(QKeySequence::Quit);
        connect(exitAct, SIGNAL(triggered()), q, SLOT(close()));

        thresholdImage = new QAction(QIcon(), tr("&Threshold Image ..."), q);
        connect(thresholdImage, SIGNAL(triggered(bool)), q, SLOT(setThreshold()));
        mopologyImage = new QAction(QIcon(), tr("&Mopology Image ..."), q);
        connect(mopologyImage, SIGNAL(triggered(bool)), q, SLOT(setMopology()));
        edgeImage = new QAction(QIcon(), tr("&Edge Image ..."), q);
        connect(edgeImage, SIGNAL(triggered(bool)), q, SLOT(setEdge()));
        blobImage = new QAction(QIcon(), tr("&Blob Image ..."), q);
        connect(blobImage, SIGNAL(triggered(bool)), q, SLOT(setBlob()));
        contourImage = new QAction(QIcon(), tr("&Contour Image ..."), q);
        connect(contourImage, SIGNAL(triggered(bool)), q, SLOT(setContour()));
        arithmeticlogicImage = new QAction(QIcon(), tr("&Arithmeticlogic Image ..."), q);
        connect(arithmeticlogicImage, SIGNAL(triggered(bool)), q, SLOT(setArithmeticlogic()));
        linefitImage = new QAction(QIcon(), tr("&LineFit Image ..."), q);
        connect(linefitImage, SIGNAL(triggered(bool)), q, SLOT(setLineFit()));
        ransaccircleImage = new QAction(QIcon(), tr("&Circle Image ..."), q);
        connect(ransaccircleImage, SIGNAL(triggered(bool)), q, SLOT(setCircle()));

        applicationImage = new QAction(QIcon(), tr("&Application ..."), q);
        connect(applicationImage, SIGNAL(triggered(bool)), q, SLOT(setApplication()));

        fileMenu = new QMenu(tr("&File"), q);
        fileMenu->addAction(saveImage);
        fileMenu->addAction(readImage);
        fileMenu->addSeparator();
        fileMenu->addAction(dialogCamera);
        fileMenu->addAction(exitAct);

        simulatorMenu = new QMenu(tr("&Simulator"), q);
        simulatorMenu->addAction(thresholdImage);
        simulatorMenu->addAction(mopologyImage);
        simulatorMenu->addAction(edgeImage);
        simulatorMenu->addAction(blobImage);
        simulatorMenu->addAction(contourImage);
        simulatorMenu->addAction(arithmeticlogicImage);
        simulatorMenu->addAction(linefitImage);
        simulatorMenu->addAction(ransaccircleImage);

        applicationMenu = new QMenu(tr("&Application"), q);
        applicationMenu->addAction(applicationImage);

        q->menuBar()->addMenu(fileMenu);
        q->menuBar()->addMenu(simulatorMenu);
        q->menuBar()->addMenu(applicationMenu);

    }

};

MainWindow* theMainWindow = 0;

MainWindow::MainWindow()
: d(new MainWindow::Private)
{
    theMainWindow = this;

    setMinimumSize(320, 240);
    g_cRecipeData = new CRecipeData(this);

    d->q = this;
    d->setupWidgets();
    d->setupActions();
    d->mViewMainPage->loadConfig();

    extern Qroilib::ParamTable ROIDSHARED_EXPORT paramTable[];
    DocumentView* v = currentView();
    if (v)
    {
        v->setParamTable(paramTable);
        v->bMultiView = false;
        connect(v, &Qroilib::DocumentView::finishNewRoiObject, this, &MainWindow::finishNewRoiObject);
        //v->selectTool(v->actSelectTool);
        v->selectTool(v->actCreateRectangle);
    }

}


MainWindow::~MainWindow()
{
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
                                   QMessageBox::Ok);
    switch(ret)
    {
        case QMessageBox::Ok:
            event->accept();

            delete d->mViewMainPage;
            delete d;

            qApp->quit(); // terminates application.
            break;
    }
}


void MainWindow::setSaveImage()
{
    DocumentView *v = (DocumentView *)viewMainPage()->currentView();
    if (v) {
        const QImage *pimg = v->image();
        if (!pimg)
            return;

        //QApplication::setOverrideCursor(Qt::WaitCursor);
        QString fileName = QFileDialog::getSaveFileName(this,
                tr("Save Image"), "",
                tr("Images (*.png *.bmp *.jpg *.tif *.ppm *.GIF);;All Files (*)"));
        if (fileName.isEmpty()) {
            //QApplication::restoreOverrideCursor();
            return;
        }
        //QApplication::setOverrideCursor(Qt::WaitCursor);
        QTimer::singleShot(1, [=] {
            pimg->save(fileName);
            //QApplication::restoreOverrideCursor();
        });
    }
}

void MainWindow::setReadImage()
{
    ViewMainPage* pView = viewMainPage();
    if (!pView)
        return;

    DocumentView* v = currentView();
    if (!v)
        return;

    QString fileName = QFileDialog::getOpenFileName(this,
            tr("Open Image"), QDir::currentPath(),
            tr("Images (*.png *.bmp *.jpg *.tif *.ppm *.GIF);;All Files (*)"));
    if (fileName.isEmpty()) {
        return;
    }

     v->selectTool(v->actCreateRectangle);

    // 기존 ROI들 clear
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

    // 파일 load
    v = currentView();

    QTimer::singleShot(1, [=] {
        //QApplication::setOverrideCursor(Qt::WaitCursor);
        QImage img;
        cv::Mat m = cv::imread(fileName.toLocal8Bit().toStdString().c_str(), cv::IMREAD_COLOR);
        mat_to_qimage(m, img);
        if (v) {
            QImage *pimg = (QImage *)v->image();
            if (pimg->isNull())
            {
                v->mRoi->setWidth(img.width());
                v->mRoi->setHeight(img.height());

                v->document()->setImageInternal(img);
                v->imageView()->updateBuffer();

                v->updateLayout();
                pView->updateLayout();

                pimg = (QImage *)v->image();
                pView->processedImage(pimg);


                QTimer *timer1 = new QTimer(this);
                timer1->setSingleShot(true);
                connect(timer1, &QTimer::timeout, [=]() {
                    pView->FitU();
                } );
                timer1->start(100);
            } else {
                QString date = QDateTime::currentDateTime().toString("yyyyMMdd");
                QString time = QDateTime::currentDateTime().toString("hhmmsszzz");
                IplImage iplImg = m;
                QString title = QString("New%1%2").arg(date).arg(time);
                outWidget(title, &iplImg);
            }
        }
        //QApplication::restoreOverrideCursor();
    });

}

void MainWindow::setDialogcamera()
{
    DialogCamera *pDlg = new DialogCamera(this);
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

//
// 같은 이름의 outWidget은 만들지 않는다.
void MainWindow::outWidget(QString title, IplImage* iplImg)
{
    OutWidget *myWidget = nullptr;
    int size = vecOutWidget.size();
    for (int i=0; i<size; i++) {
        QString name = vecOutWidget[i]->windowTitle();
        if (title == name) {
            myWidget = vecOutWidget[i];
            break;
        }
    }
    if (!myWidget){
        myWidget= new OutWidget(title, this);
        vecOutWidget.push_back(myWidget);
    }

    cv::Mat m = cv::cvarrToMat(iplImg);
    DocumentView* v = myWidget->mViewOutPage->currentView();
    if (!v)
        return;
    QImage img;
    mat_to_qimage(m, img);
    if (v) {
        v->mRoi->setWidth(img.width());
        v->mRoi->setHeight(img.height());

        v->document()->setImageInternal(img);
        v->imageView()->updateBuffer();

        v->updateLayout();
        myWidget->mViewOutPage->updateLayout();
        const QImage *pimg = v->image();
        myWidget->mViewOutPage->processedImage(pimg);

        Qroilib::RasterImageView* pImgView = myWidget->mViewOutPage->imageView();
        pImgView->setScalerSmooth(true);
    }
    myWidget->show();
}

void MainWindow::setThreshold()
{
    DialogThreshold *dlgthreshold;
    dlgthreshold = new DialogThreshold(this);
    dlgthreshold->show();
    //dlgthreshold->raise();
}

void MainWindow::setMopology()
{
    DialogMorphology *dlgmorphology;
    dlgmorphology = new DialogMorphology(this);
    dlgmorphology->show();
}

void MainWindow::setEdge()
{
    DialogEdge *dlgedge;
    dlgedge = new DialogEdge(this);
    dlgedge->show();
}

void MainWindow::setBlob()
{
    DialogBlob *dlgblob;
    dlgblob = new DialogBlob(this);
    dlgblob->show();
}

void MainWindow::setContour()
{
    DialogContour *dlgcontour;
    dlgcontour = new DialogContour(this);
    dlgcontour->show();
}

void MainWindow::setArithmeticlogic()
{
    DialogArithmeticlogic *dlgarithmeticlogic;
    dlgarithmeticlogic = new DialogArithmeticlogic(this);
    //dlgarithmeticlogic->setWindowModality(Qt::NonModal);
    dlgarithmeticlogic->show();
    //dlgarithmeticlogic->exec();
}

void MainWindow::setLineFit()
{
    DialogLinefit *dlglinefit;
    dlglinefit = new DialogLinefit(this);
    dlglinefit->show();
}
void MainWindow::setCircle()
{
    DialogCircle *dlgcircle;
    dlgcircle = new DialogCircle(this);
    dlgcircle->show();
}

void MainWindow::setApplication()
{
    DialogApplication *dlgapp;
    dlgapp = new DialogApplication(this);
    dlgapp->show();
}

