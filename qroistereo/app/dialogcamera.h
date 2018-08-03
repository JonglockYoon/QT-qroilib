#ifndef DialogCamera_H
#define DialogCamera_H

#include <QDialog>
#include <QStringListModel>

namespace Ui {
class DialogCamera;
}

class DialogCamera : public QDialog
{
    Q_OBJECT

public:
    explicit DialogCamera(QWidget *parent = 0);
    ~DialogCamera();

    QStringListModel *model2;

    void SetData();

private slots:
    void enableAutoExposure(bool on);
    void setExposureValue(int val);
    int getExposureValue(int seq);
    void on_pushButtonOpen_clicked();
    void on_pushButtonClose_clicked();

private:
    Ui::DialogCamera *ui;
    int m_nCamExposure;
    QString m_sCamera1;
    QString m_sCamera2;
};

#endif // DialogCamera_H
