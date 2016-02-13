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
#include "keywidget.h"
#include "profile.h"

#include<QHBoxLayout>
#include<QLabel>
#include<QPixmap>
#include<QDebug>
#include<QPainter>
#include<QPaintEvent>
#include<QApplication>

#define LISTENNING          "Listening..."
#define NOKEY               "None"

struct KEYBDECODE_ {
    uint16_t from;
    QString  to;

} KEYBDECODE[] = {
    {0x04, "A"},{0x05, "B"},{0x06, "C"},{0x07, "D"},{0x08, "E"},{0x09, "F"},{0x0A, "G"},{0x0B, "H"},{0x0C, "I"},
    {0x0D, "J"},{0x0E, "K"},{0x0F, "L"},{0x10, "M"},{0x11, "N"},{0x12, "O"},{0x13, "P"},{0x14, "Q"},{0x15, "R"},
    {0x16, "S"},{0x17, "T"},{0x18, "U"},{0x19, "V"},{0x1A, "W"},{0x1B, "X"},{0x1C, "Y"},{0x1D, "Z"},{0x1E, "1"},
    {0x1F, "2"},{0x20, "3"},{0x21, "4"},{0x22, "5"},{0x23, "6"},{0x24, "7"},{0x25, "8"},{0x26, "9"},{0x27, "0"},
    {0x28, "Enter"},{0x29, "Esc"},{0x2A, "Backspace"},{0x2B, "Tab"},{0x2C, "Space"},{0x2D, "-"},{0x2E, "="},
    {0x2F, "{"},{0x30, "}"},{0x31, "|"},{0x32, "INT 2"},{0x33, ":"},{0x34, ","},{0x35, "~"},{0x36, "<"},{0x37, ">"},
    {0x38, "?"},{0x39, "Capslock"},{0x3A, "F1"},{0x3B, "F2"},{0x3C, "F3"},{0x3D, "F4"},{0x3E, "F5"},{0x3F, "F6"},
    {0x40, "F7"},{0x41, "F8"},{0x42, "F9"},{0x43, "F10"},{0x44, "F11"},{0x45, "F12"},{0x46, "PtrScr"},
    {0x47, "ScrollLock"},{0x48, "Pause"},{0x49, "Ins CP"},{0x4A, "Home CP"},{0x4B, "PgUp CP"},{0x4C, "Del CP"},
    {0x4D, "End CP"},{0x4E, "PgDn CP"},{0x4F, "Right CP"},{0x50, "Left CP"},{0x51, "Down CP"},{0x52, "Up CP"},
    {0x53, "NumLock"},{0x54, "/ KP"},{0x55, "* KP"},{0x56, "- KP"},{0x57, "+ KP"},{0x58, "Enter KP"},{0x59, "End KP"},
    {0x5A, "Down KP"},{0x5B, "PgDn KP"},{0x5C, "Left KP"},{0x5D, "5 KP"},{0x5E, "Right KP"},{0x5F, "Home KP"},{0x60, "Up KP"},
    {0x61, "PgUp KP"},{0x62, "Ins KP"},{0x63, "Del KP"},{0x64, "INT 1"},{0x65, "WinMenu"},{0x68, "F13"},{0x69, "F14"},
    {0x6A, "F15"},{0x6B, "F16"},{0x6C, "F17"},{0x6D, "F18"},{0x6E, "F19"},{0x6F, "F20"},{0x70, "F21"},{0x71, "F22"},
    {0x72, "F23"},{0x73, "F24"},{0x75, "Help"},{0x7A, "Undo"},{0x7B, "Cut"},{0x7C, "Copy"},{0x7D, "Paste"},
    {0x7F, "Mute"},{0x80, "VolumeUp"},{0x81, "VolumeDn"},{0x85, ", KP"},{0x87, "INT 3"},{0x89, "INT 4"},{0x97, "5 KP"},
    {0x9A, "SysRq"},{0x9C, "Clear"},{0xA3, "CrSel"},{0xA4, "ExSel"},{0xE0, "Ctrl Left"},{0xE1, "Shift Left"},{0xE2, "Alt Left"},
    {0xE3, "Win Left"},{0xE4, "Ctrl Right"},{0xE5, "Shift Right"},{0xE6, "Alt Right"},{0xE7, "Win Right"},
};


