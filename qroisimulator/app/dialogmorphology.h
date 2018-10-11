#ifndef DIALOGMORPHOLOGY_H
#define DIALOGMORPHOLOGY_H

#include <QDialog>
#include <QSlider>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QStringListModel>
#include "imgprocbase.h"

namespace Ui {
class DialogMorphology;
}

class DialogMorphology : public QDialog
{
    Q_OBJECT
public:
    DialogMorphology(QWidget *parent);
    ~DialogMorphology();

private slots:
    void setValuea1(int val);
    void setEditValuea1(const QString &);
    void setValuea2(int val);
    void setEditValuea2(const QString &);
    void changeRealtime(bool);
    void activatedComboBoxSource(int act);

    void on_pushButtonClose_clicked();
    void on_pushButtonExec_clicked();
    void updatePlayerUI(const QImage* img);

    void on_radioButtonErode_clicked();

    void on_radioButtonDilate_clicked();

    void on_radioButtonOpen_clicked();

    void on_radioButtonClose_clicked();

    void on_radioButtonTophat_clicked();

    void on_radioButtonBlackhat_clicked();

    void on_radioButtonGradient_clicked();

    void on_radioButtonMedian_clicked();

    void on_radioButtonRect_clicked();

    void on_radioButtonEllipse_clicked();

    void on_radioButtonCross_clicked();

protected:
    void closeEvent(QCloseEvent *event);
public:
    void ExecMorphology(IplImage* iplImg);
private:
    Ui::DialogMorphology *ui;

    bool bRealtime = false;
    int method;
    int shape;
    QString mName;
    int source = 0;

    IplImage* tmp = nullptr;
    int HalfSize = 1;
    int filterSize = 3;
};

#endif // DIALOGMORPHOLOGY_H
