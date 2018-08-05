/*
 * controlsdock.cpp
 */

#include "positionsdock.h"
#include "roiobject.h"
#include "utils.h"
#include <qroilib/documentview/documentview.h>
#include "mainwindow.h"

#include <QLabel>
#include <QApplication>
#include <QHeaderView>
#include <QDebug>
#include <QStandardItemModel>

using namespace Qroilib;
using namespace std;

PositionsDock::PositionsDock(QString name, QWidget *parent)
    : QDockWidget(name, parent)
{
    setMinimumWidth(120); // it will be motify resizeEvent()..
    setMaximumWidth(400);
    setMinimumHeight(50);
    setMaximumHeight(300);

    modelScrew = new QStandardItemModel(0,2,this);
    modelScrew->setHorizontalHeaderItem(0, new QStandardItem(QString("position")));
    modelScrew->setHorizontalHeaderItem(1, new QStandardItem(QString("align")));
    tableView = new QTableView(this);
    tableView->setModel(modelScrew);
    tableView->horizontalHeader()->setStretchLastSection(true);

    tableView->setColumnWidth(0, 70);
    tableView->setColumnWidth(1, 30);
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);

    modelScrew2 = new QStandardItemModel(0,2,this);
    modelScrew2->setHorizontalHeaderItem(0, new QStandardItem(QString("position")));
    modelScrew2->setHorizontalHeaderItem(1, new QStandardItem(QString("align")));
    tableView2 = new QTableView(this);
    tableView2->setModel(modelScrew2);
    tableView2->horizontalHeader()->setStretchLastSection(true);

    tableView2->setColumnWidth(0, 70);
    tableView2->setColumnWidth(1, 30);
    tableView2->setEditTriggers(QAbstractItemView::NoEditTriggers);

    tableView2->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView2->setSelectionMode(QAbstractItemView::SingleSelection);

    QWidget *widget = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(widget);
    //layout->setMargin(0);
    //layout->setSpacing(0);
    layout->addWidget(tableView);
    layout->addWidget(tableView2);
    widget->setLayout(layout);
    setWidget(widget);

    //Add(0, "100.02,100.02,11.1", "0.5,0.5"); // test
    //Add(0, "200.02,200.02,22.2", "0.6,0.6");
    initPositionlist();

    connect(tableView->verticalHeader(), &QHeaderView::sectionClicked, [&](const int col)
        {
            theMainWindow->mTeachDock->pushButtonClear();
        });
    connect(tableView2->verticalHeader(), &QHeaderView::sectionClicked, [&](const int col)
        {
            theMainWindow->mTeachDock->pushButtonClear();
        });
    connect(tableView, &QTableView::clicked, [&](const QModelIndex index)
        {
        theMainWindow->mTeachDock->pushButtonClear();
        });
    connect(tableView2, &QTableView::clicked, [&](const QModelIndex index)
        {
        theMainWindow->mTeachDock->pushButtonClear();
        });

    connect(tableView, &QTableView::doubleClicked, [&](const QModelIndex index)
        {
            clearSelected(0);
        });
    connect(tableView2, &QTableView::doubleClicked, [&](const QModelIndex index)
        {
            clearSelected(1);
        });

    connect(tableView->verticalHeader(), &QHeaderView::sectionDoubleClicked, [&](const int row)  // move robot position
        {
            QModelIndex nindex = modelScrew->index(row, 0);
            //qDebug() << row << nindex.data().toString();
            QStringList pos = nindex.data().toString().split(",");
            theMainWindow->itfport.Move("left", pos[0].toDouble(), pos[1].toDouble(), pos[2].toDouble());

            theMainWindow->SetCameraPause(0, true);
        });
    connect(tableView2->verticalHeader(), &QHeaderView::sectionDoubleClicked, [&](const int row)  // move robot position
        {
            QModelIndex nindex = modelScrew2->index(row, 0);
            //qDebug() << row << nindex.data().toString();
            QStringList pos = nindex.data().toString().split(",");
            theMainWindow->itfport.Move("right", pos[0].toDouble(), pos[1].toDouble(), pos[2].toDouble());

            theMainWindow->SetCameraPause(1, true);
        });

    QObject::connect(theMainWindow->m_pTrsAlign[0], SIGNAL(initPositionlist(int)), this, SLOT(initPositionlist(int)));
    QObject::connect(theMainWindow->m_pTrsAlign[1], SIGNAL(initPositionlist(int)), this, SLOT(initPositionlist(int)));
    QObject::connect(theMainWindow->m_pTrsAlign[0], SIGNAL(updatePositionlist(int)), this, SLOT(updatePositionlist(int)));
    QObject::connect(theMainWindow->m_pTrsAlign[1], SIGNAL(updatePositionlist(int)), this, SLOT(updatePositionlist(int)));

}

