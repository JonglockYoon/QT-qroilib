/*
 * teachdock.cpp
 */
#include <QLabel>
#include <QApplication>
#include <QHeaderView>
#include <QDebug>
#include <QStandardItemModel>
#include <QFileDialog>
#include <QFile>
#include <QTextCodec>

#include "teachdock.h"
#include "ui_teachdock.h"
#include "roiobject.h"
#include "utils.h"
#include <qroilib/documentview/documentview.h>
#include "mainwindow.h"
#include "imgprocengine.h"

using namespace Qroilib;
using namespace std;

TeachDock::TeachDock(QString name, QWidget *parent)
    : QDockWidget(name, parent)
     ,ui(new Ui::TeachDock)
{
    theMainWindow->bWorking[0] = false;
    theMainWindow->bWorking[1] = false;
    ui->setupUi(this);

    setMinimumWidth(100);
    setMaximumWidth(400);
    setMinimumHeight(100);
    setMaximumHeight(340);

    text_item = nullptr;

    model1 = new QStringListModel(this);
    QStringList List1;
    List1 << "1 Point";
    List1 << "2 Point";
    List1 << "all Point";
    model1->setStringList(List1);
    ui->comboBoxAlignMode->setModel(model1);
    UpdateAlignMode();

    ui->pushButtonReloadScrewPos->setStyleSheet("background: seagreen;");
    ui->pushButtonSaveScrewPos->setStyleSheet("background: mediumorchid;");
    ui->pushButtonAutoTeach->setStyleSheet("background: lightblue;");

    int i = 0;
    m_btnTeach[i++] = ui->pushButtonVisionFeducial;
    m_btnTeach[i++] = ui->pushButtonScrewFeducial;
    btnTeachCount = i;
    for (int i=0; i<btnTeachCount; i++) {
        m_btnTeach[i]->setStyleSheet("background: LightGray;");
        m_btnTeach[i]->setCheckable(true);
        m_bFeducialTeachSelect[i] = false;
    }

    QString str;
    str.sprintf("%5.2f,%5.2f,%4.1f", gCfg.feducial_vision_x, gCfg.feducial_vision_y, gCfg.feducial_vision_z);
    ui->lineEditVisionFeducialPos->setText(str);
    str.sprintf("%5.2f,%5.2f,%4.1f", gCfg.feducial_screw_x, gCfg.feducial_screw_y, gCfg.feducial_screw_z);
    ui->lineEditScrewFeducialPos->setText(str);
    str.sprintf("%5.2f,%5.2f", gCfg.feducial_offset_x, gCfg.feducial_offset_y);
    ui->lineEditFeducialOffset->setText(str);

}

void TeachDock::UpdateAlignMode()
{
    ui->comboBoxAlignMode->setCurrentIndex(gCfg.m_nAlignMode-1);
}

bool TeachDock::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::KeyPress:
    case QEvent::ShortcutOverride: {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->matches(QKeySequence::Delete) || keyEvent->key() == Qt::Key_Backspace) {
            event->accept();
            return true;
        }
        break;
    }
    case QEvent::LanguageChange:
        //retranslateUi();
        break;
    default:
        break;
    }

    return QDockWidget::event(event);
}

// 티칭버턴 check clear
void TeachDock::pushButtonClear()
{
    for (int i=0; i<btnTeachCount; i++) {
        m_btnTeach[i]->setStyleSheet("background: LightGray;");
        m_btnTeach[i]->setCheckable(true);
        m_bFeducialTeachSelect[i] = false;
    }
}


void TeachDock::setFeducialVisionPos(double x, double y, double z)
{
    QString str;
    str.sprintf("%5.2f,%5.2f,%4.1f", x,y,x);
    ui->lineEditVisionFeducialPos->setText(str);

    str = ui->lineEditScrewFeducialPos->text();
    QStringList list = str.split(QRegExp(","), QString::KeepEmptyParts);
    if (list.count() >= 2)
    {
        double ox = x - list[0].toDouble();
        double oy = y - list[1].toDouble();
        str.sprintf("%5.2f,%5.2f", ox,oy);
        ui->lineEditFeducialOffset->setText(str);
    }
}

