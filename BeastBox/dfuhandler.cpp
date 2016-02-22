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

Based on:
    AN3156
    UM0424
    http://www.usb.org/developers/docs/devclass_docs/DFU_1.1.pdf
*/
#include "dfuhandler.h"
#include "dfusefile.h"
#include <QDebug>
#include <QLogger/QLogger.h>
#include <math.h>

using namespace QLogger;


#define DFU_VERSION                 0x011A

#define DFU_DETACH                  0x00	// Requests the device to leave DFU mode and enter the application.
#define DFU_DNLOAD                  0x01	// Requests data transfer from Host to the device in order to load them
                                            // into device internal Flash.
                                            // Includes also erase commands.
#define DFU_UPLOAD                  0x02	// Requests data transfer from device to Host in order to load content of
                                            // device internal Flash into a Host file.
#define DFU_GETSTATUS               0x03	// Requests device to send status report to the Host (including status
                                            // resulting from the last
                                            // request execution and the state the device will enter immediately after
                                            // this request).
#define DFU_CLRSTATUS               0x04	// Requests device to clear error status and move to next step.
#define DFU_GETSTATE                0x05	// Requests the device to send only the state it will enterimmediately after
                                            // this request.
#define DFU_ABORT                   0x06	// Requests device to exit the current state/operation and enter idle state
                                            // immediately.

#define USB_TIMEOUT                 5000

#define DFU_SET                     (LIBUSB_ENDPOINT_OUT|LIBUSB_REQUEST_TYPE_CLASS|LIBUSB_RECIPIENT_INTERFACE)
#define DFU_GET                     (LIBUSB_ENDPOINT_IN |LIBUSB_REQUEST_TYPE_CLASS|LIBUSB_RECIPIENT_INTERFACE)

#define ALTERNATE_SETT_INT_FLASH	0x00	//DFU image for the internal Flash memory select the Alternate Setting 00
#define ALTERNATE_SETT_SER_FLASH	0x01	//DFU image for the external serial Flash memory, select the Alternate Setting 01
#define ALTERNATE_SETT_NOR_FLASH	0x02	//DFU image for the NORFlash memory, select the Alternate Setting 02

#define BUFFER_SIZE                 253

#define SECTOR_READABLE             0x1                                                 //a
#define SECTOR_ERASABLE             0x2                                                 //b
#define SECTOR_READ_ERASE           SECTOR_READABLE|SECTOR_ERASABLE                     //c
#define SECTOR_WRITEABLE            0x4                                                 //d
#define SECTOR_READ_WRITE           SECTOR_READABLE|SECTOR_WRITEABLE                    //e
#define SECTOR_ERASE_WRITE          SECTOR_ERASABLE|SECTOR_WRITEABLE                    //f
#define SECTOR_ERASE_READ_WRITE     SECTOR_READABLE|SECTOR_ERASABLE|SECTOR_WRITEABLE    //g

#define DFU_INTERFACE               0


#if defined WIN32
    #define SLEEP(msecs) do { if (msecs) { Sleep(msecs); } } while (0)
#else
    #include <unistd.h>
    #define SLEEP(msecs) do { if (msecs) { usleep(msecs * 1000); } } while (0)
#endif

static const char* state2string(int state);
static const char* status2string(int status);
static int getSectorType(char ch);

DFUHandler::DFUHandler()
    : _handle(NULL)
    , _last_page(1)
    , _listener(NULL)
{
}

void DFUHandler::close()
{
    if (_handle != NULL)
        _targetMemory.destroy();

    _handle = NULL;
}

