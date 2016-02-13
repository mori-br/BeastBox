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
#ifndef GAMECOMBOBOX_H
#define GAMECOMBOBOX_H

#include <QComboBox>

class GameComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit GameComboBox(QWidget *parent = 0);

signals:

public slots:
protected:
    void paintEvent(QPaintEvent *event);
};

#endif // GAMECOMBOBOX_H
