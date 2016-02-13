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
#include "buttonsconfig.h"
#include <../common/ps.h>

#include <QDebug>
#include <QKeyEvent>
#include <QLogger/QLogger.h>
#include <QMessageBox>

#include "profile.h"

using namespace QLogger;

#define NUM_COMMON_BUTTONS         15 // First 15 buttons are fixed all consoles have it
#define FIRST_PS4_BUTTON           15 // PS_BUTTON_OPTIONS
#define NUM_PS4_BUTTONS            3
#define FIRST_PS3_BUTTON           18 // PS_BUTTON_OPTIONS
#define NUM_PS3_BUTTONS            2

ButtonsConfig::ButtonsConfig(QWidget *parent)
    : QScrollArea(parent)
    , _layoutV(NULL)
    , _panel(NULL)
    , _focus(NULL)
{
}

ButtonsConfig::~ButtonsConfig()
{
    if(_layoutV != NULL)
    {
        delete _layoutV;
        _layoutV = NULL;
    }
}

void ButtonsConfig::_createLayout()
{
    if(_layoutV != NULL)
        delete _layoutV;

    _layoutV = new QVBoxLayout( this );
    _layoutV->setMargin(0);
    _layoutV->setSpacing(6);
    _layoutV->setContentsMargins(5,0,0,0);
    _layoutV->setSizeConstraint(QLayout::SetMinAndMaxSize);

    _panel->setLayout(_layoutV);
}

void ButtonsConfig::setPanel(QWidget *panel)
{
    _panel = panel;
    _panel->setMinimumHeight(NUM_COMMON_BUTTONS * KeyLineWidget::getHeight());
    _panel->setMaximumHeight(NUM_COMMON_BUTTONS * KeyLineWidget::getHeight());

    _createLayout();
}

void ButtonsConfig::clean()
{
    _createLayout();

//qDebug() << "layout Num ctrls: " << _layoutV->count();

    _labels.clear();
}

void ButtonsConfig::update(GameDef *game, int type)
{
    clean();

qDebug() << "Num buttons: " << game->buttons()->size();
qDebug() << "name:" << game->getName();

    if(game->buttons()->size() <= 0)
        return;

    for(int i = 0; i < MAX_KEY_BUTTON; ++i)
    {
        // Get PS button definition from Game definition
        QSharedPointer<ButtonDesc> button = game->buttons()->at(i);

        BTNINFO *info = NULL;

        if(i >= 0 && i < NUM_COMMON_BUTTONS)
            info = (BTNINFO *)&BTN_COMMON_INFO[i];
        else if(i >= FIRST_PS4_BUTTON && i < FIRST_PS4_BUTTON+NUM_PS4_BUTTONS)
            info = (BTNINFO *)&BTN_PS4_INFO[i-FIRST_PS4_BUTTON];
        else if(i >= FIRST_PS3_BUTTON && i < FIRST_PS3_BUTTON+NUM_PS3_BUTTONS)
            info = (BTNINFO *)&BTN_PS3_INFO[i-FIRST_PS3_BUTTON];

        // Extract the button image
        QString imgPath = info->image;
        int id          = info->id;
        KeyType kt      = info->type;

        if(id > 0)
        {
            // Configure this line and add to list
            KeyLineWidget *wl = new KeyLineWidget();
            _layoutV->addWidget(wl);

            connect(wl, SIGNAL(startListen(KeyLineWidget *)), this, SLOT(onStartListen(KeyLineWidget *)));
            connect(wl, SIGNAL(stopListen(KeyLineWidget *)), this, SLOT(onStopListen(KeyLineWidget *)));

            // Depending on selected console, some buttons must be hidded
            if(type == CONSOLE_PS3 && ((i >= FIRST_PS4_BUTTON) && i < (FIRST_PS4_BUTTON+NUM_PS4_BUTTONS)))
                wl->setVisible(false);

            if(type == CONSOLE_PS4 && (i >= FIRST_PS3_BUTTON))
                wl->setVisible(false);

            wl->configure(id, kt, imgPath, button->getDescription());
            _labels.push_back(QSharedPointer<KeyLineWidget>(wl));
        }
    }
}