bool DFUHandler::open(libusb_device_handle *handle, struct libusb_config_descriptor *cfgd)
{
    bool valid = false;
    int res = 0;

    _handle = handle;

    _targetMemory.destroy();

    for (int i = 0; i < cfgd->bNumInterfaces; ++i)
    {
        const struct libusb_interface *itf = &cfgd->interface[i];
        if (itf)
        {
            for (int j = 0; j < itf->num_altsetting; ++j)
            {
                const struct libusb_interface_descriptor *itfd = &itf->altsetting[j];
                if (itfd != NULL)
                {
                    if (itfd->extra_length > 0)
                    {
//qDebug() << "bAlternateSetting " << itfd->bAlternateSetting;

                        if (itfd->extra[0] == 0x09 && itfd->extra[1] == 0x21)
                        {
                            memset(&_dfuDesc, 0, sizeof(DFUDESCRIPTOR));

                            _dfuDesc.bLength = itfd->extra[0];
                            _dfuDesc.bDescriptorType = itfd->extra[1];
                            _dfuDesc.bmAttributes = itfd->extra[2];
                            _dfuDesc.wDetachTimeOut |= (0xff & itfd->extra[4]) << 8;
                            _dfuDesc.wDetachTimeOut |= (0xff & itfd->extra[3]);
                            _dfuDesc.wTransferSize  |= (0xff & itfd->extra[6]) << 8;
                            _dfuDesc.wTransferSize  |= (0xff & itfd->extra[5]);
                            _dfuDesc.bcdDFUVersion  |= (0xff & itfd->extra[8]) << 8;
                            _dfuDesc.bcdDFUVersion  |= (0xff & itfd->extra[7]);

                            parseAttributes(_dfuDesc.bmAttributes);

//qDebug("bLength         0x%02X", _dfuDesc.bLength);
//qDebug("bDescriptorType 0x%02X", _dfuDesc.bDescriptorType);
//qDebug("bmAttributes    0x%02X", _dfuDesc.bmAttributes);
//qDebug("wDetachTimeOut  0x%04X", _dfuDesc.wDetachTimeOut);
//qDebug("wTransferSize   0x%04X", _dfuDesc.wTransferSize);
//qDebug("bcdDFUVersion   0x%04X", _dfuDesc.bcdDFUVersion);

                            if (_dfuDesc.bcdDFUVersion == DFU_VERSION)
                            {
                                valid = true;
                                break;
                            }
                        }
                    }

                    if (itfd->bAlternateSetting == ALTERNATE_SETT_INT_FLASH)
                    {
                        uint8_t buf[BUFFER_SIZE] = { 0 };
                        res = libusb_get_string_descriptor_ascii(_handle, itfd->iInterface, buf, BUFFER_SIZE);
                        if (res > 0)
                        {
qDebug()<< "map: " << (char *)buf;

                            if (!parseMemoryMap((char *)buf, res))
                            {
                                QLog_Error("Default", "Error parsing memory map");
                                return false;
                            }

                            QLog_Debug("Default", "Memory map parsed successfully");
                        }
                        else
                        {
                            QLog_Error("Default", "Error reading memory map");
                            return false;
                        }
                    }
                }
            }
        }

        if (valid) break;
    }

    if (valid)
    {
        if ((res = libusb_set_configuration(_handle, 1)) < 0)
        {
            QLog_Error("Default", "Cannot set configuration");
        }

        if ((res = libusb_claim_interface(_handle, DFU_INTERFACE)) < 0)
        {
            QLog_Error("Default", "Cannot claim interface");
        }

        if ((res = libusb_set_interface_alt_setting(_handle, DFU_INTERFACE, ALTERNATE_SETT_INT_FLASH)) < 0)
        {
            QLog_Error("Default", "Cannot set alternate interface");
        }

        DFUseStatus status;
        if ((res = getStatus(DFU_INTERFACE, &status)) <= 0)
        {
            QLog_Error("Default", "Cannot get status");
            return false;
        }
status.debug();

QLog_Debug("Default", QString("state %1, status %2").arg(state2string(status._bState)).arg(status2string(status._bStatus)));

        if(status._bState == STATE_dfuERROR)
        {
            SLEEP(status._bwPollTimeout == 0 ? 1000 : status._bwPollTimeout);

QLog_Debug("Default", "sending clear status...");
            if ((res = clearStatus(DFU_INTERFACE)) < 0)
            {
                QLog_Error("Default", "Cannot clear status");
                return false;
            }

            if ((res = getStatus(DFU_INTERFACE, &status)) <= 0)
            {
                QLog_Error("Default", "1-Cannot get status");
                return false;
            }

            QLog_Debug("Default", QString("state %1, status %2").arg(state2string(status._bState)).arg(status2string(status._bStatus)));
        }

        return true;
    }

    return false;
}

