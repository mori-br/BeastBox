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
#include "ps.h"
#include "../common/crc32.h"

#include <QFile>
#include <QDebug>
#include <QFileInfo>

#define G_SIGNATURE     0xA0B0C0D0
#define G_VERSION       100

GameDef::GameDef()
    : _image(":/images/noimage.png")
    , _consolePS3(false)
    , _consolePS4(false)
    , _crc32(0)
{
}

GameDef::GameDef(const GameDef *source)
{
    copy(source);
}

void GameDef::copy(const GameDef *source)
{
    _image = source->_image;
    _name = source->_name;
    _description = source->_description;
    _consolePS3 = source->_consolePS3;
    _consolePS4 = source->_consolePS4;
    _crc32 = source->_crc32;
    _adsButton = source->_adsButton;

    for(int i = 0; i < source->_buttons.size(); ++i)
    {
        QSharedPointer<ButtonDesc> p = source->_buttons.at(i);

        ButtonDesc *bd = new ButtonDesc();
        bd->setId(p->getId());
        bd->setDescription(p->getDescription());

        _buttons.push_back(QSharedPointer<ButtonDesc>(bd));
    }
}

bool GameDef::load(QDataStream &in)
{
    qint32  id;
    QString s;
    qint8   type;

    in >> _image;
    in >> _name;
    in >> _description;
    in >> _consolePS3;
    in >> _consolePS4;
    in >> _adsButton;

    quint32 crc;
    in >> crc;
    _crc32 = crc;

    qint32 counter;
    in >> counter;
    for(int i = 0; i < counter; ++i)
    {
        in >> id;
        in >> s;
        in >> type;

        ButtonDesc *bd = new ButtonDesc();
        bd->setId(id);
        bd->setDescription(s);
        bd->setType((KeyType)type);

        _buttons.push_back(QSharedPointer<ButtonDesc>(bd));
    }

    return true;
}

bool GameDef::save(QDataStream &out)
{
    out << _image;
    out << _name;
    out << _description;
    out << _consolePS3;
    out << _consolePS4;
    out << _adsButton;
    out << (quint32)_crc32;

    out << (qint32)_buttons.size();
    for(int i = 0; i < _buttons.size(); ++i)
    {
        QSharedPointer<ButtonDesc> p = _buttons.at(i);
        out << (qint32)p->getId();
        out << p->getDescription();
        out << (qint8)p->getType();
    }

    return true;
}

void GameDef::calcCRC32()
{
    QByteArray arr;
    QDataStream out(&arr, QIODevice::WriteOnly);

    out << _image;
    out << _name;
    out << _description;
    out << _consolePS3;
    out << _consolePS4;
    out << _adsButton;

    out << (qint32)_buttons.size();
    for(int i = 0; i < _buttons.size(); ++i)
    {
        QSharedPointer<ButtonDesc> p = _buttons.at(i);
        out << (qint32)p->getId();
        out << p->getDescription();
        out << (qint8)p->getType();
    }

    _crc32 = CRC32::crc32(arr);

qDebug() << "CRC: " << _crc32;
}

////////////////////////////////////////////////////////////////

QSharedPointer<GameStream> GameStream::_self;

GameStream::GameStream()
{
}

bool GameStream::load(const QString &filename, LoadError *err)
{
    _filename = filename;

    if(!QFileInfo(filename).exists())
    {
        if(err != NULL)
            *err = LOAD_ERROR_FILENOTFOUND;

        return false;
    }

    QFile file(_filename);
    file.open(QIODevice::ReadOnly);
    QDataStream in(&file);

    // Read and check the header
    quint32 magic;
    in >> magic;
    if (magic != G_SIGNATURE)
    {
        if(err != NULL)
            *err = LOAD_ERROR_SIGNATURE;

        return false;
    }

    // Read the version
    qint32 version;
    in >> version;

    if (version != G_VERSION)
    {
        if(err != NULL)
            *err = LOAD_ERROR_VERSION;

        return false;
    }

    in.setVersion(QDataStream::Qt_5_3);

    // Read the data
    qint32 counter;
    in >> counter;
    for(int i = 0; i < counter; ++i)
    {
        GameDef *gd = new GameDef();
        gd->load(in);
        _games.push_back(QSharedPointer<GameDef>(gd));
    }

    return true;
}

bool GameStream::save(const QString &filename)
{
    _filename = filename;
    return save();
}

bool GameStream::save()
{
    QFile file(_filename);
    file.open(QIODevice::WriteOnly);
    QDataStream out(&file);

    // Write a header with a "magic number" and a version
    out << (quint32)G_SIGNATURE;
    out << (qint32)G_VERSION;

    out.setVersion(QDataStream::Qt_5_3);

    // Write the data
    out << (qint32)_games.size();
    for(int i = 0; i < _games.size(); ++i)
    {
        QSharedPointer<GameDef> p = _games.at(i);
        p->save(out);
    }

    return true;
}

ButtonDesc *GameDef::getButtonFromId(int id)
{
    for(int i = 0; i < _buttons.size(); ++i)
    {
        QSharedPointer<ButtonDesc> p = _buttons.at(i);
        if(p->getId() == id)
            return p.data();
    }

    return NULL;
}

GameDef *GameDef::clone()
{
    GameDef *clone = new GameDef();

    clone->_image = _image;
    clone->_name = _name + QString(" copy");
    clone->_description = _description;
    clone->_consolePS3 = _consolePS3;
    clone->_consolePS4 = _consolePS4;
    clone->_adsButton = _adsButton;

    for(int i = 0; i < _buttons.size(); ++i)
    {
        QSharedPointer<ButtonDesc> p = _buttons.at(i);
        ButtonDesc *bd = new ButtonDesc();
        bd->setId(p->getId());
        bd->setDescription(p->getDescription());
        bd->setType(p->getType());

        clone->_buttons.push_back(QSharedPointer<ButtonDesc>(bd));
    }

    return clone;
}

GameDef *GameStream::getGameDefByCRC(uint32_t crc) const
{
    for(int i = 0; i < _games.size(); ++i)
    {
        QSharedPointer<GameDef> p = _games.at(i);
        if(p->getCRC() == crc)
            return p.data();
    }

    return NULL;
}

int GameStream::getGameDefIndexByCRC(uint32_t crc) const
{
    for(int i = 0; i < _games.size(); ++i)
    {
        QSharedPointer<GameDef> p = _games.at(i);
        if(p->getCRC() == crc)
            return i;
    }

    return -1;
}

bool GameStream::reload()
{
    _games.clear();

    if(!_filename.isEmpty())
        return load(_filename, NULL);

    return false;
}
