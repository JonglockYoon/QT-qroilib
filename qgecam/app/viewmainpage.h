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

//using namespace Qroilib;

#include <qroilib/document/document.h>
#include <qroilib/documentview/abstractdocumentviewadapter.h>
#include <qroilib/documentview/documentview.h>
#include <qroilib/documentview/rasterimageview.h>
#include <qroilib/documentview/documentviewcontainer.h>
#include <qroilib/documentview/documentviewcontroller.h>
#include "roiscene.h"

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

private Q_SLOTS:
    void completed(int seq);

    void showContextMenu();

private:
    friend struct ViewMainPagePrivate;
    ViewMainPagePrivate* const d;

public:
    QLabel* mStatusLabel;
    QObject* objContainer;

};

#endif /* VIEWMAINPAGE_H */
