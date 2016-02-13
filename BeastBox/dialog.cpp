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
#include "dialog.h"
#include "ui_dialog.h"
#include <QStyleFactory>
#include <QTimer>
#include "profiledlg.h"
#include "listdelegate.h"
#include "util.h"
#include "about.h"
#include "../common/ps.h"
#include "../common/crc32.h"
#include "version.h"

#include <QLogger/QLogger.h>
#include <QDebug>
#include <QFileDialog>
#include <QSettings>
#include <QMessageBox>
#include <QKeyEvent>
#include <QMovie>

#include <vector>
#include <memory>

#ifdef Q_OS_WIN
    #include <dbt.h>

//    static GUID GUID_DEV = { 0x4D1E55B2, 0xF16F, 0x11CF, { 0x88, 0xCB, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30 } };
#else
    #include "libudev.h"
#endif

using namespace QLogger;

#define TIMER_INTERVAL      500//1000
#define TIMER_INTERVAL1     TIMER_INTERVAL//5000
#define TIMER_INTERVAL2     1000

#define GAMEDEF_FILE_NAME   "games.gdf"
#define PROFILE_FILE_NAME   "profiles.gpf"

#define INI_GAMEDEF_KEY     "gamedef.path"
#define INI_PROFILE_KEY     "profile.path"




Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
    , _initialized(false)
    , _currentVersion(0)
    , _processingUpdate(false)
#ifndef Q_OS_WIN
    ,_usbMonitorTimer(0)
    ,_udev(0)
    ,_dev(0)
    ,_mon(0)
    ,_fd(0)
#endif
    , _timerStep(0)
    , _thread(NULL)
    , _focus(NULL)
{
    // Remove question mark from window title
    Qt::WindowFlags flags = windowFlags();
    Qt::WindowFlags helpFlag = Qt::WindowContextHelpButtonHint;
    flags = flags & (~helpFlag);
    setWindowFlags(flags);

    ui->setupUi(this);

    setWindowTitle(DEVICE_NAME);

    ui->textEdit->setHtml(string);

    ui->tabWidget->setCurrentIndex(TAB_MAIN_PROFILES);

    ui->btn_newProfile->setEnabled(true);
    ui->btn_edtProfile->setEnabled(false);
    ui->btn_updProfile->setEnabled(false);
    ui->btn_delProfile->setEnabled(false);
    ui->btn_cloneProfile->setEnabled(false);
    ui->btn_switch->setEnabled(false);

    ui->btn_load->setEnabled(false);
    ui->btn_update->setEnabled(false);
    ui->pb_firmware->setValue(0);

    ui->tabWidget->setEnabled(false);

    _resetControls();

    connect(ui->btn_newProfile, SIGNAL(clicked()), this, SLOT(buttonClicked()));
    connect(ui->btn_edtProfile, SIGNAL(clicked()), this, SLOT(buttonClicked()));
    connect(ui->btn_updProfile, SIGNAL(clicked()), this, SLOT(buttonClicked()));
    connect(ui->btn_delProfile, SIGNAL(clicked()), this, SLOT(buttonClicked()));
    connect(ui->btn_cloneProfile, SIGNAL(clicked()), this, SLOT(buttonClicked()));

    connect(ui->btn_switch, SIGNAL(clicked()), this, SLOT(buttonClicked()));

    connect(ui->btn_load, SIGNAL(clicked()), this, SLOT(loadFirmware()));
    connect(ui->btn_update, SIGNAL(clicked()), this, SLOT(updateFirmware()));

    connect(ui->lst_profiles, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(onListDoubleClick(QListWidgetItem *)));

    connect(this, SIGNAL(signal_DeviceDisconnected()), this, SLOT(deviceDisconnected()));
    connect(this, SIGNAL(signal_DeviceConnected()), this, SLOT(deviceConnected()));

    connect(&_timer, SIGNAL(timeout()), this, SLOT(tick()));

    _timer.setInterval(TIMER_INTERVAL);
    _timer.start();

    ui->btn_updProfile->setToolTip(tr("Transfer all enabled profiles to %1 device").arg(DEVICE_NAME));
    ui->btn_newProfile->setToolTip(tr("Create a new game profile"));
    ui->btn_edtProfile->setToolTip(tr("Edit selected game profile"));
    ui->btn_delProfile->setToolTip(tr("Delete selected game profile"));
    ui->btn_cloneProfile->setToolTip(tr("Create a new profile based on selected profile"));

#ifdef Q_OS_WIN
/*
    // Register for device connect notification
    //
    DEV_BROADCAST_DEVICEINTERFACE filter;
    ZeroMemory( &filter, sizeof(DEV_BROADCAST_DEVICEINTERFACE) );

    filter.dbcc_size        = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    filter.dbcc_devicetype  = DBT_DEVTYP_DEVICEINTERFACE;
    filter.dbcc_classguid   = GUID_DEV;

    HDEVNOTIFY hDeviceNotify =
                RegisterDeviceNotification((HANDLE)winId(), &filter, DEVICE_NOTIFY_WINDOW_HANDLE );

    if(hDeviceNotify == NULL)
    {
        DWORD dw = GetLastError();
        QLog_Error("Default", QString().sprintf("Error: Failed to register device notification! err: %ld", dw));
    }
*/
#else
/*    // NOT TESTED
    _udev = udev_new();
    if(_udev == NULL)
    {
        QLog_Error("Default", "Failed to create udev!");
    }
    else
    {
        // Set up a monitor to monitor usb devices
        _mon = udev_monitor_new_from_netlink(_udev, "udev");

        // We want only to receive information about usb devices
        udev_monitor_filter_add_match_subsystem_devtype(_mon, "usb", "usb_device");
        udev_monitor_enable_receiving(_mon);
        _fd = udev_monitor_get_fd(_mon);

        // Start timer that will periodicaly check if hardware is connected
        _usbMonitorTimer = new QTimer(this);
        connect(_usbMonitorTimer, SIGNAL(timeout()), this, SIGNAL(monitorTimerTick()));
        _usbMonitorTimer->start(250);
    }*/
#endif

    _initialized = _device.init();

    _dfuProcessor.setup(ui->pb_firmware, ui->lbl_update_text);

    _loadSettings();

    ui->lst_profiles->setItemDelegate(new ListDelegate(ui->lst_profiles));

    QLog_Debug("Default", "Starting program...");

    _listProfiles();

    ui->lst_profiles->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(ui->lst_profiles,
            SIGNAL(customContextMenuRequested(const QPoint &)),
            this,
            SLOT(onShowContextMenu(const QPoint &))
            );

    _createContextMenu();
}

