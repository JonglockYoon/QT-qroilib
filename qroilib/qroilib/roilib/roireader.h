/*
 * mapreader.h
 * Copyright 2008-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 *
 * This file is part of qroilib.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "roid_global.h"

#include <QImage>

#include "roiobject.h"

class QFile;

namespace Qroilib {

class RoiMap;

class RoiReaderPrivate;

/**
 * A fast QXmlStreamReader based reader for the TMX and TSX formats.
 */
class ROIDSHARED_EXPORT RoiReader
{
public:
    RoiReader(ParamTable *pParamTab);
    virtual ~RoiReader();

    /**
     * Reads a TMX roimap from the given device.
     *
     * Returns 0 and sets errorString() when reading failed.
     *
     * The caller takes ownership over the newly created roimap.
     */
    RoiMap *readRoi(QIODevice *device, const QString &path = QString());

    /**
     * Reads a TMX roimap from the given \a fileName.
     * \overload
     */
    RoiMap *readRoi(const QString &fileName);

    /**
     * Returns the error message for the last occurred error.
     */
    QString errorString() const;
    void setPath(QString _path);

protected:
    /**
     * Called for each \a reference to an external file. Should return the path
     * to be used when loading this file. \a mapPath contains the path to the
     * roimap that is currently being loaded.
     */
    QString resolveReference(const QString &reference,
                             const QString &mapPath);

private:
    QString dirpath;
    Q_DISABLE_COPY(RoiReader)

    friend class RoiReaderPrivate;
    RoiReaderPrivate *d;
};

} // namespace Qroilib
