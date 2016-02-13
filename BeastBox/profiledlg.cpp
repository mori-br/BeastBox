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
#include "profiledlg.h"
#include "ui_profiledlg.h"
#include "combodelegate.h"
#include "util.h"
#include "colordlg.h"
#include "../common/ps.h"
#include "buttonsconfig.h"

#include <QMessageBox>
#include <QColorDialog>
#include <QDebug>
#include <QTimer>
#include <QLogger/QLogger.h>

using namespace QLogger;


ProfileDlg::ProfileDlg(bool bNew, Profile *profile, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ProfileDlg)
    , _consoleType(CONSOLE_PS4)
    , _focus(NULL)
    , _ignoreUpdate(true)
    , _validationDlgVisible(false)
{
    // Remove question mark from window title
    Qt::WindowFlags flags = windowFlags();
    Qt::WindowFlags helpFlag = Qt::WindowContextHelpButtonHint;
    flags = flags & (~helpFlag);
    setWindowFlags(flags);

    _newProfile = bNew;
    _profile = profile;

    ui->setupUi(this);
    setModal(true);

    ui->tabWidget->setCurrentIndex(0);

    ui->scrollArea->setWidget(ui->panel);
    ui->scrollArea->setWidgetResizable(true);
    ui->scrollArea->setPanel(ui->panel);

    ui->cb_console->addItem(QIcon(QPixmap(":/images/ps4.png")), ""); // 0
    ui->cb_console->setItemData(0, CONSOLE_PS4, Qt::UserRole + 1);
    ui->cb_console->addItem(QIcon(QPixmap(":/images/ps3.png")), ""); // 1
    ui->cb_console->setItemData(1, CONSOLE_PS3, Qt::UserRole + 1);

    ui->cb_deadzone->addItem(QIcon(QPixmap(":/images/square.png")), tr(" Square")); // 0
    ui->cb_deadzone->setItemData(0, DZ_CIRCULAR, Qt::UserRole + 1);
    ui->cb_deadzone->addItem(QIcon(QPixmap(":/images/circle.png")), tr(" Circular")); // 1
    ui->cb_deadzone->setItemData(1, DZ_SQUARE, Qt::UserRole + 1);

    connect(parent, SIGNAL(signal_DeviceDisconnected()), this, SLOT(deviceDisconnected()));
    connect(parent, SIGNAL(signal_DeviceConnected()), this, SLOT(deviceConnected()));

    connect(ui->cb_console, SIGNAL(currentIndexChanged(int)), this, SLOT(handleSelectionChanged(int)));
    connect(ui->cb_games, SIGNAL(currentIndexChanged(int)), this, SLOT(handleGameSelectionChanged(int)));

    connect(ui->btn_save, SIGNAL(clicked()), this, SLOT(buttonClicked()));
    connect(ui->btn_cancel, SIGNAL(clicked()), this, SLOT(buttonClicked()));
    connect(ui->lbl_color, SIGNAL(clicked()), this, SLOT(labelClicked()));
    connect(ui->btn_choose_color, SIGNAL(clicked()), this, SLOT(labelClicked()));
    connect(ui->chk_smoothness, SIGNAL(stateChanged(int)), this, SLOT(smoothClicked(int)));

    connect(ui->kw_up, SIGNAL(startListen(KeyWidgetContainer *)), this, SLOT(onStartListen(KeyWidgetContainer *)));
    connect(ui->kw_up, SIGNAL(stopListen(KeyWidgetContainer *)), this, SLOT(onStopListen(KeyWidgetContainer *)));

    connect(ui->kw_down, SIGNAL(startListen(KeyWidgetContainer *)), this, SLOT(onStartListen(KeyWidgetContainer *)));
    connect(ui->kw_down, SIGNAL(stopListen(KeyWidgetContainer *)), this, SLOT(onStopListen(KeyWidgetContainer *)));

    connect(ui->kw_left, SIGNAL(startListen(KeyWidgetContainer *)), this, SLOT(onStartListen(KeyWidgetContainer *)));
    connect(ui->kw_left, SIGNAL(stopListen(KeyWidgetContainer *)), this, SLOT(onStopListen(KeyWidgetContainer *)));

    connect(ui->kw_right, SIGNAL(startListen(KeyWidgetContainer *)), this, SLOT(onStartListen(KeyWidgetContainer *)));
    connect(ui->kw_right, SIGNAL(stopListen(KeyWidgetContainer *)), this, SLOT(onStopListen(KeyWidgetContainer *)));

    connect(ui->kw_shortcut, SIGNAL(startListen(KeyWidgetContainer *)), this, SLOT(onStartListen(KeyWidgetContainer *)));
    connect(ui->kw_shortcut, SIGNAL(stopListen(KeyWidgetContainer *)), this, SLOT(onStopListen(KeyWidgetContainer *)));

    connect(ui->chk_secondSens, SIGNAL(stateChanged(int)), this, SLOT(secondaryClicked(int)));

    ui->kw_shortcut->getKeyCap()->getInfo()->type = KEY_TYPE_PROFILE;

    ui->cb_games->setItemDelegate(new ComboDelegate(ui->cb_games));

    ui->cb_deadzone->setCurrentIndex(1); // Circular

    _color.setRgb(0,0,0);
    _changeProfileColor();

    _fillGameList();

    ui->chk_secondSens->setChecked(false);
    ui->chk_enabled->setChecked(true);
    ui->chk_stick->setChecked(true);

    if(!bNew)
        _fillFields();
    else
        _fillFieldsDefault();
}