bool DFUHandler::download(const uint16_t interface, DFUseFile *file, IOperationListener *listener)
{
    DFUseStatus status;
    int res;

    _listener = listener;

    uint8_t *image = file->getImage();
    if(image == NULL)
    {
        QLog_Error("Default", "Invalid image buffer");
        return false;
    }

    TargetPrefix *tp = file->getTargetPrefix(ALTERNATE_SETT_INT_FLASH);
    if(tp != NULL)
    {
        QLog_Debug("Default", QString("num elements %1").arg(tp->getNumElements()));

        for(uint32_t i = 0; i < tp->getNumElements(); ++i)
        {
            ImageElement *ie = tp->getElement(i);

            QLog_Debug("Default", QString().sprintf("address   0x%08x", ie->_dwElementAddress));
            QLog_Debug("Default", QString().sprintf("size      0x%08x (%d)", ie->_dwElementSize, ie->_dwElementSize));
            QLog_Debug("Default", QString().sprintf("xfer size 0x%08x (%d)", _dfuDesc.wTransferSize, _dfuDesc.wTransferSize));

            uint16_t steps = (ie->_dwElementSize/_dfuDesc.wTransferSize)+
                ((ie->_dwElementSize%_dfuDesc.wTransferSize) > 0 ? 1 : 0);

            QLog_Debug("Default", QString("steps %1").arg(steps));

            if(listener != NULL)
                listener->onOperationNumSteps(steps);

            uint8_t *img = &image[ie->_offset];

            Sector *sector = _targetMemory.find(ie->_dwElementAddress + ie->_dwElementSize-1);
            if(sector != NULL)
            {
                QLog_Debug("Default", QString().sprintf("sec type 0x%02x", sector->_type));

                // Check if sector is writeable
                if(sector->_type & SECTOR_WRITEABLE)
                {
                    for (uint32_t p = 0; p < ie->_dwElementSize; p += _dfuDesc.wTransferSize)
                    {
                        uint32_t address = ie->_dwElementAddress + p;
                        int chunk_size = _dfuDesc.wTransferSize;

                        sector = _targetMemory.find(address);
                        if(sector != NULL)
                        {
                            // check if this is the last chunk
                            if (p + chunk_size > ie->_dwElementSize)
                                chunk_size = ie->_dwElementSize - p;

                            if(sector->_type & SECTOR_ERASABLE)
                            {
                                for (uint32_t addr = address; addr < address + chunk_size; addr += sector->_pagesize)
                                {
                                    if ((addr & ~(sector->_pagesize - 1)) != _last_page)
                                    {
                                        QLog_Debug("Default", QString().sprintf("erase page address 0x%08x", addr));
                                        erasePage(interface, addr);
                                    }
                                }

                                // Chunk extends into next page, erase it
                                if (((address + chunk_size - 1) & ~(sector->_pagesize - 1)) != _last_page)
                                {
                                    QLog_Debug("Default", QString().sprintf("> erase page address 0x%08x", address + chunk_size - 1));
                                    erasePage(interface, address + chunk_size - 1);
                                }
                            }

                            QLog_Debug("Default", QString().sprintf("image offset 0x%08x range %08x-%08x size %d",
                                        p, address, address + chunk_size - 1, chunk_size));

                            setAddress(interface, address);

                            res = download(interface, 2, img + p, chunk_size);
                            if(res < 0)
                            {
                                QLog_Error("Default", QString("ERROR DOWNLOADING DATA"));
                                return false;
                            }

                            do
                            {
                                if((res = getStatus(interface, &status)) <= 0)
                                {
                                    if (res != LIBUSB_ERROR_TIMEOUT)
                                    {
                                        QLog_Error("Default", QString("download error executing command "));
                                        return false;
                                    }

                                    SLEEP(status._bwPollTimeout);
                                    continue;
                                }

                                SLEEP(status._bwPollTimeout);

                            } while (status._bState != STATE_dfuDNLOAD_IDLE &&
                                     status._bState != STATE_dfuERROR &&
                                     status._bState != STATE_dfuMANIFEST);


                            if (status._bStatus != 0)
                            {
                                QLog_Error("Default", "download failed!");
                                QLog_Error("Default", QString().sprintf("state(%u) = %s, status(%u)",
                                           status._bState, state2string(status._bState), status._bStatus));
                                return -1;
                            }

                            if(listener != NULL)
                                listener->onOperationStep();
                        }
                    }

                    QLog_Debug("Default", "DOWNLOAD OK");

                    if(abort(interface) < 0)
                        QLog_Error("Default", "calling abort command");

                    if((res = getStatus(interface, &status)) <= 0)
                    {
                        if (res != LIBUSB_ERROR_TIMEOUT)
                        {
                            QLog_Error("Default", QString("> download error executing command "));
                            return false;
                        }

                        SLEEP(status._bwPollTimeout);
                        continue;
                    }

                    if (status._bState != STATE_dfuIDLE)
                        QLog_Error("Default", "Failed to enter idle state on abort");

                    SLEEP(status._bwPollTimeout);

//                    return true;
                }
            }
            else
                QLog_Error("Default", "SECTOR NOT FOUND !");
        }
 return true;

    }

    return false;
}

