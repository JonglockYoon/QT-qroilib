#include <QApplication>
#include <QHeaderView>
#include <QDesktopWidget>
#include <QVBoxLayout>
#include "logviewdock.h"
#include "mainwindow.h"
#include "mlogthread.h"
#include "config.h"
#include "common.h"

//using namespace Qroilib;


extern MainWindow* theMainWindow;

LogViewDock::LogViewDock(QString name, QWidget *parent)
    : QDockWidget(name, parent)
{
    setMinimumWidth(100);
    setMaximumWidth(640);
    setMinimumHeight(50);
    setMaximumHeight(480);

    modelLog = new QStandardItemModel(0,2,this);
    modelLog->setHorizontalHeaderItem(0, new QStandardItem(QString("time")));
    modelLog->setHorizontalHeaderItem(1, new QStandardItem(QString("content")));
    tableView = new QTableView(this);
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    clear();

    tableView->setModel(modelLog);
    tableView->horizontalHeader()->setStretchLastSection(true);

    //int w = this->width();
    tableView->setColumnWidth(0, 60);
    tableView->setColumnWidth(1, 80);

    //this->setWindowTitle("Log");

    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(tableView);
    widget->setLayout(layout);

    setWidget(widget);

    QObject::connect(this, SIGNAL(Add(QString,QString)), this, SLOT(LogAdd(QString,QString)));
}

void LogViewDock::resizeEvent(QResizeEvent *event) {
    int w = this->width();
    tableView->setColumnWidth(0, 60);
    tableView->setColumnWidth(1, w-60);


    QDockWidget::resizeEvent(event);
}

void LogViewDock::clear()
{
    QString m_strFileName;
    QDateTime time = QDateTime::currentDateTime();
    m_strFileName.sprintf("%s/Log/%s_DevLog.dat", gCfg.RootPath.toLatin1().data(),
        time.toString("yyyyMMdd").toLatin1().data());

    // log file to list
    QFile inputFile(m_strFileName);
    if (inputFile.open(QIODevice::ReadOnly))
    {
        inputFile.seek(inputFile.size()-1);
        int count = 0;
        int lines = 500;
        while ( (count < lines) && (inputFile.pos() > 0) )
        {
            QString ch = inputFile.read(1);
            inputFile.seek(inputFile.pos()-2);
            if (ch == "\n")
                count++;
        }

        QTextStream in(&inputFile);
        while (!in.atEnd())
        {
            QString line = in.readLine();
            int pos = line.indexOf(']');
            QString left = line.mid(1,pos-1);
            QString right = line.mid(pos+1);

            int row = modelLog->rowCount(QModelIndex());
            if (row >= lines) {
                modelLog->removeRow(0);
                row = modelLog->rowCount(QModelIndex());
            }

            QStandardItem *col1 = new QStandardItem(left);
            modelLog->setItem(row, 0, col1);
            QStandardItem *col2 = new QStandardItem(right);
            modelLog->setItem(row, 1, col2);
        }
        inputFile.close();
        tableView->scrollToBottom();
    }
}

int LogViewDock::LogAdd(QString lpszTime, QString lpszLog)
{
    QMutexLocker ml(&m_sync);

    int row = modelLog->rowCount(QModelIndex());
    if (row >= 500) {
        modelLog->removeRow(0);
        row = modelLog->rowCount(QModelIndex());
    }
    modelLog->insertRow(row);
    modelLog->setRowCount(row+1);

    QStandardItem *col1 = new QStandardItem(lpszTime);
    QStandardItem *col2 = new QStandardItem(lpszLog);
    modelLog->setItem(row, 0, col1);
    modelLog->setItem(row, 1, col2);

    tableView->scrollToBottom();
    return 0;
}
