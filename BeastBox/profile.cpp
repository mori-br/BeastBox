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
#include "profile.h"
#include "util.h"
#include <../common/crc32.h>
#include <QFileInfo>
#include <QDebug>
#include <QMessageBox>
#include <QLogger/QLogger.h>

#define P_SIGNATURE     0xC0D0E0F0
#define P_VERSION       100

using namespace QLogger;

Profile::Profile()
    : _enabled(false)
    , _useStick(false)
    , _joyDeadzone(0.0)
    , _deadZoneType(0)
    , _deadZone(0.0)
    , _translationExponent(0.0)
    , _YXRatio(0.0)
    , _smoothness(0.0)
    , _diagonalDampen(0.0)
    , _sensitivityPrimary(0.0)
    , _sensitivitySecondary(0.0)
    , _inputUpdateFrequency(0)
    , _smoothEnable(false)
    , _useSecondarySensEnabled(false)
{
    _console = CONSOLE_PS4;

//    memset(&_devInfo, 0, sizeof(DevInfo));
    memset(&_switchProfile, 0, sizeof(KEYINFO));
    memset(&_switchSensitivity, 0, sizeof(KEYINFO));

    for(int i = 0; i < 4; ++i)
    {
        memset(&_joykeys[i], 0, sizeof(KEYINFO));
        _joykeys[i].type = KEY_TYPE_STICK;
    }

    _joykeys[JOY_KEY_UP].psk    = PS_BUTTON_STICK_UP;
    _joykeys[JOY_KEY_DOWN].psk  = PS_BUTTON_STICK_DOWN;
    _joykeys[JOY_KEY_LEFT].psk  = PS_BUTTON_STICK_LEFT;
    _joykeys[JOY_KEY_RIGHT].psk = PS_BUTTON_STICK_RIGHT;
}

Profile::Profile(const Profile &profile)
{
    copy(profile);
}

void Profile::copy(const Profile &profile)
{
    _name = profile._name;
    _description = profile._description;
    _color = profile._color;
    _enabled = profile._enabled;
    _console = profile._console;
    _game = profile._game;
//    _devInfo = profile._devInfo;
    _joyDeadzone = profile._joyDeadzone;
    _useStick = profile._useStick;

    _deadZoneType = profile._deadZoneType;
    _deadZone = profile._deadZone;
    _translationExponent = profile._translationExponent;
    _YXRatio = profile._YXRatio;
    _smoothness = profile._smoothness;
    _diagonalDampen = profile._diagonalDampen;
    _sensitivityPrimary = profile._sensitivityPrimary;
    _sensitivitySecondary = profile._sensitivitySecondary;
    _inputUpdateFrequency = profile._inputUpdateFrequency;
    _smoothEnable = profile._smoothEnable;
    _useSecondarySensEnabled = profile._useSecondarySensEnabled;

    memcpy(&_switchProfile, &profile._switchProfile, sizeof(KEYINFO));
    memcpy(&_switchSensitivity, &profile._switchSensitivity, sizeof(KEYINFO));

    for(int i = 0; i < profile._keys.size(); ++i)
    {
        QSharedPointer<KEYINFO> k = profile._keys.at(i);
        KEYINFO *k1 = new KEYINFO;
        k1->copy(k.data());
        _keys.push_back(QSharedPointer<KEYINFO>(k1));
    }

    for(int i = 0; i < 4; ++i)
        memcpy(&_joykeys[i], &profile._joykeys[i], sizeof(KEYINFO));
}

void Profile::loadKey(KEYINFO *k, QDataStream &in)
{
    in >> k->key;
    in >> k->psk;
    in >> k->source;
    in >> k->vid;
    in >> k->pid;
    in >> k->type;
}

bool Profile::load(QDataStream &in)
{
    int t;

    in >> _name;
    in >> _description;
    in >> t;
    in >> _enabled;
    in >> _color;

    _console = (ConsoleType)t;

    in >> _deadZoneType;
    in >> _deadZone;
    in >> _translationExponent;
    in >> _YXRatio;
    in >> _smoothness;
    in >> _diagonalDampen;
    in >> _sensitivityPrimary;
    in >> _sensitivitySecondary;
    in >> _inputUpdateFrequency;
    in >> _smoothEnable;

    in >> _joyDeadzone;

    in >> _useStick;

    in >> _useSecondarySensEnabled;

    in >> t;

    for(int i = 0; i < t; ++i)
    {
        KEYINFO *k = new KEYINFO;
        loadKey(k, in);
        _keys.push_back(QSharedPointer<KEYINFO>(k));
    }

    for(int i = 0; i < 4; ++i)
    {
        loadKey(&_joykeys[i], in);
    }

    loadKey(&_switchProfile, in);
    loadKey(&_switchSensitivity, in);

    return _game.load(in);
}

