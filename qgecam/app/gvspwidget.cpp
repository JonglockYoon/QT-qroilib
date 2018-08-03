/***************************************************************************
 *   Copyright (C) 2014-2017 by Cyril BALETAUD                             *
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

#include "gvspwidget.h"

#include "gvspreceiver.h"
#include "gvsp.h"

#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QApplication>
#include <QHostAddress>
#include <QDateTime>
#include <QTimer>

using namespace Qroilib;

namespace {

QString pixelToString(quint32 pixel)
{
    const quint32 color = pixel & GVSP_PIX_COLOR_MASK;
    QString type;

    switch (pixel) {

    // Mono buffer format defines
    case GVSP_PIX_MONO8:
    case GVSP_PIX_MONO8_SIGNED:
    case GVSP_PIX_MONO10:
    case GVSP_PIX_MONO10_PACKED:
    case GVSP_PIX_MONO12:
    case GVSP_PIX_MONO12_PACKED:
    case GVSP_PIX_MONO14:
    case GVSP_PIX_MONO16:
        type = "Mono";
        break;
        // Bayer buffer format defines
    case GVSP_PIX_BAYGR8:
    case GVSP_PIX_BAYRG8:
    case GVSP_PIX_BAYGB8:
    case GVSP_PIX_BAYBG8:
    case GVSP_PIX_BAYGR10:
    case GVSP_PIX_BAYRG10:
    case GVSP_PIX_BAYGB10:
    case GVSP_PIX_BAYBG10:
    case GVSP_PIX_BAYGR12:
    case GVSP_PIX_BAYRG12:
    case GVSP_PIX_BAYGB12:
    case GVSP_PIX_BAYBG12:
    case GVSP_PIX_BAYGR10_PACKED:
    case GVSP_PIX_BAYRG10_PACKED:
    case GVSP_PIX_BAYGB10_PACKED:
    case GVSP_PIX_BAYBG10_PACKED:
    case GVSP_PIX_BAYGR12_PACKED:
    case GVSP_PIX_BAYRG12_PACKED:
    case GVSP_PIX_BAYGB12_PACKED:
    case GVSP_PIX_BAYBG12_PACKED:
    case GVSP_PIX_BAYGR16:
    case GVSP_PIX_BAYRG16:
    case GVSP_PIX_BAYGB16:
    case GVSP_PIX_BAYBG16:
        type = "Bayer";
        break;
        // RGB Packed buffer format defines
    case GVSP_PIX_RGB8_PACKED:
    case GVSP_PIX_BGR8_PACKED:
    case GVSP_PIX_RGBA8_PACKED:
    case GVSP_PIX_BGRA8_PACKED:
    case GVSP_PIX_RGB10_PACKED:
    case GVSP_PIX_BGR10_PACKED:
    case GVSP_PIX_RGB12_PACKED:
    case GVSP_PIX_BGR12_PACKED:
    case GVSP_PIX_RGB16_PACKED:
    case GVSP_PIX_RGB10V1_PACKED:
    case GVSP_PIX_RGB10V2_PACKED:
    case GVSP_PIX_RGB12V1_PACKED:
    case GVSP_PIX_RGB565_PACKED:
    case GVSP_PIX_BGR565_PACKED:
        type = "RGB Packed";
        break;
        // YUV Packed buffer format defines
    case GVSP_PIX_YUV411_PACKED:
    case GVSP_PIX_YUV422_PACKED:
    case GVSP_PIX_YUV422_YUYV_PACKED:
    case GVSP_PIX_YUV444_PACKED:
        type = "YUV Packed";
        break;
        // RGB Planar buffer format defines
    case GVSP_PIX_RGB8_PLANAR:
    case GVSP_PIX_RGB10_PLANAR:
    case GVSP_PIX_RGB12_PLANAR:
    case GVSP_PIX_RGB16_PLANAR:
        type = "RGB Planar";
        break;
    default:
        type = "Unknow";

    }

    return QString("%0 %1 bits %2")
            .arg(color==GVSP_PIX_MONO ? "Monochrome" : color==GVSP_PIX_COLOR ? "Couleur" : color==GVSP_PIX_CUSTOM ? "Custom" : "Unknow")
            .arg(GVSP_PIX_PIXEL_SIZE(pixel))
            .arg(type);
}
}


GvspWidget::GvspWidget(QWidget *parent)
    : QWidget(parent)
{}

void GvspWidget::visualizeReceiver(QSharedPointer<Jgv::Gvsp::Receiver> receiverPtr, int periode)
{
    if (!receiverPtr) return;

    QLineEdit *transmitterIP = new QLineEdit;
    transmitterIP->setFocusPolicy(Qt::NoFocus);
    transmitterIP->setReadOnly(true);
    transmitterIP->setText(QString("%0 : %1").arg(QHostAddress(receiverPtr->parameters().transmitterIP).toString()).arg(receiverPtr->parameters().transmitterPort));

    QLineEdit *receiverIP = new QLineEdit;
    receiverIP->setFocusPolicy(Qt::NoFocus);
    receiverIP->setReadOnly(true);
    receiverIP->setText(QString("%0 : %1").arg(QHostAddress(receiverPtr->parameters().receiverIP).toString()).arg(receiverPtr->parameters().receiverPort));

    QLineEdit *socketType = new QLineEdit;
    socketType->setFocusPolicy(Qt::NoFocus);
    socketType->setReadOnly(true);
    Jgv::Gvsp::SocketType st = receiverPtr->parameters().socketType;
    socketType->setText(st==Jgv::Gvsp::SocketType::Classic?trUtf8("UDP classic"):st==Jgv::Gvsp::SocketType::RingBuffer?trUtf8("raw socket(ring buffer)"):trUtf8("No active socket"));

    QLineEdit *size = new QLineEdit;
    size->setFocusPolicy(Qt::NoFocus);
    size->setReadOnly(true);

    QLineEdit *pixel = new QLineEdit;
    pixel->setFocusPolicy(Qt::NoFocus);
    pixel->setReadOnly(true);

    QComboBox *timestampSrc = new QComboBox;
    timestampSrc->addItem(trUtf8("Transmitter GigE Vision"), Jgv::Gvsp::enumType(Jgv::Gvsp::TimestampSource::GEVTransmitter));
    timestampSrc->addItem(trUtf8("correlator Timestamp"), Jgv::Gvsp::enumType(Jgv::Gvsp::TimestampSource::TimestampDate));
    timestampSrc->addItem(trUtf8("System clock"), Jgv::Gvsp::enumType(Jgv::Gvsp::TimestampSource::SystemClock));
    int current = timestampSrc->findData(Jgv::Gvsp::enumType(receiverPtr->parameters().timestampSrc));
    timestampSrc->setCurrentIndex(current);
    connect(timestampSrc, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [timestampSrc, receiverPtr]() {
        receiverPtr->setTimestampSource(static_cast<Jgv::Gvsp::TimestampSource>(timestampSrc->currentData().toInt()));
    });

    QLineEdit *frequency = new QLineEdit;
    frequency->setFocusPolicy(Qt::NoFocus);
    frequency->setReadOnly(true);
    frequency->setText(QString("%0 Hz").arg(receiverPtr->parameters().timestampFrequency));


    QLineEdit *timestamp = new QLineEdit;
    timestamp->setFocusPolicy(Qt::NoFocus);
    timestamp->setReadOnly(true);

    QCheckBox *resend = new QCheckBox;
    resend->setChecked(receiverPtr->parameters().resend);
    connect(resend, &QCheckBox::toggled, [resend, receiverPtr](bool checked) {
        receiverPtr->setResendActive(checked);
    });

    QFormLayout *layout = new QFormLayout;
    layout->addRow(trUtf8("Transmitter Address"), transmitterIP);
    layout->addRow(trUtf8("Address of receiver"), receiverIP);
    layout->addRow(trUtf8("Active socket type"), socketType);
    layout->addRow(trUtf8("Image Size"), size);
    layout->addRow(trUtf8("Pixel Format"), pixel);
    layout->addRow(trUtf8("Source of dates"), timestampSrc);
    layout->addRow(trUtf8("Transmitter Clock"), frequency);
    layout->addRow(trUtf8("Timestamp"), timestamp);
    layout->addRow(trUtf8("Management of lost packages"), resend);

    QTimer *timer = new QTimer(this);
    QObject::connect(timer, &QTimer::timeout, [receiverPtr, size, pixel, timestamp]() {
        uint32_t width = receiverPtr->parameters().geometry.width;
        uint32_t height = receiverPtr->parameters().geometry.height;
        size->setText(QString("%0 x %1").arg(width).arg(height));

        uint32_t pixelFormat = receiverPtr->parameters().geometry.pixelFormat;
        pixel->setText(pixelToString(pixelFormat));

        QDateTime zero;
        zero.setTime_t(0);
        timestamp->setText(zero.addMSecs(receiverPtr->statistics().lastTimestamp / 1000000).toUTC().toString("hh:mm:ss.zzz"));
    });

    setLayout(layout);

    timer->start(periode);
}


