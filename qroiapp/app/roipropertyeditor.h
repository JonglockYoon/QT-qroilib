#pragma once

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QtTreePropertyBrowser>
#include <opencv2/core/core.hpp>
#include <QBasicTimer>
#include <QTime>

#include "variantmanager.h"
#include "roiobject.h"
#include <qroilib/documentview/documentview.h>

using namespace Qroilib;

class RoiObject;
class Qroilib::DocumentView;

class RoiPropertyEditor : public QDialog
{
    Q_OBJECT
public:
    RoiPropertyEditor(QWidget *parent, Qroilib::RoiObject *object);
    ~RoiPropertyEditor();
private slots:
    //void createAndControl();
    void clickedInspButtonSlot();
    void clickedUpdateButtonSlot();
    void clickedPreviewButtonSlot();
    void clickedSaveButtonSlot();
    void clickedSaveTemplateSlot();
    void clickedEngineButtonSlot();
    void clickededitRoiNameButtonSlot();
    void clickClose();
private:
    QComboBox *theClassCombo;
    //ObjectController *theController;
    //QStringList theClassNames;
    //QObject *theControlledObject;
    void UpdateParamVal(QString name, int type, QString val);

    QPushButton *inspButtonBox;
    QPushButton *updateButtonBox;
    QPushButton *previewButtonBox;
    QPushButton *applyButtonBox;
    QDialogButtonBox *closeButtonBox;
    QLabel *labelRoiName;
    QLineEdit *editRoiName;
    QPushButton *editRoiNameButton;
    QPushButton *engineButtonBox;
    QPushButton *patternButtonBox;
    QLabel *labelImageDisplay;

    Qroilib::VariantManager *variantManager;
    QtVariantEditorFactory *variantFactory;
    QtTreePropertyBrowser *variantEditor;

    QtVariantProperty *item;

public:
    void ObjectListSet();
    void ObjectListGet();

public slots:
    void slotComboActivated();

public:
    Qroilib::RoiObject *parentObject;
    Qroilib::RoiObject roiObject;
    Qroilib::RoiObject *mObject; // selected roimap object;
    cv::Mat frame;
    IplImage riplImg;
    IplImage *iplImg;
private:
    IplImage* grayImg;
    QImage img;

    QBasicTimer m_timer;
    void timerEvent(QTimerEvent *ev);

signals:
    void processedImage(const QImage &image, int seq);

};

