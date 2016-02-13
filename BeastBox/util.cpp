/*

This file is a part of BeastBox.

Copyright (C) 2016 Marcos Mori de Siqueira. <mori.br@gmail.com>
website: http://softfactory.com.br

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program; if not, please visit www.gnu.org.
*/
#include "util.h"

#include <QCoreApplication>
#include <QDebug>

Util::Util()
{
}

bool Util::getApplicationPath(QString &path)
{
    path = QCoreApplication::applicationFilePath();
    int pos = path.lastIndexOf('/');
    if(pos > 0)
    {
        path = path.mid(0, pos);
        path += QString("/");
        return true;
    }

    return false;
}

void Util::LOG(QByteArray *arr, const char *__msg, ...)
{
    char buffer[512] = {0};
    va_list ap;
    va_start (ap, __msg);
    vsprintf (buffer, __msg, ap);
    va_end (ap);

    arr->append(buffer);
}

void Util::LOG_ARRAY(const uint8_t *data, int size)
{
    QByteArray mm;
    QString ss = "";

    for(int i = 0; i < size; ++i)
    {
        Util::LOG(&mm, "%02X ", data[i]);
    }

    for(int i = 0; i < mm.size(); ++i)
        ss += mm.at(i);

    qDebug() << ss.toLatin1().data();
}
