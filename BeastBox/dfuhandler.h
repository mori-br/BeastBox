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
#ifndef DFUHANDLER_H
#define DFUHANDLER_H

#include <stdint.h>
#include <libusb.h>
#include <QDebug>
#include "targetmemory.h"
#include "usbdevice.h"


#define STATUS_OK						0x00 // No error condition is present.
#define STATUS_errTARGET				0x01 // File is not targeted for use by this device.
#define STATUS_errFILE					0x02 // File is for this device but fails some vendor - specific verification test.
#define STATUS_errWRITE					0x03 // Device is unable to write memory.
#define STATUS_errERASE					0x04 // Memory erase function failed.
#define STATUS_errCHECK_ERASED			0x05 // Memory erase check failed.
#define STATUS_errPROG					0x06 // Program memory function failed.
#define STATUS_errVERIFY				0x07 // Programmed memory failed verification.
#define STATUS_errADDRESS				0x08 // Cannot program memory due to received address that is out of range.
#define STATUS_errNOTDONE				0x09 // Received DFU_DNLOAD with wLength = 0, but device does not think it has all of the data yet.
#define STATUS_errFIRMWARE				0x0A // Device’s firmware is corrupt.It cannot return to run - time (non - DFU) operations.
#define STATUS_errVENDOR				0x0B // iString indicates a vendor - specific error.
#define STATUS_errUSBR					0x0C // Device detected unexpected USB reset signaling.
#define STATUS_errPOR					0x0D // Device detected unexpected power on reset.
#define STATUS_errUNKNOWN				0x0E // Something went wrong, but the device does not know what it was.
#define STATUS_errSTALLEDPKT			0x0F // Device stalled an unexpected request

#define STATE_appIDLE					0x00 // Device is running its normal application.
#define STATE_appDETACH					0x01 // Device is running its normal application, has received the STATE_DFU_DETACH request,
                                             // and is waiting for a USB reset.
#define STATE_dfuIDLE					0x02 // Device is operating in the DFU mode and is waiting for requests.
#define STATE_dfuDNLOAD_SYNC			0x03 // Device has received a block and is waiting for the host to solicit the status via DFU_GETSTATUS.
#define STATE_dfuDNBUSY					0x04 // Device is programming a control - write block into its nonvolatile memories.
#define STATE_dfuDNLOAD_IDLE			0x05 // Device is processing a download operation.Expecting DFU_DNLOAD requests.
#define STATE_dfuMANIFEST_SYNC			0x06 // Device has received the final block of firmware from the host and is waiting for receipt of
                                             // DFU_GETSTATUS to begin the Manifestation phase; or device has completed the Manifestation phase
                                             // and is waiting for receipt of DFU_GETSTATUS. (Devices that can enter this state after
                                             // the Manifestation phase set bmAttributes bit bitManifestationTolerant to 1.)
#define STATE_dfuMANIFEST				0x07 // Device is in the Manifestation phase. (Not all devices will be able to respond to DFU_GETSTATUS
                                             // when in this state.)
#define STATE_dfuMANIFEST_WAIT_RESET	0x08 // Device has programmed its memories and is waiting for a USB reset or a power on reset.
                                             // (Devices that must enter this state clear bitManifestationTolerant to 0.)
#define STATE_dfuUPLOAD_IDLE			0x09 // The device is processing an upload operation.Expecting DFU_UPLOAD requests.
#define STATE_dfuERROR					0x0A // An error has occurred.Awaiting the DFU_CLRSTATUS request.

#define ATTR_DNLOAD_CAPABLE				0x01
#define ATTR_UPLOAD_CAPABLE				0x02
#define ATTR_MANIFESTATION_TOLERANT		0x04
#define ATTR_WILL_DETACH				0x08
#define ATTR_ST_CAN_ACCELERATE			0x80


typedef struct _DFUDESCRIPTOR
{
    uint8_t		bLength;			// Size of this descriptor, in bytes. (9)
    uint8_t		bDescriptorType;	// DFU FUNCTIONAL descriptor type (0x21)
    uint8_t		bmAttributes;		// DFU attributes:
    uint16_t    wDetachTimeOut;		// Time, in milliseconds, that the device waits after receipt of the DFU_DETACH request
    uint16_t	wTransferSize;		// Maximum number of bytes that the device can accept per control - write transaction
    uint16_t	bcdDFUVersion;		// Version of the STMicroelectronics DFU (0x011A)

} DFUDESCRIPTOR;

