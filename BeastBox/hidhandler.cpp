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
#include "hidhandler.h"
#include "util.h"
#include <QLogger/QLogger.h>
#include <QDebug>
#include <../common/profiledata.h>

using namespace QLogger;

// HID Class-Specific Requests values. See section 7.2 of the HID specifications
#define HID_GET_REPORT					0x01
#define HID_GET_IDLE					0x02
#define HID_GET_PROTOCOL				0x03
#define HID_SET_REPORT					0x09
#define HID_SET_IDLE					0x0A
#define HID_SET_PROTOCOL				0x0B
#define HID_REPORT_TYPE_INPUT			0x01
#define HID_REPORT_TYPE_OUTPUT			0x02
#define HID_REPORT_TYPE_FEATURE			0x03

#define FEATURE_SET_COMMAND				0xA0
#define FEATURE_SET_FIRMWARE			0xA1
#define FEATURE_GET_ERASEFLASH          0xA2
#define FEATURE_GET_SIGNATURE			0xA3
#define FEATURE_GET_VERSION             0xA4
#define FEATURE_GET_READKEY             0xA5
#define FEATURE_GET_NUMPROFILES         0xA6
#define FEATURE_GET_MAXPROFILES         0xA7
#define FEATURE_GET_PROFILESIZE         0xA8
#define FEATURE_GET_ALLDEVICES          0xAA
#define FEATURE_GET_CHECKTRANSFER       0XAB
#define FEATURE_GET_READCHUNK           0xAC

#define USB_INPUT_REPORT_PKT_SIZE		12
#define USB_REPORT_PKT_SIGNATURE		6+1
#define USB_REPORT_PKT_VERSION			2+1
#define USB_REPORT_PKT_MAXPROFILES      2+1
#define USB_REPORT_PKT_NUMPROFILES      2+1
#define USB_REPORT_PKT_ALLDEVICE        60+1
#define USB_REPORT_PKT_READKEY          8+1
#define USB_REPORT_PKT_PROFILESIZE      2+1
#define USB_REPORT_PKT_ERASEFLASH       1+1
#define USB_REPORT_PKT_CHECKTRANSFER    1+1
#define USB_REPORT_PKT_READCHUNK        60+1

#define OK								0xAA
#define ERR								0xBB
#define MAXPROFILESERR					0xCC
#define END                             0xDD

#define USB_TIMEOUT                     5000



const uint8_t DEVICE_SIGNATURE[] = { 'B', 'E', 'A', 'S', 'T', 'B' };

HIDHandler::HIDHandler()
    : _handle(NULL)
    , _listener(NULL)
{
}

void HIDHandler::close()
{
    if (_handle != NULL)
    {
        sendCommand(CMD_FINISH);
        QThread::msleep(1000);
QLog_Debug("Default", "HIDHandler closed...");
        _handle = NULL;
    }
}

bool HIDHandler::open(libusb_device_handle *handle, IOperationListener *listener)
{
    uint8_t signature[11] = { 0 };

    _handle = handle;

    _listener = listener;

    QLog_Debug("Default", "Hid device open");

    if(readReport(FEATURE_GET_SIGNATURE, signature, sizeof(signature)))
    {
        QLog_Debug("Default", "Device signature match");

        if (memcmp(&signature[1], DEVICE_SIGNATURE, sizeof(DEVICE_SIGNATURE)) == 0)
        {
            QLog_Debug("Default", "Device configured to PC mode");
            return true;
        }
    }

    libusb_close(_handle);
    _handle = NULL;

    return false;
}


bool HIDHandler::sendReport(uint8_t repId, uint8_t *buffer, uint16_t size)
{
    if (_handle != NULL)
    {
        int res = libusb_control_transfer(_handle,
                                        LIBUSB_ENDPOINT_OUT|LIBUSB_REQUEST_TYPE_CLASS|LIBUSB_RECIPIENT_INTERFACE,
                                        HID_SET_REPORT,
                                        MAKEWORD(repId, HID_REPORT_TYPE_FEATURE),
                                        0,
                                        buffer,
                                        size,
                                        USB_TIMEOUT);
        if (res >= 0)
            return TRUE;

        QLog_Error("Default", QString("SendCommand error %1").arg(res));
    }

    return FALSE;
}

bool HIDHandler::sendCommand(uint8_t repId, uint8_t cmd, uint8_t *param, uint16_t size)
{
    uint8_t buffer[USB_REPORT_PKT_SIZE1] = { 0 };
    buffer[0] = repId;
    buffer[1] = cmd;
    buffer[2] = size;

    if (param != NULL)
        memcpy(&buffer[3], param, size);

    return sendReport(repId, buffer, USB_REPORT_PKT_SIZE1);
}

bool HIDHandler::sendCommand(uint8_t cmd, uint8_t *param, uint16_t size)
{
    return sendCommand(FEATURE_SET_COMMAND, cmd, param, size);
}

bool HIDHandler::sendCommand(uint8_t cmd)
{
    return sendCommand(cmd, NULL, 0);
}

