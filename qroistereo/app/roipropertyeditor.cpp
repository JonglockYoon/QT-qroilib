//
// roipropertyeditor.cpp
//
//QROILIB : QT Vision ROI Library
//Copyright 2018, Created by Yoon Jong-lock <jerry1455@gmail.com,jlyoon@yj-hitech.com>
//
#include <QApplication>
#include <QDesktopWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include <QComboBox>
#include <QMessageBox>
#include <QTimerEvent>
#include <QDesktopServices>

#include "roid_global.h"

#include <qroilib/documentview/documentview.h>
#include "roipropertyeditor.h"
#include "roimap.h"
#include "roiobject.h"
#include "qpatternroif.h"
#include "recipedata.h"
#include "mainwindow.h"
#include "viewmainpage.h"
#include "common.h"

QString  StepTypeValue[] =
{
    ("MaskingValue"),
    ("ProcessValue1"),
    ("ProcessValue2"),
    ("ProcessValue3"),
    ("FilterValue"),
    ("DecideValue"),
    ("PostProcessValue1"),
    ("PostProcessValue2"),
    ("PriorityValue(0-High)"),
    ("LightValue"),
};

//using namespace cv;
using namespace Qroilib;

extern Qroilib::ParamTable ROIDSHARED_EXPORT paramTable[];

RoiPropertyEditor::RoiPropertyEditor(QWidget *parent, Qroilib::RoiObject *object)
    : QDialog(parent)
{
    grayImg = nullptr;

    if (object->mParent != nullptr) {
        parentObject = object->mParent;
    }
    else {
        parentObject = object;
    }
    roiObject = *parentObject;
    mObject = &roiObject;

    theClassCombo = new QComboBox(this);
    inspButtonBox = new QPushButton("Inspection");
    previewButtonBox = new QPushButton("Preview");
    saveButtonBox = new QPushButton("Save");
    patternButtonBox = new QPushButton("TemplateSave");
    closeButtonBox = new QDialogButtonBox(this);
    closeButtonBox->setStandardButtons(QDialogButtonBox::Close);
    labelRoiName = new QLabel(this);
    editRoiName = new QLineEdit(this);
    editRoiNameButton = new QPushButton(this);
    editRoiNameButton->setText("edit");
    labelImageDisplay = new QLabel(this);
    QString name = mObject->name();
    if (name.isEmpty())
        name = "noname";
    labelRoiName->setText("Name : ");
    editRoiName->setText(name);
    editRoiName->setDisabled(true);

    QObject::connect(inspButtonBox, SIGNAL(clicked()),this, SLOT(clickedInspButtonSlot()));
    QObject::connect(previewButtonBox, SIGNAL(clicked()),this, SLOT(clickedPreviewButtonSlot()));
    QObject::connect(saveButtonBox, SIGNAL(clicked()),this, SLOT(clickedSaveButtonSlot()));
    connect(closeButtonBox, SIGNAL(rejected()), this, SLOT(clickClose()));
    QObject::connect(editRoiNameButton, SIGNAL(clicked()),this, SLOT(clickededitRoiNameButtonSlot()));

    QVBoxLayout *layout = new QVBoxLayout(this);

    if (parentObject->shape() == Qroilib::RoiObject::Pattern)
    {
        // ROI Name & PatternImage BOX
        QHBoxLayout *internalLayout0 = new QHBoxLayout(this);
        layout->addLayout(internalLayout0);
        QVBoxLayout *v1 = new QVBoxLayout();
        QHBoxLayout *h1 = new QHBoxLayout();
        h1->addWidget(labelRoiName);
        h1->addWidget(editRoiName);
        h1->addWidget(editRoiNameButton);
        v1->addLayout(h1);
        v1->addWidget(patternButtonBox);
        internalLayout0->addLayout(v1);
        internalLayout0->addStretch();
        internalLayout0->addWidget(labelImageDisplay);
        QObject::connect(patternButtonBox, SIGNAL(clicked()),this, SLOT(clickedSaveTemplateSlot()));
        patternButtonBox->setFixedSize(QSize(150, 32));

        QImage image;
        if (parentObject->mPattern && parentObject->iplTemplate) {
            Mat mat = cvarrToMat(parentObject->iplTemplate);
            mat_to_qimage(mat, image);
        } else
            image = QImage(":/resources/camimg.png");

        image = image.scaled(64, 64, Qt::KeepAspectRatio);
        labelImageDisplay->setPixmap(QPixmap::fromImage(image));
    } else {
        QHBoxLayout *internalLayout0 = new QHBoxLayout(this);
        layout->addLayout(internalLayout0);
        QVBoxLayout *v1 = new QVBoxLayout();
        QHBoxLayout *h1 = new QHBoxLayout();
        h1->addWidget(labelRoiName);
        h1->addWidget(editRoiName);
        h1->addWidget(editRoiNameButton);
        h1->addStretch();
        v1->addLayout(h1);
        internalLayout0->addLayout(v1);
    }

    // Select Combo BOX
    QHBoxLayout *internalLayout1 = new QHBoxLayout();
    internalLayout1->addWidget(theClassCombo);
    layout->addLayout(internalLayout1);

    // ROI Editor BOX
    variantEditor = new QtTreePropertyBrowser();
    variantFactory = new QtVariantEditorFactory();
    variantManager = new VariantManager();
    QtVariantPropertyManager *varMan = variantManager;
    variantEditor->setFactoryForManager(varMan, variantFactory);
    layout->addWidget(variantEditor);

    // Button BOX
    QHBoxLayout *internalLayout2 = new QHBoxLayout();
    internalLayout2->addWidget(inspButtonBox);
    internalLayout2->addWidget(previewButtonBox);
    internalLayout2->addWidget(saveButtonBox);
    internalLayout2->addWidget(closeButtonBox);
    layout->addLayout(internalLayout2);

    QRect r = geometry();
    r.setSize(sizeHint());
    r.setWidth(qMax(r.width(), 400));
    r.setHeight(qMax(r.height(), 600));
    r.moveCenter(QApplication::desktop()->geometry().center());
    setGeometry(r);

    QStringList theClassNames;

    connect(theClassCombo, SIGNAL(activated(int)), this, SLOT(slotComboActivated()));

    int nCbIdx = 0;
    switch(mObject->shape())
    {
        case Qroilib::RoiObject::Pattern:
            for(int i=_Inspect_Patt_Start+1; i<_Inspect_Patt_End; i++){
                theClassNames.append(g_cRecipeData->m_sInspList[i]);
            }
            nCbIdx = mObject->mInspectType - (_Inspect_Patt_Start + 1);
            break;
        case Qroilib::RoiObject::Rectangle:
            for(int i=_Inspect_Roi_Start+1; i<_Inspect_Roi_End; i++){
                theClassNames.append(g_cRecipeData->m_sInspList[i]);
            }
            nCbIdx = mObject->mInspectType - (_Inspect_Roi_Start + 1);
            break;

        case Qroilib::RoiObject::Point:
            for(int i=_Inspect_Point_Start+1; i<_Inspect_Point_End; i++){
                theClassNames.append(g_cRecipeData->m_sInspList[i]);
            }
            nCbIdx = mObject->mInspectType - (_Inspect_Point_Start + 1);
            break;
    }

    theClassCombo->addItems(theClassNames);
    theClassCombo->setCurrentIndex(nCbIdx);

    ObjectListSet();

}


