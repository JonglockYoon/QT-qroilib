#ifndef EDGEDIALOG_H
#define EDGEDIALOG_H

#include <QDialog>
#include <QSlider>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QStringListModel>
#include "imgprocbase.h"

namespace Ui {
class DialogEdge;
}

class DialogEdge : public QDialog
{
    Q_OBJECT
public:
    DialogEdge(QWidget *parent);
    ~DialogEdge();

private slots:
    void setValue1(int val);
    void setEditValue1(const QString &);
    void setValue2(int val);
    void setEditValue2(const QString &);
    void changeRealtime(bool);
    void changeSmooth(bool);
    void setEditGaussian(const QString &);
    void activatedComboBoxSource(int act);
    void activatedComboBoxEdgeMethod(int);
    void changeWithSign(bool);

    void on_pushButtonClose_clicked();

    void setSobelEditValueX(const QString &val);
    void setSobelEditValueY(const QString &val);

    void seteditAptSizeValue(int val);

    void on_pushButtonExec_clicked();

    void updatePlayerUI(const QImage* img);
public:
    void ExecEdge(IplImage* iplImg);
protected:
    void closeEvent(QCloseEvent *event);
    void SobelNormalize(IplImage *sobel, IplImage *image, int x, int y);
private:
    Ui::DialogEdge *ui;

    bool bRealtime = false;
    int method;
    int nGaussian;
    QString mName;
    int source = 0;

    bool bSmooth;
    IplImage* tmp = nullptr;
    int val1 = 200;
    int val2 = 300;

    IplImage* sobel_tmp = nullptr;
    int sobelX = 1;
    int sobelY = 1;
    bool bWithSign = false;

    int aperture_size = 3;
};

#endif // EDGEDIALOG_H
