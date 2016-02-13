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

#ifndef PS_H
#define PS_H

#include <stdint.h>
#include <QPixmap>
#include <QString>
#include <QMutex>

#define PS_BUTTON_SELECT   				0x00001
#define PS_BUTTON_L3   					0x00002
#define PS_BUTTON_R3   					0x00004
#define PS_BUTTON_START    				0x00008
#define PS_BUTTON_UP     				0x00010
#define PS_BUTTON_RIGHT					0x00020
#define PS_BUTTON_DOWN     				0x00040
#define PS_BUTTON_LEFT     				0x00080
#define PS_BUTTON_L2       				0x00100
#define PS_BUTTON_R2       				0x00200
#define PS_BUTTON_L1       				0x00400
#define PS_BUTTON_R1    				0x00800
#define PS_BUTTON_TRIANGLE 				0x01000
#define PS_BUTTON_CIRCLE   				0x02000
#define PS_BUTTON_CROSS   				0x04000
#define PS_BUTTON_SQUARE   				0x08000

// Don't forget that exists a conversion on firmware for those values
#define PS_BUTTON_TOUCH                 0x03
#define PS_BUTTON_OPTIONS				0x05
#define PS_BUTTON_SHARE                 0x07
#define PS_BUTTON_PS                    0x11

#define PS_BUTTON_STICK_UP              0x01111
#define PS_BUTTON_STICK_DOWN            0x02222
#define PS_BUTTON_STICK_RIGHT           0x04444
#define PS_BUTTON_STICK_LEFT            0x08888


#define BTN_COMMON_INFO_SIZE            15
#define BTN_PS4_INFO_SIZE               3
#define BTN_PS3_INFO_SIZE               2

typedef enum KeyType_
{
    KEY_TYPE_NORMAL= 0,
    KEY_TYPE_STICK,
    KEY_TYPE_PSBUTTON,
    KEY_TYPE_PROFILE,
    KEY_TYPE_ADS,
    KEY_TYPE_EXTRA,

} KeyType;

typedef enum ConsoleType_
{
    CONSOLE_PS3=0,
    CONSOLE_NONE,
    CONSOLE_PS4,

} ConsoleType;

typedef enum LoadError_
{
    LOAD_ERROR_FILENOTFOUND,
    LOAD_ERROR_SIGNATURE,
    LOAD_ERROR_VERSION,

} LoadError;

typedef struct _BTNINFO
{
    int id;
    const char *image;
    KeyType type;

} BTNINFO;

const BTNINFO BTN_COMMON_INFO[] = {
    {PS_BUTTON_TRIANGLE, ":/images/buttons/btn_triangle.png",KEY_TYPE_NORMAL},
    {PS_BUTTON_CIRCLE, ":/images/buttons/btn_circle.png",KEY_TYPE_NORMAL},
    {PS_BUTTON_SQUARE, ":/images/buttons/btn_square.png",KEY_TYPE_NORMAL},
    {PS_BUTTON_CROSS, ":/images/buttons/btn_cross.png",KEY_TYPE_NORMAL},

    {PS_BUTTON_UP, ":/images/buttons/btn_up.png",KEY_TYPE_NORMAL},
    {PS_BUTTON_DOWN, ":/images/buttons/btn_down.png",KEY_TYPE_NORMAL},
    {PS_BUTTON_LEFT, ":/images/buttons/btn_left.png",KEY_TYPE_NORMAL},
    {PS_BUTTON_RIGHT, ":/images/buttons/btn_right.png",KEY_TYPE_NORMAL},

    {PS_BUTTON_L1, ":/images/buttons/btn_l1.png",KEY_TYPE_NORMAL},
    {PS_BUTTON_L2, ":/images/buttons/btn_l2.png",KEY_TYPE_NORMAL},
    {PS_BUTTON_L3,":/images/buttons/btn_l3.png",KEY_TYPE_NORMAL},

    {PS_BUTTON_R1, ":/images/buttons/btn_r1.png",KEY_TYPE_NORMAL},
    {PS_BUTTON_R2, ":/images/buttons/btn_r2.png",KEY_TYPE_NORMAL},
    {PS_BUTTON_R3, ":/images/buttons/btn_r3.png",KEY_TYPE_NORMAL},

    {PS_BUTTON_PS, ":/images/buttons/btn_ps.png",KEY_TYPE_PSBUTTON},
};

