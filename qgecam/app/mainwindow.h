/*
mainwindow.h
*/
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#define SIMULATION

// Qt
#include <QMainWindow>
#include <QAction>
#include <QToolBar>
#include <QMutex>

#include "viewmainpage.h"
#include "gvspreceiver.h"

class QModelIndex;
class QUrl;

using namespace Jgv::Gvsp;

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
    bool bCamPause[2];

    /**
     * Defines the url to display when the window is shown for the first time.
     */
    void setInitialUrl(const QUrl&);

    ViewMainPage* viewMainPage() const;
    DocumentView* currentView() const;

protected:
    bool eventFilter(QObject *, QEvent *);

protected:
    void closeEvent(QCloseEvent *event);

private:
    struct Private;
    MainWindow::Private* const d;

public Q_SLOTS:
    void updatePlayerGvspUI(const QImage &);

public  Q_SLOTS:
    void setSaveImage();
    void setReadImage();
};

extern MainWindow* theMainWindow;

#endif /* MAINWINDOW_H */
