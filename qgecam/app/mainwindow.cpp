/*

QROILIB : QT Vision ROI Library
Copyright 2018, Created by Yoon Jong-lock <jerry1455@gmail.com,jlyoon@yj-hitech.com>

이 Program은 Machine Vision Program을 QT 환경에서 쉽게 구현할수 있게 하기 위해 작성되었다.

Gwenview 의 Multi view 기능과,
Tiled의 Object drawing 기능을 가져와서 camera의 이미지를 실시간 display하면서 vision ROI를 작성하여,
내가 원하는 결과를 OpenCV를 통하여 쉽게 구현할수 있도록 한다.

이 Program은 ARM계열 linux  와 X86계열 linux에서 test되었다.

------------

qgecam : Giga Ethernet Camera Control & Display Program
         jiguiviou project를 qroilib 와 접목을 시도중이다.

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
#include "camerasession.h"
#include "gvsp.h"

using namespace Qroilib;

struct MainWindow::Private
{
    MainWindow* q;
    QWidget* mContentWidget;
    ViewMainPage* mViewMainPage;
    QStackedWidget* mViewStackedWidget;
    QActionGroup* mViewModeActionGroup;

    QString mCaption;


    QMenu *fileMenu;
    QMenu *sessionMenu;

    QAction *saveImage;
    QAction *exitAct;
    QAction * mDeleteAction;

    QScopedPointer<CameraSession> session;

    void setupContextManager()
    {

    }

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

        exitAct = new QAction(QIcon(":/resources/exit.png"), tr("E&xit"), q);
        exitAct->setShortcuts(QKeySequence::Quit);
        connect(exitAct, SIGNAL(triggered()), q, SLOT(close()));

        fileMenu = new QMenu(tr("&File"), q);

        fileMenu->addAction(saveImage);
        fileMenu->addSeparator();
        fileMenu->addAction(exitAct);

        q->menuBar()->addMenu(fileMenu);

        sessionMenu = new QMenu(trUtf8("Session"), q);
        q->menuBar()->addMenu(sessionMenu);
    }

    void createToolBars()
    {
        QToolBar* toolBar;
        toolBar = q->addToolBar(tr("&Tool"));
        toolBar->addAction(exitAct);

        toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        toolBar->setIconSize(QSize(32,32));

        toolBar->installEventFilter(q);
    }

};

MainWindow* theMainWindow = 0;

MainWindow::MainWindow()
:d(new MainWindow::Private)
{
    theMainWindow = this;

    setMinimumSize(800, 480);

    d->q = this;
    d->setupContextManager();
    d->setupWidgets();
    d->setupActions();
    d->createToolBars();
    d->mViewMainPage->loadConfig();

    qApp->installEventFilter(this);

    QTimer *timer1 = new QTimer(this);
    timer1->setSingleShot(true);
    connect(timer1, &QTimer::timeout, [=]() {

        static QScopedPointer<CameraSession> locale(new CameraSession(this));
        if (locale->startSession()) {
            d->sessionMenu->addActions(locale->menuActions());
            locale->leftDockedWidget()->show();
        }

    } );
    timer1->start(1000);
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

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    //Q_UNUSED(obj);
    //Q_UNUSED(event);

    if (event->type() == QEvent::MouseMove || event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);

        QObject* tmp = viewMainPage()->objContainer;
        if (obj->parent() != tmp) {
            d->mViewMainPage->mStatusLabel->setText("");
            return false;
        }

        DocumentView *v = (DocumentView *)viewMainPage()->currentView();
        if (!v)
            return false;
        const RasterImageView* imgView = viewMainPage()->imageView();
        if (imgView) {
            //qDebug() << obj;
            const QImage *pimg = v->image();
            //const QPointF pt = imgView->getLastMousePos();
            int x = mouseEvent->pos().x();
            int y = mouseEvent->pos().y();
            const QPointF pt1 = imgView->mImageOffset;
            x = x - pt1.x();
            y = y - pt1.y();
            const QPointF pt2 = imgView->mScrollPos;
            x = x + pt2.x();
            y = y + pt2.y();

            const qreal z = v->zoom();
            x = x / z;
            y = y / z;

            if (x >= 0 && y >= 0 && x < pimg->width() && y < pimg->height())
            {
                QRgb c = pimg->pixel(x, y);
                const QColor src(c);

                int r = src.red();
                int g = src.green();
                int b = src.blue();

                QString str, str1;
                str.sprintf("x:%d y:%d  r:%d g:%d b:%d", x,y ,r,g,b);
                double gray = r * 0.299f + g * 0.587f + b * 0.114f; // https://docs.opencv.org/3.3.0/de/d25/imgproc_color_conversions.html
                if (gray < 0.0) gray = 0;
                if (gray > 255.0) gray = 255;
                str1.sprintf(" Gray: %3.0f", ceil(gray));
                str += str1;

                d->mViewMainPage->mStatusLabel->setText(str);
            }
            else d->mViewMainPage->mStatusLabel->setText("");
        }
        else
            d->mViewMainPage->mStatusLabel->setText(QString("Mouse move (%1,%2)").arg(mouseEvent->pos().x()).arg(mouseEvent->pos().y()));
    }

    return false;
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


void mat_to_qimage(cv::InputArray image, QImage& out)
{
    switch(image.type())
    {
        case CV_8UC4:
        {
            cv::Mat view(image.getMat());
            QImage view2(view.data, view.cols, view.rows, view.step[0], QImage::Format_ARGB32);
            out = view2.copy();
            break;
        }
        case CV_8UC3:
        {
            cv::Mat mat;
            cv::cvtColor(image, mat, cv::COLOR_BGR2BGRA); //COLOR_BGR2RGB doesn't behave so use RGBA
            QImage view(mat.data, mat.cols, mat.rows, mat.step[0], QImage::Format_ARGB32);
            out = view.copy();
            break;
        }
        case CV_8UC1:
        {
            cv::Mat mat;
            cv::cvtColor(image, mat, cv::COLOR_GRAY2BGRA);
            QImage view(mat.data, mat.cols, mat.rows, mat.step[0], QImage::Format_ARGB32);
            out = view.copy();
            break;
        }
        default:
        {
            //throw invalid_argument("Image format not supported");
            break;
        }
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
    //DocumentView* pdocview = pView->currentView();
    DocumentView* v = currentView();
    if (!v)
        return;


    QString fileName = QFileDialog::getOpenFileName(this,
            tr("Open Image"), QDir::currentPath(),
            tr("Images (*.png *.bmp *.jpg *.tif *.ppm *.GIF);;All Files (*)"));
    if (fileName.isEmpty()) {
        return;
    }

    QImage img;
    cv::Mat m = cv::imread(fileName.toLocal8Bit().toStdString().c_str(), 0);
    mat_to_qimage(m, img);
    if (v) {
        v->document()->setImageInternal(img);
        v->imageView()->updateBuffer();
    }
}

void MainWindow::updatePlayerGvspUI(const QImage &qimg)
{

    Qroilib::DocumentView *view = viewMainPage()->currentView();
    view->document()->setImageInternal(qimg);
    view->imageView()->updateBuffer();
}
