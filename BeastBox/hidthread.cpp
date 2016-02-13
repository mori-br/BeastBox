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
#include "hidthread.h"
#include "util.h"
#include "profile.h"

#include <QDebug>
#include <QLogger/QLogger.h>

#define THREAD_TIMEOUT              3000000//5000000
#define THREAD_TRANSFER_TIMEOUT     500

using namespace QLogger;

std::vector< uint8_t *> HIDThread::_list;

HIDThread::HIDThread(HIDHandler *handler, QObject *parent)
    : QThread(parent)
    , _handler(handler)
    , _running(true)
    , _transfer(false)
{
}

void HIDThread::run()
{
    static long counter = THREAD_TIMEOUT+1;

    while (_running)
    {
        if(_handler->isConnected())
        {
            if(!_transfer)
            {
                if(counter++ > THREAD_TIMEOUT)
                {
                    counter = 0;
                    if(!_readKey())
                    {
                        QLog_Error("Default", "Error reading device key");

                        // On Error kill the thread
                        emit handleError(true);
                        break;
                    }
                }
            }
            else
            {
                if(!_downloadData())
                {
                    QLog_Error("Default", "Error downloading profiles");

                    // On Error kill the thread
                    emit handleError(false);
                    break;
                }

                _cleanupList();

                QLog_Debug("Default", "Profile download finished");

                emit downloadDone();

                _transfer = false;
            }
        }
    }
}

void HIDThread::stop()
{
    QLog_Debug("Default", "Stopping HID thread");

    _running = false;
}

void HIDThread::download()
{
    _transfer = true;
}

bool HIDThread::_readKey()
{
    KEYINFO ki;
    memset(&ki, 0, sizeof(KEYINFO));
    if(_handler->receiveKey(&ki))
    {
        if(ki.vid > 0 && ki.pid > 0 && ki.source < MAX_KEY_SOURCE)
        {
qDebug() << "Emiting key";

            emit handleKey(&ki);
        }

        return true;
    }

    return false;
}

std::vector< uint8_t* > *HIDThread::getExportList()
{
    _cleanupList();

    return &_list;
}

bool HIDThread::_downloadData()
{
    int pos = 1;
    _configureProgress();

    // Erasing the flash
    emit updateTransferStatus(pos++, 0,0,0,0);

    if(!_handler->eraseFlash())
        return false;

//qDebug() << "profiles: " << _list.size();

    for(size_t i = 0; i < _list.size(); ++i)
    {
        const uint8_t *p = _list.at(i);

//Util::LOG_ARRAY(p, PROFILE_SIZE);

        int packets = _handler->getNumPackets(PROFILE_SIZE);
//qDebug() << "packets: " << packets;
        int sent = 0;

        uint8_t buffer[USB_REPORT_PKT_SIZE];
        for(int j = 0; j < packets; ++j)
        {
            int len = (PROFILE_SIZE - sent) > USB_REPORT_PKT_SIZE ? USB_REPORT_PKT_SIZE : (PROFILE_SIZE - sent);
//qDebug() << "len: " << len;

            memset(buffer, 0, USB_REPORT_PKT_SIZE);
            memcpy(buffer, &p[sent], len);

            if(!_handler->sendFirmware(buffer, USB_REPORT_PKT_SIZE))
            {
                QLog_Error("Default", "Error sending data to device");
                return false;
            }

            emit updateTransferStatus(pos++, i+1, _list.size(), j+1, packets);

            sent += len;

            QThread::msleep(THREAD_TRANSFER_TIMEOUT);
        }

        QThread::msleep(THREAD_TRANSFER_TIMEOUT);

//qDebug() << "checking transfer state";
        if(!_handler->checkTransfer())
            return false;
    }

//qDebug() << "download finished";

    return true;
}

void HIDThread::_configureProgress()
{
    int packets = 0;
    for(size_t i = 0; i < _list.size(); ++i)
    {
        packets += _handler->getNumPackets(PROFILE_SIZE);
    }

    emit setupProgressBar(packets+1);
}


void HIDThread::_cleanupList()
{
    for(size_t i = 0; i < _list.size(); ++i)
        delete _list.at(i);

    _list.clear();
}