Dialog::~Dialog()
{
    _save();

    if(_thread != NULL)
    {
        _thread->stop();
        _thread->quit();
        _thread->wait();
        delete _thread;
        _thread = NULL;
    }

    // Let's back to HID mode
    if(_device.isDFU())
        _switchMode(false);

    _timer.stop();

    _hid.close();
    _dfu.close();

    if(_initialized)
        _device.close();

    _device.deinit();

    delete ui;
}

/**
 * @brief Dialog::tick
 * Timer tick used to check usb connections
 */
void Dialog::tick()
{
    QTimer *timer = (QTimer *)sender();

    if(!_initialized)
    {
        QLog_Error("Default", "closing windows");
        Dialog::close();
        return;
    }

    if(_timerStep == 0)
    {
        _showWaitDeviceDlg(true);
        _timerStep = 1;
        timer->setInterval(TIMER_INTERVAL1);
        QLog_Debug("Default", "Setting up timer interval");
    }
    else if(_timerStep == 1)
    {
        if (_device.tryToOpen())
        {
            if(_device.isDFU())
            {
                if(_dfu.open(_device.getHandle(), _device.getConfigDescriptor()))
                {
                    timer->stop();
                    QLog_Debug("Default", "DFU device open OK");

                    ui->tabWidget->setCurrentIndex(TAB_MAIN_FIRMWARE);

                    // Enable firmaware page
                    ui->tab1->setEnabled(false);
                    ui->tabWidget->setEnabled(true);

                    ui->btn_load->setEnabled(true);
                    ui->btn_switch->setEnabled(true);
                    _showWaitDeviceDlg(false);
                    ui->lbl_status->setText(QString(tr("%1 device is connected in DFU mode")).arg(DEVICE_NAME));

                    if(_processingUpdate)
                    {
                        _processingUpdate = false;
                        _downloadFirmware();
                    }
                }
            }
            else
            {
                if(_hid.open(_device.getHandle(), NULL))
                {
                    timer->stop();
                    QLog_Debug("Default", "HID device open OK");

                    ui->tabWidget->setCurrentIndex(TAB_MAIN_PROFILES);

                    // Enable pages
                    ui->tab1->setEnabled(true);
                    ui->tabWidget->setEnabled(true);

                    ui->btn_switch->setEnabled(true);
                    _showWaitDeviceDlg(false);
                    ui->lbl_status->setText(QString(tr("%1 device is connected")).arg(DEVICE_NAME));

                    _hid.sendCommand(CMD_STARTSETUP);

                    _hid.getFirmwareVersion(&_devInfo.firmwareVersion);
                    _hid.getProfileSize(&_devInfo.profileSize);

                    if(_devInfo.profileSize != sizeof(ProfileData))
                    {
                        QMessageBox* msg = new QMessageBox(this);
                        msg->setWindowTitle(tr("Profile Size"));
                        msg->setStyleSheet("QLabel { color: #FFF; }");
                        msg->setText(tr("The size of profile are different"));
                        msg->exec();
                    }

                    _setupVersions();
                    _setupProfileProgress();
                    _syncProfiles();

                    ui->btn_updProfile->setEnabled(true);

                    _thread = new HIDThread(&_hid, this);

                    // Connect our thread signals to local slots
                    connect(_thread, SIGNAL(handleKey(KEYINFO *)), this, SLOT(onHandleKey(KEYINFO *)));
                    connect(_thread, SIGNAL(handleError(bool)), this, SLOT(onHandleError(bool)));
                    connect(_thread, SIGNAL(downloadDone()), this, SLOT(onDownloadDone()));

                    _thread->start();

                    emit signal_DeviceConnected();
                }
            }
        }
        else
        {
            _resetControls();
            _showWaitDeviceDlg(true);
        }
    }
    else if(_timerStep == 3)
    {
        qApp->quit();
    }
}

