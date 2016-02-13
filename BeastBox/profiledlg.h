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
#ifndef PROFILEDLG_H
#define PROFILEDLG_H

#include <stdint.h>
#include <QDialog>
#include <QLabel>
#include <QList>
#include <QSharedPointer>

#include "keywidget.h"
#include "profile.h"

#include "keywidgetcontainer.h"

namespace Ui {
class ProfileDlg;
}

typedef enum
{
    TAB_PROFILE_GENERAL,
    TAB_PROFILE_BUTTONS,
    TAB_PROFILE_MOUSE,
    TAB_PROFILE_NAVIGATION,

} PROFILE_TABID;


class ProfileDlg : public QDialog
{
    Q_OBJECT
public:
    explicit ProfileDlg(bool nNew, Profile *profile, QWidget *parent = 0);
    ~ProfileDlg();
    void setTitle(const QString &title);

private:
    Ui::ProfileDlg *ui;

    ConsoleType         _consoleType;
    QColor              _color;
    Profile             *_profile;
    KeyWidgetContainer  *_focus;
    bool                _ignoreUpdate;
    bool                _newProfile;
    bool                _validationDlgVisible;

    void _changeProfileColor();
    void _fillGameList();
    void _showError(const QString &text);
    void _fillFields();
    void _fillFieldsDefault();

    bool _validateProfileSwitcher(uint16_t key);
    bool _showQuestionDialog(const QString &msg);

signals:
    void handleKey(KEYINFO *ki);

public slots:
    void handleSelectionChanged(int index);
    void handleGameSelectionChanged(int index);
    void buttonClicked();
    void labelClicked();
    void smoothClicked(int state);
    void onHandleKey(KEYINFO *ki);
    void onStartListen(KeyWidgetContainer *key);
    void onStopListen(KeyWidgetContainer *key);
    void deviceDisconnected();
    void deviceConnected();
    void secondaryClicked(int state);

protected:
    void keyPressEvent(QKeyEvent *event);

};

#endif // PROFILEDLG_H
