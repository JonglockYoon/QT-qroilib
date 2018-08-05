#include <QApplication>
#include <QHeaderView>
#include <QDesktopWidget>
#include <QVBoxLayout>
#include <QDateTime>
#include <QTimer>
#include <QGraphicsLayout>
#include "aligngraphdock.h"
#include "mainwindow.h"
#include "config.h"
#include "common.h"

AlignGraphDock::AlignGraphDock(QString name, QWidget *parent)
    : QDockWidget(name, parent)
{
    setMinimumHeight(100);
    setMaximumHeight(300);

    QPen pen1(QRgb(0xfd8140));
    //QPen pen1(QRgb(0x00ff00));
    pen1.setWidth(2);
    QPen pen2(QRgb(0x5f005f));
    pen2.setWidth(2);

    QLinearGradient backgroundGradient;
    backgroundGradient.setStart(QPointF(0, 0));
    backgroundGradient.setFinalStop(QPointF(0, 1));
    backgroundGradient.setColorAt(0.0, QRgb(0xd2d0d1));
    backgroundGradient.setColorAt(1.0, QRgb(0x4c4547));
    backgroundGradient.setCoordinateMode(QGradient::ObjectBoundingMode);

    QLinearGradient plotAreaGradient;
    plotAreaGradient.setStart(QPointF(0, 1));
    plotAreaGradient.setFinalStop(QPointF(1, 0));
    plotAreaGradient.setColorAt(0.0, QRgb(0x555555));
    plotAreaGradient.setColorAt(1.0, QRgb(0x55aa55));
    plotAreaGradient.setCoordinateMode(QGradient::ObjectBoundingMode);

    QFont labelsFont;
    labelsFont.setPixelSize(10);

    QPen axisPen(QRgb(0xd18952));
    axisPen.setWidth(2);

    QBrush axisBrush(Qt::white);


    for (int i=0; i<2; i++)
    {
        chart[i] = new QChart;
        chart[i]->setBackgroundBrush(backgroundGradient);
        chart[i]->setPlotAreaBackgroundBrush(plotAreaGradient);
        chart[i]->setPlotAreaBackgroundVisible(true);
        chart[i]->layout()->setContentsMargins(0, 0, 0, 0);
        //chart[i]->setBackgroundRoundness(0);
        chart[i]->setMargins(QMargins(10,0,10,20));
        chart[i]->legend()->hide();

        chartView[i] = new QChartView(chart[i]);
        series[i][0] = new QLineSeries();
        series[i][0]->setPen(pen1);
        series[i][1] = new QLineSeries();
        series[i][1]->setPen(pen2);

//        axisX[i] = new QCategoryAxis();
//        axisY[i] = new QCategoryAxis();
        axisX[i] = new QValueAxis();
        axisY[i] = new QValueAxis();

        axisX[i]->setTickCount(10);
        axisX[i]->setLabelFormat("%i");
        axisX[i]->setLabelsFont(labelsFont);
        axisY[i]->setLabelsFont(labelsFont);
        axisX[i]->setLinePen(axisPen);
        axisY[i]->setLinePen(axisPen);
        axisX[i]->setLabelsBrush(axisBrush);
        axisY[i]->setLabelsBrush(axisBrush);

        chart[i]->addSeries(series[i][0]);
        chart[i]->addSeries(series[i][1]);

//        axisY[i]->append("-1.5", -1.5);
//        axisY[i]->append("0", 0);
//        axisY[i]->append("1.5", 1.5);

        axisY[i]->setRange(-1.5, 1.5);

        chart[i]->setAxisX(axisX[i], series[i][0]);
        chart[i]->setAxisY(axisY[i], series[i][0]);
        chart[i]->setAxisX(axisX[i], series[i][1]);
        chart[i]->setAxisY(axisY[i], series[i][1]);

    }

    QWidget *widget = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(widget);
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(chartView[0]);
    layout->addWidget(chartView[1]);
    widget->setLayout(layout);

    clear();

#if 0
    ///
    ///
    /// test
    int cnt = 0;
    data.seq = 0;
    QDateTime momentInTime = QDateTime::currentDateTime();
    for (int i=0; i<10; i++) {

        QTime t = QTime(0,0,i);
        momentInTime.setTime(t);


        data.seq = i;
        data.d = momentInTime;
        data.x = ((cnt % 10) - 5.0) / 5.0;
        data.y = ((cnt % 8) - 4.0) / 4.0 + 0.5;
        datapoints.append(data);
        cnt++;
    }
    ///
    ///
    ///
    ///
#endif
    int sz = datapoints.size();

    QVector<QPointF> pointx;
    QVector<QPointF> pointy;
    for (int i=0; i<sz; i++) {
        pointx.append(QPointF(datapoints[i].seq, datapoints[i].x));
        pointy.append(QPointF(datapoints[i].seq, datapoints[i].y));
    }

    unsigned long s1 = 0;
    unsigned long e1 = 0;
    if (sz > 0) {
        s1 = datapoints[0].seq;
        e1 = datapoints[sz-1].seq;
    }
    series[0][0]->replace(pointx);
    series[0][1]->replace(pointy);
    axisX[0]->setRange(s1, e1);

    series[1][0]->replace(pointx);
    series[1][1]->replace(pointy);
    axisX[1]->setRange(s1, e1);

    setWidget(widget);

    // test
#if 0
    QTimer *timer2 = new QTimer(this);
    timer2->setSingleShot(false);
    QObject::connect(timer2, &QTimer::timeout, [&]() {

    } );
    timer2->start(1000);
#endif


    QObject::connect(theMainWindow->mTeachDock, SIGNAL(alignValue(int, int, double, double)), this, SLOT(alignValue(int, int, double, double)));

}