ProfileDlg::~ProfileDlg()
{
    delete ui;
}

void ProfileDlg::setTitle(const QString &title)
{
    setWindowTitle(title);
}

void ProfileDlg::handleSelectionChanged(int index)
{
    _consoleType = (ConsoleType)ui->cb_console->itemData(index, Qt::UserRole + 1).toInt();

    _fillGameList();
}

void ProfileDlg::_changeProfileColor()
{
    QPalette palette = ui->lbl_color->palette();

    if(ui->lbl_color->isEnabled())
        palette.setColor(ui->lbl_color->backgroundRole(), _color);
    else
    {
        QColor c = _color;
        c.setAlpha(80);
        palette.setColor(ui->lbl_color->backgroundRole(), c);
    }

    ui->lbl_color->setPalette(palette);
    ui->lbl_color->repaint();
}

void ProfileDlg::_fillGameList()
{
    _ignoreUpdate = true;

    ui->cb_games->clear();

    // Add an empty item for imported profiles
    ui->cb_games->addItem(tr("None"));
    ui->cb_games->setItemData(0, tr("No game selected"), Qt::UserRole + 1);
    ui->cb_games->setItemData(0, QPixmap(":/images/noimg.png"), Qt::DecorationRole);

//qDebug() << "console: " << _consoleType << " item count: " << _games->games()->size();

    for(int i = 0; i < GameStream::instance()->games()->size(); ++i)
    {
        QSharedPointer<GameDef> p = GameStream::instance()->games()->at(i);

//qDebug() << "game: " << p->getName() << " ps4 " << p->isConsolePS4() << " ps3 " << p->isConsolePS3();

        // Load only games def with selected console
        if(
            ((_consoleType == CONSOLE_PS4) && p->isConsolePS4()) ||
            ((_consoleType == CONSOLE_PS3) && p->isConsolePS3()) )
        {
            int j = ui->cb_games->count();

            ui->cb_games->addItem(p->getName());
            ui->cb_games->setItemData(j, p->getDescription(), Qt::UserRole + 1);
            ui->cb_games->setItemData(j, p->getImage(), Qt::DecorationRole);
            ui->cb_games->setItemData(j, i, Qt::UserRole + 2);
        }
    }

    _ignoreUpdate = false;

//qDebug() << "_fillGameList()";

    if(ui->cb_games->count() > 0)
    {
//qDebug() << "ui->cb_games->setCurrentIndex(0);";
        ui->cb_games->setCurrentIndex(0);
        handleGameSelectionChanged(0);
    }
}

void ProfileDlg::handleGameSelectionChanged(int index)
{
    if(index >= 0 && !_ignoreUpdate)
    {
//qDebug() << "handleGameSelectionChanged();";

        // Lets get the gamedef index from cb itemdata
        int idx = ui->cb_games->itemData(index, Qt::UserRole + 2).toInt();
        QSharedPointer<GameDef> p = GameStream::instance()->games()->at(idx);
        ui->scrollArea->loadkeys(_profile->getKeys(), p.data(), _consoleType);

        QString s = PsButton::getButtonImagePath(p->getADSButton(), _profile->getConsoleType());

        ui->lb_adsbutton->setPixmap(QPixmap(s));
        ui->lb_adsbutton->setProperty("pskey", p->getADSButton());

        // Fill secundarySensitivity button
/*        KEYINFO ki;
        memset(&ki, 0, sizeof(KEYINFO));
        ki.psk = p->getADSButton();
        ki.type = KEY_TYPE_ADS;
        _profile->setSensitivityKey(&ki);*/
    }
}