bool HIDHandler::readReport(uint8_t repId, uint8_t *buffer, uint16_t length)
{
    int res = libusb_control_transfer(_handle,
                                    LIBUSB_ENDPOINT_IN|LIBUSB_REQUEST_TYPE_CLASS|LIBUSB_RECIPIENT_INTERFACE,
                                    HID_GET_REPORT,
                                    MAKEWORD(repId, HID_REPORT_TYPE_FEATURE),
                                    0,
                                    buffer,
                                    length,
                                    USB_TIMEOUT);
    if (res >= 0)
    {
        if(buffer[0] != repId)
        {
            memset(buffer, 0, length);
            buffer[0] = repId;
        }

        return true;
    }

    // ERROR:
    // LIBUSB_ERROR_TIMEOUT
    // LIBUSB_ERROR_PIPE
    // LIBUSB_ERROR_NO_DEVICE
    // LIBUSB_ERROR
    QLog_Error("Default", QString("readReport() - response error %1").arg(res));

    return false;
}

bool HIDHandler::getFirmwareVersion(uint16_t *version)
{
    QLog_Debug("Default", "Trying to get the firmware version...");

    uint8_t buffer[USB_REPORT_PKT_VERSION] = { 0 };
    if (readReport(FEATURE_GET_VERSION, buffer, USB_REPORT_PKT_VERSION))
    {
        uint16_t wVersion = MAKEWORD(buffer[2], buffer[1]);
        if(version != NULL)
            *version = wVersion;

        QLog_Debug("Default", QString().sprintf("Firmware version 0x%04X", wVersion));

        return true;
    }

    QLog_Error("Default", "Error Getting firmware version");

    return false;
}

bool HIDHandler::getNumProfiles(uint16_t *count)
{
    QLog_Debug("Default", "Trying to get number of profiles...");

    uint8_t buffer[USB_REPORT_PKT_NUMPROFILES] = { 0 };
    if (readReport(FEATURE_GET_NUMPROFILES, buffer, USB_REPORT_PKT_NUMPROFILES))
    {
        uint16_t dwCount = MAKEWORD(buffer[2], buffer[1]);
        if(count != NULL)
            *count = dwCount;
        QLog_Debug("Default", QString("num profiles %1").arg(dwCount));
        return true;
    }

    QLog_Error("Default", "Error Getting profiles count");

    return false;
}

bool HIDHandler::getNumMaxProfiles(uint16_t *count)
{
    QLog_Debug("Default", "Trying to get max number of profiles...");

    uint8_t buffer[USB_REPORT_PKT_MAXPROFILES] = { 0 };
    if (readReport(FEATURE_GET_MAXPROFILES, buffer, USB_REPORT_PKT_MAXPROFILES))
    {
        uint16_t dwCount = MAKEWORD(buffer[2], buffer[1]);
        if(count != NULL)
            *count = dwCount;
        QLog_Debug("Default", QString("max profiles %1").arg(dwCount));
        return true;
    }

    QLog_Error("Default", "Error Getting max profiles count");

    return false;
}

bool HIDHandler::getAllDevicesInfo(ALLDEVINFO *info)
{
    if(info == NULL)
        return false;

    QLog_Debug("Default", "Trying to get alldevices info...");

    uint8_t buffer[USB_REPORT_PKT_ALLDEVICE] = { 0 };
    if (readReport(FEATURE_GET_ALLDEVICES, buffer, USB_REPORT_PKT_ALLDEVICE))
    {
        int idx = 1;
        for(int i = 0; i < 5; ++i)
        {
            uint16_t dwVID = MAKEWORD(buffer[idx+1], buffer[idx]);
            uint16_t dwPID = MAKEWORD(buffer[idx+3], buffer[idx+2]);
            uint8_t  proto = buffer[idx+4];
            uint8_t  cls   = buffer[idx+5];
            uint8_t  addr  = buffer[idx+6];

            info->devices[i].vid     = dwVID;
            info->devices[i].pid     = dwPID;
            info->devices[i].proto   = proto;
            info->devices[i].cls     = cls;
            info->devices[i].address = addr;

            if(buffer[idx+7] != 0) // if it have a hid interface
            {
                int idy = 8;
                info->devices[i].numitfs = buffer[idx+7];
                for(int j = 0; j < info->devices[i].numitfs; ++j)
                {
                    info->devices[i].hiditf[j].itf = buffer[idx+(idy++)];
                    info->devices[i].hiditf[j].proto = buffer[idx+(idy++)];
                    info->devices[i].hiditf[j].cls = buffer[idx+(idy++)];
                    info->devices[i].hiditf[j].subcls = buffer[idx+(idy++)];
                }

                idx += idy;
            }
            else
                idx += 8;
        }

        return true;
    }

    QLog_Error("Default", "Error getting 'all devices' info");

    return false;
}

bool HIDHandler::enterDFUMode()
{
    QLog_Debug("Default", "Trying to enter in DFU mode...");
    if(sendCommand(CMD_DFU_MODE))
    {
        QThread::msleep(1000);

        int ret = libusb_reset_device(_handle);
        if (ret < 0 && ret != LIBUSB_ERROR_NOT_FOUND)
        {
            QLog_Error("Default", "error resetting after download");
            return false;
        }

        return true;
    }

    return false;
}

