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
#include "keylinewidget.h"

#include<QHBoxLayout>
#include<QPixmap>
#include<QDebug>

#define HEIGHT  50

KeyLineWidget::KeyLineWidget(QWidget *parent)
    : QWidget(parent)
    , _id(0)
{
    setMinimumHeight(HEIGHT);

    setStyleSheet("QWidget { background-color: #111; } ");

    _image  = new QLabel(this);
    _desc   = new QLabel(this);
    _keyCap = new KeyWidget(this);

    QHBoxLayout *layoutH = new QHBoxLayout( this );
    layoutH->setContentsMargins(QMargins(0, 0, 5, 0));
    layoutH->setSpacing(0);

    layoutH->addWidget( _image );
    layoutH->addWidget( _desc );
    layoutH->addWidget( _keyCap );

    _desc->setFont(QFont("Lucida", 12));
    _desc->setMinimumWidth(200);
    _desc->setMaximumWidth(200);
    _desc->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    _desc->setStyleSheet("QWidget { color: #DDD; padding-right:10; } QWidget:disabled { color: #888; }");

    connect(_keyCap, SIGNAL(startListen()), this, SLOT(onStartListen()));
    connect(_keyCap, SIGNAL(stopListen()), this, SLOT(onStopListen()));
}

void KeyLineWidget::configure(int id, KeyType type, const QString &picture, const QString &description)
{
    _id = id;
    _type = type;
    _image->setPixmap(QPixmap(picture));
    _desc->setText(description);
}

int KeyLineWidget::getHeight()
{
    return HEIGHT + 6;
}

void KeyLineWidget::onStartListen()
{
    emit startListen(this);
}

void KeyLineWidget::onStopListen()
{
    emit stopListen(this);
}