void Profile::saveKey(const KEYINFO *k, QDataStream &out)
{
    out << k->key;
    out << k->psk;
    out << k->source;
    out << k->vid;
    out << k->pid;
    out << k->type;
}

bool Profile::save(QDataStream &out)
{
    out << _name;
    out << _description;
    out << (int)_console;
    out << _enabled;
    out << _color;

    out << _deadZoneType;
    out << _deadZone;
    out << _translationExponent;
    out << _YXRatio;
    out << _smoothness;
    out << _diagonalDampen;
    out << _sensitivityPrimary;
    out << _sensitivitySecondary;
    out << _inputUpdateFrequency;
    out << _smoothEnable;

    out << _joyDeadzone;

    out << _useStick;
    out << _useSecondarySensEnabled;

    out << _keys.size();

    for(int i = 0; i < _keys.size(); ++i)
    {
        QSharedPointer<KEYINFO> k = _keys.at(i);
        saveKey(k.data(), out);
    }

    for(int i = 0; i < 4; ++i)
    {
        saveKey(&_joykeys[i], out);
    }

    saveKey(&_switchProfile, out);
    saveKey(&_switchSensitivity, out);

    return _game.save(out);
}

void Profile::setSwitchProfileKey(KEYINFO *k)
{
    if(k == NULL)
        _switchProfile.reset();
    else
        _switchProfile.copy(k);
}

void Profile::setSensitivityKey(KEYINFO *k)
{
    if(k == NULL)
        _switchSensitivity.reset();
    else
        _switchSensitivity.copy(k);
}

void Profile::setKey(int idx, KEYINFO *k)
{
    if(idx >= 0 && idx < _keys.size())
    {
        QSharedPointer<KEYINFO> k1 = _keys.at(idx);
        k1->copy(k);
    }
}

KEYINFO *Profile::getKey(int idx)
{
    if(idx >= 0 && idx < _keys.size())
        return _keys.at(idx).data();

    return NULL;
}

void Profile::setGameDef(const GameDef *game)
{
    _game.copy(game);
}

void Profile::setJoyKey(JoyKeyDir direction, KEYINFO *k)
{
    if(direction >= 0 && direction < 4)
        _joykeys[direction].copy(k);
}

KEYINFO *Profile::getJoyKey(JoyKeyDir direction)
{
    if(direction >= 0 && direction < 4)
        return &_joykeys[direction];

    return NULL;
}

void Profile::clone(const Profile *profile)
{
    copy(*profile);
    _name += " Clone";
}

void Profile::importData(const ProfileData *data)
{
    if(data == NULL)
        return;

    _name = data->name;

    _useStick = data->useStick == 1 ? true : false;
    _color = data->color;
    _joyDeadzone = data->joyDeadzone;
    _console = (ConsoleType) data->target;

    _deadZone = data->translation.deadZone;
    _deadZoneType = data->translation.deadZoneType;
    _translationExponent = data->translation.translationExponent;
    _YXRatio = data->translation.YXRatio;
    _smoothness = data->translation.smoothness ;
    _diagonalDampen = data->translation.diagonalDampen;
    _sensitivityPrimary = data->translation.sensitivityPrimary;

    _sensitivitySecondary = data->translation.sensitivitySecondary;
    _inputUpdateFrequency = data->translation.inputUpdateFrequency;
    _smoothEnable = data->translation.smoothEnable != 0 ? true : false;
    _useSecondarySensEnabled = data->translation.secondarySensEnable != 0 ? true :false;

    for(int i = 0; i < MAX_KEY_BUTTON; ++i)
    {
        KEYINFO *kk = new KEYINFO;
        kk->fromKey(&data->keys[i]);
        _keys.push_back(QSharedPointer<KEYINFO>(kk));
    }

    _switchProfile.fromKey(&data->switchProfile);

    _switchSensitivity.fromKey(&data->switchSensitivity);

    for(int i = 0; i < 4; ++i)
    {
        _joykeys[i].fromKey(&data->joykeys[i]);
    }
}

