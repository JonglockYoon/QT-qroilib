#ifndef DIALOGMODEL_H
#define DIALOGMODEL_H

#include <QDialog>
#include <QStringListModel>
#include <QDir>
#include <QListWidgetItem>

namespace Ui {
class DialogModel;
}

class DialogModel : public QDialog
{
    Q_OBJECT

public:
    explicit DialogModel(QWidget *parent = 0);
    ~DialogModel();

    void SetData();
    QStringListModel* model;
    QDir m_dir;
    QString m_sCurrentPath;

private slots:
    void on_pushButtonAdd_clicked();

    void on_pushButtonExit_clicked();

    void on_pushButtonDelete_clicked();

    void on_pushButtonChange_clicked();

    void handleSelectionChanged(const QItemSelection& selection);

private:
    Ui::DialogModel *ui;
};

#endif // DIALOGMODEL_H
