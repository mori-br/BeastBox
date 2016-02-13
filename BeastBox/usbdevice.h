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
#ifndef USBDEVICE_H
#define USBDEVICE_H

#include <libusb.h>


class IOperationListener
{
public:
    virtual void onOperationNumSteps(uint16_t steps)=0;
    virtual void onOperationStep()=0;
};

////////////////////////////////////////////////////////////////////////////////

class UsbDevice
{
public:
    explicit UsbDevice();

    bool init();
    void deinit();

    bool tryToOpen();
    void close();
    bool isDFU();

    inline libusb_device_handle *getHandle() { return _handle; }
    inline struct libusb_config_descriptor *getConfigDescriptor() { return _cfgd; }

    static char *getStrVID();
    static char *getStrPID(bool dfu);

private:
    uint16_t _pid;
    libusb_device_handle *_handle;
    struct libusb_config_descriptor *_cfgd;
};

#endif // USBDEVICE_H
