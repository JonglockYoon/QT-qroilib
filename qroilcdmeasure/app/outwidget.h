#ifndef OUTWIDGET_H
#define OUTWIDGET_H

#include <QWidget>
#include <QStackedWidget>
#include "viewoutpage.h"

class OutWidget : public QWidget
{
    Q_OBJECT
public:
    explicit OutWidget(QString name, QWidget *parent = nullptr);
    ~OutWidget();

protected:
    void closeEvent(QCloseEvent *event);

signals:

public slots:

public:
    ViewOutPage* mViewOutPage;
    QStackedWidget* mStackedWidget;
};

#endif // OUTWIDGET_H
