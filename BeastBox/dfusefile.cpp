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
    UM0391
*/

#include "dfusefile.h"
#include "../common/crc32.h"

#include <QFile>
#include <QLogger/QLogger.h>

using namespace QLogger;

#define DFUPREFIX_SIZE			11
#define DFUSIGNATURE			"DfuSe"
#define DFUSIGNATURE_SIZE		4
#define DFUSUFFFIX_SIZE			16
#define SUFFIXSIGNATURE			"UFD"
#define SUFFIXSIGNATURE_SIZE	3
#define CRC32_SIZE				4
#define TARGETSIGNATURE			"Target"
#define TARGETSIGNATURE_SIZE	6
#define TARGETPREFIX_SIZE       274
#define IMAGEELEMENTHEADER_SIZE 8

TargetPrefix::TargetPrefix()
    : _bAlternateSetting(0)
    , _dwTargetSize(0)
    , _dwNbElements(0)
{
}

TargetPrefix::TargetPrefix( const TargetPrefix* other )
{
    _bAlternateSetting = other->_bAlternateSetting;
    _dwTargetSize = other->_dwTargetSize;
    _dwNbElements = other->_dwNbElements;

    memcpy(_bTargetNamed, other->_bTargetNamed, 4);
    memcpy(_szTargetName, other->_szTargetName, 254);
}

TargetPrefix& TargetPrefix::operator=( const TargetPrefix& other )
{
    _bAlternateSetting = other._bAlternateSetting;
    _dwTargetSize = other._dwTargetSize;
    _dwNbElements = other._dwNbElements;

    memcpy(_bTargetNamed, other._bTargetNamed, 4);
    memcpy(_szTargetName, other._szTargetName, 254);

    return *this;
}

ImageElement *TargetPrefix::addElement()
{
	ImageElement *ie = new ImageElement();
	_elements.push_back(QSharedPointer<ImageElement>(ie));
	return ie;
}

void TargetPrefix::destroy()
{
    _elements.clear();
}

///////////////////////////////////////////////////////
/// \brief DFUseFile::DFUseFile
///////////////////////////////////////////////////////
DFUseFile::DFUseFile()
    : _idVendor(0xffff)
    , _idProduct(0xffff)
    , _bcdDevice(0xffff)
    , _image(NULL)
    , _imageSize(0)
{
}

DFUseFile::~DFUseFile()
{
    close();
}

void DFUseFile::close()
{
/*    for (int i = 0; i < _targets.size(); ++i)
    {
        TargetPrefix *p = _targets.at(i);
        p->destroy();
        delete p;
    }
    */
    _targets.clear();

/*	if (_targets != NULL)
	{
		delete _targets;
		_targets = NULL;
    }*/


    if (_image != NULL)
    {
        delete[] _image;
        _image = NULL;
    }
}

