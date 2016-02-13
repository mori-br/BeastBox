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
#ifndef TARGETMEMORY_H
#define TARGETMEMORY_H

#include <stdint.h>
#include <QList>
#include <QString>

class Sector
{
public:
    Sector();
    Sector( const Sector* other );
    Sector& operator=( const Sector& other );

    uint32_t _start;
    uint32_t _end;
    uint32_t _pagesize;
    uint8_t  _type;
};

class TargetMemory
{
public:
    TargetMemory();
    ~TargetMemory();

    QString _name;

    QList<Sector *> _sectors;

    void add(Sector *param);
    void destroy();
    Sector *find(uint32_t address);
};

#endif // TARGETMEMORY_H
