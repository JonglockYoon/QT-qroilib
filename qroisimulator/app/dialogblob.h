#ifndef DIALOGBLOB_H
#define DIALOGBLOB_H

#include <QDialog>
#include <QSlider>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QStringListModel>
#include "imgprocbase.h"
#include "voronoithinner.h"

namespace Ui {
class DialogBlob;
}

class DialogBlob : public QDialog
{
    Q_OBJECT
public:
    DialogBlob(QWidget *parent);
    ~DialogBlob();

private slots:
    void changeRealtime(bool);
    void changeThinner(bool);
    void activatedComboBoxSource(int act);
    void on_pushButtonClose_clicked();
    void on_pushButtonExec_clicked();
    void updatePlayerUI(const QImage* img);

    void setEditValue1(const QString &);
    void setEditValue2(const QString &);
    void setEditValue3(const QString &);
    void setEditValue4(const QString &);
    void setEditValuea1(const QString &);
    void setEditValuea2(const QString &);

    void setEditValuex1(const QString &);
    void setEditValuex2(const QString &);
    void setEditValuey1(const QString &);
    void setEditValuey2(const QString &);
protected:
    void closeEvent(QCloseEvent *event);
public:
    void ExecBlob(IplImage* iplImg);
    void FilterArea(IplImage* grayImg);
    void FilterDiameter(IplImage* grayImg);
    void Thinner(IplImage* grayImg);
    void DrawRotatedRect( IplImage * iplSrc,CvBox2D rect,CvScalar color, int thickness, int line_type, int shift );

public:
    int NoiseOut(IplImage* grayImg);
private:
    VoronoiThinner thinner;
    Ui::DialogBlob *ui;
    QString mName;
    int source = 0;
    bool bRealtime = false;
    IplImage* tmp = nullptr;

    int Noiseout1 = 0;
    int Noiseout2 = 0;
    int Noiseout3 = 0;
    int Noiseout4 = 0;
    double MinArea = 0;
    double MaxArea = 0;

    int MinX = 0;
    int MaxX = 0;
    int MinY = 0;
    int MaxY = 0;
    bool bThinner = false;
};

#endif // DIALOGBLOB_H