/**
 * @brief Dialog::buttonClicked
 */
void Dialog::buttonClicked()
{
    QToolButton *button = (QToolButton *)sender();
    if(button == ui->btn_newProfile)
    {
        Profile newProfile;
        _editProfile(&newProfile, "Untitled");
    }
    else if(button == ui->btn_edtProfile)
    {
        onEdit();
    }
    else if(button == ui->btn_delProfile)
    {
        onDelete();
    }
    else if(button == ui->btn_cloneProfile)
    {
        QListWidgetItem *item = ui->lst_profiles->currentItem();
        if(item != NULL)
        {
            int index = item->data(Qt::UserRole + 4).toInt();
            const QSharedPointer<Profile> p = ProfileStream::instance()->profiles()->at(index);

            // Create a clone, adding a 'clone' at the end of profile's name
            Profile cloneProfile;
            cloneProfile.clone(p.data());

            // Add profile to profile list
            Profile *pp = ProfileStream::instance()->add(cloneProfile);

            // Get last added index for selection
            const int idx = ProfileStream::instance()->getProfileIndexByPtr(pp);

            // Save and show list
            _save();
            _listProfiles(idx);
        }
    }
    else if(button == ui->btn_updProfile)
    {
        // Update device with profiles
        if(_thread != NULL)
        {
            _showExportDataDlg(true);

            std::vector< uint8_t* > *list = HIDThread::getExportList();

            if(ProfileStream::instance()->exportToByteArray(list))
            {
                if(_hid.setupSendFirmware(list->size()))
                    _thread->download();
            }
        }
    }
    else if(button == ui->btn_switch)
    {
        ui->pb_profiles->setValue(0);

        if(_device.getHandle() != NULL)
            _switchMode(!_device.isDFU());
    }
}

/**
 * @brief Dialog::loadFirmware
 */
void Dialog::loadFirmware()
{
    _dfuFileName =
        QFileDialog::getOpenFileName(this,
                                     tr("Open File"),
                                     "",
                                     "DFU Files (*.dfu);;All Files (*.*)");

    if (!_dfuFileName.isEmpty())
    {
        if(!_firmware.load(_dfuFileName.toLatin1().data()))
        {
            QMessageBox* msg = new QMessageBox(this);
            msg->setWindowTitle(tr("Load File Error"));
            msg->setStyleSheet("QLabel { color: #FFF; }");
            msg->setText(tr("Error loading firmware from ") + _dfuFileName);
            msg->exec();

            return;
        }

        ui->btn_update->setEnabled(true);
    }
}

/**
 * @brief Dialog::updateFirmware
 */
void Dialog::updateFirmware()
{
    if(!_device.isDFU())
    {
        _currentVersion = _devInfo.firmwareVersion;
        _processingUpdate = true;
        _switchMode(true);
    }
    else
    {
        _downloadFirmware();
    }
}

/**
 * @brief Dialog::_downloadFirmware
 */
