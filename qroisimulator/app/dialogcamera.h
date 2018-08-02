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

private slots:
    void setExposureValue(int val);
    int getExposureValue();

    void on_pushButtonClose_clicked();
    void on_buttonOpenCamera_clicked();
    void on_buttonCloseCamera_clicked();

private:
    Ui::DialogCamera *ui;
    int m_nCamExposure;
    QString m_sCamera1;
    QStringListModel *model2;
};

#endif // DialogCamera_H
