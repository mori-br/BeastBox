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
#ifndef KEYWIDGET_H
#define KEYWIDGET_H

#include <QWidget>
#include <QLabel>

#include "hidhandler.h"
#include "clickablelabel.h"

class KeyWidget : public QWidget
{
    Q_OBJECT
public:
    explicit KeyWidget(QWidget *parent = 0);

    void setInfo(KEYINFO *ki, bool update=false);
    inline KEYINFO *getInfo() { return &_keyInfo; }

signals:
    void startListen();
    void stopListen();

public slots:
    void onClicked();
    void focusChanged(QWidget *in, QWidget *out);

protected:
    QLabel         *_image;
    ClickableLabel *_key;
    KEYINFO        _keyInfo;

    void paintEvent(QPaintEvent * event);
    void keyPressEvent(QKeyEvent *event);
};

#endif // KEYWIDGET_H