void ProfileDlg::buttonClicked()
{
    QToolButton *button = (QToolButton *)sender();
    if(button == ui->btn_cancel)
    {
        reject();
    }
    else if(button == ui->btn_save)
    {
        if(ui->edt_name->text().isEmpty())
        {
            _showError(tr("A profile name is always required."));
            return;
        }

        QString title = ui->edt_name->text();
        // Only invalid if new profile
        if(_newProfile && !ProfileStream::instance()->isNameValid(title))
        {
            _showError(tr("A profile named '%1' already exists. Please choose another one.").arg(title));
            return;
        }

        ui->scrollArea->savekeys(_profile->getKeys());

        _profile->setName(title);
        _profile->setDescription(ui->edt_desc->toPlainText());
        _profile->setColor(_color);
        _profile->setEnabled(ui->chk_enabled->isChecked());
        _profile->setConsoleType(_consoleType);

        if(ui->cb_games->currentIndex() >= 0)
        {
            int idx = ui->cb_games->itemData(ui->cb_games->currentIndex(), Qt::UserRole + 2).toInt();
            QSharedPointer<GameDef> p = GameStream::instance()->games()->at( idx );
            if(p)
                _profile->setGameDef(p.data());
        }

        _profile->setUseStick(ui->chk_stick->isChecked());

        _profile->setDeadZone((float)ui->spin_deadzone->value());
        _profile->setDeadZoneType(ui->cb_deadzone->currentData(Qt::UserRole + 1).toInt());
        _profile->setDiagonalDampen((float)ui->spin_diagonal->value());
        _profile->setSensitivityPrimary((float)ui->spin_primary->value());
        _profile->setSmoothEnable(ui->chk_smoothness->isChecked());
        _profile->setSmoothness((float)ui->spin_smoothness->value());
        _profile->setTranslationExponent((float)ui->spin_exponent->value());
        _profile->setYXRatio((float)ui->spin_yxratio->value());

        _profile->setSwitchProfileKey(ui->kw_shortcut->getKeyCap()->getInfo());

        KEYINFO ki;
        memset(&ki, 0, sizeof(KEYINFO));
        ki.psk = ui->lb_adsbutton->property("pskey").toInt();
        ki.type = KEY_TYPE_ADS;
        _profile->setSensitivityKey(&ki);
        _profile->setSecondarySensEnable(ui->chk_secondSens->isChecked());
        _profile->setSensitivitySecondary((float)ui->spin_secondary->value());

        /*if(ui->chk_secondSens->isChecked())
        {
            KEYINFO ki;
            memset(&ki, 0, sizeof(KEYINFO));
            ki.psk = ui->lb_adsbutton->property("pskey").toInt();
            ki.type = KEY_TYPE_ADS;
            _profile->setSensitivityKey(&ki);
        }
        else
        {
            _profile->setSensitivityKey(NULL);
        }*/

        _profile->setJoyDeadzone(ui->sb_deadzone->value());

        _profile->setJoyKey(JOY_KEY_UP, ui->kw_up->getKeyCap()->getInfo());
        _profile->setJoyKey(JOY_KEY_DOWN, ui->kw_down->getKeyCap()->getInfo());
        _profile->setJoyKey(JOY_KEY_LEFT, ui->kw_left->getKeyCap()->getInfo());
        _profile->setJoyKey(JOY_KEY_RIGHT, ui->kw_right->getKeyCap()->getInfo());

        accept();
    }
}

void ProfileDlg::labelClicked()
{
    ColorDlg dlg(&_color, this);
    int res = dlg.exec();

    if(res == QDialog::Accepted )
        _changeProfileColor();
}

void ProfileDlg::_showError(const QString &text)
{
    QMessageBox* msg = new QMessageBox(this);
    msg->setWindowTitle(tr("Profile Error"));
    msg->setStyleSheet("QLabel { color: #FFF; }");
    msg->setText("<font color=\"#FFF\" size=\"4\">"+text+"</font>");
    msg->exec();
}

/*! Load default values for mouse parameters */
void ProfileDlg::_fillFieldsDefault()
{
    ui->spin_diagonal->setValue(0.25);
    ui->spin_primary->setValue(20.0);
    ui->spin_secondary->setValue(0.0);
    ui->chk_smoothness->setChecked(true);
    ui->spin_smoothness->setValue(0.10);
    ui->spin_exponent->setValue(0.55);
    ui->spin_yxratio->setValue(1.1);
    ui->spin_deadzone->setValue(9.0);   // mouse
    ui->sb_deadzone->setValue(4.0);    // joystick (G13)
}

 /*! Fill fileds with values loaded from file */
