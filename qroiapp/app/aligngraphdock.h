#pragma once

#include <QDockWidget>
#include <QStandardItemModel>
#include <QTableView>
#include <string>
#include <stdio.h>
#include <QDateTime>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QCategoryAxis>

QT_CHARTS_USE_NAMESPACE
using namespace std;

class AlignGraphDock : public QDockWidget
{
    Q_OBJECT
public:
    typedef struct {
        unsigned long seq;
        QDateTime d;
        int point;      // screw point
        double x;
        double y;
    } DATA;
    DATA data;
    QVector<AlignGraphDock::DATA> datapoints;

public:
    explicit AlignGraphDock(QString name, QWidget *parent = nullptr);

private:
    QChartView *chartView[2];
    QChart *chart[2];
    QLineSeries *series[2][2]; // chart당 x,y graph
//    QCategoryAxis *axisX[2];
//    QCategoryAxis *axisY[2];
    QValueAxis *axisX[2];
    QValueAxis *axisY[2];

public:
    virtual void resizeEvent(QResizeEvent *event);
    void clear();

public Q_SLOTS:
    void alignValue(int nScrew, int nPoint, double x, double y);

};

