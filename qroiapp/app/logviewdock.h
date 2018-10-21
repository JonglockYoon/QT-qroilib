#pragma once

#include <QDockWidget>
#include <QStandardItemModel>
#include <QTableView>
#include <QMutexLocker>
#include <string>
#include <stdio.h>

using namespace std;

class LogViewDock : public QDockWidget
{
    Q_OBJECT
public:
    explicit LogViewDock(QString name, QWidget *parent = nullptr);

private:
    QStandardItemModel *modelLog;
    QTableView *tableView;
    QMutex m_sync;

public:
    virtual void resizeEvent(QResizeEvent *event);
    void clear();
signals:
    int Add(QString lpszTime, QString lpszLog);
private Q_SLOTS:
    int LogAdd(QString lpszTime, QString lpszLog);
};