void PositionsDock::initPositionlist()
{
    for (int i = 0; i < 2; i++)
    {
        initPositionlist(i);
    }
}
void PositionsDock::initPositionlist(int ch)
{
    QMutexLocker ml(&m_sync);

    QString qstr, qstr1, temp;
    MInfoTeachManager*	pInfo = theMainWindow->m_pInfoTeachManager;

    Clear(ch);
    int size = pInfo->m_vecScrewTable[ch].size();
    for (int j = 0; j < size; j++)
    {
        SCREWITEM *pitem = &pInfo->m_vecScrewTable[ch][j];
        qstr = "";
        temp.sprintf(("%5.2f,"), pitem->x); qstr += temp;
        temp.sprintf(("%5.2f,"), pitem->y); qstr += temp;
        temp.sprintf(("%3.1f"), pitem->z); qstr += temp;
        qstr1 = "";
        temp.sprintf(("%5.2f,"), pitem->dx); qstr1 += temp;
        temp.sprintf(("%5.2f"), pitem->dy); qstr1 += temp;

        Add(ch, qstr, qstr1);
    }
}


int PositionsDock::Clear(int ch)
{
    if (ch == 0)
    {
        while (modelScrew->rowCount() > 0)
        {
            modelScrew->removeRow(0);
        }
        tableView->setModel(modelScrew);
    }
    else {
        while (modelScrew2->rowCount() > 0)
        {
            modelScrew2->removeRow(0);
        }
        tableView2->setModel(modelScrew2);
    }
    return 0;
}
int PositionsDock::Add(int ch, QString pos, QString align)
{
    if (ch == 0)
    {
        int row = modelScrew->rowCount(QModelIndex());
        modelScrew->insertRow(row);
        modelScrew->setRowCount(row+1);

        QStandardItem *col1 = new QStandardItem(pos);
        QStandardItem *col2 = new QStandardItem(align);
        modelScrew->setItem(row, 0, col1);
        modelScrew->setItem(row, 1, col2);
//        QList<QStandardItem*> row1;
//        row1 << col1 << col2;
//        modelScrew->appendRow( row1 );

        tableView->scrollToBottom();
        //qDebug() << "Add row" << row;
    }
    else {
        int row = modelScrew2->rowCount(QModelIndex());
        modelScrew2->insertRow(row);
        modelScrew2->setRowCount(row+1);

        QStandardItem *col1 = new QStandardItem(pos);
        QStandardItem *col2 = new QStandardItem(align);
        modelScrew2->setItem(row, 0, col1);
        modelScrew2->setItem(row, 1, col2);
//        QList<QStandardItem*> row2;
//        row2 << col1 << col2;
//        modelScrew2->appendRow( row2 );

        tableView2->scrollToBottom();
    }
    return 0;
}

void PositionsDock::updatePositionlist()
{
    for (int i = 0; i < 2; i++)
    {
        updatePositionlist(i);
    }
}

void PositionsDock::updatePositionlist(int ch)
{
    QMutexLocker ml(&m_sync);

    QString qstr, qstr1, temp;
    MInfoTeachManager*	pInfo = theMainWindow->m_pInfoTeachManager;

    int size = pInfo->m_vecScrewTable[ch].size();
    for (int j = 0; j < size; j++)
    {
        SCREWITEM *pitem = &pInfo->m_vecScrewTable[ch][j];
        qstr = "";
        temp.sprintf(("%5.2f,"), pitem->x); qstr += temp;
        temp.sprintf(("%5.2f,"), pitem->y); qstr += temp;
        temp.sprintf(("%3.1f"), pitem->z); qstr += temp;
        qstr1 = "";
        temp.sprintf(("%5.2f,"), pitem->dx); qstr1 += temp;
        temp.sprintf(("%5.2f"), pitem->dy); qstr1 += temp;

        Update(ch, j, qstr, qstr1);
    }
}

int PositionsDock::Update(int ch, int row, QString pos, QString align)
{
    if (ch == 0)
    {
        modelScrew->item(row, 0)->setData(pos, Qt::DisplayRole);
        modelScrew->item(row, 1)->setData(align, Qt::DisplayRole);
    }
    else {
        modelScrew2->item(row, 0)->setData(pos, Qt::DisplayRole);
        modelScrew2->item(row, 1)->setData(align, Qt::DisplayRole);
    }
    return 0;
}