/*
+------------+----------------+------------+
| DFU PREFIX |   DFU Images   | DFU SUFFIX |
+------------+----------------+------------+
                     |
                    \ /
                +-------------+-----------------+---------------+-------------+
                | DFU Image 1 | DFU Image2 .... | DFU Image N-1 | DFU Image N |
                +-------------+-----------------+---------------+-------------+
                     |
                    \ /
+---------------+-----------------+----------------+------------------+----------------+
| Target Prefix | Image Element 1 |	ImageElement 2 | ImageElement N-1 | ImageElement N |
+---------------+-----------------+----------------+------------------+----------------+


Prefix Format:
+-----------------+------+----------------+-----------+
|  0  1  2  3  4  |   5  |   6  7  8  9   |     10    |
+-----------------+------+----------------+-----------+
|  szSignature    | bVer |  DFUImageSize  |  bTargets |
|    “DfuSe”      | 0x01 |                |           |
+-----------------+------+----------------+-----------+

Suffix Format:
+------+-----+-----+-----+-----+-----+-----+----+----------------+---------+-------------+
|  0   |  1  |  2  |  3  |  4  |  5  |  6  |  7 |   8    9   10  |   11    | 12 13 14 15 |
+------+-----+-----+-----+-----+-----+-----+----+----------------+---------+-------------+
|  bcdDevice | dProduct  |  dVendor  |  bcdDFU  | ucDfuSignature | bLength |             |
|  Lo  | Hi  |  Lo | Hi  |  Lo |  Hi |  Lo | Hi |     (UFD)      |   16    |    dwCRC    |
|      |     |     |     |     |     |  1A | 01 |                |         |             |
+------+-----+-----+-----+-----+-----+-----+----+----------------+---------+-------------+
*/
bool DFUseFile::load(const char *filename)
{
    if (!loadFile(filename))
        return false;

    // Process prefix
    if (strncmp((const char *)_image, DFUSIGNATURE, DFUSIGNATURE_SIZE) != 0)
    {
        QLog_Error("Default", QString("invalid DFU prefix signature: %1").arg(filename));
        return false;
    }

    _imageSize  = _image[9] << 24;
    _imageSize |= _image[8] << 16;
    _imageSize |= _image[7] << 8;
    _imageSize |= _image[6];

    int totalTargets = _image[10];

    QLog_Debug("Default", QString("image has %1 bytes and %2 target(s)").arg(_imageSize).arg(totalTargets));


    if (totalTargets <= 0)
    {
        QLog_Error("Default", "invalid number of targets");
        return false;
    }

    if ((_fileSize + CRC32_SIZE)-_imageSize != DFUSUFFFIX_SIZE)
    {
        QLog_Error("Default", "invalid image size - no suffix space");
        return false;
    }

    // Process suffix
    uint16_t bcdDFU = (_image[_imageSize + 7] << 8) | _image[_imageSize + 6];

    if ((_image[_imageSize+11] != DFUSUFFFIX_SIZE) ||
        strncmp((const char *)&_image[_imageSize+8], SUFFIXSIGNATURE, SUFFIXSIGNATURE_SIZE) != 0 ||
        bcdDFU != 0x011A)
    {
        QLog_Error("Default", "invalid suffix DFU signature");
        return false;
    }

    _bcdDevice = (_image[_imageSize + 1] << 8) | _image[_imageSize + 0];
    _idProduct = (_image[_imageSize + 3] << 8) | _image[_imageSize + 2];
    _idVendor  = (_image[_imageSize + 5] << 8) | _image[_imageSize + 4];

    QLog_Debug("Default", QString().sprintf("device 0x%04X, vendor 0x%04X, product 0x%04X",
               _bcdDevice, _idVendor, _idProduct));

    return loadTargets(totalTargets);
}

bool DFUseFile::loadFile(const char *filename)
{
    uint32_t crc[2] = { 0, 0 };
    uint8_t buffer[CRC32_SIZE] = { 0 };

    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly))
    {
        QLog_Error("Default", QString().sprintf("Error opening %s for read", filename));
        return false;
    }

    _fileSize = (file.size() - CRC32_SIZE);

    // Load data, except CRC
    _image = new uint8_t[_fileSize];
    if (_image == NULL)
    {
        QLog_Error("Default", "cannot allocate memory");
        file.close();
        return false;
    }

    // data
    if(file.read((char *)_image, _fileSize) < 0)
    {
        QLog_Error("Default", QString("1-Could not read %1 bytes from %2").arg(_fileSize).arg(filename));
        file.close();
        return false;
    }

    // crc
    if(file.read((char *)buffer, CRC32_SIZE) < 0)
    {
        QLog_Error("Default", QString("1-Could not read %1 bytes from %2").arg(_fileSize).arg(filename));
        file.close();
        return false;
    }

    file.close();

    crc[0]  = buffer[3] << 24;
    crc[0] |= buffer[2] << 16;
    crc[0] |= buffer[1] << 8;
    crc[0] |= buffer[0];

    // Test CRC ...
    crc[1] = CRC32::crc32(_image, _fileSize);

    if (crc[0] != crc[1])
    {
        QLog_Error("Default", "invalid DFU crc");
        return false;
    }

    return true;
}

