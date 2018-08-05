/*
 * teachdock.h
 */

#pragma once

#include <QDockWidget>
#include <QVariant>
#include <QTableView>
#include <QStandardItemModel>
#include <qroilib/documentview/documentview.h>
#include <QStringListModel>

#include "common.h"

using namespace std;
class Qroilib::DocumentView;

namespace Ui
{
class TeachDock;
}


class TeachDock : public QDockWidget
{
    Q_OBJECT

public:
    explicit TeachDock(QString name, QWidget *parent = nullptr);

    QGraphicsTextItem* text_item;
    void UpdateAlignMode();
    void setFeducialVisionPos(double x, double y, double z);
    void setFeducialScrewPos(double x, double y, double z);
    void DisplayCurPointTextString();

public:
    bool m_bFeducialTeachSelect[2];

    QTimer *timer1;
    QTimer *timer2;
    QTimer *timer3;
    QTimer *timer4;

    bool WaitMoveComplete(int ch);
    void WaitMoveAndHoleFind(int ch);
    void WaitMoveAndHoleFindAndMove(int ch);
    Qroilib::RoiObject *FindScrewHole(int ch); // Screw Hole 찾기.
    void NextdHoleFindAndMove();

private slots:

    void on_pushButtonVisionFeducial_clicked();
    void on_pushButtonScrewFeducial_clicked();
    void on_pushButtonFeducialOffset_clicked();
    void on_pushButtonFeducialOffsetSave_clicked();
    void on_pushButtonMove_clicked();
    void on_pushButtonMoveNext_clicked();
    void on_pushButtonMoveScrew_clicked();
    void on_pushButtonHoleFind_clicked();
    void on_pushButtonAlign_clicked();
    void on_pushButtonReloadScrewPos_clicked();
    void on_pushButtonAutoTeach_clicked();
    void on_pushButtonLoadCSV_clicked();
    void on_pushButtonSaveScrewPos_clicked();
    void on_pushButtonVisionFeducialMove_clicked();
    void on_pushButtonScrewFeducialMove_clicked();

    void on_pushButtonAF_clicked();

private:
    Ui::TeachDock *ui;
    QStringListModel *model1;
    QPushButton *m_btnTeach[2];
    int btnTeachCount;
    int m_nOrgAlignMode;

public:
    void pushButtonClear();

protected:
    bool event(QEvent *event) override;
    void pushButtonClick(int n);

signals:
    void alignValue(int nScrew, int nPoint, double x, double y);
};