RoiPropertyEditor::~RoiPropertyEditor()
{
    m_timer.stop();
    if (grayImg)
        cvReleaseImage(&grayImg);
}

void RoiPropertyEditor::timerEvent(QTimerEvent *ev)
{
    if (ev->timerId() != m_timer.timerId()) {
      return;
    }

    ViewMainPage* pMainView = theMainWindow->viewMainPage();
    DocumentView* pdocview = pMainView->currentView();

    pdocview->document()->setImageInternal(img);
    pdocview->imageView()->updateBuffer();
}

void RoiPropertyEditor::clickClose()
{
    m_timer.stop();

    delete variantFactory;
    delete variantManager;
    delete variantEditor;
    delete inspButtonBox;
    delete previewButtonBox;
    delete closeButtonBox;
    delete theClassCombo;
    delete patternButtonBox;
    delete labelRoiName;
    delete labelImageDisplay;

    reject();
}

void RoiPropertyEditor::clickedSaveButtonSlot()
{
    ObjectListGet();

    *parentObject = *mObject;
    g_cRecipeData->SaveRecipeData();

}

void RoiPropertyEditor::clickedPreviewButtonSlot()
{
    m_timer.stop();
    ViewMainPage* pMainView = theMainWindow->viewMainPage();
    pMainView->bPreview = true;
}

