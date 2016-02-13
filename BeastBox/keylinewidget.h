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
#ifndef KEYLINEWIDGET_H
#define KEYLINEWIDGET_H

#include <QWidget>
#include <QLabel>

#include "keywidget.h"
#include "../common/ps.h"

class KeyLineWidget : public QWidget
{
    Q_OBJECT
public:
    explicit KeyLineWidget(QWidget *parent = 0);

    void configure(int id, KeyType type, const QString &picture, const QString &description);

    static int getHeight();

    inline KeyWidget *getKeyCap() { return _keyCap; }
    int getId() { return _id; }
    int getType() { return _type; }

signals:
    void startListen(KeyLineWidget *key);
    void stopListen(KeyLineWidget *key);

public slots:
    void onStartListen();
    void onStopListen();

protected:
    int       _id;
    QLabel    *_image;
    QLabel    *_desc;
    KeyWidget *_keyCap;
    KeyType _type;
};


#endif // KEYLINEWIDGET_H
