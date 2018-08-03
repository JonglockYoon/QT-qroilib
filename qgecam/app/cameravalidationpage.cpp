/***************************************************************************
 *   Copyright (C) 2014-2017 by Cyril BALETAUD                                  *
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

#include "cameravalidationpage.h"
#include "cameravalidationpage_p.h"
#include "lineseparator.h"
#include "gvcputils.h"
#include "discoveryhelper.h"

#include <QFormLayout>
#include <QRadioButton>
#include <QTextEdit>
#include <QHostAddress>
#include <QNetworkAddressEntry>
#include <QCoreApplication>
#include <QProgressDialog>
#include <QPushButton>
#include <QFileDialog>

using namespace Qroilib;

void CameraValidationPagePrivate::fillOutput()
{
    static const char * okColor = "green";
    static const char * failedColor = "red";

    QString color = okColor;

    output->insertHtml(QString("adresse locale : <b>%0</b><br>")
                       .arg(gvcpUtils->controller().ip().toString()));
    output->insertHtml(QString("Login address : <b>%0</b><br>")
                       .arg(QHostAddress(Jgv::Gvcp::DiscoveryAckHelper::currentIP(gvcpUtils->transmitterAck())).toString()));
    output->insertHtml(QString("Connection status :  <font color=%0><b>%1</b></font><br>")
                       .arg(connected?okColor:failedColor).arg(connected?"OK":"failed"));
    output->insertHtml(QString("Privilege of the transmitter : <font color=%0><b>%1</b></font><br>")
                       .arg(accessMode.isEmpty()?failedColor:"black").arg(accessMode.isEmpty()?"failed":accessMode));
    output->insertHtml(QString("Xml file name : <font color=%0><b>%1</b></font><br>")
                       .arg(color).arg(QString(xmlFileName)));
    output->insertHtml(QString("Xml file size : <font color=%0><b>%1</b> octets</font><br>")
                       .arg(xmlFile.isEmpty()?failedColor:okColor).arg(xmlFile.size()));
}


CameraValidationPage::CameraValidationPage(QSharedPointer<GvcpUtils> discoverer, QWidget *parent)
    : JgvWizardPage(discoverer, *new CameraValidationPagePrivate, parent)
{
    Q_D(CameraValidationPage);
    setTitle(trUtf8("Connection Mode"));
    setSubTitle (trUtf8 ("You can connect as a controller (primary application)"
                            "Or as a secondary application"));


    QFormLayout *mainLayout = new QFormLayout;
    mainLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    d->primaryApp = new QRadioButton("Mode controller");
    d->secondaryApp = new QRadioButton("Mode monitor");
    d->primaryApp->setChecked(false);
    d->primaryApp->setEnabled(false);
    d->secondaryApp->setChecked(true);
    d->secondaryApp->setEnabled(false);
    //mainLayout->addRow(trUtf8("Connection Mode"), d->primaryApp);
    //mainLayout->addWidget(d->secondaryApp);
    //mainLayout->addRow(new LineSeparator);

    d->output = new QTextEdit;
    mainLayout->addRow("Connection Status", d->output);

    QPushButton *saveXML = new QPushButton(trUtf8("To save"));
    connect(saveXML, &QPushButton::clicked, [d]() {
        if (d->xmlFile.isEmpty() || d->xmlFileName.isEmpty()) {
            return;
        }
        QString dir = QFileDialog::getExistingDirectory(nullptr, trUtf8("Backup folder"), QDir::homePath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        if (!dir.isEmpty()) {
            QString current = QDir::currentPath();
            QDir::setCurrent(dir);
            QFile xml;
            xml.setFileName(d->xmlFileName);
            xml.open(QIODevice::WriteOnly | QIODevice::Text);
            xml.write(d->xmlFile);
            xml.close();
            QDir::setCurrent(current);
        }
    });

    mainLayout->addRow(trUtf8("Save the XML file"), saveXML);


    setLayout(mainLayout);

    connect(d->secondaryApp, &QRadioButton::toggled, [this, d] () {
        this->setFinalPage(d->secondaryApp->isChecked());
    });
}

CameraValidationPage::~CameraValidationPage()
{}


void CameraValidationPage::initializePage()
{
    Q_D(CameraValidationPage);

    QProgressDialog progress(trUtf8("Transmitter inquiry"), QString(), 0, 0, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.show();

    QCoreApplication::processEvents();
    d->connected = false;
    d->accessOk = false;
    d->accessMode.clear();
    d->xmlFileName.clear();
    d->xmlFile.clear();
    d->gvcpUtils->setXmlFilename(QByteArray());
    d->gvcpUtils->setXmlFile(QByteArray());

    d->primaryApp->setChecked(d->gvcpUtils->wantControl());

    d->output->clear();

    if (d->gvcpUtils->monitorTransmitter()) {
        d->connected = true;

        //The current access level is requested
        Jgv::Gvcp::CCPPrivilege accessMode = d->gvcpUtils->transmitterAccessMode();
        QCoreApplication::processEvents();

        switch (accessMode) {
        case Jgv::Gvcp::CCPPrivilege::OpenAcces:
            d->primaryApp->setEnabled(true);
            d->secondaryApp->setEnabled(true);
            d->accessMode = QString::fromStdString(Jgv::Gvcp::CCP::privilegeToString(accessMode));
            break;
        case Jgv::Gvcp::CCPPrivilege::ControlAcces:
        case Jgv::Gvcp::CCPPrivilege::ControlAccesWithSwitchoverEnabled:
            d->primaryApp->setEnabled(false);
            d->secondaryApp->setEnabled(true);
            d->accessMode = QString::fromStdString(Jgv::Gvcp::CCP::privilegeToString(accessMode));
            break;
        case Jgv::Gvcp::CCPPrivilege::ExclusiveAcces:
        case Jgv::Gvcp::CCPPrivilege::AccesDenied:
            d->primaryApp->setEnabled(false);
            d->secondaryApp->setEnabled(false);
        }

        QCoreApplication::processEvents();

        d->xmlFileName = d->gvcpUtils->readXmlFilenameFromDevice();
        QCoreApplication::processEvents();
        d->xmlFile = d->gvcpUtils->readXmlFileFromDevice();
        QCoreApplication::processEvents();


    }
    d->fillOutput();
}

bool CameraValidationPage::validatePage()
{
    Q_D(CameraValidationPage);

    if (!d->xmlFileName.isEmpty() && !d->xmlFile.isEmpty()) {
        d->gvcpUtils->setXmlFilename(d->xmlFileName);
        d->gvcpUtils->setXmlFile(d->xmlFile);
        d->gvcpUtils->releaseTransmitter();
        d->gvcpUtils->setWantControl(false/*d->primaryApp->isChecked()*/);
        return true;
    }

    return false;
}

void CameraValidationPage::cleanupPage()
{
    Q_D(CameraValidationPage);
    d->gvcpUtils->releaseTransmitter();
    d->output->clear();
}

int CameraValidationPage::nextId() const
{
    Q_D(const CameraValidationPage);
    if (d->secondaryApp->isChecked()) {
        return -1;
    }
    return QWizardPage::nextId();
}



