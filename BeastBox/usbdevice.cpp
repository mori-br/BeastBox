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
#include "usbdevice.h"

#include <QLogger/QLogger.h>

using namespace QLogger;

#define USBD_VID                   		0x0483
#define USBD_PID_DEV               		0x5710 // Dont mixture things 0x5710 cmd, 0x5420 ps4 controller
#define USBD_PID_DFU               		0xDF11

UsbDevice::UsbDevice()
    : _handle(NULL)
    , _cfgd(NULL)
{
}

void UsbDevice::deinit()
{
    libusb_exit(NULL);
}

bool UsbDevice::init()
{
    int rc = libusb_init(NULL);
    if (rc < 0)
    {
        QLog_Error("Default", QString("failed to initialise libusb: %1").arg(libusb_error_name(rc)));
        return false;
    }

//    libusb_set_debug(NULL, 255);

    QLog_Debug("Default", "libusb loaded ok.");

    return true;
}

bool UsbDevice::tryToOpen()
{
    int i = 0;
    libusb_device **devs;

    ssize_t cnt = libusb_get_device_list(NULL, &devs);
    if (cnt > 0)
    {
        libusb_device *dev;
        while ((dev = devs[i++]) != NULL)
        {
            struct libusb_device_descriptor desc;
            int r = libusb_get_device_descriptor(dev, &desc);
            if (r == 0)
            {
                if (desc.idVendor == USBD_VID && (desc.idProduct == USBD_PID_DEV || desc.idProduct == USBD_PID_DFU))
                {
                    r = libusb_open(dev, &_handle);
                    if (r == 0)
                    {
                        r = libusb_get_config_descriptor(dev, 0, &_cfgd);
                        if (r == 0)
                        {
                            libusb_free_device_list(devs, 1);
                            _pid = desc.idProduct;
                            return true;
                        }
                    }
                }
            }
        }

        libusb_free_device_list(devs, 1);
    }

    return false;
}

bool UsbDevice::isDFU()
{
    return (_pid == USBD_PID_DFU);
}

void UsbDevice::close()
{
    if (_cfgd != NULL)
    {
        libusb_free_config_descriptor(_cfgd);
        _cfgd = NULL;
    }

    if (_handle != NULL)
    {
        libusb_release_interface(_handle, 0);
        libusb_close(_handle);
        _handle = NULL;
    }
}

char *UsbDevice::getStrVID()
{
    return QString().sprintf("0x%04X", USBD_VID).toLatin1().data();
}

char *UsbDevice::getStrPID(bool dfu)
{
    if(dfu)
        return QString().sprintf("0x%04X", USBD_PID_DFU).toLatin1().data();

    return QString().sprintf("0x%04X", USBD_PID_DEV).toLatin1().data();
}