const BTNINFO BTN_PS4_INFO[] = {
    {PS_BUTTON_OPTIONS, ":/images/buttons/btn_opt.png",KEY_TYPE_NORMAL},
    {PS_BUTTON_TOUCH, ":/images/buttons/btn_touch.png",KEY_TYPE_NORMAL},
    {PS_BUTTON_SHARE, ":/images/buttons/btn_share.png",KEY_TYPE_NORMAL},
};

const BTNINFO BTN_PS3_INFO[] = {
    {PS_BUTTON_SELECT, ":/images/buttons/btn_sel.png",KEY_TYPE_NORMAL},
    {PS_BUTTON_START, ":/images/buttons/btn_start.png",KEY_TYPE_NORMAL},
};

///////////////////////////////////////////////////////////////////////////

class ButtonDesc
{
public:
    inline QString getDescription() const { return _description; }
    inline void setDescription(const QString &desc) { _description = desc; }

    inline int getId() const { return _id; }
    inline void setId(int id) { _id = id; }

    inline KeyType getType() const { return _type; }
    inline void setType(KeyType type) { _type = type; }

private:
    int _id;
    KeyType _type;
    QString _description;
};


class GameDef
{
public:
    explicit GameDef();
    explicit GameDef(const GameDef *source);

    bool load(QDataStream &in);
    bool save(QDataStream &out);

    void copy(const GameDef *source);

    inline QPixmap getImage() const { return _image; }
    inline QString getName() const { return _name; }
    inline QString getDescription() const { return _description; }

    inline void setImage(const QPixmap &pixmap) { _image = pixmap; }
    inline void setName(const QString &name) { _name = name; }
    inline void setDescription(const QString &desc) { _description = desc; }

    ButtonDesc* getButtonFromId(int id);

    inline QList< QSharedPointer<ButtonDesc> > *buttons() { return &_buttons; }

    inline bool isConsolePS4() { return _consolePS4; }
    inline bool isConsolePS3() { return _consolePS3; }

    inline void setConsolePS3(bool value) { _consolePS3 = value; }
    inline void setConsolePS4(bool value) { _consolePS4 = value; }

    inline uint32_t getCRC() const { return _crc32; }

    inline int getButtonCount() const { return _buttons.size(); }

    inline uint16_t getADSButton() const { return _adsButton; }
    inline void setADSButton(uint16_t btn) { _adsButton = btn; }

    GameDef *clone();

    void calcCRC32();

private:
    QPixmap     _image;
    QString     _name;
    QString     _description;
    bool        _consolePS3;
    bool        _consolePS4;
    uint32_t    _crc32;
    uint16_t    _adsButton;

    QList< QSharedPointer<ButtonDesc> > _buttons;
};

//////////////////////////////////////////////////////////////////////////

class GameStream
{
public:
    static GameStream* instance()
    {
        static QMutex mutex;
        if(_self.data() == NULL)
        {
            mutex.lock();

            if(_self.data() == NULL)
                _self = QSharedPointer<GameStream>(new GameStream);

            mutex.unlock();
        }

        return _self.data();
    }

    bool load(const QString &filename, LoadError *err);
    bool save(const QString &filename);
    bool save();
    bool reload();

    inline QList< QSharedPointer<GameDef> > *games() { return &_games; }

    inline QString getFilename() const { return _filename; }

    GameDef *getGameDefByCRC(uint32_t crc) const;

    int getGameDefIndexByCRC(uint32_t crc) const;

private:
    explicit GameStream();

    QString _filename;
    QList< QSharedPointer<GameDef> > _games;

    static QSharedPointer<GameStream> _self;
};


#endif // PS_H