void Dialog::_downloadFirmware()
{
    ui->btn_update->setEnabled(false);

    if(_currentVersion != 0)
    {
        QString s1 = QString().sprintf("%d.%02d",((uint8_t)((_currentVersion >> 8) & 0xff)), ((uint8_t)(_currentVersion & 0xff)));
        QString s2 = QString().sprintf("%d.%02d",((uint8_t)((_firmware.getDeviceVersion() >> 8) & 0xff)),((uint8_t)(_firmware.getDeviceVersion() & 0xff)));

        QString s = tr("Do you really want to update your device from version %1 to %2 ?").arg(s1).arg(s2);

        if(QMessageBox::question(this,
                                tr("Firmware Confirmation"),
                                "<font color=\"#FFF\" size=\"4\">"+s+"</font>",
                                QMessageBox::Yes|QMessageBox::No) == QMessageBox::No)
        {
            return;
        }
    }

    ui->btn_load->setEnabled(false);

    QApplication::setOverrideCursor(Qt::WaitCursor);

    _dfu.download(0, &_firmware, &_dfuProcessor);

    QApplication::restoreOverrideCursor();

    _processingUpdate = false;
    _switchMode(false);

    ui->btn_load->setEnabled(true);
}

/**
 * @brief Dialog::_loadSettings
 */
void Dialog::_loadSettings()
{
    QSettings settings(INI_FILE_NAME, QSettings::IniFormat);
    QString pathGameDef  = settings.value(INI_GAMEDEF_KEY, "").toString();
    QString pathProfiles = settings.value(INI_PROFILE_KEY, "").toString();

    if(pathGameDef.isEmpty() && pathProfiles.isEmpty())
    {
        QString appPath;
        if(Util::getApplicationPath(appPath))
        {
            pathGameDef  = appPath + GAMEDEF_FILE_NAME;
            pathProfiles = appPath + PROFILE_FILE_NAME;

            settings.setValue(INI_GAMEDEF_KEY, pathGameDef);
            settings.setValue(INI_PROFILE_KEY, pathProfiles);
        }
    }

    _loadGames();

    _loadProfiles();
}

/**
 * @brief Dialog::_loadGames
 * Load all games definitions from .gdf file
 * @return
 */
bool Dialog::_loadGames()
{
    QString path;
    LoadError err;

    if(Util::getApplicationPath(path))
    {
        path += QString(GAMEDEF_FILE_NAME);

        if(GameStream::instance()->load(path, &err))
            return true;

        _displayFileOpenError(path, err);
    }

    return false;
}

/**
 * @brief Dialog::_loadProfiles
 * @return
 */
bool Dialog::_loadProfiles()
{
    QString path;

    if(Util::getApplicationPath(path))
    {
        path += QString(PROFILE_FILE_NAME);
        if(ProfileStream::instance()->load(path, NULL))
            return true;
    }

    return false;
}

/**
 * @brief Dialog::_displayFileOpenError
 * @param path
 * @param err
 */
void Dialog::_displayFileOpenError(const QString &path, const LoadError err)
{
    _timerStep = 2;

    QString s;
    if(err == LOAD_ERROR_FILENOTFOUND)
        s = tr("The file %1 was not found.\nUse GDE to create your game definitions").arg(path);
    else if(err == LOAD_ERROR_SIGNATURE)
        s = tr("The file %s have an invalid signature").arg(path);
    else if(err == LOAD_ERROR_VERSION)
        s = tr("The file %s have an invalid version").arg(path);
    else
        s = tr("Unknown error opening %s").arg(path);

    QMessageBox::critical(this, tr("Load File Error"), "<font color=\"#FFF\" size=\"4\">"+s+"</font>");

    Dialog::close();
    _timerStep = 3;
}

/**
 * @brief Dialog::_listProfiles
 * @param selected
 */
void Dialog::_listProfiles(int selected)
{
    ui->lst_profiles->clear();

    for(int i = 0; i < ProfileStream::instance()->profiles()->size(); ++i)
    {
        const QSharedPointer<Profile> p = ProfileStream::instance()->profiles()->at(i);

        QListWidgetItem *item = new QListWidgetItem();
        item->setData(Qt::DisplayRole, p->getName());
        item->setData(Qt::DecorationRole, p->getGameDef()->getImage());
        item->setData(Qt::UserRole + 1, p->getDescription());
        item->setData(Qt::UserRole + 2, p->getGameDef()->getName());
        item->setData(Qt::UserRole + 3, p->getColor());
        item->setData(Qt::UserRole + 4, i);
        item->setData(Qt::UserRole + 5, p->getConsoleType());
        item->setData(Qt::UserRole + 6, p->isEnabled());

        ui->lst_profiles->addItem(item);
    }

    int count = ProfileStream::instance()->profiles()->size();

    if(count > 0 && selected < 0)
        ui->lst_profiles->setCurrentRow(0);
    else if(selected >= 0)
        ui->lst_profiles->setCurrentRow(selected);

    ui->btn_edtProfile->setEnabled(count > 0);
    ui->btn_cloneProfile->setEnabled(count > 0);
    ui->btn_delProfile->setEnabled(count > 0);
}