void TeachDock::setFeducialScrewPos(double x, double y, double z)
{
    QString str;
    str.sprintf("%5.2f,%5.2f,%4.1f", x,y,z);
    ui->lineEditScrewFeducialPos->setText(str);

    str = ui->lineEditVisionFeducialPos->text();
    QStringList list = str.split(QRegExp(","), QString::KeepEmptyParts);
    if (list.count() >= 2)
    {
        double ox = list[0].toDouble() - x;
        double oy = list[1].toDouble() - y;
        str.sprintf("%5.2f,%5.2f", ox,oy);
        ui->lineEditFeducialOffset->setText(str);
    }
}

// 티칭 좌표 버턴 표시 및 Target 버턴에 티칭 좌표 표시
void TeachDock::pushButtonClick(int n)
{
    if (theMainWindow->m_iActiveView < 0)
        return;
    pushButtonClear();
    m_btnTeach[n]->setStyleSheet("background: DarkGreen;");
    m_bFeducialTeachSelect[n] = true;
    theMainWindow->mPositionsDock->clearSelected(n);
}
void TeachDock::on_pushButtonVisionFeducial_clicked() { pushButtonClick(0); }
void TeachDock::on_pushButtonScrewFeducial_clicked() {  pushButtonClick(1); }
void TeachDock::on_pushButtonFeducialOffset_clicked() {  pushButtonClear(); }


void TeachDock::on_pushButtonVisionFeducialMove_clicked()
{
    int ch = theMainWindow->m_iActiveView;
    if (ch < 0) { // 두개의 비젼이 동시에 보일때는 버턴 무시.
        theMainWindow->DevLogSave("Fail Move. not teaching mode");
        return;
    }
    if (theMainWindow->bWorking[ch])
    {
        theMainWindow->DevLogSave("Do not move. working...");
        return;
    }

    QString str;
    double x, y, z;
    str = ui->lineEditVisionFeducialPos->text();
    QStringList list = str.split(QRegExp(","), QString::KeepEmptyParts);
    if (list.count() >= 3)
    {
        x = list[0].toDouble();
        y = list[1].toDouble();
        z = list[2].toDouble();

        if (ch == 0)
            str = "left";
        else
            str = "right";
        theMainWindow->itfport.Move(str, x, y, z);
    }
}

void TeachDock::on_pushButtonScrewFeducialMove_clicked()
{
    int ch = theMainWindow->m_iActiveView;
    if (ch < 0) { // 두개의 비젼이 동시에 보일때는 버턴 무시.
        theMainWindow->DevLogSave("Fail Move. not teaching mode");
        return;
    }
    if (theMainWindow->bWorking[ch])
    {
        theMainWindow->DevLogSave("Do not move. working...");
        return;
    }

    QString str;
    double x, y, z;
    str = ui->lineEditScrewFeducialPos->text();
    QStringList list = str.split(QRegExp(","), QString::KeepEmptyParts);
    if (list.count() >= 3)
    {
        x = list[0].toDouble();
        y = list[1].toDouble();
        z = list[2].toDouble();

        if (ch == 0)
            str = "left";
        else
            str = "right";
        theMainWindow->itfport.Move(str, x, y, z);
    }
}

void TeachDock::on_pushButtonFeducialOffsetSave_clicked()
{
    if (theMainWindow->m_iActiveView < 0)
        return;

    QString str;
    str = ui->lineEditVisionFeducialPos->text();
    QStringList list = str.split(QRegExp(","), QString::KeepEmptyParts);
    if (list.count() >= 3)
    {
        gCfg.feducial_vision_x = list[0].toDouble();
        gCfg.feducial_vision_y = list[1].toDouble();
        gCfg.feducial_vision_z = list[2].toDouble();
    }
    str = ui->lineEditScrewFeducialPos->text();
    list = str.split(QRegExp(","), QString::KeepEmptyParts);
    if (list.count() >= 3)
    {
        gCfg.feducial_screw_x = list[0].toDouble();
        gCfg.feducial_screw_y = list[1].toDouble();
        gCfg.feducial_screw_z = list[2].toDouble();
    }
    str = ui->lineEditFeducialOffset->text();
    list = str.split(QRegExp(","), QString::KeepEmptyParts);
    if (list.count() >= 3)
    {
        gCfg.feducial_offset_x = list[0].toDouble();
        gCfg.feducial_offset_y = list[1].toDouble();
    }
    gCfg.WriteConfig();
}




