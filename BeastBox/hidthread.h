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
#ifndef HIDTHREAD_H
#define HIDTHREAD_H

#include <QThread>
#include <QList>
#include <QSharedPointer>
#include <QByteArray>

#include "hidhandler.h"

#include <vector>
#include <memory>

class HIDThread : public QThread
{
    Q_OBJECT
public:
    explicit HIDThread(HIDHandler *handler, QObject *parent = 0);

    void stop();

    static std::vector< uint8_t* > *getExportList();

    void download();

signals:
    void handleKey(KEYINFO *ki);
    void handleError(bool fatal);
    void setupProgressBar(int range);
    void updateTransferStatus(int pos, int curProfile, int profiles, int curPacket, int packets);
    void downloadDone();

public slots:


protected:
    HIDHandler   *_handler;
    volatile bool _running;
    volatile bool _transfer;

    static std::vector< uint8_t* > _list;

    void run();

    bool _readKey();
    bool _downloadData();
    void _configureProgress();

    static void _cleanupList();
};

#endif // HIDTHREAD_H