void ButtonsConfig::onStartListen(KeyLineWidget *key)
{
    _focus = key;
//qDebug() << QString().sprintf("<< psk 0x%04X >>", _focus->getKeyCap()->getInfo()->psk);
}

void ButtonsConfig::onStopListen(KeyLineWidget *key)
{
Q_UNUSED(key);

    _focus = NULL;
}

bool ButtonsConfig::setupKey(KEYINFO *ki)
{
    if(_focus != NULL)
    {
        // We need to feed manually psk to use in comparison
        ki->psk = _focus->getKeyCap()->getInfo()->psk;
        _focus->getKeyCap()->setInfo(ki, true);
    }

    return true;
}

void ButtonsConfig::getKeyList(QList< KEYINFO *> *list)
{
    for(int i = 0; i < _labels.size(); ++i)
    {
        KEYINFO *ki1 = _labels.at(i)->getKeyCap()->getInfo();
        list->push_back(ki1);
    }
}

void ButtonsConfig::loadkeys(const QList< QSharedPointer<KEYINFO> > *list, GameDef *game, int type)
{
    clean();

    update(game, type);

    if(list->size() > 0)
    {
        for(int i = 0; i < _labels.size(); ++i)
        {
            QSharedPointer<KeyLineWidget> klw = _labels.at(i);

            if(list->size() > i)
            {
                // Update controls with information saved
                QSharedPointer<KEYINFO> ki = list->at(i);
                klw->getKeyCap()->setInfo(ki.data());

//qDebug() << QString().sprintf("1-psk 0x%04X, key 0x%04X", klw->getKeyCap()->getInfo()->psk, klw->getKeyCap()->getInfo()->key);
            }
//            else
//qDebug() << QString().sprintf("2->> psk 0x%04X, key 0x%04X", klw->getKeyCap()->getInfo()->psk, klw->getKeyCap()->getInfo()->key);
        }
    }
}

void ButtonsConfig::savekeys(QList< QSharedPointer<KEYINFO> > *list)
{
    list->clear();

    for(int i = 0; i < _labels.size(); ++i)
    {
        QSharedPointer<KeyLineWidget> klw = _labels.at(i);

        KEYINFO *ki = new KEYINFO;
        KEYINFO *k  = klw->getKeyCap()->getInfo();

        ki->copy(k);
        ki->psk = klw->getId();
        ki->type = klw->getType();

        list->push_back(QSharedPointer<KEYINFO>(ki));
    }
}

void ButtonsConfig::updateItem(KEYINFO *ki)
{
    for(int i = 0; i < _labels.size(); ++i)
    {
        KEYINFO *ki1 = _labels.at(i)->getKeyCap()->getInfo();
        if(ki1->psk == ki->psk)
        {
            if(ki->key == 0)
                _labels.at(i)->getKeyCap()->setInfo(NULL);
            else
                _labels.at(i)->getKeyCap()->setInfo(ki, true);

            break;
        }
    }
}

//////////////////////////////

QString PsButton::getButtonImagePath(uint16_t buttonId, int consoleType)
{
    for(int i = 0; i < BTN_COMMON_INFO_SIZE; ++i)
    {
        if(BTN_COMMON_INFO[i].id == buttonId)
            return BTN_COMMON_INFO[i].image;
    }

    if(consoleType == CONSOLE_PS4)
        for(int i = 0; i < BTN_PS4_INFO_SIZE; ++i)
        {
            if(BTN_PS4_INFO[i].id == buttonId)
                return BTN_PS4_INFO[i].image;
        }
    if(consoleType == CONSOLE_PS3)
        for(int i = 0; i < BTN_PS3_INFO_SIZE; ++i)
        {
            if(BTN_PS3_INFO[i].id == buttonId)
                return BTN_PS3_INFO[i].image;
        }

    return "";
}
