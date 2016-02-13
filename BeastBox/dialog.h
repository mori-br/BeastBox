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
#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <libusb.h>
#include "hidhandler.h"
#include "dfuhandler.h"
#include "usbdevice.h"

#include <QProgressBar>
#include <QListWidgetItem>
#include <QLabel>
#include <QMenu>
#include <QTimer>

#include "profile.h"
#include "dfusefile.h"
#include "hidthread.h"

#include "keywidgetcontainer.h"


typedef enum
{
    TAB_MAIN_PROFILES,
    TAB_MAIN_FIRMWARE,
    TAB_MAIN_ABOUT,

} MAIN_TABID;

namespace Ui {

class Dialog;
}

////////////////////////////////////////////////////
/// \brief The DFUProcessor class
////////////////////////////////////////////////////
class DFUProcessor : public IOperationListener
{
public:
    explicit DFUProcessor();
    ~DFUProcessor();
    void setup(QProgressBar *progress, QLabel *lbl);
    void onOperationNumSteps(uint16_t steps);
    void onOperationStep();

private:
    QProgressBar *_progress;
    QLabel       *_label;
    uint16_t      _step;
    uint16_t      _steps;
};

////////////////////////////////////////////////////
/// \brief The DeviceInfo class
////////////////////////////////////////////////////
class DeviceInfo
{
public:
    class VidPidInfo
    {
    public:
        uint16_t vid;
        uint16_t pid;
        uint8_t  proto;
    };

    DeviceInfo()
    {
        reset();
    }

    void reset()
    {
        firmwareVersion = 0;
        numProfiles = 0;
        maxProfiles = 0;
        mouseInfo.pid = 0;
        mouseInfo.vid = 0;
        mouseInfo.proto = 0;
        actionInfo.vid = 0;
        actionInfo.pid = 0;
        actionInfo.proto = 0;
        profileSize = 0;
    }

    uint16_t firmwareVersion;
    uint16_t numProfiles;
    uint16_t maxProfiles;
    uint16_t profileSize;

    VidPidInfo mouseInfo;
    VidPidInfo actionInfo;
};

////////////////////////////////////////////////////
/// \brief The Dialog class
////////////////////////////////////////////////////
class Dialog : public QDialog
{
    Q_OBJECT
public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();

signals:
    void signal_DeviceDisconnected();
    void signal_DeviceConnected();
    void handleKey(KEYINFO *ki);

public slots:
    void tick();
    void buttonClicked();
    void loadFirmware();
    void updateFirmware();
    void onListDoubleClick(QListWidgetItem *item);
    void deviceDisconnected();
    void deviceConnected();
    void buttonCloseClicked();
    void onHandleKey(KEYINFO *ki);
    void onHandleError(bool fatal);
    void onShowContextMenu(const QPoint &pos);

    void onEdit();
    void onDelete();
    void onToggleEnable();
    void onDownloadDone();

    void onStartListen(KeyWidgetContainer *key);
    void onStopListen(KeyWidgetContainer *key);

    void selectListItem(const Profile *p);

#ifndef Q_OS_WIN
//    void monitorTimerTick();
#endif

private:
    Ui::Dialog      *ui;

    UsbDevice       _device;
    HIDHandler      _hid;
    DFUHandler      _dfu;
    bool            _initialized;
    DFUProcessor    _dfuProcessor;
    QTimer          _timer;
    QString         _dfuFileName;
    DeviceInfo      _devInfo;
    DFUseFile       _firmware;
    uint16_t        _currentVersion;
    bool            _processingUpdate;
    int             _timerStep;
    HIDThread      *_thread;
    QMenu           _contextMenu;
    QAction        *_contextActions[3];

    QSharedPointer<QWidget> _transfDlg;
    QSharedPointer<QWidget> _waitDlg;

    KeyWidgetContainer  *_focus;

#ifndef Q_OS_WIN
    QTimer          *_usbMonitorTimer;

    struct udev             *_udev;
    struct udev_device      *_dev;
    struct udev_monitor     *_mon;
    int                     _fd;
#endif

    void _loadSettings();
    bool _loadGames();
    bool _loadProfiles();
    void _displayFileOpenError(const QString &path, const LoadError err);
    void _listProfiles(int selected = -1);

    bool _editProfile(Profile *profile, const QString &title, int index=-1);
    void _switchMode(bool bDFU);
    void _setupVersions();
    void _downloadFirmware();
    void _createContextMenu();
    void _resetControls();
    void _showWaitDeviceDlg(bool show);
    void _showExportDataDlg(bool show);
    void _setupProfileProgress();
    void _save();
    void _syncProfiles();

protected:
    void keyPressEvent(QKeyEvent *event);

#ifdef Q_OS_WIN
//    bool nativeEvent(const QByteArray& eventType, void* message, long* result);
#endif

};

#endif // DIALOG_H
