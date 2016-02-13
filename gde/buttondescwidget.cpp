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
#include "buttondescwidget.h"
#include <QHBoxLayout>

#define CONTROL_HEIGHT 40

ButtonDescWidget::ButtonDescWidget(QWidget *parent) :
    QWidget(parent)
    , _id(0)
{
    setMinimumHeight(CONTROL_HEIGHT);

    QHBoxLayout *layoutH = new QHBoxLayout( this );
    layoutH->setMargin(0);
    layoutH->setSpacing(5);

    layoutH->addWidget( &_lblPic );
    layoutH->addWidget( &_ads );
    layoutH->addWidget( &_edtDesc );

    _edtDesc.setMinimumWidth(100);
    _edtDesc.setMaximumHeight(30);
    _edtDesc.setStyleSheet("QLineEdit { background-color: #222; background: #222; } ");
    _edtDesc.setFrame(false);

    QFont font;
    font.setPointSize(11);
    _edtDesc.setFont(font);
}

void ButtonDescWidget::setup(int id, const QString &picture, KeyType type)
{
    setup(id, picture, type, false, "");
}

void ButtonDescWidget::setup(int id, const QString &picture, KeyType type, bool ads)
{
    setup(id, picture, type, ads, "");
}

void ButtonDescWidget::setup(int id, const QString &picture, KeyType type, const QString &description)
{
    setup(id, picture, type, false, description);
}

void ButtonDescWidget::setup(int id, const QString &picture, KeyType type, bool ads, const QString &description)
{
    _id = id;
    _type = type;
    _lblPic.setPixmap(QPixmap(picture));
    _edtDesc.setText(description);
    _lblPic.setFocusPolicy(Qt::NoFocus);
    _ads.setChecked(ads);
    _ads.setProperty("pskid", _id);
}

int ButtonDescWidget::getHeight()
{
    return CONTROL_HEIGHT;
}

