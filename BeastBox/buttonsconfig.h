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
#ifndef BUTTONSCONFIG_H
#define BUTTONSCONFIG_H

#include <QComboBox>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QThread>

#include "../common/ps.h"
#include "hidhandler.h"
#include "keylinewidget.h"
#include "keywidgetcontainer.h"

class Profile;

class PsButton
{
private:
    PsButton(){}

public:
    static QString getButtonImagePath(uint16_t buttonId, int consoleType);
};

////////////////////////////////////////////////////////////////
/// \brief The ButtonsConfig class
///
class ButtonsConfig : public QScrollArea
{
    Q_OBJECT
public:
    explicit ButtonsConfig(QWidget *parent = 0);
    ~ButtonsConfig();

    void setPanel(QWidget *panel);
    void update(GameDef *game, int type);
    void clean();

    void loadkeys(const QList< QSharedPointer<KEYINFO> > *list, GameDef *game, int type);
    void savekeys(QList< QSharedPointer<KEYINFO> > *list);

    bool setupKey(KEYINFO *ki);

    void getKeyList(QList< KEYINFO *> *list);

    void updateItem(KEYINFO *ki);

    inline KeyLineWidget *getFocus() { return _focus; }

signals:

public slots:
    void onStartListen(KeyLineWidget *key);
    void onStopListen(KeyLineWidget *key);

protected:
    void _createLayout();

private:
    QList< QSharedPointer<KeyLineWidget> > _labels;

    QVBoxLayout    *_layoutV;
    QWidget        *_panel;
    KeyLineWidget  *_focus;
};

#endif // BUTTONSCONFIG_H
