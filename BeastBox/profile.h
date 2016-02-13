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
#ifndef PROFILE_H
#define PROFILE_H

#include <../common/ps.h>
#include <../common/profiledata.h>
#include <QString>
#include <QSharedPointer>
#include <QMutex>
#include <QByteArray>

#include "hidhandler.h"

#include <vector>
#include <memory>

#ifdef _MSC_VER
#pragma pack(1)
#endif

typedef struct SWITCHER_
{
    uint16_t    key;
    ConsoleType console;

} SWITCHER;

////////////////////////////////////////////////////////////////////////

class Profile
{
public:
    Profile();
    Profile(const Profile &profile);

    bool load(QDataStream &in);
    bool save(QDataStream &out);

    void loadKey(KEYINFO *k, QDataStream &in);
    void saveKey(const KEYINFO *k, QDataStream &out);

    void copy(const Profile &profile);
    void clone(const Profile *profile);

    void exportData(ProfileData *data);
    void importData(const ProfileData *data);

    inline QString getName() const { return _name; }
    inline void setName(const QString &name) { _name = name; }

    inline QString getDescription() const { return _description; }
    inline void setDescription(const QString &desc) { _description = desc; }

    inline QColor getColor() const { return _color; }
    inline void setColor(const QColor &color) { _color = color; }

    inline bool isEnabled() const { return _enabled; }
    inline void setEnabled(bool enabled) { _enabled = enabled; }
    inline void toggleEnabled() { _enabled = !_enabled; }

    inline ConsoleType getConsoleType() const { return _console; }
    inline void setConsoleType(ConsoleType console) { _console = console; }

    inline GameDef *getGameDef() { return &_game; }
    void setGameDef(const GameDef *game);

    inline bool isUseStick() const { return _useStick; }
    inline void setUseStick(bool enabled) { _useStick = enabled; }

//    inline DevInfo *getDevicesInfo() { return &_devInfo; }

    inline KEYINFO *getSwitchProfileKey()   { return &_switchProfile; }
    inline KEYINFO *getSensitivityKey()     { return &_switchSensitivity; }

    void setSwitchProfileKey(KEYINFO *k);
    void setSensitivityKey(KEYINFO *k);

    KEYINFO *getKey(int idx);
    void setKey(int idx, KEYINFO *k);

    inline float getJoyDeadzone() const { return _joyDeadzone; }
    inline void setJoyDeadzone(float value) { _joyDeadzone = value; }

    void setJoyKey(JoyKeyDir direction, KEYINFO *k);
    KEYINFO *getJoyKey(JoyKeyDir direction);

    inline QList< QSharedPointer<KEYINFO> > *getKeys() { return &_keys; }

    inline float getDeadZone() const { return _deadZone; }
    inline void setDeadZone(float value) { _deadZone = value; }

    inline int getDeadZoneType() const { return _deadZoneType; }
    inline void setDeadZoneType(int value) { _deadZoneType = value; }

    inline float getTranslationExponent() const { return _translationExponent; }
    inline void setTranslationExponent(float value) { _translationExponent = value; }

    inline float getYXRatio() const { return _YXRatio; }
    inline void setYXRatio(float value) { _YXRatio = value; }

    inline float getSmoothness() const { return _smoothness; }
    inline void setSmoothness(float value) { _smoothness = value; }

    inline float getDiagonalDampen() const { return _diagonalDampen; }
    inline void setDiagonalDampen(float value) { _diagonalDampen = value; }

    inline float getSensitivityPrimary() const { return _sensitivityPrimary; }
    inline void setSensitivityPrimary(float value) { _sensitivityPrimary = value; }

    inline float getSensitivitySecondary() const { return _sensitivitySecondary; }
    inline void setSensitivitySecondary(float value) { _sensitivitySecondary = value; }

    inline int getInputUpdateFrequency() const { return _inputUpdateFrequency; }
    inline void setInputUpdateFrequency(int value) { _inputUpdateFrequency = value; }

    inline bool isSmoothEnabled() const { return _smoothEnable; }
    inline void setSmoothEnable(bool value) { _smoothEnable = value; }

    inline bool isSecondarySensEnabled() const { return _useSecondarySensEnabled; }
    inline void setSecondarySensEnable(bool value) { _useSecondarySensEnabled = value; }

    static void debugProfile(ProfileData *profile);

    bool validate(QList< KEYINFO *> *list, const KEYINFO *pKey, KEYINFO *keyUsed=NULL);

    uint32_t getCRC();
    void debug();

private:
    QString         _name;
    QString         _description;
    QColor          _color;
    GameDef         _game;
    bool            _enabled;
    ConsoleType     _console;

    QList< QSharedPointer<KEYINFO> > _keys;

    KEYINFO         _joykeys[4];
    KEYINFO         _switchProfile;
    KEYINFO         _switchSensitivity;
    bool            _useStick;
//DevInfo         _devInfo;
    float           _joyDeadzone;

    int             _deadZoneType;
    float           _deadZone;
    float           _translationExponent;
    float           _YXRatio;
    float           _smoothness;
    float           _diagonalDampen;
    float           _sensitivityPrimary;
    float           _sensitivitySecondary;
    int             _inputUpdateFrequency;
    bool            _smoothEnable;
    bool            _useSecondarySensEnabled;
};

///////////////////////////////////////////////////////////////////////////////

class ProfileStream
{
public:
    static ProfileStream* instance()
    {
        static QMutex mutex;
        if(_self.data() == NULL)
        {
            mutex.lock();

            if(_self.data() == NULL)
                _self = QSharedPointer<ProfileStream>(new ProfileStream);

            mutex.unlock();
        }

        return _self.data();
    }

    bool load(const QString &filename, LoadError *err);
    bool save();

    Profile *add(const ProfileData *data);
    Profile *add(const Profile &profile);

    bool isNameValid(const QString &name);

    inline QList< QSharedPointer<Profile> > *profiles() { return &_profiles; }

    bool exportData(QList< QSharedPointer<ProfileData> > *list);
    bool importData(QList< QSharedPointer<ProfileData> > *list);

    bool exportToByteArray(std::vector< uint8_t* > *list);

    bool importFromByteArray(QList< QSharedPointer<QByteArray> > *list);

//    inline KEYINFO *getGlobalProfileSwitcherKey() { return &_gblSwitchProfile; }
//    inline void setGlobalProfileSwitcherKey(KEYINFO *src) { _gblSwitchProfile.copy(src); }

    inline QList< QSharedPointer<SWITCHER> > *getProfileSwitcherList() { return &_shortcutkeys; }

    Profile *getProfileByName(const QString &name);

    int getProfileIndexByPtr(const Profile *profile) const;

private:
    ProfileStream();

//    KEYINFO _gblSwitchProfile;

    QString _filename;

    // Kepp all profile change switch
    QList< QSharedPointer<SWITCHER> > _shortcutkeys;

    QList< QSharedPointer<Profile> > _profiles;

    static QSharedPointer<ProfileStream> _self;
};


#endif // PROFILE_H