bool HIDHandler::receiveKey(KEYINFO *ki)
{
    if(ki == NULL)
        return false;

    uint8_t buffer[USB_REPORT_PKT_READKEY] = { 0 };
    if (readReport(FEATURE_GET_READKEY, buffer, USB_REPORT_PKT_READKEY))
    {
        if(buffer[0] == FEATURE_GET_READKEY)
        {
            ki->vid = MAKEWORD(buffer[1], buffer[2]);
            ki->pid = MAKEWORD(buffer[3], buffer[4]);
            ki->key = MAKEWORD(buffer[5], buffer[6]);
            ki->source = buffer[7];

            if(ki->vid > 0 && ki->pid > 0)
            {
                QLog_Debug("Default",
                           QString().sprintf(
                               "receiveKey: 0x%02X source: %d (0:mouse,2:keyboard,3:keypad)",
                               ki->key, ki->source));
            }

            return true;
        }
    }

    return false;
}

bool HIDHandler::getProfileSize(uint16_t *size)
{
QLog_Debug("Default", "Trying to get profile size...");

    uint8_t buffer[USB_REPORT_PKT_PROFILESIZE] = { 0 };
    if (readReport(FEATURE_GET_PROFILESIZE, buffer, USB_REPORT_PKT_PROFILESIZE))
    {
        uint16_t value = MAKEWORD(buffer[2], buffer[1]);
        if(size != NULL)
            *size = value;

        QLog_Debug("Default", QString("profile size %1").arg(value));
        return true;
    }

    QLog_Error("Default", "Error Getting profile size");

    return false;
}

int HIDHandler::getNumPackets(int lenght)
{
    int packet_size = USB_REPORT_PKT_SIZE1 - 1;

    int pkts = lenght / (packet_size);

    if ((lenght % (packet_size)) != 0)
        pkts += 1;

    return pkts;
}

bool HIDHandler::sendFirmware(uint8_t *buffer, uint16_t size)
{
    uint8_t data[USB_REPORT_PKT_SIZE1] = { 0 };
    data[0] = FEATURE_SET_FIRMWARE;

    memcpy(&data[1], buffer, size);

    return sendReport(FEATURE_SET_FIRMWARE, data, USB_REPORT_PKT_SIZE1);
}

bool HIDHandler::setupSendFirmware(int numProfiles)
{
    uint8_t data[1] = { LOBYTE(numProfiles) };
    return sendCommand(CMD_SETUP_DOWNLOAD, data, 1);
}

bool HIDHandler::eraseFlash()
{
    uint8_t buffer[USB_REPORT_PKT_ERASEFLASH] = { 0 };
    if (readReport(FEATURE_GET_ERASEFLASH, buffer, USB_REPORT_PKT_ERASEFLASH))
    {
        if(buffer[0] == FEATURE_GET_ERASEFLASH && buffer[1] == OK)
            return true;
    }

    QLog_Error("Default", "Error erasing device flash");

    return false;
}

bool HIDHandler::checkTransfer()
{
    uint8_t buffer[USB_REPORT_PKT_CHECKTRANSFER] = { 0 };
    if (readReport(FEATURE_GET_CHECKTRANSFER, buffer, USB_REPORT_PKT_CHECKTRANSFER))
    {
        if(buffer[0] == FEATURE_GET_CHECKTRANSFER && buffer[1] == OK)
            return true;
    }

    QLog_Error("Default", "Error copying data to device");

    return false;
}


bool HIDHandler::getDeviceProfiles(QList< QSharedPointer<QByteArray> > *list)
{
    if(list == NULL)
        return false;

    QLog_Debug("Default", "Trying to get all profiles from device");

    uint8_t buffer[USB_REPORT_PKT_ALLDEVICE] = { 0 };
    uint16_t idx = 0;

    while(idx < (list->size() * sizeof(ProfileData)))
    {
        if(!readReport(FEATURE_GET_READCHUNK, buffer, USB_REPORT_PKT_READCHUNK))
        {
            QLog_Error("Default", "Error getting 'all profiles' info");
            return false;
        }

        //Util::LOG_ARRAY(buffer, USB_REPORT_PKT_ALLDEVICE);

        if(buffer[0] != FEATURE_GET_READCHUNK)
        {
            QLog_Error("Default", "Error getting 'all profiles' info");
            return false;
        }

        // While we have packets or if its last one, treat it
        if(buffer[1] == OK || buffer[1] == END)
        {
            //QLog_Debug("Default", QString("block size %1").arg(buffer[3]));

            uint8_t num  = buffer[2]; // profile num
            uint8_t size = buffer[3]; // buf size
            if(size > 0)
            {
                list->at(num)->append((char *)&buffer[4], size);
                idx += size;
            }

            QThread::msleep(20);
        }
        else
        {
QLog_Error("Default", "Error getting profile");
            return false;
        }
    }

    QLog_Debug("Default", "profiles readed");

    return true;
}
