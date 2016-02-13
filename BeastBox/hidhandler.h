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
#ifndef HIDHANDLER_H
#define HIDHANDLER_H

#include <QList>
#include <QSharedPointer>
#include <QByteArray>

#include <libusb.h>
#include <stdint.h>

#include "usbdevice.h"
#include <../common/profiledata.h>

#ifdef _MSC_VER
#pragma pack(1)
#endif

#define USB_REPORT_PKT_SIZE             64
#define USB_REPORT_PKT_SIZE1			USB_REPORT_PKT_SIZE+1


#define CMD_SIGNATURE					0x00
#define CMD_STARTSETUP					0x01
#define CMD_DFU_MODE					0x02
#define CMD_GET_VERSION					0x03
#define CMD_GETKEY						0x05
#define CMD_SETUP_DOWNLOAD				0x06
#define CMD_RESET						0x0C
#define CMD_PING						0x0F
#define CMD_FINISH						0xFF


#ifdef _MSC_VER
typedef struct _HIDINFO
#else
typedef struct __attribute__ ((packed)) _HIDINFO
#endif
{
    uint8_t  itf;
    uint8_t  proto;
    uint8_t  cls;
    uint8_t  subcls;

} HIDINFO;

#ifdef _MSC_VER
typedef struct _DEVINFO
#else
typedef struct __attribute__ ((packed)) _DEVINFO
#endif
{
    uint16_t vid;
    uint16_t pid;
    uint8_t  proto;
    uint8_t  cls;
    uint8_t  address;
    uint8_t  numitfs;

    _HIDINFO hiditf[4]; // max 4 interfaces

} DEVINFO;

typedef struct _ALLDEVINFO
{
    DEVINFO devices[5]; // 0 = hub, 1-4 are devices

} ALLDEVINFO;

typedef struct _KEYINFO
{
    uint16_t vid;
    uint16_t pid;
    uint16_t key;    // device key
    uint8_t  source; // KeySource (profile.h)
    uint16_t psk;    // ps3/ps4 key
    uint8_t  type;   // KeyType (ps.h)

    _KEYINFO(){}

    _KEYINFO(const struct _KEYINFO *src)
    {
        copy(src);
    }

    void copy(const struct _KEYINFO *src)
    {
        source = src->source;
        key  = src->key;
        pid  = src->pid;
        psk  = src->psk;
        vid  = src->vid;
        type = src->type;
    }

    void reset()
    {
        source = 0;
        key  = 0;
        pid  = 0;
        vid  = 0;
//        type = 0;
    }

    void toKey(Key *k)
    {
        k->key = key;
        k->ps3_key = psk;
        k->source = source;
        k->type = type;
        k->pid = pid;
        k->vid = vid;
    }

    void fromKey(const Key *k)
    {
        key = k->key;
        psk = k->ps3_key;
        source = k->source;
        type = k->type;
        pid = k->pid;
        vid = k->vid;
    }

} KEYINFO;


/////////////////////////////////////////////////////////////////////////////

class HIDHandler
{
public:
    HIDHandler();

    void close();
    bool open(libusb_device_handle *handle, IOperationListener *listener=NULL);

    bool sendCommand(uint8_t repId, uint8_t cmd, uint8_t *param, uint16_t size);
    bool sendCommand(uint8_t cmd, uint8_t *param, uint16_t size);
    bool sendCommand(uint8_t cmd);
    bool readReport(uint8_t repId, uint8_t *buffer, uint16_t length);
    bool sendReport(uint8_t repId, uint8_t *buffer, uint16_t size);

    bool getProfileSize(uint16_t *size);
    bool getFirmwareVersion(uint16_t *version);
    bool getNumProfiles(uint16_t *count);
    bool getNumMaxProfiles(uint16_t *count);
    bool getAllDevicesInfo(ALLDEVINFO *devices);
    bool enterDFUMode();
    bool receiveKey(KEYINFO *ki);
    bool setupSendFirmware(int numProfiles);

    bool getDeviceProfiles(QList< QSharedPointer<QByteArray> > *list);

    inline bool isConnected() { return _handle != NULL; }

    int getNumPackets(int lenght);
    bool sendFirmware(uint8_t *buffer, uint16_t size);
    bool eraseFlash();
    bool checkTransfer();

protected:
    libusb_device_handle *_handle;
    IOperationListener   *_listener;
};

#endif // HIDHANDLER_H