void Profile::exportData(ProfileData *data)
{
    if(data == NULL)
        return;

    memset(data, 0, sizeof(ProfileData));

    int size = _name.length() > 40 ? 40 : _name.length();
    memset(data->name, 0, 41);
    memcpy(data->name, _name.toLatin1().data(), size);

    data->useStick = _useStick ? 1 : 0;

    int r, g, b;
    _color.getRgb(&r, &g, &b);

    data->color = (r & 0xFF);
    data->color = data->color << 8 | (g & 0xFF);
    data->color = data->color << 8 | (b & 0xFF);

    data->joyDeadzone = _joyDeadzone;
    data->target = _console;

    //qDebug() << "data->target: " << data->target << " name: " << data->name;

    //qDebug() << QString().sprintf("COLOR: 0x%12X", data->color);

    data->translation.deadZone = _deadZone;
    data->translation.deadZoneType = _deadZoneType;
    data->translation.translationExponent = _translationExponent;
    data->translation.YXRatio = _YXRatio;
    data->translation.smoothness = _smoothness;
    data->translation.diagonalDampen =_diagonalDampen;
    data->translation.sensitivityPrimary = _sensitivityPrimary;
    data->translation.sensitivitySecondary = _sensitivitySecondary;
    data->translation.inputUpdateFrequency = CLAMP(_inputUpdateFrequency, 0, 0xFF);

    data->translation.smoothEnable = _smoothEnable ? 1 : 0;

    data->translation.secondarySensEnable = _useSecondarySensEnabled ? 1 : 0;

    for(int i = 0; i < _keys.size(); ++i)
    {
        if(i < MAX_KEY_BUTTON)
        {
            const QSharedPointer<KEYINFO> k = _keys.at(i);

            k->toKey(&data->keys[i]);
        }
    }

    _switchProfile.toKey(&data->switchProfile);
    data->switchProfile.type = KEY_TYPE_PROFILE;

data->translation.sensitivitySecondary = _sensitivitySecondary;

//qDebug() << "SECONDARY SENS " << data->switchSensitivity.ps3_key;
//qDebug() << "SECONDARY SENS " << data->switchSensitivity.key;
//qDebug() << "SECONDARY SENS " << data->switchSensitivity.source;
//qDebug() << "SECONDARY SENS " << data->switchSensitivity.type;
//qDebug() << "SECONDARY SENS " << data->switchSensitivity.vid;
//qDebug() << "SECONDARY SENS " << data->switchSensitivity.pid;
//qDebug() << "SECONDARY SENS " << data->translation.sensitivitySecondary;


    if(_switchSensitivity.psk > 0)
    {
        data->switchSensitivity.ps3_key = _switchSensitivity.psk;
        data->switchSensitivity.type = KEY_TYPE_ADS;
    }

//qDebug() << "SECONDARY " << _switchSensitivity.psk;
//qDebug() << "SECONDARY SENS " << data->switchSensitivity.ps3_key;

    for(int i = 0; i < 4; ++i)
    {
        _joykeys[i].toKey(&data->joykeys[i]);
        data->joykeys[i].type = KEY_TYPE_STICK;
    }

    data->joykeys[JOY_KEY_UP].ps3_key    = PS_BUTTON_STICK_UP;
    data->joykeys[JOY_KEY_DOWN].ps3_key  = PS_BUTTON_STICK_DOWN;
    data->joykeys[JOY_KEY_LEFT].ps3_key  = PS_BUTTON_STICK_LEFT;
    data->joykeys[JOY_KEY_RIGHT].ps3_key = PS_BUTTON_STICK_RIGHT;

//Util::LOG_ARRAY(data->bytes, PROFILE_SIZE);

    data->crc = 0;
    data->crc = CRC32::crc32(data->bytes, sizeof(ProfileData));

qDebug() << "crc: " << QString().sprintf("0x%08X", data->crc).toLatin1().data();

    //Profile::debugProfile(data);
}

/**
 * @brief Profile::validate
 * @param list - list with all keys
 * @param shortcutKey - key to check
 * @param keyUsed - key actually used
 * @return true if all right with key
 */
