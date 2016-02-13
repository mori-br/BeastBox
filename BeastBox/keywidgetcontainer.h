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
#ifndef KEYWIDGETCONTAINER_H
#define KEYWIDGETCONTAINER_H

#include <QWidget>
#include <QHBoxLayout>

#include "keywidget.h"

class KeyWidgetContainer : public QWidget
{
    Q_OBJECT
public:
    explicit KeyWidgetContainer(QWidget *parent = 0);

    inline KeyWidget *getKeyCap() { return _keyCap; }
    inline int getId() { return _id; }
    inline void setId(int id) { _id = id; }

signals:
    void startListen(KeyWidgetContainer *key);
    void stopListen(KeyWidgetContainer *key);

public slots:
    void onStartListen();
    void onStopListen();

protected:
    int       _id;
    KeyWidget *_keyCap;
};

#endif // KEYWIDGETCONTAINER_H