class DFUseFile;

typedef enum _tagDFUSE_COMMANDS
{
    DFUSE_COMMAND_SET_ADDRESS=0x21,
    DFUSE_COMMAND_ERASE=0x41,
    DFUSE_COMMAND_READ_UNPROTECT=0x92,

} DFUSE_COMMANDS;

class DFUseStatus
{
public:
    static const uint8_t SIZE = 6;

    // An indication of the status resulting from the execution of the most recent request.
    uint8_t  _bStatus;			// 1
    // Minimum time, in milliseconds, that the host should wait before sending a subsequent DFU_GETSTATUS request.
    uint32_t _bwPollTimeout;	// 3
    // An indication of the state that the device is going to enter immediately following transmission of this
    // response. (By the time the host receives this information, this is the current state of the device.)
    uint8_t  _bState;			// 1
    // Index of status description in string table.
    uint8_t  _iString;			// 1


    explicit DFUseStatus()
        : _bStatus(STATUS_errUNKNOWN)
        , _bwPollTimeout(0)
        , _bState(STATE_dfuERROR)
        , _iString(0)
    {
    }

    inline void reset()
    {
        _bStatus = STATUS_errUNKNOWN;
        _bwPollTimeout = 0;
        _bState = STATE_dfuERROR;
        _iString = 0;
    }

    inline void debug()
    {
        qDebug() << "bStatus " << _bStatus;
        qDebug() << "bwPollTimeout " << _bwPollTimeout;
        qDebug() << "bState " << _bState;
        qDebug() << "iString " << _iString;
    }
};

class DFUAttributes
{
public:
    bool	 _OptCanDnload;
    bool	 _OptCanUpload;
    bool	 _OptCanDetach;
    bool	 _OptCanManifestTolerant;
    bool	 _OptCanAccel;

    void reset()
    {
        _OptCanDnload = false;
        _OptCanUpload = false;
        _OptCanDetach = false;
        _OptCanManifestTolerant = false;
        _OptCanAccel = false;
    }

    inline void debug()
    {
        qDebug() << "OptCanDnload " << _OptCanDnload;
        qDebug() << "OptCanUpload " << _OptCanUpload;
        qDebug() << "OptCanDetach " << _OptCanDetach;
        qDebug() << "OptCanManifestTolerant " << _OptCanManifestTolerant;
        qDebug() << "OptCanAccel " << _OptCanAccel;
    }
};

//////////////////////////////////////////////////////////////////////////////////////

class DFUHandler
{
public:
    DFUHandler();

    void close();
    bool open(libusb_device_handle *handle, struct libusb_config_descriptor *cfgd);

    bool download(const uint16_t interface, DFUseFile *file, IOperationListener *listener=NULL);

    int  detach(const uint16_t interface, const uint16_t timeout);
    int  clearStatus(const uint16_t interface);
    int  getState(const uint16_t interface);
    int  abort(const uint16_t interface);
    int  upload(const uint16_t interface, uint8_t *data, const uint16_t length);
    int  download(const uint16_t interface, const uint16_t blocknum, uint8_t *data, const uint16_t length);
    int  getStatus(const uint16_t interface, DFUseStatus *status);

    int  erasePage(const uint16_t interface, uint32_t addr);
    int  erase(const uint16_t interface);
    int  setAddress(const uint16_t interface, uint32_t addr);
    int  readUnprotect(const uint16_t interface);

    bool leaveDFUMode(const uint16_t interface=0);

protected:
    libusb_device_handle    *_handle;
    TargetMemory            _targetMemory;
    DFUDESCRIPTOR           _dfuDesc;
    uint32_t                _last_page;
    IOperationListener      *_listener;

    bool command(const uint16_t interface, DFUSE_COMMANDS cmd, uint32_t addr);
    bool wait4Command(const uint16_t interface);
    bool parseMemoryMap(char *buffer, uint16_t size);
    void parseAttributes(uint8_t bmAttributes);
};

#endif // DFUHANDLER_H
