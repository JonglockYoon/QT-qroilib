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

    QStringListModel *model2;

    void SetData();

private slots:
    void enableAutoExposure(bool on);
    void enableAutoFocus(bool on);
    void setExposureValue(int val);
    void setFocusValue(int val);
    int getAutoExposureValue(int seq);
    int getAutoFocusValue(int seq);
    void on_pushButtonSave_clicked();
    void on_pushButtonClose_clicked();

    void on_pushButtonAutoExposure_clicked();

    void on_pushButtonAutoFocus_clicked();

private:
    Ui::DialogConfig *ui;
};

#endif // DialogConfig_H