/*
+-----------+------------+----------+-----------+---------+--------+
| bmRequest | bRequest   |  wValue  |  wIndex   | wLength |  Data  |
+-----------+------------+----------+-----------+---------+--------+
| 00100001b | DFU_DETACH | wTimeout | Interface |  Zero   |  None  |
+-----------+------------+----------+-----------+---------+--------+
*/
int DFUHandler::detach(const uint16_t interface, const uint16_t timeout)
{
    if(_handle == NULL)
        return LIBUSB_ERROR_NO_DEVICE ;

    return libusb_control_transfer(_handle,
                        DFU_SET,				// bmRequestType
                        DFU_DETACH,				// bRequest
                        timeout,				// wValue
                        interface,				// wIndex
                        NULL,					// Data
                        0,						// wLength
                        USB_TIMEOUT);
}

/*
+-----------+---------------+----------+-----------+---------+--------+
| bmRequest |    bRequest   |  wValue  |  wIndex   | wLength |  Data  |
+-----------+---------------+----------+-----------+---------+--------+
| 00100001b | DFU_GETSTATUS |   Zero   | Interface |    6    | Status |
   ??????? 10100001b ????
+-----------+---------------+----------+-----------+---------+--------+
*/
int DFUHandler::getStatus(const uint16_t interface, DFUseStatus *status)
{
    int result = -1;

    if(_handle == NULL)
        return LIBUSB_ERROR_NO_DEVICE ;

    if(status != NULL)
    {
        status->reset();

        uint8_t buffer[DFUseStatus::SIZE];

        result = libusb_control_transfer(_handle,
                                         DFU_GET,
                                         DFU_GETSTATUS,
                                         0,
                                         interface,
                                         buffer,
                                         DFUseStatus::SIZE,
                                         USB_TIMEOUT);

        if (result == DFUseStatus::SIZE)
        {
            status->_bStatus = buffer[0];
            status->_bwPollTimeout  = (0xff & buffer[3]) << 16;
            status->_bwPollTimeout |= (0xff & buffer[2]) << 8;
            status->_bwPollTimeout |= (0xff & buffer[1]);
            status->_bState  = buffer[4];
            status->_iString = buffer[5];
        }
    }

    return result;
}