bool Profile::validate(QList< KEYINFO *> *list, const KEYINFO *pKey, KEYINFO *keyUsed)
{
    // Check key against common keys
    for(int i = 0; i < list->size(); ++i)
    {
        const KEYINFO *k = list->at(i);

        if(k->source == pKey->source && k->key == pKey->key && k->psk != pKey->psk && k->key > 0)
        {
qDebug() << "VALIDATION FAILED key: " << QString().sprintf("0x%04X", k->key).toLatin1().data();
qDebug() << "VALIDATION FAILED psk1: " << QString().sprintf("0x%04X", k->psk).toLatin1().data();
qDebug() << "VALIDATION FAILED psk2: " << QString().sprintf("0x%04X", pKey->psk).toLatin1().data();
qDebug() << "VALIDATION FAILED type: " << QString().sprintf("%d", pKey->type).toLatin1().data();

            if(keyUsed != NULL)
                *keyUsed = k;

            return false;
        }
    }

    // Do not validate switch profile against itself
    if(pKey->type != KEY_TYPE_PROFILE)
    {
        // Check against profile switch key
        if(_switchProfile.key == pKey->key && _switchProfile.source == pKey->source)
        {
qDebug() << "VALIDATION FAILED key1: " << QString().sprintf("0x%04X", _switchProfile.key).toLatin1().data();
qDebug() << "VALIDATION FAILED key2: " << QString().sprintf("0x%04X", pKey->key).toLatin1().data();

            if(keyUsed != NULL)
                *keyUsed = &_switchProfile;

            return false;
        }
    }

    // Check key against moviment keys
    for(int i = 0; i < 4; ++i)
    {
        if(_joykeys[i].key == pKey->key && _joykeys[i].source == pKey->source)
        {
            if(keyUsed != NULL)
                *keyUsed = &_joykeys[i];

            return false;
        }
    }

    return true;
}

uint32_t Profile::getCRC()
{
    ProfileData data;
    exportData(&data);
    return data.crc;
}

//////////////////////////////////////////////////////////////////////

QSharedPointer<ProfileStream> ProfileStream::_self;

ProfileStream::ProfileStream()
{
}

bool ProfileStream::load(const QString &filename, LoadError *err)
{
    _filename = filename;

    _profiles.clear();
    _shortcutkeys.clear();

    if(!QFileInfo(filename).exists())
    {
        if(err != NULL)
            *err = LOAD_ERROR_FILENOTFOUND;
        return false;
    }

    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly))
    {
        QLog_Error("Default", QString().sprintf("Error opening %s for read", _filename.toLatin1().data()));
        return false;
    }

    QDataStream in(&file);

    // Read and check the header
    quint32 magic;
    in >> magic;
    if (magic != P_SIGNATURE)
    {
        if(err != NULL)
            *err = LOAD_ERROR_SIGNATURE;

        return false;
    }

    // Read the version
    qint32 version;
    in >> version;

    if (version != P_VERSION)
    {
        if(err != NULL)
            *err = LOAD_ERROR_VERSION;

        return false;
    }

    in.setVersion(QDataStream::Qt_5_3);

    // Read the data
    qint32 counter;
    in >> counter;
    for(int i = 0; i < counter; ++i)
    {
        Profile *profile = new Profile();
        profile->load(in);
        _profiles.push_back(QSharedPointer<Profile>(profile));

        // Remember of profile switcher key
        SWITCHER *switcher = new SWITCHER;
        switcher->console = profile->getConsoleType();
        switcher->key = profile->getSwitchProfileKey()->key;

        _shortcutkeys.push_back(QSharedPointer<SWITCHER>(switcher));
    }

    return true;
}

bool ProfileStream::save()
{
    QFile file(_filename);
    if(!file.open(QIODevice::WriteOnly))
    {
        QLog_Error("Default", QString().sprintf("Error opening %s for write", _filename.toLatin1().data()));
        return false;
    }

    QDataStream out(&file);

    // Write a header with a "magic number" and a version
    out << (quint32)P_SIGNATURE;
    out << (qint32)P_VERSION;

    out.setVersion(QDataStream::Qt_5_3);

    // Write the data
    out << (qint32)_profiles.size();
    for(int i = 0; i < _profiles.size(); ++i)
    {
        QSharedPointer<Profile> p = _profiles.at(i);
        p->save(out);
    }

    return true;
}

Profile *ProfileStream::add(const Profile &profile)
{
    Profile *p = new Profile(profile);
    _profiles.push_back(QSharedPointer<Profile>(p));
    return p;
}

Profile *ProfileStream::add(const ProfileData *data)
{
    Profile *p = new Profile();
    p->importData(data);
    _profiles.push_back(QSharedPointer<Profile>(p));
    return p;
}