void RoiPropertyEditor::clickedInspButtonSlot()
{
    m_timer.stop();

    ViewMainPage* pMainView = theMainWindow->viewMainPage();
    DocumentView* pdocview = pMainView->currentView();

    ObjectListGet();

    const QImage *camimg = pdocview->image();
    if (camimg->isNull())
        return;

    QApplication::setOverrideCursor(Qt::WaitCursor);
    qimage_to_mat(camimg, frame);

    riplImg = frame;
    iplImg = &riplImg;

    CvSize isize = cvSize(iplImg->width, iplImg->height);
    if (grayImg)
        cvReleaseImage(&grayImg);
    grayImg = cvCreateImage(isize, IPL_DEPTH_8U, 1);
    if (iplImg->nChannels == 3)
        cvCvtColor(iplImg, grayImg, CV_RGB2GRAY);
    else if (iplImg->nChannels == 4) {
        if (strncmp(iplImg->channelSeq, "BGRA", 4) == 0)
            cvCvtColor(iplImg, grayImg, CV_BGRA2GRAY);
        else
            cvCvtColor(iplImg, grayImg, CV_RGBA2GRAY);
    } else
        cvCopy(iplImg, grayImg);


    mObject->setBounds(parentObject->bounds()); // dialog창을 띄워놓고 ROI영역을 변경할수 있으므로..

    mObject->m_vecDetectResult.clear();
    //theMainWindow->InspectOneItem(grayImg, mObject);

    // 결과이미지를 화면에 반영.
    Mat mat = cvarrToMat(grayImg);
    mat_to_qimage(mat, img);

    if (pdocview->document()) {
            //pdocview->document()->setImageInternal(img);
            //pdocview->imageView()->updateBuffer();
            m_timer.start(200, this);
    }

    QApplication::restoreOverrideCursor();

}

void RoiPropertyEditor::clickedSaveTemplateSlot()
{

    QString str = ("Save");
    int ret = QMessageBox::question(this, str,
                                   ("Do you want to save Templelate image ?"),
                                   QMessageBox::Ok | QMessageBox::Cancel,
                                   QMessageBox::Ok);
    if (ret == QMessageBox::Ok)
    {
        ViewMainPage* pMainView = theMainWindow->viewMainPage();
        Qroilib::DocumentView* pdoc = pMainView->currentView();
        const QImage *img = pdoc->image();

        qimage_to_mat(img, frame);
        riplImg = frame;
        iplImg = &riplImg;


        mObject->setName(editRoiName->text());
        g_cRecipeData->SaveTemplelateImage(mObject, iplImg);

        QString name = mObject->name();
        QString strTemp;
        if (name.isEmpty())
            name = "noname";
        strTemp = QString("./%1.bmp").arg(name);
        QImage image = QImage(strTemp);
        image = image.scaled(64, 64, Qt::KeepAspectRatio);
        labelImageDisplay->setPixmap(QPixmap::fromImage(image));

        mObject->iplTemplate = cvLoadImage(strTemp.toStdString().c_str(), 0);
    }

}

void RoiPropertyEditor::slotComboActivated()
{
    QString str;
    int n = theClassCombo->currentIndex();

    InspectType inspType = (InspectType)0;
    switch(mObject->shape())
    {
        case Qroilib::RoiObject::Pattern:
            inspType = (InspectType)(_Inspect_Patt_Start+n+1);
            break;
        case Qroilib::RoiObject::Rectangle:
            inspType = (InspectType)(_Inspect_Roi_Start+n+1);
            break;

        case Qroilib::RoiObject::Point:
            inspType = (InspectType)(_Inspect_Point_Start+n+1);
            break;
    }

    if (mObject->mInspectType == inspType)
        return;

    mObject->m_vecParams.clear();
    mObject->mInspectType = inspType;

    if (mObject->iplTemplate){
        cvReleaseImage(&mObject->iplTemplate);
        mObject->iplTemplate = NULL;
    }

    for (int n = 0;; n++)
    {
        if (paramTable[n].nInspectType == _Inspect_Type_End)
            break;

        if (paramTable[n].nInspectType == mObject->mInspectType)
            mObject->m_vecParams.push_back(paramTable[n]);
    }

    ObjectListSet();
}