/**
 * @brief Dialog::onListDoubleClick
 * @param item
 */
void Dialog::onListDoubleClick(QListWidgetItem *item)
{
    int index = item->data(Qt::UserRole + 4).toInt();
    const QSharedPointer<Profile> p = ProfileStream::instance()->profiles()->at(index);
    _editProfile(p.data(), p.data()->getName(), index);
}

/**
 * @brief Dialog::_editProfile
 * @param profile
 * @param title
 * @param index
 * @return
 */
bool Dialog::_editProfile(Profile *profile, const QString &title, int index)
{
    ProfileDlg dlg((index < 0), profile, this);
    dlg.setTitle(tr("Profile: ") + title);

    connect(this, SIGNAL(handleKey(KEYINFO*)), &dlg, SLOT(onHandleKey(KEYINFO*)));


    int res = dlg.exec();

    if(res == QDialog::Accepted )
    {
        if(index < 0)
            ProfileStream::instance()->add(*profile);

        _save();

        _listProfiles(index);

        return true;
    }

    return false;
}

/**
 * @brief Dialog::keyPressEvent
 * @param event
 */
void Dialog::keyPressEvent(QKeyEvent *event)
{
    if(event->key() != Qt::Key_Escape)
    {
        QDialog::keyPressEvent(event);
    }
}

/**
 * @brief Dialog::_switchMode
 * Toggle between HID and DFU mode
 * @param bDFU
 */
void Dialog::_switchMode(bool bDFU)
{
    QLog_Debug("Default", "Switching mode");

    QApplication::setOverrideCursor(Qt::WaitCursor);

    ui->btn_load->setEnabled(false);
    ui->btn_switch->setEnabled(false);

    _devInfo.reset();

    if(bDFU)
    {
        if(_hid.enterDFUMode())
            _hid.close();
    }
    else
    {
        if(_dfu.leaveDFUMode())
            _dfu.close();
    }

    _timer.start();

    QApplication::restoreOverrideCursor();
}

/**
 * @brief Dialog::_setupVersions
 * Prepare the exhibition of program and firmware versions
 */
void Dialog::_setupVersions()
{
    if(_devInfo.firmwareVersion != 0)
    {
        uint8_t lo = ((uint8_t)(_devInfo.firmwareVersion & 0xff));
        uint8_t hi = ((uint8_t)((_devInfo.firmwareVersion >> 8) & 0xff));

        ui->lbl_versionFirmware->setText(tr("Firmware version: ") + QString().sprintf("%X.%02X", hi, lo));
    }
    else
        ui->lbl_versionFirmware->setText("");

    ui->lbl_version->setText(QString(tr("Program version: %1")).arg(PROG_VERSION));
}

