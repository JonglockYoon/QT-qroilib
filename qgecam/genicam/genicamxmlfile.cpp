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

#include "genicamxmlfile.h"
#include "genicamxmlfile_p.h"
#include "inode_p.h"

#include "xmlhelper.h"
#include "category.h"
#include "qtiocompressor/qtiocompressor.h"

#include <QDomDocument>
#include <QStringList>
#include <QBuffer>
#include <QTextStream>
#include <QFile>
#include <QDataStream>
#include <QDebug>

using namespace Jgv::GenICam;

const char *DESCRIPTION = "RegisterDescription";
const char *GROUP = "Group";
const char *STRUCTREG = "StructReg";
const char *STRUCTENTRY = "StructEntry";

void GenICamXMLFilePrivate::createIndex()
{
    index.clear();

    QDomNodeList registerDescription = dom.elementsByTagName(DESCRIPTION);
    if (registerDescription.count() != 1) {
        qWarning("%s", qPrintable(QObject::trUtf8("The DOM is not a valid document")));
        return;
    }

    // parcourt les noeuds à la racine
    fillIndex(registerDescription.at(0));

    // parcourt les noeuds "Group"
    QDomNode n = registerDescription.at(0).firstChildElement(GROUP);
    while(!n.isNull()) {
        fillIndex(n);
        n = n.nextSiblingElement(GROUP);
    }
}

void GenICamXMLFilePrivate::fillIndex(const QDomNode &node)
{
    QDomElement child = node.firstChildElement();
    while (!child.isNull()) {
        // les noeuds GROUP
        if (child.tagName() == GROUP) {

        }
        // on traite structreg de manière séparée
        else if (child.tagName() == STRUCTREG) {
            // on va recopier les éléments communs du StructReg dans chaque StructEntry.
            QDomNodeList children = node.childNodes();

            QDomElement structEntry = child.firstChildElement(STRUCTENTRY);
            while (!structEntry.isNull()) {
                const QString name = structEntry.attribute(Inode::Attributes::Name);
                if (name.isEmpty()) {
                    qWarning("%s", qPrintable(QObject::trUtf8("Un noeud du DOM a un nom vide %0").arg(structEntry.tagName())));
                }
                else {
                    if (index.contains(name)) {
                        qWarning("%s", qPrintable(QObject::trUtf8("Le noeud %0 du DOM n'est pas unique").arg(name)));
                    }

                    // on insert les communs
                    for (int i=0; i<children.count(); ++i) {
                        if (children.item(i).toElement().tagName() != STRUCTENTRY) {
                            //structEntry.appendChild(children.item(i));
                        }
                    }

                    index.insert(name, structEntry);
                }
                structEntry = structEntry.nextSiblingElement(STRUCTENTRY);
            }
        }
        else {
            QString name = child.attribute(Inode::Attributes::Name);
            if (name.isEmpty()) {
                qWarning("%s", qPrintable(QObject::trUtf8("Un noeud du DOM a un nom vide %0").arg(child.tagName())));
            }
            else {
                if (index.contains(name)) {
                    qWarning("%s", qPrintable(QObject::trUtf8("Le noeud %0 du DOM n'est pas unique").arg(name)));
                }
                index.insert(name, child);
            }
        }

        child = child.nextSiblingElement();
    }

}

GenICamXMLFile::GenICamXMLFile()
    : d(new GenICamXMLFilePrivate)
{}

GenICamXMLFile::~GenICamXMLFile()
{}

void GenICamXMLFile::setFileName(const QString &fileName)
{
    d->fileName = fileName;
}

void GenICamXMLFile::setContent(const QByteArray &content)
{
    d->dom.clear();
    d->dom.setContent(content);
    d->createIndex();
}

void GenICamXMLFile::setZippedContent(const QByteArray &zippedContent)
{
    QByteArray zip = zippedContent;
    QBuffer buf(&zip);
    buf.open(QIODevice::ReadOnly);
    QDataStream s(&buf);
    s.setByteOrder(QDataStream::LittleEndian);

    /* Local file header:

            local file header signature     4 bytes  (0x04034b50)
            version needed to extract       2 bytes
            general purpose bit flag        2 bytes
            compression method              2 bytes
            last mod file time              2 bytes
            last mod file date              2 bytes
            crc-32                          4 bytes
            compressed size                 4 bytes
            uncompressed size               4 bytes
            file name length                2 bytes
            extra field length              2 bytes */

    quint32 signature;
    while (!s.atEnd()) {
        s >> signature;
        // on cherche la première signature
        if (signature == 0x04034b50) {
            quint32 crc, compSize, unCompSize;
            quint16 extractVersion, bitFlag, compMethod, modTime, modDate;
            quint16 nameLen, extraLen;

            s >> extractVersion >> bitFlag >> compMethod;
            s >> modTime >> modDate >> crc >> compSize >> unCompSize;
            s >> nameLen >> extraLen;

            // saute les champs à taille variable
            buf.seek(buf.pos() + nameLen + extraLen);

            // crée un buffer des données compressées
            QByteArray compData = buf.read(compSize);
            QBuffer compBuf(&compData);

            // decompresse
            QtIOCompressor compressor(&compBuf);
            compressor.setStreamFormat(QtIOCompressor::RawZipFormat);
            compressor.open(QIODevice::ReadOnly);
            QByteArray unCompData = compressor.readAll();
            compressor.close();

            // affecte au dom
            d->dom.clear();
            d->dom.setContent(unCompData);
            d->createIndex();

        }
    }
    buf.close();
}

void GenICamXMLFile::saveContent(const QString &path)
{
    if (!d->dom.isNull() && !d->fileName.isEmpty()) {
        QString fileName = d->fileName;
        if (fileName.endsWith(".zip")) {
            fileName.replace(".zip", ".xml");
        }
        QFile file(path +"/"+ fileName);
        if (file.open(QIODevice::WriteOnly)) {
            QTextStream stream(&file);
            d->dom.save(stream, 4);
            file.close();
        }
    }
}

QDomNode GenICamXMLFile::getRootChildNodeByNameAttribute(const QString &name) const
{
    return d->index.value(name);
}