//
// Property UI로 ROI값을 설정한다.
//
void RoiPropertyEditor::ObjectListSet()
{
    QtProperty *topItem = nullptr;

    ViewMainPage* pMainView = theMainWindow->viewMainPage();
    DocumentView* pdocview = pMainView->currentView();

    variantEditor->clear();

    int size = mObject->m_vecParams.size();
    int nOldStep = -1;
    for (int i = 0; i < size; i++)
    {
        CParam *c = &mObject->m_vecParams[i].param;
        if (nOldStep != c->stepType) {

            if (topItem != nullptr)
                variantEditor->addProperty(topItem);

            topItem = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(),
                        StepTypeValue[c->stepType]);
            nOldStep = c->stepType;
        }

        if (c->valueType == _BoolValue) {
            QtVariantProperty *item = variantManager->addProperty(QVariant::Bool, c->Name);
            item->setValue(c->Value.toInt());
            topItem->addSubProperty(item);
        }
        else if (c->valueType == _IntValue) {
            item = variantManager->addProperty(QVariant::Int, c->Name);
            item->setValue(c->Value.toInt());
            topItem->addSubProperty(item);
        }
        else if (c->valueType == _DoubleValue) {
            item = variantManager->addProperty(QVariant::Double, c->Name);
            item->setValue(c->Value.toDouble());
            topItem->addSubProperty(item);
        }
        else if (c->valueType == _StringValue) {
            item = variantManager->addProperty(QVariant::String, c->Name);
            item->setValue(c->Name);
            topItem->addSubProperty(item);
        }
        else if (c->valueType == _ComboValue) { // ,로 구분된 선택 Table처리

            item = variantManager->addProperty(QtVariantPropertyManager::enumTypeId(),
                            c->Name);
            QStringList enumNames;

            if (c->Detail == ("${ROI}"))
            {
                enumNames << "No Parent/Criteria RoiMap";
                if (c->Value == (""))
                    c->Value = enumNames[0];

                for (Layer *layer : pdocview->mRoi->layers())
                {
                    const ObjectGroup &objectGroup = *static_cast<const ObjectGroup*>(layer);
                    for (const Qroilib::RoiObject *pdata : objectGroup) {
                        if (c->Value == pdata->name()) {
                            QString str;
                            str.sprintf(("%d"), i + 1);
                            c->Value = str;
                        }
                        enumNames << pdata->name();
                    }
                }
            }
            else {
                QStringList list = c->Detail.split(QRegExp(","), QString::KeepEmptyParts);
                for (QString token : list)
                {
                    enumNames << token;
                }
            }
            item->setAttribute(QLatin1String("enumNames"), enumNames);
            item->setValue(c->Value.toInt());
            topItem->addSubProperty(item);
        }
    }

    if (topItem != nullptr)
        variantEditor->addProperty(topItem);

    variantEditor->raise();
}

//
// Property UI에서 ROI값을 가져온다.
//
void RoiPropertyEditor::ObjectListGet()
{
    int nTypeSize = sizeof(StepTypeValue);

    QList<QtProperty*> l = variantEditor->properties();
    int size = l.size();
    for (int i = 0; i < size; i++)
    {
        QtProperty *p = l[i];
        QString groupName = p->propertyName();
        if (p->hasValue()) {
            int type = -1;
            for (int j = 0; j < nTypeSize; j++) {
                if (groupName == StepTypeValue[j]) {
                    type = j;
                    break;
                }
            }
            QString name = p->propertyName();
            QString val = p->valueText();
            UpdateParamVal(name, type, val);
        } else {
            QList<QtProperty*> l1 = p->subProperties();
            QString groupName = p->propertyName();
            for (int j = 0; j < l1.size(); j++)
            {
                QtProperty *p1 = l1[j];
                if (p1->hasValue()) {
                    int type = -1;
                    for (int j = 0; j < nTypeSize; j++) {
                        if (groupName == StepTypeValue[j]) {
                            type = j;
                            break;
                        }
                    }
                    QString name1 = p1->propertyName();
                    QString val1 = p1->valueText();
                    UpdateParamVal(name1, type, val1);
                }
            }
        }
    }
    mObject->setName(editRoiName->text());

}

void RoiPropertyEditor::UpdateParamVal(QString name, int type, QString val)
{
    int size = mObject->m_vecParams.size();
    for (int i = 0; i < size; i++)
    {
        CParam *c = &mObject->m_vecParams[i].param;
        if (c->stepType == type && c->Name == name)
        {
            if (c->valueType == _ComboValue) {
                QStringList list = c->Detail.split(QRegExp(","), QString::KeepEmptyParts);
                int seq = 0;
                for (QString token : list)
                {
                    if (token == val) {
                        mObject->m_vecParams[i].param.Value.sprintf("%d", seq);
                        break;
                    }
                    seq++;
                }
            }
            else
                mObject->m_vecParams[i].param.Value = val;
            break;
        }
    }
}