void TeachDock::on_pushButtonMove_clicked() // 비젼을 체결위치로 이동시킨다.
{
    if (text_item)
        delete text_item;
    text_item = nullptr;

#if 0
    static double x = 0.0;
    static double y = 0.0;
    //emit alignValue(0, 0, x, y);
    theMainWindow->AlignLogSave(0, 1, x, y); // graph test
    x += 0.1;
    y += 0.1;
    if (x > 1.0) x = 0;
    if (y > 1.0) y = 0;
#endif
    int ch = theMainWindow->m_iActiveView;
    if (ch < 0) // 두개의 비젼이 동시에 보일때는 버턴 무시.
        return;
    if (theMainWindow->bWorking[ch])
    {
        theMainWindow->DevLogSave("Do not move. working...");
        return;
    }

    QString str;
    if (ch == 0)
        str = "left";
    else
        str = "right";

    theMainWindow->itfport.bMoveComplete[ch] = false;
    double x, y, z;
    int n = theMainWindow->mPositionsDock->GetSelectedRow(ch, x, y, z); // 체결티칭위치.
    if (n >= 0) {
        theMainWindow->m_pTrsAlign[ch]->S2V(x, y); // 비젼위치로 변환.
        z += gCfg.vision_offset_z; // 체결위치에서 비젼 Z Offset을 뺀 높이로 이동.

        theMainWindow->itfport.Move(str, x, y, z);

        if (ui->checkBoxFindAndMove->isChecked())
            WaitMoveAndHoleFindAndMove(ch);
        else
            WaitMoveAndHoleFind(ch);
    }
}

void TeachDock::on_pushButtonMoveNext_clicked() // 비젼을 다음 체결위치로 이동시킨다.
{
    if (text_item)
        delete text_item;
    text_item = nullptr;

#if 0
    static double x = 0.0;
    static double y = 0.0;
    //emit alignValue(1, 0, x, y);
    theMainWindow->AlignLogSave(1, 1, x, y); // graph test
    x += 0.1;
    y += 0.1;
    if (x > 1.0) x = 0;
    if (y > 1.0) y = 0;
#endif

    int ch = theMainWindow->m_iActiveView;
    if (ch < 0) // 두개의 비젼이 동시에 보일때는 버턴 무시.
        return;
    if (theMainWindow->bWorking[ch])
    {
        theMainWindow->DevLogSave("Do not move. working...");
        return;
    }

    QString str;
    if (ch == 0)
        str = "left";
    else
        str = "right";

    theMainWindow->itfport.bMoveComplete[ch] = false;
    double x, y, z;
    int n = theMainWindow->mPositionsDock->GetNextRow(ch, x, y, z);
    if (n >= 0) {
        theMainWindow->m_pTrsAlign[ch]->S2V(x, y); // 비젼위치로 변환.
        z += gCfg.vision_offset_z; // 체결위치에서 비젼 Z Offset을 뺀 높이로 이동.

        theMainWindow->itfport.Move(str, x, y, z);
        if (ui->checkBoxFindAndMove->isChecked())
            WaitMoveAndHoleFindAndMove(ch);
        else
            WaitMoveAndHoleFind(ch);
    }
}

bool TeachDock::WaitMoveComplete(int ch)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    MTickTimer tick;
    while (true)
    {
        double d = tick.GetElaspTime();
        if (d > 2.0)
        {
            return false;
        }
        if (theMainWindow->itfport.bMoveComplete[ch])
        {
            QApplication::restoreOverrideCursor();
            return true;
        }
    }

    QApplication::restoreOverrideCursor();

}
//
// 이동명령을 내리고 이 함수를 호출하면
// 이동이 완료되었다는 신호를 받고난후 Hole을 찾는다.
//
void TeachDock::WaitMoveAndHoleFind(int ch)
{
    theMainWindow->SetCameraPause(ch, true);

    QApplication::setOverrideCursor(Qt::WaitCursor);

    theMainWindow->bWorking[ch] = true;
    static int count = 0;
    static int _ch = 0;
    count = 0;
    _ch = ch;
    timer1 = new QTimer(this);
    timer1->setSingleShot(false);
    connect(timer1, &QTimer::timeout, [&]() {
        count++;
        if (count > 20) // 2초.
        {
            theMainWindow->SetCameraPause(_ch, false);
            theMainWindow->bWorking[_ch] = false;
            timer1->deleteLater();
        }
        if (theMainWindow->itfport.bMoveComplete)
        {
            timer1->deleteLater();
            theMainWindow->bWorking[_ch] = false;
            FindScrewHole(_ch); // if find hole return true.



            if (ui->checkBoxAutoFocus->isChecked()) {

            }

        }
    } );
    timer1->start(100);

    QApplication::restoreOverrideCursor();

}

