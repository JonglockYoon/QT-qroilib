//
// dialogconfig.cpp
//

#ifndef Q_OS_WIN
#include <cstdlib>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <fcntl.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h> 
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <sys/mman.h> 
#include <libv4l1.h>
#include <libv4l2.h>
#endif

#include <QMessageBox>
#include <QDebug>
#include <QCameraInfo>

#include "dialogconfig.h"
#include "ui_dialogconfig.h"
#include "qextserialenumerator.h"
#include "mainwindow.h"
#include "config.h"

DialogConfig::DialogConfig(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogConfig)
{
    ui->setupUi(this);

    model1 = new QStringListModel(this);
    model2 = new QStringListModel(this);

    SetData();

    this->setWindowTitle("Config");


}

DialogConfig::~DialogConfig()
{
    gCfg.ReadConfig();
    delete ui;
}

void DialogConfig::SetData()
{
    ///////////////////////// RS232c port setting //////////////////////////////////
    QStringList List1;
    List1 << "None";

#ifndef Q_OS_WIN // test
    {
        char tty0tty[64];
        for (int i=0; i<12; i++) {
            sprintf(tty0tty, "/dev/tnt%d", i);
            int fd = ::open(tty0tty,O_RDWR);
            if (fd != -1) {
                List1 << tty0tty;
                ::close(fd);
            }
        }
    }
#endif


    ///////////////////////// Camera setting //////////////////////////////////
    QStringList List2;
    List2 << "None";
    int i = 0;
    foreach (const QCameraInfo &cameraInfo, QCameraInfo::availableCameras()) {
        i++;
        QString str;
        str.sprintf("%02d:%s", i, cameraInfo.description().toLatin1().data());
        List2 << str;
    }
    int count = List2.count();
#ifndef Q_OS_WIN
    {
        char video[64];
        for (int i=0; i<12; i++) {
            sprintf(video, "/dev/video%d", i);
            int fd = ::v4l2_open(video,O_RDWR);
            if (fd != -1) {
                sprintf(video, "%02d:/dev/video%d", i+1, i);
                List2 << video;
                ::v4l2_close(fd);
            }
        }
    }
#endif
    model2->setStringList(List2);
    ui->comboBoxCam1Port->setModel(model2);

    ui->comboBoxCam1Port->setCurrentIndex(-1);

    qDebug() << gCfg.m_sCamera1;

    count = List2.count();
    for (int i=0; i<count; i++) {
        if (List2[i] == gCfg.m_sCamera1)
            ui->comboBoxCam1Port->setCurrentIndex(i);
    }


    QString str;
    str.sprintf("%.3f", gCfg.m_pCamInfo[0].dResX);
    ui->lineEditResX->setText(str);
    str.sprintf("%.3f", gCfg.m_pCamInfo[0].dResY);
    ui->lineEditResY->setText(str);

}

void DialogConfig::on_pushButtonSave_clicked()
{
    if (QMessageBox::Yes != QMessageBox::question(this, tr("   Save   "), tr("Save setting data?"), QMessageBox::Yes | QMessageBox::No))
        return;

    QApplication::setOverrideCursor(Qt::WaitCursor);

    ViewMainPage* pView = theMainWindow->viewMainPage();
    int ncam1 = ui->comboBoxCam1Port->currentIndex();


    QString str;
    str = ui->comboBoxCam1Port->currentText();
    if (str == "None" || str == "")
        gCfg.m_sCamera1 = "";
    else {
        gCfg.m_sCamera1 = str;
    }

    gCfg.m_pCamInfo[0].dResX = ui->lineEditResX->text().toDouble();
    gCfg.m_pCamInfo[0].dResY = ui->lineEditResY->text().toDouble();

    gCfg.WriteConfig();

    QApplication::restoreOverrideCursor();

    hide();
}



void DialogConfig::on_pushButtonClose_clicked()
{
    hide();
}

