/***************************************************************************
 *   Copyright (C) 2014-2017 by Cyril BALETAUD *
 *   cyril.baletaud@gmail.com                                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "camerasession.h"
#include "camerasession_p.h"

#include "gvcputils.h"
#include "networkselectionpage.h"
#include "discoverypage.h"
#include "forceippage.h"
#include "cameravalidationpage.h"
#include "gvsppage.h"
#include "gvspwidget.h"
//#include "endatwidget.h"

#include "gui/genicammodel.h"
#include "genicamtreeview.h"
#include "geviport.h"
#include "gvspdevices.h"
#include "genicam.h"
#include "mainwindow.h"

#include <QAction>
#include <QHostAddress>
#include <QVBoxLayout>
#include <QDialog>
#include <QWizard>
#include <QDialogButtonBox>
#include <QTimer>

using namespace Jgv::Gvsp;
using namespace Qroilib;

WidgetDialog::WidgetDialog(QWidget *widget, const QString &windowTitle)
{
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(widget);

    setWindowTitle(windowTitle);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttonBox);

    setLayout(layout);
}

CameraSessionPrivate::~CameraSessionPrivate()
{}

void CameraSessionPrivate::initControllerReceiverSession(QSharedPointer<Qroilib::GvcpUtils> gvcpUtilsPtr, QObject *parent)
{
    QHostAddress controller = gvcpUtilsPtr->controllerAddress();
    QHostAddress transmitter = gvcpUtilsPtr->transmitterAddress();

    // We create the controller
    QSharedPointer<GevIPort> iport(new GevIPort);
    if (!iport->controleDevice(controller.toIPv4Address(), transmitter.toIPv4Address())) {
        qWarning("CameraSession failed to control device");
    }

    // Creation of the model on the file of the device, with our controller
    Jgv::GenICam::Model *deviceModel = new Jgv::GenICam::Model(iport, parent);
    deviceModel->setGenICamXml(gvcpUtilsPtr->xmlFilename(), gvcpUtilsPtr->xmlFile());

    //The flow control actions on the device model
    QAction *action = new QAction(QIcon("://ressources/icons/run.png"), QObject::trUtf8("Start broadcasting"), parent);
    QObject::connect(action, &QAction::triggered, [deviceModel]() {
        deviceModel->setValue("AcquisitionStart", QVariant());
    });
    toolbarActions.append(action);
    menuActions.append(action);
    action = new QAction(QIcon("://ressources/icons/stop.png"), QObject::trUtf8("Stop broadcasting"), parent);
    QObject::connect(action, &QAction::triggered, [deviceModel]() {
        deviceModel->setValue("AcquisitionStop", QVariant());
    });
    toolbarActions.append(action);
    menuActions.append(action);

    // The treeview on the device model will be docked to the left
    GenICamTreeView *treeview = new GenICamTreeView;
    treeview->setModel(deviceModel);
    // Transfer the widget's property to the scoped pointer
    leftWidgetPtr.reset(treeview);

    createReceiver(parent);

    // Puts the receiver in listening mode
    receiverPtr->listenOn(controller.toIPv4Address(), transmitter.toIPv4Address());

    // Informs the address of the receiver
    iport->addReceiver(0, controller, receiverPtr->parameters().receiverPort);

    // Generic model on the bootstrap
    Jgv::GenICam::Model bootstrapModel(iport);
    bootstrapModel.setGenICamXml("bootstrap.xml", Jgv::GenICam::BootstrapXml::file());
    bootstrapModel.setValue("GevStreamChannelSelector", 0); // selectionne la sortie 0

    // filtre
    receiverPtr->acceptFrom(transmitter.toIPv4Address(), bootstrapModel.getValue("GevSCSP").toUInt());

    // Pre-allocation of images
    Jgv::Gvsp::Geometry geometry;


    QString s1 = deviceModel->getValue("Width").toString();
    QString s2 = deviceModel->getValue("Height").toString();
    QString s3 = deviceModel->getValue("PixelFormat").toString();
    QString s4 = bootstrapModel.getValue("GevSCPSPacketSize").toString();

    qDebug() << s1 << s2 << s3 << s4;

    geometry.width = deviceModel->getValue("Width").toUInt();
    geometry.height = deviceModel->getValue("Height").toUInt();
    geometry.pixelFormat = deviceModel->getValue("PixelFormat").toUInt();
    uint32_t packetSize = bootstrapModel.getValue("GevSCPSPacketSize").toUInt();
    if (geometry.width == 0 || geometry.height == 0) {
        geometry.width = s1.toInt();
        geometry.height = s2.toInt();
        geometry.pixelFormat = s3.toInt();
        packetSize = s4.toInt();
    }
    receiverPtr->preallocImages(geometry, packetSize);

    qDebug() << geometry.width << geometry.height << geometry.pixelFormat;

    // Provides clock frequency
    receiverPtr->setTransmitterTimestampFrequency(bootstrapModel.getValue("TimestampTickFrequency").toULongLong());
    // Activates the dating system
    auto date = [this, iport] () {
        const Jgv::Gvcp::Datation datation = iport->getDatation();
        if (datation.timestamp != 0) {
            receiverPtr->pushDatation(datation.timestamp, datation.dateMin, datation.dateMax);
        }
    };

    QObject::connect(&dateTimer, &QTimer::timeout, date);
    dateTimer.start(10000);
#if 0
    // We joust an action to edit the bootstrap
    action = new QAction(QObject::trUtf8("Bootstrap Editor"), parent);
    QObject::connect(action, &QAction::triggered, [iport]() {
        Jgv::GenICam::Model *bootstrapModel = new Jgv::GenICam::Model(iport);
        bootstrapModel->setGenICamXml("bootstrap.xml", Jgv::GenICam::BootstrapXml::file());
        GenICamTreeView *treeView = new GenICamTreeView;
        treeView->setModel(bootstrapModel, false);
        WidgetDialog dialog(treeView, "Bootstrap Editor");
        QObject::connect(&dialog, &QDialog::destroyed, bootstrapModel, &QAbstractItemModel::deleteLater);
        dialog.setMinimumSize(dialog.sizeHint() * 2);
        dialog.exec();
    });
    menuActions.append(action);
#endif
    // Initializes the statistics widget
    //statusBarPtr.reset(new GvspStatusBar);
    //statusBarPtr->visualizeReceiver(receiverPtr);

    QModelIndex index = deviceModel->searchFeature(0,"AcquisitionMode");
    deviceModel->setData(index, 1, Qt::EditRole); // SingleFrame

    // Launches broadcast
    deviceModel->setValue("AcquisitionStart", QVariant());

    QTimer *timer = new QTimer(parent);
    timer->setInterval(1000);
    timer->start();
    QObject::connect(timer, &QTimer::timeout, [deviceModel]() { // 1초마다 SingleFrame 가져오기
        deviceModel->setValue("AcquisitionStart", QVariant());
    });
}

void CameraSessionPrivate::initSimpleMonitorSession(QSharedPointer<Qroilib::GvcpUtils> gvcpUtilsPtr, QObject *parent)
{
    QHostAddress controller = gvcpUtilsPtr->controllerAddress();
    QHostAddress transmitter = gvcpUtilsPtr->transmitterAddress();

    // We create the controller
    QSharedPointer<GevIPort> iport(new GevIPort);
    iport->monitorDevice(controller.toIPv4Address(), transmitter.toIPv4Address());

    // Creation of the MVC model, with our controller
    Jgv::GenICam::Model *model = new Jgv::GenICam::Model(iport, parent);
    model->setGenICamXml(gvcpUtilsPtr->xmlFilename(), gvcpUtilsPtr->xmlFile());

    // The treeview will be the central widget
    GenICamTreeView *treeview = nullptr;
    mainWidgetPtr.reset(treeview = new GenICamTreeView);
    treeview->setModel(model);

#if 0
    QAction *bootstrapAction = new QAction(QObject::trUtf8("Bootstrap"), parent);
    QObject::connect(bootstrapAction, &QAction::triggered, [iport]() {
        Jgv::GenICam::Model *bootstrapModel = new Jgv::GenICam::Model(iport);
        bootstrapModel->setGenICamXml("bootstrap.xml", Jgv::GenICam::BootstrapXml::file());
        GenICamTreeView *treeView = new GenICamTreeView;
        treeView->setModel(bootstrapModel, false);
        WidgetDialog dialog(treeView, "Bootstrap Editor");
        QObject::connect(&dialog, &QDialog::destroyed, bootstrapModel, &QAbstractItemModel::deleteLater);
        dialog.setMinimumSize(dialog.sizeHint() * 2);
        dialog.exec();
    });
    menuActions.append(bootstrapAction);
#endif
}

void CameraSessionPrivate::createReceiver(QObject *parent)
{
    receiverPtr = QSharedPointer<Jgv::Gvsp::Receiver>::create();
    //receiverPtr.reset(new Jgv::Gvsp::Receiver);

    Jgv::Gvsp::Receiver *rcv = (Jgv::Gvsp::Receiver *)receiverPtr.data();
    MainWindow *m = (MainWindow *)parent->parent();
    QObject::connect(rcv, SIGNAL(processedGvspImage(const QImage &)), m, SLOT(updatePlayerGvspUI(const QImage &)));

    GvspWidget *allocatorWidget = new GvspWidget;
    allocatorWidget->visualizeReceiver(receiverPtr);
    mainWidgetPtr.reset(allocatorWidget);

    // Adds action to manage receiver settings
    QAction *action = new QAction(QObject::trUtf8("Receiver Settings"), parent);
    QObject::connect(action, &QAction::triggered, [=]() {
        GvspWidget *w = new GvspWidget;
        w->visualizeReceiver(receiverPtr, 200);
        WidgetDialog dialog(w, QObject::trUtf8("Recipient"));
        dialog.exec();
    });
    menuActions.append(action);

}

CameraSession::CameraSession(QObject *parent)
    : QObject(parent), d_ptr(new CameraSessionPrivate)
{}

CameraSession::CameraSession(CameraSessionPrivate &dd, QObject *parent)
    : QObject(parent), d_ptr(&dd)
{}

CameraSession::~CameraSession()
{}


QWidget *CameraSession::mainWidget() const
{
    Q_D(const CameraSession);
    return d->mainWidgetPtr.data();
}

QWidget *CameraSession::leftDockedWidget() const
{
    Q_D(const CameraSession);
    return d->leftWidgetPtr.data();
}

//QStatusBar *CameraSession::statusBar() const
//{
//    Q_D(const CameraSession);
//    return d->statusBarPtr.data();
//}

QWidget *CameraSession::bootstrapWidget() const
{
    Q_D(const CameraSession);
    return d->bootstrapPtr.data();
}

QList<QAction *> CameraSession::menuActions() const
{
    Q_D(const CameraSession);
    return d->menuActions;
}

QList<QAction *> CameraSession::toolbarActions() const
{
    Q_D(const CameraSession);
    return d->toolbarActions;
}

bool CameraSession::startSession()
{
    Q_D(CameraSession);

    QSharedPointer<Qroilib::GvcpUtils> gvcpUtils(new Qroilib::GvcpUtils);
    QWizard wizard;
    wizard.setWindowTitle(trUtf8("Configure for a GiGE Vision camera"));

    wizard.addPage(new NetworkSelectionPage(gvcpUtils));
    wizard.addPage(new DiscoveryPage(gvcpUtils));
    wizard.addPage(new ForceIPPage(gvcpUtils));
    wizard.addPage(new CameraValidationPage(gvcpUtils));
    wizard.addPage(new GvspPage(gvcpUtils));


    if (wizard.exec() == QDialog::Accepted) {
        //if (gvcpUtils->wantControl()) {
        //    d->initControllerReceiverSession(gvcpUtils, this);
        //}
        //else {
        d->initControllerReceiverSession(gvcpUtils, this);
        d->initSimpleMonitorSession(gvcpUtils, this);
        //}
        return true;
    }

    return false;
}

