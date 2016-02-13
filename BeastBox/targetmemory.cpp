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
#include "targetmemory.h"

Sector::Sector()
    : _start(0)
    , _end(0)
    , _pagesize(0)
    , _type(0)
{
}

Sector::Sector( const Sector* other )
    : _start(other->_start)
    , _end(other->_end)
    , _pagesize(other->_pagesize)
    , _type(other->_type)
{
}

Sector& Sector::operator=( const Sector& other )
{
    _start = other._start;
    _end = other._end;
    _pagesize = other._pagesize;
    _type = other._type;

    return *this;
}

///////////////////////////////////////////////////////
/// \brief TargetMemory::TargetMemory
///////////////////////////////////////////////////////
TargetMemory::TargetMemory()
{
}

TargetMemory::~TargetMemory()
{
    destroy();
}

void TargetMemory::add(Sector *param)
{
    _sectors.append(param);
}

void TargetMemory::destroy()
{
    for (int i = 0; i < _sectors.size(); ++i)
    {
        Sector *sector = _sectors.at(i);
        delete sector;
    }

    _sectors.clear();
}

Sector *TargetMemory::find(uint32_t address)
{
    for(int i = 0; i < _sectors.size(); ++i)
    {
        Sector *s = _sectors.at(i);

        if (s->_start <= address && s->_end >= address)
            return s;
    }

    return NULL;
}