void AlignGraphDock::resizeEvent(QResizeEvent *event) {
    int h = this->height();
    int w = this->width();
    if (gCfg.m_nCamNumber < 2) {
        chartView[0]->resize(w, h);
        chartView[0]->show();
        chartView[1]->hide();
    } else {
        chartView[0]->resize(w/2, h);
        chartView[1]->resize(w/2, h);
        chartView[0]->show();
        chartView[1]->show();
    }
    //chart[0]->layout()->setContentsMargins(0, 0, 0, 0.1);
    //chart[1]->layout()->setContentsMargins(0, 0, 0, 0.1);

    QDockWidget::resizeEvent(event);
}

void AlignGraphDock::clear()
{
    QString m_strFileName;
    QDateTime time = QDateTime::currentDateTime();
    m_strFileName.sprintf("%s/Log/%s_alignlog.dat", gCfg.RootPath.toLatin1().data(),
        time.toString("yyyyMM").toLatin1().data());

    // log file to list
    QFile inputFile(m_strFileName);
    if (inputFile.open(QIODevice::ReadOnly))
    {
        inputFile.seek(inputFile.size()-1);
        int count = 0;
        int lines = 50;
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
            QString sdate = line.mid(1,pos-1);
            QString info = line.mid(pos+1);
            QDateTime date = QDateTime::fromString(sdate,"yyyy-MM-dd hh:mm:ss.zzz");
            QStringList list = info.split(QRegExp(" "), QString::SkipEmptyParts);
            int seq = 0;
            int point = 0;
            double x = 0.0, y = 0.0;
            //qDebug() << date;
            if (list.count() >= 4)
            {
                for (int i = 0; i < list.count(); i++) {
                    if (i == 0) seq = list[i].toInt();
                    else if (i == 1) point = list[i].toInt();
                    else if (i == 2) x = list[i].toDouble();
                    else if (i == 3) y = list[i].toDouble();
                }

                data.seq = seq;
                data.d = date;
                data.point = point;
                data.x = x;
                data.y = y;
                datapoints.append(data);
                if (datapoints.size() > 10)
                    datapoints.erase(datapoints.begin());
            }
        }
        inputFile.close();
    }
}

// nPoint : screw point
void AlignGraphDock::alignValue(int nScrew, int nPoint, double x, double y)
{
    int sz = datapoints.size();
    if (sz > 0)
    {
        data = datapoints[sz-1];
        data.seq++;
    } else {
        data.seq = 0;
    }
    data.d = QDateTime::currentDateTime();
    data.point = nPoint;
    data.x = x;
    data.y = y;
    datapoints.append(data);
    if (datapoints.size() > 10)
        datapoints.erase(datapoints.begin());
    sz = datapoints.size();

    QVector<QPointF> pointx;
    QVector<QPointF> pointy;
    for (int i=0; i<sz; i++) {
        pointx.append(QPointF(datapoints[i].seq, datapoints[i].x));
        pointy.append(QPointF(datapoints[i].seq, datapoints[i].y));
    }

    unsigned long s1 = 0;
    unsigned long e1 = 0;
    s1 = datapoints[0].seq;
    e1 = datapoints[sz-1].seq;

    series[nScrew][0]->replace(pointx);
    series[nScrew][1]->replace(pointy);
    axisX[nScrew]->setRange(s1, e1);
}