/*
+-----------+---------------+----------+-----------+---------+--------+
| bmRequest |    bRequest   |  wValue  |  wIndex   | wLength |  Data  |
+-----------+---------------+----------+-----------+---------+--------+
| 00100001b | DFU_CLRSTATUS |   Zero   | Interface |   Zero  |  None  |
+-----------+---------------+----------+-----------+---------+--------+
*/
int DFUHandler::clearStatus(const uint16_t interface)
{
    if(_handle != NULL)
        return libusb_control_transfer(_handle, DFU_SET, DFU_CLRSTATUS, 0, interface, NULL, 0, USB_TIMEOUT);

    return LIBUSB_ERROR_NO_DEVICE ;
}

/*
+-----------+---------------+----------+-----------+---------+--------+
| bmRequest |    bRequest   |  wValue  |  wIndex   | wLength |  Data  |
+-----------+---------------+----------+-----------+---------+--------+
| 00100001b | DFU_GETSTATE  |   Zero   | Interface |    1    | State  |
+-----------+---------------+----------+-----------+---------+--------+
*/
int DFUHandler::getState(const uint16_t interface)
{
    int result;
    uint8_t buffer[1];

    if(_handle == NULL)
        return LIBUSB_ERROR_NO_DEVICE ;

    result = libusb_control_transfer(_handle, DFU_GET, DFU_GETSTATE, 0, interface, buffer, 1, USB_TIMEOUT);

    if (result < 1)
        return result;

    return buffer[0];
}

/*
+-----------+---------------+----------+-----------+---------+--------+
| bmRequest |    bRequest   |  wValue  |  wIndex   | wLength |  Data  |
+-----------+---------------+----------+-----------+---------+--------+
| 00100001b |   DFU_ABORT   |   Zero   | Interface |   Zero  |  None  |
+-----------+---------------+----------+-----------+---------+--------+
*/
int DFUHandler::abort(const uint16_t interface)
{
    if(_handle == NULL)
        return LIBUSB_ERROR_NO_DEVICE ;

    return libusb_control_transfer(_handle, DFU_SET, DFU_ABORT, 0, interface, NULL, 0, USB_TIMEOUT);
}

/*
+-----------+---------------+-----------+-----------+---------+----------+
| bmRequest |    bRequest   |  wValue   |  wIndex   | wLength |  Data    |
+-----------+---------------+-----------+-----------+---------+----------+
| 00100001b |   DFU_DNLOAD  | wBlockNum | Interface | Length  | Firmware |
+-----------+---------------+-----------+-----------+---------+----------+
*/
int DFUHandler::download(const uint16_t interface, const uint16_t blocknum, uint8_t *data, const uint16_t length)
{
    if(_handle == NULL)
        return LIBUSB_ERROR_NO_DEVICE ;

    return libusb_control_transfer(_handle, DFU_SET, DFU_DNLOAD, blocknum, interface, data, length, USB_TIMEOUT);
}

/*
+-----------+---------------+----------+-----------+---------+----------+
| bmRequest |    bRequest   |  wValue  |  wIndex   | wLength |   Data   |
+-----------+---------------+----------+-----------+---------+----------+
| 10100001b |   DFU_UPLOAD  |    Zero  | Interface |  Length | Firmware |
+-----------+---------------+----------+-----------+---------+----------+
*/
int DFUHandler::upload(const uint16_t interface, uint8_t *data, const uint16_t length)
{
    if(_handle == NULL)
        return LIBUSB_ERROR_NO_DEVICE ;

    return libusb_control_transfer(_handle, DFU_GET, DFU_UPLOAD, 0, interface, data, length, USB_TIMEOUT);
}