void TeachDock::WaitMoveAndHoleFindAndMove(int ch)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    theMainWindow->bWorking[ch] = true;
    static int count = 0;
    static int _ch = 0;
    count = 0;
    _ch = ch;
    timer2 = new QTimer(this);
    timer2->setSingleShot(false);
    connect(timer2, &QTimer::timeout, [&]() {
        count++;
        if (count > 20) // 2초.
        {
            theMainWindow->bWorking[_ch] = false;
            timer2->deleteLater();
        }
        if (theMainWindow->itfport.bMoveComplete)
        {
            theMainWindow->SetCameraPause(_ch, true);
            timer2->deleteLater();
            theMainWindow->bWorking[_ch] = false;
            NextdHoleFindAndMove(); // 이동한위치에서 ScrewHole을 찾고 찾은위치로 재이동한다.
        }
    } );
    timer2->start(100);

    QApplication::restoreOverrideCursor();

}

// ScrewHole을 찾고 찾은위치로 이동한다.
void TeachDock::NextdHoleFindAndMove()
{
    int ch = theMainWindow->m_iActiveView;
    if (ch < 0) { // 두개의 비젼이 동시에 보일때는 버턴 무시.
        theMainWindow->DevLogSave("Fail Apply. not teaching mode");
        return;
    }
    if (theMainWindow->bWorking[ch])
    {
        theMainWindow->DevLogSave("Do not move. working...");
        return;
    }

    QString lrstr, str;
    if (ch == 0)
        lrstr = "left";
    else
        lrstr = "right";

    double x = theMainWindow->itfport.dScrewCurX;
    double y = theMainWindow->itfport.dScrewCurY;
    double z = theMainWindow->itfport.dScrewCurZ;

    Qroilib::RoiObject *pData = FindScrewHole(ch);
    if (pData == nullptr)
        return;

    DocumentView* v = theMainWindow->currentView();
    if (v == nullptr)
        return;
    const QImage *camimg = v->image();
    QSize imgsize = camimg->size();

    // hole중심으로부터 거리 산정.

    double dX, dY;
    int size = pData->m_vecDetectResult.size();
    for (int i = 0; i < size; i++)
    {
        DetectResult *prst = &pData->m_vecDetectResult[i];
        if (prst->pt.x + prst->pt.y > 0)
        {
            QRectF rect = pData->bounds();	// Area로 등록된 ROI
            rect.normalized();
            double x = prst->pt.x + rect.x();// / gCfg.m_pCamInfo[0].dResX;
            double y = prst->pt.y + rect.y();// / gCfg.m_pCamInfo[0].dResY;

            str.sprintf(("Find Screw Hold (Pixel) : %.2f %.2f"), x, y);
            qDebug() << (str);

            double dCenterX = imgsize.width() / 2.0;
            double dCenterY = imgsize.height() / 2.0;

            double dTmpX = dCenterX - x;
            double dTmpY = dCenterY - y;
            str.sprintf(("Screw Hold Offset(Pixel) : %.2f %.2f"), dTmpX, dTmpY);
            qDebug() << (str);

            dX = (dTmpX * gCfg.m_pCamInfo[0].dResX); // 로봇좌표 변환
            dY = -(dTmpY * gCfg.m_pCamInfo[0].dResY);
            str.sprintf((" ===> Offset:%.2f mm %.2f mm"), dX, dY);
            qDebug() << (str);
            theMainWindow->DevLogSave(str.toLatin1().data());

            x += dX;
            y += dY;
            break;
        }
    }

    theMainWindow->SetCameraPause(ch, false);
    theMainWindow->itfport.Move(lrstr, x, y, z);

    WaitMoveAndHoleFind(ch);
}

