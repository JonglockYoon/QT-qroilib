/*
viewmainpage.h
*/

#pragma once

#ifndef VIEWMAINPAGE_H
#define VIEWMAINPAGE_H

// Local
#include <qroilib/document/document.h>

// Qt
#include <QUrl>
#include <QToolButton>
#include <QWidget>
#include <QLabel>

#include "camcapture.h"

class QGraphicsWidget;

//using namespace Qroilib;

#include <qroilib/document/document.h>
#include <qroilib/documentview/abstractdocumentviewadapter.h>
#include <qroilib/documentview/documentview.h>
#include <qroilib/documentview/rasterimageview.h>
#include <qroilib/documentview/documentviewcontainer.h>
#include <qroilib/documentview/documentviewcontroller.h>
#include "roiscene.h"
//#include "roipropertyeditor.h"

//class RasterImageView;
//class RoiScene;
//class RoiPropertyEditor;
//class DocumentView;
class RoiPropertyEditor;

struct ViewMainPagePrivate;

/**
 * Holds the active document view and associated widgetry.
 */
class ViewMainPage : public QWidget
{
    Q_OBJECT
public:
    static const int MaxViewCount;

    ViewMainPage(QWidget* parent);
    ~ViewMainPage();

    void updateLayout();

    /**
     * Reset the view
     */
    void reset();

    int statusBarHeight() const;

    virtual QSize sizeHint() const;

    /**
     * Returns the url of the current document, or an invalid url if unknown
     */
    QUrl url() const;


    /**
     * Opens up to MaxViewCount urls, and set currentUrl as the current one
     */
    void openUrls(const QList<QUrl>& urls, const QUrl &currentUrl);

    bool isEmpty() const;

    /**
     * Returns the image view, if the current adapter has one.
     */
    Qroilib::RasterImageView* imageView() const;

    /**
     * Returns the document view
     */
    Qroilib::DocumentView* currentView() const;

    Qroilib::DocumentView* view(int n) const;

    void OpenCam(int nv, int nCam);
    void CloseCam(int seq);

Q_SIGNALS:

    /**
     * Emitted when the part has finished loading
     */

    //void previousImageRequested();

    //void nextImageRequested();

    void toggleFullScreenRequested();

    void goToBrowseModeRequested();

    //void captionUpdateRequested(const QString&);

public Q_SLOTS:
    //void slotViewFocused(Qroilib::DocumentView*);
    void setCurrentView(Qroilib::DocumentView* view);
    void editRoiProperty();

private Q_SLOTS:
    void completed(int seq);

    void showContextMenu();

//    void trashView(DocumentView*);
//    void deselectView(DocumentView*);

//    void timerTick();

    void updatePlayerUI(const QImage& img, int seq);

private:
    friend struct ViewMainPagePrivate;
    ViewMainPagePrivate* const d;
protected:
    bool eventFilter(QObject *, QEvent *);

public:
    QLabel* mStatusLabel;
    CamCapture* myCamCapture[2]; // DocumentView 와 갯수동일하게 유지 필요.
    QObject* objContainer;
    RoiPropertyEditor *roipropertyeditor;

};

#endif /* VIEWMAINPAGE_H */