bool ProfileStream::isNameValid(const QString &name)
{
    for(int i = 0; i < _profiles.size(); ++i)
    {
        QSharedPointer<Profile> p = _profiles.at(i);
        if(name == p->getName())
            return false;
    }

    return true;
}

bool ProfileStream::exportData(QList< QSharedPointer<ProfileData> > *list)
{
    for(int i = 0; i < _profiles.size(); ++i)
    {
        QSharedPointer<Profile> p = _profiles.at(i);
        if(p->isEnabled())
        {
            ProfileData *profile = new ProfileData;
            p->exportData(profile);
            list->push_back(QSharedPointer<ProfileData>(profile));
        }
    }

    return true;
}

// The problem with import is that device just have a piece of data needed to build a profile....
// An update profile when in-game setup is working will be usefull
bool ProfileStream::importData(QList< QSharedPointer<ProfileData> > *list)
{
    for(int i = 0; i < list->size(); ++i)
    {
        QSharedPointer<ProfileData> p = list->at(i);

        QSharedPointer<Profile> profile( new Profile() );

        profile->importData(p.data());

        Profile *pp = getProfileByName(profile->getName());
        if(pp != NULL)
        {
            QLog_Debug("Default", QString().sprintf("profile %s found on list", profile->getName().toLatin1().data()));

            // Already exists, check if changed
            if(pp->getCRC() != profile->getCRC())
            {
                QLog_Debug("Default", "profile was changed, adding a copy");
                profile->setName(profile->getName().append("_1"));

                // Already generated?
                if(getProfileByName(profile->getName()) == NULL)
                {
                    QString s =
                            QObject::tr("profile %1 was changed on device, update the original "
                                        "profile with values changed and delete this one.").arg(pp->getName().toLatin1().data());

                    profile->setDescription(s);
                    _profiles.push_back(profile);
                }
            }
        }
        else
        {
            QLog_Debug("Default", "profile not found on list, adding new one");
            // If not found, add it
            profile->setDescription(QObject::tr("profile imported from device"));
            profile->setEnabled(true);
            _profiles.push_back(profile);
        }
    }

    return true;
}

bool ProfileStream::exportToByteArray(std::vector< uint8_t* > *list)
{
    QList< QSharedPointer<ProfileData> > l;
    if(exportData(&l))
    {
        for(int i = 0; i < l.size(); ++i)
        {
            const QSharedPointer<ProfileData> p = l.at(i);

QLog_Debug("Default", QString().sprintf("profile crc 0x%08X", p->crc));

            uint8_t *buffer = new uint8_t[PROFILE_SIZE];
            memcpy(buffer, (const char *)p->bytes, PROFILE_SIZE);
            list->push_back(buffer);
        }

        return true;
    }

    return false;
}

bool ProfileStream::importFromByteArray(QList< QSharedPointer<QByteArray> > *list)
{
    QList< QSharedPointer<ProfileData> > l;

    for(int i = 0; i < list->size(); ++i)
    {
        QSharedPointer<QByteArray> ba = list->at(i);
        QSharedPointer<ProfileData> p = QSharedPointer<ProfileData>(new ProfileData());

        memcpy(p->bytes, ba->constData(), ba->size());

        uint32_t crc1 = p->crc;
        p->crc = 0;
        uint32_t crc = CRC32::crc32(p->bytes, sizeof(ProfileData));

QLog_Debug("Default", QString().sprintf("CRC1 0x%08x CRC 0x%08x", crc1, crc));

        if(crc1 != crc)
        {
            QLog_Error("Default", "Importing profile, CRC dont match");
            continue; // try next one
        }

        QLog_Debug("Default", QString().sprintf("adding for %s", p->name));

        l.push_back(p);
    }

    return importData(&l);
}

Profile *ProfileStream::getProfileByName(const QString &name)
{
    for(int i = 0; i < _profiles.size(); ++i)
    {
        QSharedPointer<Profile> p = _profiles.at(i);
        if(p->getName().compare(name, Qt::CaseInsensitive) == 0)
            return p.data();
    }

    return NULL;
}

int ProfileStream::getProfileIndexByPtr(const Profile *profile) const
{
    for(int i = 0; i < _profiles.size(); ++i)
    {
        QSharedPointer<Profile> p = _profiles.at(i);
        if(p == profile)
            return i;
    }

    return -1;
}

