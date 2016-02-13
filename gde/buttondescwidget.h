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
#ifndef BUTTONDESCWIDGET_H
#define BUTTONDESCWIDGET_H

#include "../common/ps.h"

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>

class ButtonDescWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ButtonDescWidget(QWidget *parent = 0);

    void setup(int id, const QString &picture, KeyType type, bool ads, const QString &description);
    void setup(int id, const QString &picture, KeyType type, const QString &description);
    void setup(int id, const QString &picture, KeyType type, bool ads);
    void setup(int id, const QString &picture, KeyType type);
    static int getHeight();

    inline void setDescription(const QString &text) { _edtDesc.setText(text); }
    inline QString getDescription() { return _edtDesc.text(); }

    inline int getId() { return _id; }
    inline KeyType getType() { return _type; }

    inline bool isADS() { return _ads.isChecked(); }
    inline void resetADS() { _ads.setChecked(false); }
    inline void setADS() { _ads.setChecked(true); }

    inline QCheckBox *getADSCheck() { return &_ads; }

signals:

public slots:

protected:
    int    _id;             // Button id
    QLabel _lblPic;         // Button picture
    QLineEdit _edtDesc;     // Button description
    QCheckBox _ads;         // Selected button is used as ADS
    KeyType _type;
};

#endif // BUTTONDESCWIDGET_H