#ifdef Q_OS_WIN
#if 0
bool Dialog::nativeEvent(const QByteArray& eventType, void *message, long *result)
{
Q_UNUSED(result);
Q_UNUSED(eventType);

    MSG *msg = reinterpret_cast<MSG *>(message);

    if(msg->message == WM_DEVICECHANGE)
    {
        switch(msg->wParam)
        {
        case DBT_DEVNODES_CHANGED:
            break;

        case DBT_DEVICEARRIVAL:
qDebug() << "NATIVE EVENT DBT_DEVICEARRIVAL";
            emit signal_DeviceConnected();
            break;

        case DBT_DEVICEREMOVECOMPLETE:
qDebug() << "NATIVE EVENT DBT_DEVICEREMOVECOMPLETE";
            emit signal_DeviceDisconnected();
            break;
        }
    }

    return false;
}
#endif
#else
/*
void Dialog::monitorTimerTick() // NOT TESTED
{
    fd_set          fds;
    struct timeval  tv;

    FD_ZERO(&fds);
    FD_SET(_fd, &fds);
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    int ret = select(_fd+1, &fds, NULL, NULL, &tv);

    // Check if our file descriptor has received data.
    if (ret > 0 && FD_ISSET(_fd, &fds))
    {
        // We encounter some event!

qDebug() << "[i] udev event";

        // Get the device
        if((_dev = udev_monitor_receive_device(_mon)) == NULL)
        {
            //qDebug() << "[w] receive_device returned no device!";
            return;
        }

        const char* action = udev_device_get_action(_dev);
        const char* pid = udev_device_get_property_value(_dev, "ID_MODEL_ID");
        const char* vid = udev_device_get_property_value(_dev, "ID_VENDOR_ID");

        if((pid==NULL) || (vid==NULL) || (action==NULL))
        {
            qDebug() << "[e] udev null string!";
            return;
        }

        if ( (strcmp(UsbDevice::getStrPID(false), pid) == 0 || strcmp(UsbDevice::getStrPID(true), pid) == 0)
             && (strcmp(UsbDevice::getStrVID(), vid) == 0))
        {
            if(strcmp("add", action) == 0)
            {
                emit signal_DeviceConnected();
            }
            else if(strcmp("remove", action) == 0)
            {
                emit signal_DeviceDisconnected();
            }
            else
                qDebug() << "[w] unknown device action!";
        }

        udev_device_unref(_dev);
    }

    // If there are more events to process, do not wait for next tick!
    if (ret-1 > 0)
        monitorTimerTick();
}
*/
#endif

/**
 * @brief Dialog::deviceDisconnected
 */
void Dialog::deviceDisconnected()
{
//    if(_device.getHandle() != NULL)
    {
        QLog_Debug("Default", "Device disconnected");

        ui->tabWidget->setEnabled(false);

        // firmware
        ui->btn_load->setEnabled(false);
        ui->btn_switch->setEnabled(false);
        ui->lbl_update_text->setText("");
        ui->pb_firmware->setValue(0);

        _showWaitDeviceDlg(true);

        ui->pb_profiles->setValue(0);

        _devInfo.reset();

        if(_device.isDFU())
            _dfu.close();
        else
            _hid.close();

        _device.close();

        _timer.start();
    }
}

/**
 * @brief Dialog::deviceConnected
 */
void Dialog::deviceConnected()
{
    QLog_Debug("Default", "Device connected");
}

/**
 * @brief Dialog::buttonCloseClicked
 */
void Dialog::buttonCloseClicked()
{
    qApp->quit();
}

/**
 * @brief Dialog::onHandleKey
 * @param ki
 */
void Dialog::onHandleKey(KEYINFO *ki)
{
    emit handleKey(ki);
}

/**
 * @brief Dialog::onHandleError
 * Handle transfer or command errors
 * @param fatal
 */
void Dialog::onHandleError(bool fatal)
{
    QLog_Error("Default", "HID Thread error");

    if(!fatal)
    {
        _setupProfileProgress();
        _showExportDataDlg(false);

        QString s = tr("An error occurred during profiles transfer.");
        QMessageBox::critical(this, tr("Transfer Error"), "<font color=\"#FFF\" size=\"4\">"+s+"</font>");
    }
    else
    {
        if(_thread != NULL)
        {
            _thread->stop();
            _thread->quit();
            _thread->wait();
            delete _thread;
            _thread = NULL;
        }

        emit signal_DeviceDisconnected();
    }
}

/**
 * @brief Dialog::onShowContextMenu
 * @param pos
 */
void Dialog::onShowContextMenu(const QPoint &pos)
{
    _contextMenu.exec(ui->lst_profiles->viewport()->mapToGlobal(pos));
}

/**
 * @brief Dialog::_createContextMenu
 * Create our context menu
 */
void Dialog::_createContextMenu()
{
    _contextActions[0] = _contextMenu.addAction(tr("Edit"));
    _contextActions[1] = _contextMenu.addAction(tr("Delete"));
    _contextActions[2] = _contextMenu.addAction(tr("Toggle Enable"));

    connect(_contextActions[0], SIGNAL(triggered()), this, SLOT(onEdit()));
    connect(_contextActions[1], SIGNAL(triggered()), this, SLOT(onDelete()));
    connect(_contextActions[2], SIGNAL(triggered()), this, SLOT(onToggleEnable()));
}

/**
 * @brief Dialog::onEdit
 * Edit selected profile
 */
void Dialog::onEdit()
{
    QListWidgetItem *item = ui->lst_profiles->currentItem();
    int index = item->data(Qt::UserRole + 4).toInt();

    const QSharedPointer<Profile> p = ProfileStream::instance()->profiles()->at(index);

    _editProfile(p.data(), p.data()->getName(), index);
}

