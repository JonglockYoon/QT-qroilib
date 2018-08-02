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

protected:
    void closeEvent(QCloseEvent *event);

private:
    struct Private;
    MainWindow::Private* const d;

public  Q_SLOTS:
    void setSaveImage();
    void setReadImage();
};

extern MainWindow* theMainWindow;

#endif /* MAINWINDOW_H */