void TeachDock::on_pushButtonMoveScrew_clicked() // 슬리브를 체결위치로 이동시킨다.
{
    if (text_item)
        delete text_item;
    text_item = nullptr;

    int ch = theMainWindow->m_iActiveView;
    if (ch < 0) // 두개의 비젼이 동시에 보일때는 버턴 무시.
        return;
    if (theMainWindow->bWorking[ch])
    {
        theMainWindow->DevLogSave("Do not move. working...");
        return;
    }

    QString str;
    if (ch == 0)
        str = "left";
    else
        str = "right";

    theMainWindow->itfport.bMoveComplete[ch] = false;
    double x, y, z;
    int n = theMainWindow->mPositionsDock->GetSelectedRow(ch, x, y, z); // 체결티칭위치.
    if (n >= 0) {
        theMainWindow->itfport.Move(str, x, y, 0.0);
    }
}

Qroilib::RoiObject *TeachDock::FindScrewHole(int ch) // Screw Hole 찾기.
{
//    ViewMainPage* pView = theMainWindow->viewMainPage();
//    DocumentView* v = pView->currentView();
//    if (v == nullptr) {
//        theMainWindow->DevLogSave("Find fail. not teaching mode");
//        return nullptr;
//    }

    Qroilib::RoiObject *pData = theMainWindow->FindScrewHole(ch);
    if (pData != nullptr)
    {
        int size = pData->m_vecDetectResult.size();
        if (size > 0) {
            //DetectResult Result = pData->m_vecDetectResult[0];

            return pData;
        }
    } else {
        qDebug() << "Screw Hole find fail";
    }
    return nullptr;
}

void TeachDock::on_pushButtonHoleFind_clicked() // Screw Hole 찾기.
{
    int ch = theMainWindow->m_iActiveView;
    if (ch < 0) { // 두개의 비젼이 동시에 보일때는 버턴 무시.
        theMainWindow->DevLogSave("Find fail. not teaching mode");
        return;
    }
    FindScrewHole(ch);

}

void TeachDock::DisplayCurPointTextString()
{
    int ch = theMainWindow->m_iActiveView;
    if (ch < 0) // 두개의 비젼이 동시에 보일때는 버턴 무시.
        return;

    QString str;
    if (ch == 0)
        str = "left";
    else
        str = "right";

    double x, y, z;
    int n = theMainWindow->mPositionsDock->GetSelectedRow(ch, x, y, z);
    if (n < 0)
        return;

    QString text = QString("Point %1").arg(n+1);

    QFont font;
    font.setPixelSize(20);
    font.setBold(true);
    font.setFamily("Verdana");

    if (text_item)
        delete text_item;

    ViewMainPage* pView = theMainWindow->viewMainPage();
    DocumentView* v = pView->currentView();
    if (v == nullptr) {
        return;
    }
    text_item = v->mRoiScene->addText(text, font);
    QColor text_color = QColor();
    text_color.setNamedColor("#FFFF00");
    text_color.setAlphaF(0.6);
    text_item->setDefaultTextColor(text_color);

    double text_x = 5;
    double text_y = 5;
    text_item->setPos(text_x, text_y < 0 ? 0 : text_y);
}

void TeachDock::on_pushButtonAutoTeach_clicked() // 모든 Screw Hole 자동 티칭 실행.
{
    if (theMainWindow->m_iActiveView < 0)
        return;
    int ch = theMainWindow->m_iActiveView;
    if (theMainWindow->bWorking[ch])
    {
        theMainWindow->DevLogSave("Do not move. working...");
        return;
    }

    // gCfg.m_nAlignMode = 3과 동일한 동작며,
    // 단. align결과를 티칭 table에 반영하는것이 다르다.

    theMainWindow->bWorking[ch] = true;
    m_nOrgAlignMode = gCfg.m_nAlignMode;
    gCfg.m_nAlignMode = 3; // All align

    theMainWindow->m_pTrsAlign[ch]->point = -1;
    theMainWindow->m_pTrsAlign[ch]->bDoAlign = true;
    theMainWindow->m_pTrsAlign[ch]->bAllTeachingMode = true;
    static int _ch = 0;
    _ch = ch;
    timer4 = new QTimer(this);
    timer4->setSingleShot(false);
    connect(timer4, &QTimer::timeout, [&]() {
        if (theMainWindow->m_pTrsAlign[_ch]->bDoAlign == false) // 모든 point 티칭완료
        {
            // align결과를 티칭 table에 반영.
//            vector<SCREWITEM> *pPoints = &theMainWindow->m_pInfoTeachManager->m_vecScrewTable[_ch];
//            for (int i = 0; i < pPoints->size(); i++) { // Align 좌표 수정.
//                SCREWITEM *pitem = &(*pPoints)[i];
//                pitem->x = pitem->x + pitem->dx;
//                pitem->y = pitem->y + pitem->dy;
//                pitem->dx = 0;
//                pitem->dy = 0;
//            }

            gCfg.m_nAlignMode = m_nOrgAlignMode;
            theMainWindow->bWorking[_ch] = false;
            timer4->deleteLater();
        }
    } );
    timer4->start(100);
}

