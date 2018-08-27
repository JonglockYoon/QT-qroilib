#include "outwidget.h"
#include "mainwindow.h"

OutWidget::OutWidget(QString name, QWidget *parent) : QWidget(parent)
{
    this->setWindowTitle(name);
    this->setMinimumSize(320, 240);
    this->setWindowFlags(parent->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    this->setWindowFlags(parent->windowFlags() | Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint);

    mStackedWidget = new QStackedWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(mStackedWidget);
    layout->setMargin(0);
    layout->setSpacing(0);
    mViewOutPage = new ViewOutPage(mStackedWidget);
    mStackedWidget->addWidget(mViewOutPage);
    this->setLayout(layout);
}

OutWidget::~OutWidget()
{
}

void OutWidget::closeEvent(QCloseEvent *event)
{
    int size = theMainWindow->vecOutWidget.size();
    for (int i=0; i<size; i++) {
        if (this == theMainWindow->vecOutWidget[i]) {
            theMainWindow->vecOutWidget.erase(theMainWindow->vecOutWidget.begin()+i);
            break;
        }
    }

    //delete mViewOutPage;
}