/*
Target Prefix Format:
+-------------+-------------------+--------------+--------------+------------------+-----------------+
| 0 1 2 3 4 5 |         6         |  7  8  9  10 |   11...265   | 266 267 268 269  | 270 271 272 273 |
+-------------+-------------------+--------------+--------------+------------------+-----------------+
| szSignature | bAlternateSetting | bTargetNamed | szTargetName |   dwTargetSize   |   dwNbElements  |
|  “Target”   |                   |              |              |                  |                 |
+-------------+-------------------+--------------+--------------+------------------+-----------------+

Image Element Format:
+------------------+---------------+-------+
|   0   1   2   3  |   4  5  6  7  | 8 ... |
+------------------+---------------+-------+
| dwElementAddress | dwElementSize | Data  |
+------------------+---------------+-------+
*/
bool DFUseFile::loadTargets(int totalTargets)
{
    uint32_t offset = DFUPREFIX_SIZE;

    for (int i = 0; i < totalTargets; ++i)
    {
        if (strncmp((const char *)&_image[offset], TARGETSIGNATURE, TARGETSIGNATURE_SIZE) != 0)
        {
            QLog_Error("Default", "Target signature error");
            return false;
        }

        TargetPrefix *p = (TargetPrefix *) &_image[offset + TARGETSIGNATURE_SIZE];
        TargetPrefix *target = new TargetPrefix(p);
        if (target == NULL)
        {
            QLog_Error("Default", "Memory allocation error");
            return false;
        }

        offset += TARGETPREFIX_SIZE;

        uint32_t os = offset;

        for(uint32_t k = 0; k < target->_dwNbElements; ++k)
        {
			ImageElement *ie = target->addElement();
			ie->_dwElementAddress = ((ImageElement *)&_image[os])->_dwElementAddress;
			ie->_dwElementSize = ((ImageElement *)&_image[os])->_dwElementSize;

            os += IMAGEELEMENTHEADER_SIZE;
            ie->_offset = os;
            os += ie->_dwElementSize;

/*            ImageElement *ie = new ImageElement();
            ie->_dwElementAddress = ((ImageElement *) &_image[os])->_dwElementAddress;
            ie->_dwElementSize = ((ImageElement *) &_image[os])->_dwElementSize;

            os += IMAGEELEMENTHEADER_SIZE;
            ie->_offset = os;
            os += ie->_dwElementSize;

            target->addElement(ie);*/
        }

        _targets.push_back(QSharedPointer<TargetPrefix>(target));
    }


    QLog_Debug("Default", QString("Num targets %1").arg(_targets.size()));

    for(int a = 0; a < _targets.size(); ++a)
    {
		QSharedPointer<TargetPrefix> target = _targets.at(a);

        QLog_Debug("Default", QString().sprintf("_bAlternateSetting 0x%02X, %s, %s, %d, %d",
                   target->_bAlternateSetting, (char*)target->_bTargetNamed,
                    (char*)target->_szTargetName, target->_dwTargetSize, target->_dwNbElements));

        QLog_Debug("Default", QString("Num elements: %1").arg(target->_dwNbElements));

        for(uint32_t b = 0; b < target->_dwNbElements; ++b)
        {
            ImageElement *ie = target->getElement(b);

            QLog_Debug("Default", QString().sprintf("element addr   0x%08X", ie->_dwElementAddress));
            QLog_Debug("Default", QString().sprintf("element size   0x%08X", ie->_dwElementSize));
            QLog_Debug("Default", QString().sprintf("element offset 0x%08X", ie->_offset));
        }
    }

    return true;
}

TargetPrefix *DFUseFile::getTargetPrefix(uint8_t bAlternateSetting)
{
    for (int i = 0; i < _targets.size(); ++i)
    {
		QSharedPointer<TargetPrefix> p = _targets.at(i);
        if (p->_bAlternateSetting == bAlternateSetting)
            return p.data();
    }

    return NULL;
}