void TeachDock::on_pushButtonAlign_clicked() // Align 실행
{
    if (theMainWindow->m_iActiveView < 0)
        return;
    int ch = theMainWindow->m_iActiveView;
    if (theMainWindow->bWorking[ch])
    {
        theMainWindow->DevLogSave("Do not move. working...");
        return;
    }
    int idx = ui->comboBoxAlignMode->currentIndex(); // 0 ~ 2

    theMainWindow->bWorking[ch] = true;
    m_nOrgAlignMode = gCfg.m_nAlignMode;
    gCfg.m_nAlignMode = idx + 1;

    theMainWindow->m_pTrsAlign[ch]->point = -1;
    theMainWindow->m_pTrsAlign[ch]->bDoAlign = true;
    theMainWindow->m_pTrsAlign[ch]->bAllTeachingMode = false;
    if (gCfg.m_nAlignMode == 3)
        theMainWindow->m_pTrsAlign[ch]->bAllTeachingMode = true;
    static int _ch = 0;
    _ch = ch;
    timer3 = new QTimer(this);
    timer3->setSingleShot(false);
    connect(timer3, &QTimer::timeout, [&]() {
        if (theMainWindow->m_pTrsAlign[_ch]->bDoAlign == false) // align완료
        {
            theMainWindow->DevLogSave("align complete!!!");

            gCfg.m_nAlignMode = m_nOrgAlignMode;
            theMainWindow->bWorking[_ch] = false;
            timer3->deleteLater();
        }
    } );
    timer3->start(1000);
}

//
// 저장되어 있는 티칭Data가 먼저 보여지고 체결기에서 응답이오면 다시 Update된다.
//
void TeachDock::on_pushButtonReloadScrewPos_clicked() // 저장되어 있는 티칭Data Load 및 체결기로 티칭Data 요구.
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

//    int ch = theMainWindow->m_iActiveView;
//    if (ch < 0) // 두개의 비젼이 동시에 보일때는 버턴 무시.
//        return;
//    QString str;
//    if (ch == 0)
//        str = "left";
//    else
//        str = "right";
    theMainWindow->m_pInfoTeachManager->ReadData();
    theMainWindow->mPositionsDock->initPositionlist();

    theMainWindow->itfport.ReadScrewPos("left");
    theMainWindow->itfport.ReadScrewPos("right");

    QApplication::restoreOverrideCursor();
}

void TeachDock::on_pushButtonSaveScrewPos_clicked() // 현재 보여지는 티칭 Data 저장 및 체결기로 전송.
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    theMainWindow->m_pInfoTeachManager->WriteData();

    // send teaching data to screw machine.
    theMainWindow->itfport.WriteScrewPos("left");
    theMainWindow->itfport.WriteScrewPos("right");

    QApplication::restoreOverrideCursor();

}

