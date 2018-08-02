/*

QROILIB : QT Vision ROI Library
Copyright 2018, Created by Yoon Jong-lock <jerry1455@gmail.com,jlyoon@yj-hitech.com>

이 Program은 Machine Vision Program을 QT 환경에서 쉽게 구현할수 있게 하기 위해 작성되었다.

Gwenview 의 Multi view 기능과,
Tiled의 Object drawing 기능을 가져와서 camera의 이미지를 실시간 display하면서 vision ROI를 작성하여,
내가 원하는 결과를 OpenCV를 통하여 쉽게 구현할수 있도록 한다.

이 Program은 ARM계열 linux  와 X86계열 linux 및 windows에서 test되었다.

------

qroiviewer:
    image read/write sample program

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

struct MainWindow::Private
{
    MainWindow* q;
    QWidget* mContentWidget;
    ViewMainPage* mViewMainPage;
    QStackedWidget* mViewStackedWidget;
    QActionGroup* mViewModeActionGroup;

    QMenu *fileMenu;

    QAction *saveImage;
    QAction *readImage;
    QAction *exitAct;
    QAction * mDeleteAction;

    void setupWidgets()
    {
        mContentWidget = new QWidget(q);
        q->setCentralWidget(mContentWidget);

        mViewStackedWidget = new QStackedWidget(mContentWidget);
        QVBoxLayout* layout = new QVBoxLayout(mContentWidget);
        layout->addWidget(mViewStackedWidget);
        layout->setMargin(0);
        layout->setSpacing(0);

        setupViewMainPage(mViewStackedWidget);
        mViewStackedWidget->addWidget(mViewMainPage);

    }

    void setupViewMainPage(QWidget* parent)
    {
        mViewMainPage = new ViewMainPage(parent);
    }

    void setupActions()
    {
        saveImage = new QAction(QIcon(), tr("&Save Image ..."), q);
        connect(saveImage, SIGNAL(triggered(bool)), q, SLOT(setSaveImage()));
        readImage = new QAction(QIcon(), tr("&Read Image ..."), q);
        connect(readImage, SIGNAL(triggered(bool)), q, SLOT(setReadImage()));

        exitAct = new QAction(QIcon(), tr("E&xit"), q);
        exitAct->setShortcuts(QKeySequence::Quit);
        connect(exitAct, SIGNAL(triggered()), q, SLOT(close()));

        fileMenu = new QMenu(tr("&File"), q);

        fileMenu->addAction(saveImage);
        fileMenu->addAction(readImage);
        fileMenu->addSeparator();
        fileMenu->addAction(exitAct);

        q->menuBar()->addMenu(fileMenu);

    }

};

MainWindow* theMainWindow = 0;

MainWindow::MainWindow()
: d(new MainWindow::Private)
{
    theMainWindow = this;

    setMinimumSize(800, 480);

    d->q = this;
    d->setupWidgets();
    d->setupActions();
    d->mViewMainPage->loadConfig();

    qApp->installEventFilter(this);

    Qroilib::DocumentView* v = currentView();
    if (v)
        v->selectTool(v->actSelectTool);

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