// clear selected row
void PositionsDock::clearSelected(int ch)
{
    if (ch == 0) {
        QModelIndex nindex = tableView->model()->index(-1, 0);
        tableView->setCurrentIndex(nindex);
    } else {
        QModelIndex nindex = tableView2->model()->index(-1, 0);
        tableView2->setCurrentIndex(nindex);
    }
}


void PositionsDock::resizeEvent(QResizeEvent *event) {
    if (theMainWindow->m_iActiveView < 0) {
        int w = this->width()/2;
        tableView->setMaximumWidth(140);
        tableView2->setMaximumWidth(140);

        tableView->show();
        tableView->setColumnWidth(0, w/3*2-40);
        tableView->setColumnWidth(1, w/3-10);

        tableView2->show();
        tableView2->setColumnWidth(0, w/3*2-40);
        tableView2->setColumnWidth(1, w/3-10);

    } else {
        int w = this->width();
        if (theMainWindow->m_iActiveView == 0) {
            tableView->show();
            tableView->setColumnWidth(0, w/3*2-40);
            tableView->setColumnWidth(1, w/3-10);
            tableView->setMaximumWidth(280);
            tableView2->hide();
        } else {
            tableView2->show();
            tableView2->setColumnWidth(0, w/3*2-40);
            tableView2->setColumnWidth(1, w/3-10);
            tableView2->setMaximumWidth(280);
            tableView->hide();
        }
    }

    QDockWidget::resizeEvent(event);
}

int PositionsDock::setSelectedRowValue(int ch, double x, double y, double z)
{
    double x1, y1, z1;
    int row = GetSelectedRow(ch, x1, y1, z1);
    if (row < 0)
        return -1;

    QString str;
    str.sprintf("%5.2f,%5.2f,%4.1f", x, y, z);
    QStandardItem *col1 = new QStandardItem(str);
    if (ch == 0)
    {
        modelScrew->setItem(row, 0, col1);
    }
    else {
        modelScrew2->setItem(row, 0, col1);
    }
    return 0;
}
int PositionsDock::GetSelectedRow(int ch, double &x, double &y, double &z)
{
    int row = -1;
    if (ch == 0)
    {
        QItemSelectionModel *select = tableView->selectionModel();
        QModelIndexList l = select->selectedRows(); // return selected row(s)
        if (l.count() > 0) {
            row = l[0].row();
            QModelIndex nindex = modelScrew->index(row, 0);
            QStringList pos = nindex.data().toString().split(",");
            if (pos.count() >= 3) {
                x = pos[0].toDouble();
                y = pos[1].toDouble();
                z = pos[2].toDouble();
            }
        }

    }
    else {
        QItemSelectionModel *select = tableView2->selectionModel();
        QModelIndexList l = select->selectedRows(); // return selected row(s)
        if (l.count() > 0) {
            row = l[0].row();
            QModelIndex nindex = modelScrew->index(row, 0);
            QStringList pos = nindex.data().toString().split(",");
            if (pos.count() >= 3) {
                x = pos[0].toDouble();
                y = pos[1].toDouble();
                z = pos[2].toDouble();
            }
        }
    }
    return row;
}

int PositionsDock::GetNextRow(int ch, double &x, double &y, double &z)
{
    int n = GetSelectedRow(ch, x, y, z);
    if (n < 0)
        return -1;

    int row = -1;
    if (ch == 0)
    {
        QItemSelectionModel *select = tableView->selectionModel();
        QModelIndex nindex = tableView->model()->index(n+1, 0);
        row = nindex.row();
        if (row >= 0) {
            select->select( nindex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
            QStringList pos = nindex.data().toString().split(",");
            if (pos.count() >= 3) {
                x = pos[0].toDouble();
                y = pos[1].toDouble();
                z = pos[2].toDouble();
            }
            tableView->setCurrentIndex(nindex);
        }
    }
    else {
        QItemSelectionModel *select = tableView2->selectionModel();
        QModelIndex nindex = tableView2->model()->index(n+1, 0);
        row = nindex.row();
        if (row >= 0) {
            select->select( nindex, QItemSelectionModel::ClearAndSelect );
            QStringList pos = nindex.data().toString().split(",");
            if (pos.count() >= 3) {
                x = pos[0].toDouble();
                y = pos[1].toDouble();
                z = pos[2].toDouble();
            }
            tableView2->setCurrentIndex(nindex);
        }
    }
    return row;

}