void Profile::debug()
{
    ProfileData data;
    exportData(&data);

    Util::LOG_ARRAY(data.bytes, sizeof(ProfileData));
}

void Profile::debugProfile(ProfileData *profile)
{
    if(profile != NULL)
    {
        qDebug() << QString().sprintf("deadZone: %f", profile->translation.deadZone);
        qDebug() << QString().sprintf("translationExponent: %f", profile->translation.translationExponent);
        qDebug() << QString().sprintf("YXRatio: %f", profile->translation.YXRatio);
        qDebug() << QString().sprintf("smoothness: %f", profile->translation.smoothness);
        qDebug() << QString().sprintf("diagonalDampen: %f", profile->translation.diagonalDampen);
        qDebug() << QString().sprintf("sensitivityPrimary: %f", profile->translation.sensitivityPrimary);
        qDebug() << QString().sprintf("sensitivitySecondary: %f", profile->translation.sensitivitySecondary);
        qDebug() << QString().sprintf("inputUpdateFrequency: %d", profile->translation.inputUpdateFrequency);
        qDebug() << QString().sprintf("deadZoneType: %d", profile->translation.deadZoneType);
        qDebug() << QString().sprintf("smoothEnable: %d", profile->translation.smoothEnable);
        qDebug() << QString().sprintf("secondarySensEnable: %d", profile->translation.secondarySensEnable);

        for(int i = 0; i < 20; ++i)
        {
            qDebug() << QString().sprintf("--------- key %d --------", i);
            qDebug() << QString().sprintf("key: %04X", profile->keys[i].key);
            qDebug() << QString().sprintf("ps3_key: %04X", profile->keys[i].ps3_key);
            qDebug() << QString().sprintf("pid: %04X", profile->keys[i].pid);
            qDebug() << QString().sprintf("vid: %04X", profile->keys[i].vid);
            qDebug() << QString().sprintf("source: %02X", profile->keys[i].source);
            qDebug() << QString().sprintf("type: %02X", profile->keys[i].type);
        }

        qDebug() << "------ switchProfile --------";
        qDebug() << QString().sprintf("key: %04X", profile->switchProfile.key);
        qDebug() << QString().sprintf("ps3_key: %04X", profile->switchProfile.ps3_key);
        qDebug() << QString().sprintf("pid: %04X", profile->switchProfile.pid);
        qDebug() << QString().sprintf("vid: %04X", profile->switchProfile.vid);
        qDebug() << QString().sprintf("source: %02X", profile->switchProfile.source);
        qDebug() << QString().sprintf("type: %02X", profile->switchProfile.type);

        qDebug() << "------ switchSensitivity --------";
        qDebug() << QString().sprintf("key: %04X", profile->switchSensitivity.key);
        qDebug() << QString().sprintf("ps3_key: %04X", profile->switchSensitivity.ps3_key);
        qDebug() << QString().sprintf("pid: %04X", profile->switchSensitivity.pid);
        qDebug() << QString().sprintf("vid: %04X", profile->switchSensitivity.vid);
        qDebug() << QString().sprintf("source: %02X", profile->switchSensitivity.source);
        qDebug() << QString().sprintf("type: %02X", profile->switchSensitivity.type);

        qDebug() << "--------- joykeys --------";
        for(int i = 0; i < 4; ++i)
        {
            qDebug() << QString().sprintf("--------- joykeys %d --------", i);
            qDebug() << QString().sprintf("key: %04X", profile->joykeys[i].key);
            qDebug() << QString().sprintf("ps3_key: %04X", profile->joykeys[i].ps3_key);
            qDebug() << QString().sprintf("pid: %04X", profile->joykeys[i].pid);
            qDebug() << QString().sprintf("vid: %04X", profile->joykeys[i].vid);
            qDebug() << QString().sprintf("source: %02X", profile->joykeys[i].source);
            qDebug() << QString().sprintf("type: %02X", profile->joykeys[i].type);
        }

        qDebug() << QString().sprintf("color: %08X", profile->color);
        qDebug() << QString().sprintf("crc: %08X", profile->crc);
        qDebug() << QString().sprintf("joyDeadzone: %f", profile->joyDeadzone);

        qDebug() << QString().sprintf("name: %s", profile->name);
        qDebug() << QString().sprintf("useStick: %02X", profile->useStick);
        qDebug() << QString().sprintf("target: %02X", profile->target);
    }

}
