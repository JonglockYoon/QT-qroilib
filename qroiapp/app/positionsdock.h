/*
 * controlsdock.h
 */

#pragma once

#include <QDockWidget>
#include <QVariant>
#include <QTableView>
#include <QStandardItemModel>
#include <QMutexLocker>
#include <qroilib/documentview/documentview.h>

//using namespace Qroilib;
using namespace std;
class Qroilib::DocumentView;

class PositionsDock : public QDockWidget
{
    Q_OBJECT

public:
    explicit PositionsDock(QString name, QWidget *parent = nullptr);
    void initPositionlist();
    void updatePositionlist();
    int Add(int ch, QString no, QString align);
    int Update(int ch, int row, QString pos, QString align);
    int Clear(int ch);
    int setSelectedRowValue(int ch, double x, double y, double z);
    int GetSelectedRow(int ch, double &x, double &y, double &z);
    int GetNextRow(int ch, double &x, double &y, double &z);
    void clearSelected(int ch);

public Q_SLOTS:
    void initPositionlist(int ch);
    void updatePositionlist(int ch);

protected:
    //bool event(QEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event);

private:
    QStandardItemModel *modelScrew;
    QTableView *tableView;
    QStandardItemModel *modelScrew2;  // Right machine
    QTableView *tableView2;
    QMutex m_sync;

};