/*
Set Address Pointer command
Byte 1: 0x21 - Set Address Pointer command
Byte 2: A[7:0] - LSB of the address pointer
Byte 3: A[15:8] - Second byte of the address pointer
Byte 4: A[22:16] - Third byte of the address pointer
Byte 5: A[31:23] - MSB of the address pointer

Erase command
Byte 1: 0x41 - Erase command
Byte 2: A[7:0] - LSB of the page address
Byte 3: A[15:8] - Second byte of the page address
Byte 4: A[22:16] - Third byte of the page address
Byte 5: A[31:23] - MSB of the page address

Read Unprotect command
Byte 1: 0x92 - Read Unprotect command
*/
bool DFUHandler::command(const uint16_t interface, DFUSE_COMMANDS cmd, uint32_t addr)
{
    uint8_t data[5];
    uint8_t size = 5;

    data[0] = cmd & 0xFF;
    data[1] = addr & 0xFF;
    data[2] = (addr >> 8) & 0xFF;
    data[3] = (addr >> 16) & 0xFF;
    data[4] = (addr >> 24) & 0xFF;

    switch (cmd)
    {
    case DFUSE_COMMAND_SET_ADDRESS:
        size = 5;
        break;

    case DFUSE_COMMAND_ERASE:
        if (addr <= 0)
            size = 1;
        break;

    case DFUSE_COMMAND_READ_UNPROTECT:
        size = 1;
        break;
    }


    int ret = download(interface, 0, data, size);
    if (ret < 0)
        QLog_Error("Default", "Error executing command ");

    return wait4Command(interface);
}

int DFUHandler::erasePage(const uint16_t interface, uint32_t addr)
{
    Sector *sector = _targetMemory.find(addr);
    if(sector != NULL)
    {
        if(sector->_type & SECTOR_ERASABLE)
        {
            QLog_Debug("Default", QString().sprintf("Erasing page size %i at address 0x%08x, page starting at 0x%08x",
                        sector->_pagesize, addr, addr & ~(sector->_pagesize - 1)));

            int res = command(interface, DFUSE_COMMAND_ERASE, addr);
            _last_page = addr & ~(sector->_pagesize - 1);
            return res;
        }
    }

    return -1;
}

int DFUHandler::erase(const uint16_t interface)
{
    return command(interface, DFUSE_COMMAND_ERASE, 0);
}

int DFUHandler::setAddress(const uint16_t interface, uint32_t addr)
{
    return command(interface, DFUSE_COMMAND_SET_ADDRESS, addr);
}

int DFUHandler::readUnprotect(const uint16_t interface)
{
    return command(interface, DFUSE_COMMAND_READ_UNPROTECT, 0);
}

bool DFUHandler::wait4Command(const uint16_t interface)
{
    int result = 0;
    DFUseStatus status;
    int counter = 0;

    do
    {
        if((result = getStatus(interface, &status)) <= 0)
        {
            if (result != LIBUSB_ERROR_TIMEOUT)
            {
                QLog_Error("Default", QString("1-Error executing command %d").arg(result));
                return false;
            }

            SLEEP(status._bwPollTimeout);
            continue;
        }

        SLEEP(status._bwPollTimeout);

        if (status._bState == STATE_dfuERROR)
        {
            if ((result = clearStatus(0)) < 0)
            {
                if (counter++ > 5)
                {
                    QLog_Debug("Default", QString("1-clearing status %1").arg(result));
                    return false;
                }
            }
        }

    } while (status._bState == STATE_dfuDNBUSY);

//qDebug() << "wait4Command > state = " << state2string(status._bState) << ", status = " << status._bStatus;
//qDebug("DFUse::wait4Command - state(%u) = %s, status(%u)", status._bState, state2string(status._bState), status._bStatus);

    return result > 0;
}

const char* status2string(int status)
{
    static const char *messages[] = {
        "STATUS_OK","STATUS_errTARGET","STATUS_errFILE","STATUS_errWRITE","STATUS_errERASE","STATUS_errCHECK_ERASED",
        "STATUS_errPROG","STATUS_errVERIFY","STATUS_errADDRESS","STATUS_errNOTDONE","STATUS_errFIRMWARE",
        "STATUS_errVENDOR","STATUS_errUSBR","STATUS_errPOR","STATUS_errUNKNOWN","STATUS_errSTALLEDPKT"
    };

    if (status >= STATUS_OK && status <= STATUS_errSTALLEDPKT)
        return messages[status];

    return NULL;
}

