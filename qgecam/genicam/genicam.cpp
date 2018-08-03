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

#include "genicam.h"

#include <QString>
#include <QFile>

using namespace Jgv::GenICam;

QString Representation::toString(const QString &stdFeatureName, qint64 value)
{
    if (stdFeatureName == "GevMACAddress") {
        return QString("%0:%1:%2:%3:%4:%5")
                .arg((value & Q_UINT64_C(0x0000FF0000000000))>>0x28, 2 , 16, QLatin1Char('0'))
                .arg((value & Q_UINT64_C(0x000000FF00000000))>>0x20, 2 , 16, QLatin1Char('0'))
                .arg((value & Q_UINT64_C(0x00000000FF000000))>>0x18, 2 , 16, QLatin1Char('0'))
                .arg((value & Q_UINT64_C(0x0000000000FF0000))>>0x10, 2 , 16, QLatin1Char('0'))
                .arg((value & Q_UINT64_C(0x000000000000FF00))>>0x08, 2 , 16, QLatin1Char('0'))
                .arg((value & Q_UINT64_C(0x00000000000000FF)), 2 , 16, QLatin1Char('0')).toUpper();
    }
    if (stdFeatureName == "GevCurrentIPAddress"
            || stdFeatureName == "GevCurrentSubnetMask"
            || stdFeatureName == "GevCurrentDefaultGateway"
            || stdFeatureName == "GevPersistentIPAddress"
            || stdFeatureName == "GevPersistentSubnetMask"
            || stdFeatureName == "GevPersistentDefaultGateway"
            || stdFeatureName == "GevPrimaryApplicationIPAddress"
            || stdFeatureName == "GevMCDA"
            || stdFeatureName == "GevSCDA")
    {
        return QString("%0.%1.%2.%3")
                .arg((value & Q_UINT64_C(0x00000000FF000000))>>0x18)
                .arg((value & Q_UINT64_C(0x0000000000FF0000))>>0x10)
                .arg((value & Q_UINT64_C(0x000000000000FF00))>>0x08)
                .arg((value & Q_UINT64_C(0x00000000000000FF)));
    }
    if (stdFeatureName == "GevMCTT") {
        return QString("%0 ms").arg(value);
    }


    return QString::number(value);
}

QString Representation::toString(const QString &stdFeatureName, double value)
{
    if (stdFeatureName == "AcquisitionFrameRate") {
        return QString::fromUtf8("%0 Hz").arg(value, 0, 'f', 1);
    }
    if (stdFeatureName == "DeviceTemperature") {
        return QString::fromUtf8("%0 °").arg(value, 0, 'f', 1);
    }
    // par défaut on fixe la précison à 3 digits
    return QString("%0").arg(value, 0, 'f', 3);
}

QByteArray BootstrapXml::file()
{
    QFile file("://bootstrap.xml");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning("Can't open resources");
        return QByteArray();
    }
    return file.readAll();
}
