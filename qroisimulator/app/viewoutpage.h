/*
ViewOutPage.h
*/

#pragma once

#ifndef ViewOutPage_H
#define ViewOutPage_H

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

class RoiPropertyEditor;

struct ViewOutPagePrivate;

/**
 * Holds the active document view and associated widgetry.
 */
class ViewOutPage : public QWidget
{
    Q_OBJECT
public:
    static const int MaxViewCount;

    ViewOutPage(QWidget* parent);
    ~ViewOutPage();

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

    IplImage* getIplgray();
    IplImage* getIplcolor();

Q_SIGNALS:

    /**
     * Emitted when the part has finished loading
     */
    void processedImage(const QImage *image);


public Q_SLOTS:
    void setCurrentView(Qroilib::DocumentView* view);

    void FitU();
    void SaveU();
    void SetROIU();

private Q_SLOTS:
    void completed(int seq);

    void ZoomInU();
    void ZoomOutU();
    void showContextMenu();

private:
    friend struct ViewOutPagePrivate;
    ViewOutPagePrivate* const d;
protected:
    bool eventFilter(QObject *, QEvent *);

public:
    QLabel* mStatusLabel;
    QObject* objContainer;
};

#endif /* ViewOutPage_H */
