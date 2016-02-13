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
#ifndef DFUSEFILE_H
#define DFUSEFILE_H

#include <stdint.h>
#include <QList>
#include <QSharedPointer>

class ImageElement
{
public:
    ImageElement()
        : _dwElementAddress(0)
        ,_dwElementSize(0)
        ,_offset(0)
    {
    }

    uint32_t _dwElementAddress;
    uint32_t _dwElementSize;
    uint32_t _offset;
};

class TargetPrefix
{
public:
    TargetPrefix();
    TargetPrefix( const TargetPrefix* other );
    TargetPrefix& operator=( const TargetPrefix& other );
	ImageElement *addElement();
    void destroy();
    inline uint32_t getNumElements() { return _dwNbElements; }
    inline ImageElement *getElement(uint32_t idx) { return _elements.at(idx).data(); }

//BYTE SIGNATURE - "Target"
    uint8_t  _bAlternateSetting;
    uint8_t  _bTargetNamed[4];
    uint8_t  _szTargetName[254];
    uint32_t _dwTargetSize;
    uint32_t _dwNbElements;

	QList< QSharedPointer<ImageElement> > _elements;
};


class DFUseFile
{
public:
    DFUseFile();
    virtual ~DFUseFile();

    bool load(const char *filename);
    void close();

    inline uint8_t getNumTargets() { return _targets.size(); }
    inline uint16_t getVID() { return _idVendor; }
    inline uint16_t getPID() { return _idProduct; }
    inline uint16_t getDeviceVersion() { return _bcdDevice; }

    TargetPrefix *getTargetPrefix(uint8_t bAlternateSetting);

    inline uint8_t *getImage() { return _image; }

protected:
    bool loadFile(const char *filename);
    bool loadTargets(int totalTargets);

private:
    uint16_t _idVendor;
    uint16_t _idProduct;
    uint16_t _bcdDevice;

    uint8_t *_image;
    uint32_t _imageSize;

    uint32_t _fileSize;

    QList< QSharedPointer<TargetPrefix> > _targets;
};

#endif // DFUSEFILE_H