const char* state2string(int state)
{
    static const char *messages[] = {
        "appIDLE", "appDETACH", "dfuIDLE", "dfuDNLOAD-SYNC", "dfuDNBUSY", "dfuDNLOAD-IDLE",
        "dfuMANIFEST-SYNC", "dfuMANIFEST", "dfuMANIFEST-WAIT-RESET", "dfuUPLOAD-IDLE", "dfuERROR"
    };

    if (state >= STATE_appIDLE && state <= STATE_dfuERROR)
        return messages[state];

    return NULL;
}

/*
Each Alternate setting string descriptor must follow this memory mapping so that the PC
Host Software can decode the right mapping for the selected device :
? @ : To detect that this is a special mapping descriptor(to avoid decoding standard
descriptor)
? / : for separator between zones
? Maximum 8 digits per address starting by “0x”
? / : for separator between zones
? Maximum of 2 digits for the number of sectors
? * : For separator between number of sectors and sector size
? Maximum 3 digits for sector size between 0 and 999
? 1 digit for the sector size multiplier.Valid entries are : B(byte), K(Kilo), M(Mega)
? 1 digit for the sector type as follows :
– a(0x41) : Readable
– b(0x42) : Erasable
– c(0x43) : Readable and Erasabled(0x44) : Writeable
– e(0x45) : Readable and Writeable
– f(0x46) : Erasable and Writeable
– g(0x47) : Readable, Erasable and Writeable
Note : If the target memory is not contiguous, the user can add the new sectors to be decoded just
after a slash"/" as shown in the following example :
"@Flash /0xF000/1*4Ka/0xE000/1*4Kg/0x8000/2*24Kg"

@Target Memory Name/Start Address/Sector(1)_Count*Sector(1)_SizeSector(1)_Type,Sector(2)_Count*Sector(2)_SizeSector(2)_Type,...
...,Sector(n)_Count*Sector(n)_SizeSector(n)_Type

Interface Descriptor:
------------------------------
0x09	bLength
0x04	bDescriptorType
0x00	bInterfaceNumber
0x00	bAlternateSetting
0x00	bNumEndPoints
0xFE	bInterfaceClass   (Application Specific)
0x01	bInterfaceSubClass
0x02	bInterfaceProtocol
0x04	iInterface   "@Internal Flash  /0x08000000/04*016Kg,01*064Kg,07*128Kg"

*/
bool DFUHandler::parseMemoryMap(char *buffer, uint16_t size)
{
Q_UNUSED(size)

    bool ready = false;
    uint32_t address = 0;

    QStringList list = QString(buffer).split("/");
    for(int i = 0; i < list.size(); ++i)
    {
//        QLog_Debug("Default", QString(" %1").arg(list.at(i)));

        QString s = list.at(i);

        if(ready)
        {
            ready = false;
            QStringList sectorList = QString(s).split(",");
            for(int j = 0; j < sectorList.size(); ++j)
            {

                QString s1 = sectorList.at(j);

//                QLog_Debug("Default", QString(" %1").arg(s1));

                int pos = 0;
                if((pos = s1.indexOf('*')) > 0)
                {
                    int sec = s1.left(pos).toInt();

//                    QLog_Debug("Default", QString("sect %1").arg(sec));

                    QString ss = s1.right(s1.size()-(pos+1));

//                    QLog_Debug("Default", QString("xxx %1").arg(ss));

                    int p = 0;
                    do {

                        if(!ss.at(p).isDigit())
                            break;

                    } while(p++ < ss.size());

                    int siz = ss.left(p).toInt();

//                    QLog_Debug("Default", QString("size %1").arg(siz));

                    if(ss.at(p) == 'K')
                        siz *= 1024;
                    else if(ss.at(p) == 'M')
                        siz *= (1024 * 1024);

//                    QLog_Debug("Default", QString("size %1").arg(siz));
//                    QLog_Debug("Default", QString("multi %1, type %2").arg(ss.at(p)).arg(ss.at(p+1)));
//                    QLog_Debug("Default", QString().sprintf("address 0x%08X", address));

                    Sector *sector = new Sector();
                    sector->_start = address;
                    sector->_end = address + (sec * siz) - 1;
                    sector->_pagesize = siz;
                    sector->_type = getSectorType(ss.at(p+1).toLatin1());
                    address += sec * siz;
                    _targetMemory.add(sector);

//                    QLog_Debug("Default", QString().sprintf("Start 0x%08X, end 0x%08X, pagesize %d, type 0x%02X",
//                               sector->_start, sector->_end, sector->_pagesize, sector->_type));
                }
            }
        }

        if(s.startsWith("@"))
            _targetMemory._name = s;

        if(s.startsWith("0x") || s.startsWith("0X"))
        {
            address = s.toLong(NULL, 16);
  //          QLog_Debug("Default", QString().sprintf("address 0x%08X", address));
            ready = true;
        }
    }

//    QLog_Debug("Default", "---------------------------------------------");
//    QLog_Debug("Default", QString("TOTAL %1").arg(_targetMemory._sectors.size()));

    for(int i = 0; i < _targetMemory._sectors.size(); ++i)
    {
        Sector *ss = _targetMemory._sectors.at(i);
        QLog_Debug("Default", QString().sprintf("Start 0x%08X, end 0x%08X, pagesize %d, type 0x%02X",
                   ss->_start, ss->_end, ss->_pagesize, ss->_type));
    }

    return true;
}

