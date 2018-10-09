/********************************************************************************
** Form generated from reading UI file 'dialogcamera.ui'
**
** Created by: Qt User Interface Compiler version 5.9.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOGCAMERA_H
#define UI_DIALOGCAMERA_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_DialogCamera
{
public:
    QPushButton *buttonOpenCamera;
    QGroupBox *groupBox_2;
    QPushButton *buttonCloseCamera;
    QLabel *label_3;
    QComboBox *comboBoxCam1Port;
    QSlider *sliderExposure;
    QPushButton *pushButtonClose;
    QLabel *label_5;

    void setupUi(QWidget *DialogCamera)
    {
        if (DialogCamera->objectName().isEmpty())
            DialogCamera->setObjectName(QStringLiteral("DialogCamera"));
        DialogCamera->resize(398, 235);
        DialogCamera->setMouseTracking(false);
        DialogCamera->setTabletTracking(true);
        buttonOpenCamera = new QPushButton(DialogCamera);
        buttonOpenCamera->setObjectName(QStringLiteral("buttonOpenCamera"));
        buttonOpenCamera->setGeometry(QRect(260, 180, 111, 41));
        QFont font;
        font.setFamily(QStringLiteral("Arial"));
        font.setPointSize(12);
        font.setBold(true);
        font.setWeight(75);
        buttonOpenCamera->setFont(font);
        groupBox_2 = new QGroupBox(DialogCamera);
        groupBox_2->setObjectName(QStringLiteral("groupBox_2"));
        groupBox_2->setGeometry(QRect(10, 10, 361, 131));
        QFont font1;
        font1.setFamily(QStringLiteral("Arial"));
        font1.setBold(false);
        font1.setWeight(50);
        groupBox_2->setFont(font1);
        buttonCloseCamera = new QPushButton(groupBox_2);
        buttonCloseCamera->setObjectName(QStringLiteral("buttonCloseCamera"));
        buttonCloseCamera->setGeometry(QRect(240, 80, 111, 41));
        buttonCloseCamera->setFont(font);
        label_3 = new QLabel(DialogCamera);
        label_3->setObjectName(QStringLiteral("label_3"));
        label_3->setGeometry(QRect(30, 40, 81, 21));
        label_3->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        comboBoxCam1Port = new QComboBox(DialogCamera);
        comboBoxCam1Port->setObjectName(QStringLiteral("comboBoxCam1Port"));
        comboBoxCam1Port->setGeometry(QRect(114, 40, 241, 22));
        sliderExposure = new QSlider(DialogCamera);
        sliderExposure->setObjectName(QStringLiteral("sliderExposure"));
        sliderExposure->setGeometry(QRect(30, 110, 181, 19));
        sliderExposure->setAutoFillBackground(true);
        sliderExposure->setStyleSheet(QStringLiteral("QSlider::groove:horizontal"));
        sliderExposure->setMinimum(-13);
        sliderExposure->setMaximum(-1);
        sliderExposure->setOrientation(Qt::Horizontal);
        pushButtonClose = new QPushButton(DialogCamera);
        pushButtonClose->setObjectName(QStringLiteral("pushButtonClose"));
        pushButtonClose->setGeometry(QRect(140, 180, 111, 41));
        pushButtonClose->setFont(font);
        label_5 = new QLabel(DialogCamera);
        label_5->setObjectName(QStringLiteral("label_5"));
        label_5->setGeometry(QRect(30, 90, 71, 21));
        label_5->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);

        retranslateUi(DialogCamera);

        QMetaObject::connectSlotsByName(DialogCamera);
    } // setupUi

    void retranslateUi(QWidget *DialogCamera)
    {
        DialogCamera->setWindowTitle(QApplication::translate("DialogCamera", "Config", Q_NULLPTR));
        buttonOpenCamera->setText(QApplication::translate("DialogCamera", "Open Camera", Q_NULLPTR));
        groupBox_2->setTitle(QApplication::translate("DialogCamera", "Camera Setting...", Q_NULLPTR));
        buttonCloseCamera->setText(QApplication::translate("DialogCamera", "Close Camera", Q_NULLPTR));
        label_3->setText(QApplication::translate("DialogCamera", "Channel #1", Q_NULLPTR));
        pushButtonClose->setText(QApplication::translate("DialogCamera", "Close", Q_NULLPTR));
        label_5->setText(QApplication::translate("DialogCamera", "exposure", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class DialogCamera: public Ui_DialogCamera {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOGCAMERA_H
