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

class QGraphicsWidget;

#include <qroilib/document/document.h>
#include <qroilib/documentview/abstractdocumentviewadapter.h>
#include <qroilib/documentview/documentview.h>
#include <qroilib/documentview/rasterimageview.h>
#include <qroilib/documentview/documentviewcontainer.h>
#include <qroilib/documentview/documentviewcontroller.h>
#include "roiscene.h"
#include "camcapture.h"

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

    void loadConfig();

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

    IplImage* getIplgray(int n);
    IplImage* getIplcolor(int n);

Q_SIGNALS:

    /**
     * Emitted when the part has finished loading
     */
    void processedImage(const QImage *image);

    //void previousImageRequested();

    //void nextImageRequested();

    void toggleFullScreenRequested();

    void goToBrowseModeRequested();

    //void captionUpdateRequested(const QString&);

public Q_SLOTS:
    //void slotViewFocused(Qroilib::DocumentView*);
    void setCurrentView(Qroilib::DocumentView* view);
    void editRoiProperty();

    void FitU();

private Q_SLOTS:
    void completed(int seq);

    void ZoomInU();
    void ZoomOutU();
    void showContextMenu();

    void updatePlayerUI(const QImage& img, int seq);

private:
    friend struct ViewMainPagePrivate;
    ViewMainPagePrivate* const d;
protected:
    bool eventFilter(QObject *, QEvent *);

public:
    QLabel* mStatusLabel;
    QObject* objContainer;
     CamCapture* myCamCapture[2]; // DocumentView 와 갯수동일하게 유지 필요.
    RoiPropertyEditor *roipropertyeditor;
    bool bPreview = true;
};

#endif /* VIEWMAINPAGE_H */
