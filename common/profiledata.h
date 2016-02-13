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
#ifndef PROFILEDATA_H
#define PROFILEDATA_H

#include <vector>
#include <memory>

#ifdef _MSC_VER
#pragma pack(1)
#endif

#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

typedef enum JoyKeyDir_
{
    JOY_KEY_UP=0,
    JOY_KEY_DOWN,
    JOY_KEY_LEFT,
    JOY_KEY_RIGHT,

} JoyKeyDir;

typedef enum KeySource_
{
    KEY_MAP_MOUSE=0,
    KEY_MAP_CONTROLLER,
    KEY_MAP_KEYBOARD,
    KEY_MAP_KEYPAD,

} KeySource;

#define MAX_KEY_SOURCE  4
#define MAX_KEY_BUTTON  20

#ifdef _MSC_VER
typedef struct Key_
#else
typedef struct __attribute__ ((packed)) Key_
#endif
{
    uint16_t 	key;       // device key
    uint16_t 	ps3_key;   // ps3 key to send
    uint8_t     source;    //
    uint8_t     type;      // key type
    uint16_t 	pid;       // device pid
    uint16_t 	vid;       // device vid

} Key;

typedef enum DeadZoneType_
{
    DZ_CIRCULAR,
    DZ_SQUARE

} DeadZoneType;

#ifdef _MSC_VER
typedef struct TranslationData_
#else
typedef struct __attribute__ ((packed)) TranslationData_
#endif
{
    float    deadZone;
    float 	 translationExponent;
    float 	 YXRatio;
    float 	 smoothness;
    float 	 diagonalDampen;
    float 	 sensitivityPrimary;
    float 	 sensitivitySecondary;

    uint8_t  inputUpdateFrequency;
    uint8_t	 deadZoneType;
    uint8_t	 smoothEnable;
    uint8_t	 secondarySensEnable;

} TranslationData;


#define PROFILE_SIZE ( sizeof(TranslationData) + \
                      (sizeof(Key)  * 26) + \
                       sizeof(uint32_t) + \
                       sizeof(float) + \
                      (sizeof(char) * 41) + \
                      (sizeof(uint8_t) * 3) + \
                       sizeof(uint32_t))


typedef union ProfileData_
{
#ifdef _MSC_VER
    struct
#else
    struct __attribute__ ((packed))
#endif
    {
        TranslationData	translation;
        Key             keys[MAX_KEY_BUTTON];
        Key             switchProfile;
        Key             switchSensitivity;
        Key             joykeys[4];
        uint32_t        color;
        float           joyDeadzone;
        char            name[41];
        uint8_t         useStick;
        uint8_t         target;                 // ConsoleType
        uint8_t         filler;
        uint32_t        crc;
    };

#ifdef _MSC_VER
    struct
#else
    struct __attribute__ ((packed))
#endif
    {
        uint8_t			bytes[PROFILE_SIZE];
    };

} ProfileData;

#ifndef LOBYTE
#define LOBYTE(w) ((unsigned char) (((unsigned long) (w)) & 0xff))
#endif

#ifndef MAKEWORD
#define MAKEWORD(a,b) ((unsigned short)(((unsigned char)((unsigned long)(a) & 0xff)) | ((unsigned short)((unsigned char)((unsigned long)(b) & 0xff))) << 8))
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#endif // PROFILEDATA_H
