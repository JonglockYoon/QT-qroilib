#ifndef DialogConfig_H
#define DialogConfig_H

#include <QDialog>
#include <QStringListModel>

namespace Ui {
class DialogConfig;
}

class DialogConfig : public QDialog
{
    Q_OBJECT

public:
    explicit DialogConfig(QWidget *parent = 0);
    ~DialogConfig();

    QStringListModel *model1;
    QStringListModel *model2;

    void SetData();

private slots:
    void enableAutoExposure(bool on);
    void setExposureValue(int val);
    void setIlluminationValue(int val);
    int getExposureValue(int seq);
    void on_pushButtonSave_clicked();

    void on_pushButtonClose_clicked();

    void on_radioButtonAlign1_clicked();
    void on_radioButtonAlign2_clicked();
    void on_radioButtonAlign3_clicked();

private:
    Ui::DialogConfig *ui;
};

#endif // DialogConfig_H