KeyWidget::KeyWidget(QWidget *parent)
    : QWidget(parent)
{
    setFixedHeight(36);
    setFixedWidth(128);

    setStyleSheet("QWidget { background-color: #111; }");

    QHBoxLayout *layoutH = new QHBoxLayout( this );
    layoutH->setMargin(1);
    layoutH->setSpacing(2);

    _image = new QLabel(this);
    _key   = new ClickableLabel(this);

    layoutH->addWidget( _key );
    layoutH->addWidget( _image );

    _image->setFixedHeight(34);
    _image->setFixedWidth(32);
    _image->setStyleSheet("background-color: #000; border: 0px; padding-left: 5px;");

    _key->setFixedHeight(34);
    _key->setFixedWidth(90);
    _key->setFont(QFont("Lucida", 12));
    _key->setText("None");
    _key->setStyleSheet("QLabel { background-color: #000; color: #EEE; border: 0px;} QLabel:disabled { color:#888; }");
    _key->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
    _key->setFocusPolicy(Qt::ClickFocus);

    memset(&_keyInfo, 0, sizeof(KEYINFO));

    QObject::connect(_key, SIGNAL(clicked()), this, SLOT(onClicked()));

    QObject::connect(QApplication::instance(),
                     SIGNAL(focusChanged(QWidget*, QWidget*)),
                     this,
                     SLOT(focusChanged(QWidget*, QWidget*)));
}

void KeyWidget::setInfo(KEYINFO *ki, bool update)
{
    QPixmap deviceImage;

    // No key defined yet
    if(ki == NULL || ki->key == 0)
    {
        _keyInfo.reset();

        if(ki != NULL)
        {
            _keyInfo.psk = ki->psk;
            _keyInfo.type = ki->type;
        }

        _key->setText(NOKEY);
        _image->clear();

        return;
    }

    if(!update)
    {
        _keyInfo.psk = ki->psk;
        _keyInfo.vid = ki->vid;
        _keyInfo.pid = ki->pid;
        _keyInfo.key = ki->key;
        _keyInfo.type = ki->type;
        _keyInfo.source = ki->source;
    }
    else
    {
        _keyInfo.vid = ki->vid;
        _keyInfo.pid = ki->pid;
        _keyInfo.key = ki->key;
        _keyInfo.source = ki->source;
    }

    QString text = QString().sprintf("%d", ki->key);

    // Translate mouse buttons
    if(ki->source == KEY_MAP_MOUSE)
    {
        switch(_keyInfo.key)
        {
            case 1:
                text = "Left";
                break;
            case 2:
                text = "Right";
                break;
            case 4:
                text = "Middle";
                break;
            case 8:
                text = "Forward";
                break;
            case 16:
                text = "Backward";
                break;
        }

        deviceImage.load(":/images/kmouse24.png");
    }
    // Try to translate keyboard keys
    else if(ki->source == KEY_MAP_KEYBOARD)
    {
        for(int j = 0; j < (int)(sizeof(KEYBDECODE)/sizeof(KEYBDECODE[0])); ++j)
        {
            if(ki->key == KEYBDECODE[j].from)
            {
                text = KEYBDECODE[j].to;
                break;
            }
        }

        deviceImage.load(":/images/kkeyb24.png");
    }
    // Otherwise put this image
    else if(ki->source == KEY_MAP_CONTROLLER)
        deviceImage.load(":/images/kjoy24.png");
    else if(ki->source == KEY_MAP_KEYPAD)
        deviceImage.load(":/images/kkeyb24.png");

    // Setup control
    _key->setText(text);
    _image->setPixmap(deviceImage);
}

void KeyWidget::onClicked()
{
    // On mouse click we start to read a key
    if(_key != NULL)
        _key->setText(LISTENNING);

    emit startListen();
}

void KeyWidget::paintEvent(QPaintEvent * event)
{
    QPainter painter(this);

    QRect r = event->rect().adjusted(0, 0, -1, -1);

    if(isEnabled())
    {
        QPen linePen(QColor::fromRgb(80,80,80), 1, Qt::SolidLine);
        painter.setPen(linePen);
    }
    else
    {
        QPen linePen(QColor::fromRgb(40,40,40), 1, Qt::SolidLine);
        painter.setPen(linePen);
    }

    painter.drawRect(r);
    painter.setPen(Qt::NoPen);
    QWidget::paintEvent(event);
}

void KeyWidget::focusChanged(QWidget *in, QWidget *out)
{
Q_UNUSED(out);

    // If control lost focus & text == listenning
    if(_key != NULL && in == _key && _key->text().compare(LISTENNING) == 0)
    {
        _keyInfo.reset();
        _key->setText(NOKEY);
        _image->clear();
    }

    emit stopListen();
}

void KeyWidget::keyPressEvent(QKeyEvent *event)
{
    // If ESC key was pressed & text == listenning
    if(event->key() == Qt::Key_Escape)
    {
        if(_key != NULL && _key->text().compare(LISTENNING) == 0)
        {
            _keyInfo.reset();
            _key->setText(NOKEY);
            _image->clear();
        }

        emit stopListen();
    }
}