static int getSectorType(char ch)
{
    struct SECTORTYPE
    {
        char c;
        int  type;

    } sectorType[] = {
        {'a', SECTOR_READABLE           },
        {'b', SECTOR_ERASABLE           },
        {'c', SECTOR_READ_ERASE         },
        {'d', SECTOR_WRITEABLE          },
        {'e', SECTOR_READ_WRITE         },
        {'f', SECTOR_ERASE_WRITE        },
        {'g', SECTOR_ERASE_READ_WRITE   },
    };

    for(size_t k = 0; k < sizeof(sectorType); ++k)
    {
        if(ch == sectorType[k].c)
            return sectorType[k].type;
    }

    return -1;
}

bool DFUHandler::leaveDFUMode(const uint16_t interface)
{
    QLog_Debug("Default", "Leaving DFU mode...");

    int ret = download(interface, 0, 0, 0);
    if (ret < 0)
    {
        QLog_Error("Default", "Error leaving DFU mode");
        return false;
    }

    if(!wait4Command(interface))
        return false;

    ret = libusb_reset_device(_handle);
    if (ret < 0 && ret != LIBUSB_ERROR_NOT_FOUND)
    {
        QLog_Error("Default", "error resetting after download");
        return false;
    }

    return true;
}
/*
DFU attributes:
    – Bit7: if bit1 is set, the device will have an accelerated upload speed of 4096 byes per upload
            command (bitCanAccelerate) 0: No 1:Yes
    – Bits 6:4: reserved
    – Bit 3: device will perform a bus detach-attach sequence when it receives a DFU_DETACH request.
            0 = no 1 = yes
            Note: The host must not issue a USB Reset. (bitWillDetach)
    – Bit 2: device is able to communicate via USB after Manifestation phase (bitManifestation tolerant)
            0 = no, must see bus reset - 1 = yes
    – Bit 1: upload capable (bitCanUpload)
            0 = no  1 = yes
    – Bit 0: download capable (bitCanDnload)
            0 = no  1 = yes
*/
void DFUHandler::parseAttributes(uint8_t bmAttributes)
{
    DFUAttributes attr;

    if(bmAttributes & ATTR_DNLOAD_CAPABLE)			attr._OptCanDnload = true;
    if(bmAttributes & ATTR_UPLOAD_CAPABLE)			attr._OptCanUpload = true;
    if(bmAttributes & ATTR_WILL_DETACH)				attr._OptCanDetach = true;
    if(bmAttributes & ATTR_MANIFESTATION_TOLERANT)	attr._OptCanManifestTolerant = true;
    if(bmAttributes & ATTR_ST_CAN_ACCELERATE)		attr._OptCanAccel = true;

    attr.debug();
}