void ProfileDlg::_fillFields()
{
//qDebug() << "_fillFields()";

    ui->edt_name->setText(_profile->getName());
    ui->edt_desc->setPlainText(_profile->getDescription());
    _color = _profile->getColor();
    ui->chk_enabled->setChecked(_profile->isEnabled());
    _consoleType = _profile->getConsoleType();

    ui->chk_stick->setChecked(_profile->isUseStick());

    ui->spin_deadzone->setValue(_profile->getDeadZone());

    ui->cb_deadzone->setCurrentIndex(_profile->getDeadZoneType());

    ui->spin_diagonal->setValue(_profile->getDiagonalDampen());
    ui->spin_primary->setValue(_profile->getSensitivityPrimary());
    ui->spin_secondary->setValue(_profile->getSensitivitySecondary());
    ui->chk_smoothness->setChecked(_profile->isSmoothEnabled());
    ui->spin_smoothness->setValue(_profile->getSmoothness());
    ui->spin_exponent->setValue(_profile->getTranslationExponent());
    ui->spin_yxratio->setValue(_profile->getYXRatio());

    ui->kw_shortcut->getKeyCap()->setInfo(_profile->getSwitchProfileKey());

    ui->chk_secondSens->setChecked(_profile->isSecondarySensEnabled());

    ui->sb_deadzone->setValue(_profile->getJoyDeadzone());

    ui->kw_up->getKeyCap()->setInfo(_profile->getJoyKey(JOY_KEY_UP));
    ui->kw_down->getKeyCap()->setInfo(_profile->getJoyKey(JOY_KEY_DOWN));
    ui->kw_left->getKeyCap()->setInfo(_profile->getJoyKey(JOY_KEY_LEFT));
    ui->kw_right->getKeyCap()->setInfo(_profile->getJoyKey(JOY_KEY_RIGHT));

    _changeProfileColor();

    if(_consoleType == CONSOLE_PS4)
        ui->cb_console->setCurrentIndex(0);
    else if(_consoleType == CONSOLE_PS3)
        ui->cb_console->setCurrentIndex(1);

    // Calc GameDef CRC to see if it still exists
    uint32_t crc = _profile->getGameDef()->getCRC();
    if(crc > 0)
    {
        int idx = GameStream::instance()->getGameDefIndexByCRC(crc);
        if(idx < 0)
        {
            // Maybe the game was deleted, so re-insert again
            GameDef *gameDef = new GameDef(_profile->getGameDef());
            GameStream::instance()->games()->push_back(QSharedPointer<GameDef>(gameDef));
            GameStream::instance()->save();
            GameStream::instance()->reload();
            _fillGameList();

            // Try to select game definition again
            idx = GameStream::instance()->getGameDefIndexByCRC(crc);
        }

qDebug() << "Gamedef name " << _profile->getGameDef()->getName() << " idx " << idx;

        for(int i = 0; i < ui->cb_games->count(); ++i)
        {
            int x = ui->cb_games->itemData(i, Qt::UserRole + 2).toInt();
            if(idx == x)
            {
                ui->cb_games->setCurrentIndex(i);
                break;
            }
        }
    }

    // Load buttons into scroll pane
    ui->scrollArea->loadkeys(_profile->getKeys(), _profile->getGameDef(), _consoleType);
}

void ProfileDlg::secondaryClicked(int state)
{
    ui->spin_secondary->setEnabled(state == Qt::Checked);

/*    // Remove th selected key
    if(state != Qt::Checked)
        _profile->setSensitivityKey(NULL);
    else
    {
        KEYINFO ki;
        memset(&ki, 0, sizeof(KEYINFO));
        ki.psk = ui->lb_adsbutton->property("pskey").toInt();
        ki.type = KEY_TYPE_ADS;
        _profile->setSensitivityKey(&ki);
    }*/
}

void ProfileDlg::smoothClicked(int state)
{
    ui->spin_smoothness->setEnabled(state == Qt::Checked);
}

void ProfileDlg::keyPressEvent(QKeyEvent *event)
{
    // ESC can close this dialog
    if(event->key() != Qt::Key_Escape)
    {
        QDialog::keyPressEvent(event);
    }
}