/**
 * @brief Dialog::onDelete
 * Delete the selected profile
 */
void Dialog::onDelete()
{
    QListWidgetItem *item = ui->lst_profiles->currentItem();
    if(item != NULL)
    {
        int index = item->data(Qt::UserRole + 4).toInt();

        QString s(tr("Do you really want to delete the selected profile?"));

        if(QMessageBox::question(this,
                                 tr("Profile Deletion"),
                                 "<font color=\"#FFF\" size=\"4\">"+s+"</font>",
                                 QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes)
        {
            ProfileStream::instance()->profiles()->removeAt(index);
            _save();
            _listProfiles();
        }
    }
}

/**
 * @brief Dialog::onToggleEnable
 * Enable or disable selected profile
 */
void Dialog::onToggleEnable()
{
    QListWidgetItem *item = ui->lst_profiles->currentItem();
    int index = item->data(Qt::UserRole + 4).toInt();

    const QSharedPointer<Profile> p = ProfileStream::instance()->profiles()->at(index);
    p->toggleEnabled();
    _save();
    _listProfiles();
    ui->lst_profiles->setCurrentRow(index);
}


void Dialog::_resetControls()
{
    ui->btn_updProfile->setEnabled(false);
    ui->pb_profiles->setValue(0);
    ui->lbl_status->setText(tr("No device connected"));

    // firmware
    ui->btn_load->setEnabled(false);
    ui->btn_switch->setEnabled(false);
    ui->lbl_update_text->setText("");
    ui->pb_firmware->setValue(0);

    _setupVersions();
}

/**
 * @brief Dialog::_showWaitDeviceDlg
 * Show or hide wait for device dialog
 * @param show
 */
void Dialog::_showWaitDeviceDlg(bool show)
{
    if(_waitDlg.data() == NULL)
    {
        QWidget *waitDlg = new QWidget(this);
        _waitDlg = QSharedPointer<QWidget>(waitDlg);

        waitDlg->setWindowModality(Qt::WindowModal);
        waitDlg->setWindowFlags(Qt::Dialog|Qt::CustomizeWindowHint|Qt::FramelessWindowHint|Qt::NoDropShadowWindowHint);
        waitDlg->setStyleSheet("background-color:#000;border-style: outset;border-width: 1px;border-color:#888;");

        QVBoxLayout* layoutV = new QVBoxLayout(waitDlg);
        layoutV->setSpacing(15);

        QHBoxLayout* layoutH = new QHBoxLayout();
        layoutH->setSpacing(5);
        layoutV->addLayout(layoutH);

        QLabel* img = new QLabel(waitDlg);
        img->setStyleSheet("border-width: 0px;padding-left:10px");
        QMovie *movie = new QMovie;
        movie->setFileName(":/images/alert.png");
        img->setMovie(movie);
        movie->start();
        layoutH->addWidget(img);

        QLabel* lbl = new QLabel(tr("Device not connected.\nPlease connect the device to continue.\n\nIf it already connected, reset the device."), waitDlg);
        lbl->setStyleSheet("color:#FFF;border-width: 0px;font:14px;padding:6px;");
        layoutH->addWidget(lbl);

        QToolButton *btn = new QToolButton(waitDlg);
        btn->setStyleSheet("border-width:1px;border-color:#333;color:#FFF;background-color:#222;font:14px;padding:6px;");
        btn->setText(tr("Close Application"));
        layoutV->addWidget(btn, 0, Qt::AlignHCenter);
        connect(btn, SIGNAL(clicked()), this, SLOT(buttonCloseClicked()));

        QRect rect = QStyle::alignedRect(layoutDirection(),
                                         Qt::AlignHCenter|Qt::AlignVCenter,
                                         QSize(width()-100, height()/6),
                                         geometry());
        if(waitDlg != NULL)
            waitDlg->setGeometry(rect);
    }

    if(_waitDlg != NULL)
    {
        if(show)
            _waitDlg->show();
        else
            _waitDlg->hide();
    }
}

/**
 * @brief Dialog::_showExportDataDlg
 * Show or Hide the transfer profiles dialog
 * @param show
 */
void Dialog::_showExportDataDlg(bool show)
{
    if(_transfDlg.data() == NULL)
    {
        QWidget *transfDlg = new QWidget(this);
        _transfDlg = QSharedPointer<QWidget>(transfDlg);

        transfDlg->setWindowModality(Qt::WindowModal);
        transfDlg->setWindowFlags(Qt::Dialog|Qt::CustomizeWindowHint|Qt::FramelessWindowHint|Qt::NoDropShadowWindowHint);
        transfDlg->setStyleSheet("background-color:#000;border-style: outset;border-width: 1px;border-color:#888;");

        QVBoxLayout* layoutV = new QVBoxLayout(transfDlg);
        layoutV->setSpacing(15);

        QHBoxLayout* layoutH = new QHBoxLayout();
        layoutH->setSpacing(5);
        layoutV->addLayout(layoutH);

        QLabel* img = new QLabel(transfDlg);
        img->setStyleSheet("border-width: 0px;padding-left:10px");
        QMovie *movie = new QMovie;
        movie->setFileName(":/images/wait.gif");
        img->setMovie(movie);
        movie->start();
        layoutH->addWidget(img);

        QLabel* lbl = new QLabel(tr("Updating profiles on device..."), transfDlg);
        lbl->setStyleSheet("color:#FFF;border-width: 0px;font:14px;padding:6px;");
        layoutH->addWidget(lbl);

        QRect rect = QStyle::alignedRect(layoutDirection(),
                                         Qt::AlignHCenter|Qt::AlignVCenter,
                                         QSize(width()-100, height()/6),
                                         geometry());

        if(transfDlg != NULL)
            transfDlg->setGeometry(rect);
    }

    if(show)
        _transfDlg->show();
    else
        _transfDlg->hide();
}

/**
 * @brief Dialog::onDownloadDone
 * Event dispatched when transferring profilesd to device was done
 */
void Dialog::onDownloadDone()
{
    QThread::sleep(3);
    _setupProfileProgress();
    _showExportDataDlg(false);

    //_hid.sendCommand(CMD_RESET);
}

/**
 * @brief Dialog::_setupProfileProgress
 * read the number of profiles and maximum to draw progressbar correctly
 */
void Dialog::_setupProfileProgress()
{
    _hid.getNumProfiles(&_devInfo.numProfiles);
    _hid.getNumMaxProfiles(&_devInfo.maxProfiles);
    ui->pb_profiles->setRange(0, _devInfo.maxProfiles);
    ui->pb_profiles->setValue(_devInfo.numProfiles);
}

/**
 * @brief Dialog::_syncProfiles
 * read profiles from device and process its changes
 */
void Dialog::_syncProfiles()
{
    QList< QSharedPointer<QByteArray> > list;

    for(int i = 0; i < _devInfo.numProfiles; ++i)
        list.append(QSharedPointer<QByteArray>(new QByteArray()));

    // Load all profiles from device
    if(_hid.getDeviceProfiles(&list))
    {
        // Parse, validate and insert if necessary
        ProfileStream::instance()->importFromByteArray(&list);

        // Update the profile list
        _listProfiles(0);
    }
}

/**
 * @brief Dialog::onStartListen
 * Setup focus key to receive notifications
 * @param key
 */
void Dialog::onStartListen(KeyWidgetContainer *key)
{
    _focus = key;
}

/**
 * @brief Dialog::onStopListen
 * No keys are focused
 * @param key
 */
void Dialog::onStopListen(KeyWidgetContainer *key)
{
    Q_UNUSED(key);
    _focus = NULL;
}

void Dialog::_save()
{
    ProfileStream::instance()->save();
}

void Dialog::selectListItem(const Profile *p)
{
    const int idx = ProfileStream::instance()->getProfileIndexByPtr(p);
    if(idx >= 0)
        ui->lst_profiles->setCurrentRow(idx);
}

//////////////////////////////////////////
DFUProcessor::DFUProcessor()
    : _progress(NULL)
    , _step(0)
{
}

DFUProcessor::~DFUProcessor()
{
}

void DFUProcessor::setup(QProgressBar *progress, QLabel *lbl)
{
    _progress = progress;
    _label = lbl;
}

void DFUProcessor::onOperationNumSteps(uint16_t steps)
{
    if(_progress != NULL)
    {
        _step = 0;
        _progress->setRange(0, steps);
        _progress->setValue(_step);
        _steps = steps;
    }
}

/**
 * @brief DFUProcessor::onOperationStep
 * Increments the progressbar of firmware download
 */
void DFUProcessor::onOperationStep()
{
    if(_progress != NULL)
    {
        _step++;
        _progress->setValue(_step);

        if(_label != NULL)
        {
            _label->setText(QObject::tr("Writing firmware %1% ...").arg((_step * 100)/_steps));
        }
    }
}