// CSV파일을 로드할때는 Screw 첫위치에 robot이 위치해야한다.
// 현재위치(x,y,z) 와 CSV 첫위치 차이를 가지고 CSV위치를 robot티칭위치로 가져오게된다.
// CSV파일을 읽어서 mPositionsDock의 티칭테이블로 Display한다.
void TeachDock::on_pushButtonLoadCSV_clicked()
{
    theMainWindow->kbdNum.DeleteInstance();
    theMainWindow->kbdAlpha.DeleteInstance();

//    ViewMainPage* pView = theMainWindow->viewMainPage();
//    if (!pView) {
//        theMainWindow->DevLogSave("Load fail. not teaching mode");
//        return;
//    }
    DocumentView* pdocview = theMainWindow->currentView();
    if (!pdocview) {
        theMainWindow->DevLogSave("Load fail. not teaching mode");
        return;
    }
    int ch = theMainWindow->m_iActiveView;
    if (ch < 0) { // 두개의 비젼이 동시에 보일때는 버턴 무시.
        theMainWindow->DevLogSave("Load fail. not teaching mode");
        return;
    }
    QString str;
    if (ch == 0)
        str = "left";
    else
        str = "right";
    theMainWindow->itfport.LoadCSV(str);

    QString fileName = QFileDialog::getOpenFileName(this,
            tr("Open CSV file"), QDir::currentPath(),
            tr("CSV Files (*.csv);;All Files (*)"));
    if (fileName.isEmpty()) {
        return;
    }

    MInfoTeachManager*	pInfo = theMainWindow->m_pInfoTeachManager;
    pInfo->m_vecScrewTable[ch].clear();
    //QString seqs = QString::fromUtf8("ⓐⓑⓒⓓⓔⓕⓖⓗⓘⓙⓚⓛⓜⓝⓞⓟⓠⓡⓢⓣⓤⓥⓦⓧⓨⓩ①②③④⑤⑥⑦⑧⑨⑩⑪⑫⑬⑭⑮");
    std::wstring kstr = L"\u24D0\u24D1\u24D2\u24D3\u24D4\u24D5\u24D6\u24D7\u24D8\u24D9\u24DA\u24DB\u24DC\u24DD\u24DE\u24DF\u24E0\u24E1\u24E2\u24E3\u24E4\u24E5\u24E6\u24E7\u24E8\u24E9\u2460\u2461\u2462\u2463\u2464\u2465\u2466\u2467\u2468\u2469\u246A\u246B\u246C\u246D\u246E";
    QString seqs = QString::fromStdWString(kstr);

    int maxidx = 0;
    QFile inputFile(fileName);
    if (inputFile.open(QIODevice::ReadOnly))
    {
        pInfo->m_vecScrewTable[ch].resize(MAX_SCREW);
        int linecount = 0;
        QTextStream in(&inputFile);
        while (!in.atEnd())
        {
            linecount++;
            QString line = in.readLine();
            int pos = line.indexOf(',', 0);
            bool b = (line.mid(pos,6) == ",,,,,,");
            QStringList list = line.split(QRegExp(","), QString::KeepEmptyParts);
            if (list.count() >= 11 && linecount >= 8) {
                QString seq = list[7];
                bool a = seqs.contains(seq);
                if (seq.length()>0 && (a||b)) {
                    int idx = list[11].toInt() - 1;
                    if (idx >= 0 && idx < MAX_SCREW)
                    {
                        double val = list[8].toDouble();

                        SCREWITEM *pitem = &pInfo->m_vecScrewTable[ch][idx];
                        pitem->point = idx;
                        if (pitem->x == 0.0)
                            pitem->x = val;
                        else if (pitem->y == 0.0)
                            pitem->y = val;

                        if (maxidx < idx)
                            maxidx = idx;
                    }
                }
            }
        }
        pInfo->m_vecScrewTable[ch].resize(maxidx+1);
    }

    double x = theMainWindow->itfport.dScrewCurX;
    double y = theMainWindow->itfport.dScrewCurY;
    double z = theMainWindow->itfport.dScrewCurZ;

    double dX = 0;
    double dY = 0;
    if (maxidx > 0) {
        dX = x - pInfo->m_vecScrewTable[ch][0].x;
        dY = y - pInfo->m_vecScrewTable[ch][0].y;
    }

    theMainWindow->mPositionsDock->Clear(ch);
    for (int i=0; i<=maxidx; i++)
    {
        SCREWITEM *pitem = &pInfo->m_vecScrewTable[ch][i];
        double x1 = pitem->x + dX;
        double y1 = pitem->y + dY;
        pitem->x = x1;
        pitem->y = y1;
        pitem->z = z;
        pitem->dx = 0;
        pitem->dy = 0;
        QString str = QString("%1,%2,%3").arg(x1).arg(y1).arg(z);
        QString str1 = QString("%1,%2").arg(0).arg(0);
        theMainWindow->mPositionsDock->Add(ch, str, str1);
    }
}

void TeachDock::on_pushButtonAF_clicked()
{
    if (theMainWindow->m_iActiveView < 0)
        return;
    //int ch = theMainWindow->m_iActiveView;

    cv::Mat frame;

    DocumentView* v = theMainWindow->currentView();
    if (v == nullptr)
        return;

    const QImage *camimg = v->image();
    qimage_to_mat(camimg, frame);

    double d = theMainWindow->pImgProcEngine->rateFrame(frame);
    QString str;
    str.sprintf("rateFrame : %.3f", d);
    theMainWindow->DevLogSave(str.toLatin1().data());
}
