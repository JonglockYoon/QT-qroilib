/*
 * mapwriter.h
 * Copyright 2008-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2010, Dennis Honeyman <arcticuno@gmail.com>
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

#include "roimap.h"
#include "roid_global.h"
#include "savefile.h"

#include <QString>

class QIODevice;

namespace Qroilib {

class RoiMap;

class RoiWriterPrivate;

/**
 * A QXmlStreamWriter based writer for the TMX and TSX formats.
 */
class ROIDSHARED_EXPORT RoiWriter
{
public:
    RoiWriter();
    ~RoiWriter();

    /**
     * Writes a TMX roimap to the given device.
     *
     * Error checking will need to be done on the \a device after calling this
     * function.
     */
    bool writeRoiGroupStart(const RoiMap *roimap, const QString &fileName);
    bool writeRoiGroup(const RoiMap *roimap);
    bool writeRoiGroupEnd();

    /**
     * Writes a TMX roimap to the given \a fileName.
     *
     * Returns false and sets errorString() when reading failed.
     * \overload
     */
    bool writeRoi(const RoiMap *roimap, const QString &fileName);

    /**
     * Returns the error message for the last occurred error.
     */
    QString errorString() const;

    /**
     * Sets whether the DTD reference is written when saving the roimap.
     */
    void setDtdEnabled(bool enabled);
    bool isDtdEnabled() const;
    void setPath(QString _path);

private:
    SaveFile *file;
    QString dirpath;
    Q_DISABLE_COPY(RoiWriter)

    RoiWriterPrivate *d;
};

} // namespace Qroilib