bool ProfileDlg::_showQuestionDialog(const QString &msg)
{
    bool retValue = true;

    // If dialog is already showed, return
    if(_validationDlgVisible)
        return retValue;

    // Mark as showed
    _validationDlgVisible = true;

    if(QMessageBox::question(this,
                            tr("Key Validation"),
                            "<font color=\"#FFF\" size=\"4\">"+msg+"</font>",
                            QMessageBox::Yes|QMessageBox::No) == QMessageBox::No)
    {
        retValue = false;
    }

    // Turn off mark
    _validationDlgVisible = false;

    return retValue;
}

void ProfileDlg::onHandleKey(KEYINFO *ki)
{
    if(_focus == NULL && ui->scrollArea->getFocus() == NULL)
        return;

    QList< KEYINFO *> list;
    KeyWidgetContainer *kc = _focus;
    KEYINFO kic(ki);

    // Depending on selected TAB, check correct key container
    if(ui->tabWidget->currentIndex() == TAB_PROFILE_GENERAL)
    {
        // Profile switcher
        if(_focus == ui->kw_shortcut)
        {
            // Setup correct type
            kic.type = KEY_TYPE_PROFILE;

            _focus->getKeyCap()->setInfo(&kic, true);

            if(!_validateProfileSwitcher(kic.key))
            {
                QString s = tr("The key pressed was already used in another profile. Do you want to keep it?");

                if(!_showQuestionDialog(s))
                    kc->getKeyCap()->setInfo(NULL);

                return;
            }

            // Update the key in profile
            _profile->setSwitchProfileKey(&kic);
        }
    }
    else if(ui->tabWidget->currentIndex() == TAB_PROFILE_BUTTONS)
    {
        // Dispatch key to scrollarea that holds the ps buttons list
        ui->scrollArea->setupKey(&kic);
    }
    else if(ui->tabWidget->currentIndex() == TAB_PROFILE_NAVIGATION)
    {
        // Treat directional buttons
        if(_focus == ui->kw_up || _focus == ui->kw_down || _focus == ui->kw_left || _focus == ui->kw_right)
        {
            _focus->getKeyCap()->setInfo(&kic, true);
        }
    }

    bool err = false;
    QString s = "";
    KEYINFO keyFound;

    // Get key list from button list control
    ui->scrollArea->getKeyList(&list);

    // Proceed with validation
    if(!_profile->validate(&list, &kic, &keyFound))
    {
qDebug() << "VALIDATE ERR " << keyFound.psk;
        err = true;
        s = "<font color=\"#FFF\" size=\"4\">" +
            tr("The key pressed was already used") +
            " (<img src=\"" + PsButton::getButtonImagePath(keyFound.psk, _profile->getConsoleType()) + "\" height=\"15\" width=\"15\">) " +
            tr("in this profile. Do you want to keep it?") +
            "</font>";
    }

    if(err)
    {
        if(!_showQuestionDialog(s))
        {
            if(kc != NULL)
                kc->getKeyCap()->setInfo(NULL);
            else
            {
                kic.key = 0;
                ui->scrollArea->updateItem(&kic);
            }
        }
    }
}

void ProfileDlg::onStartListen(KeyWidgetContainer *key)
{
//qDebug() << "onStartListen";

    _focus = key;
}

void ProfileDlg::onStopListen(KeyWidgetContainer *key)
{
Q_UNUSED(key);

//qDebug() << "onStopListen";

    _focus = NULL;
}

/*! Device disconnection */
void ProfileDlg::deviceDisconnected()
{
qDebug() << "PROFILE DLG - deviceDisconnected()";

    // Disable fields
    ui->tabWidget->setEnabled(false);
    _changeProfileColor();
    ui->btn_save->setEnabled(false);
    ui->btn_cancel->setEnabled(false);
}

 /*! Device connection */
void ProfileDlg::deviceConnected()
{
qDebug() << "PROFILE DLG - deviceConnected()";

    // Enable fields
    ui->tabWidget->setEnabled(true);
    _changeProfileColor();
    ui->btn_save->setEnabled(true);
    ui->btn_cancel->setEnabled(true);
}

/*! Function to validade profile switcher key - check if the key was alaready used in other profile */
bool ProfileDlg::_validateProfileSwitcher(uint16_t key)
{
    QList< QSharedPointer<SWITCHER> > *profileSwitcherList =
            ProfileStream::instance()->getProfileSwitcherList();

qDebug() << QString().sprintf(">> num %d, key 0x%04X", profileSwitcherList->size(), key);

    for(int i = 0; i < profileSwitcherList->size(); ++i)
    {
        QSharedPointer<SWITCHER> ki = profileSwitcherList->at(i);
        if(key == ki->key && ki->console == _profile->getConsoleType())
            return false;
    }

    return true;
}
