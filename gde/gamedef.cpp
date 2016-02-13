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
#include "gamedef.h"
#include "ui_gamedef.h"
#include "../common/ps.h"

#include <QDebug>
#include <QPixmap>
#include <QFileDialog>


GameDefDialog::GameDefDialog(GameDef *game, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::GameDefDialog)
{
    ui->setupUi(this);

    _game = game;

    // Remove question mark from window title
    Qt::WindowFlags flags = windowFlags();
    Qt::WindowFlags helpFlag = Qt::WindowContextHelpButtonHint;
    flags = flags & (~helpFlag);
    setWindowFlags(flags);

    ui->lbl_image->setScaledContents(true);

    // Button cancel
    connect(ui->btncancel, SIGNAL(clicked(bool)), this, SLOT(reject()));

    // Checkboxes
    connect(ui->chk_ps3, SIGNAL(stateChanged(int)), this, SLOT(onConsoleChecked(int)));
    connect(ui->chk_ps4, SIGNAL(stateChanged(int)), this, SLOT(onConsoleChecked(int)));

    ui->page_ps3->setEnabled(false);
    ui->page_ps4->setEnabled(false);

    ui->chk_ps4->setChecked(true);

    createButtons();

    fillButtons();
}

GameDefDialog::~GameDefDialog()
{
    delete ui;
}

void GameDefDialog::createButtons()
{
    int numItems = (sizeof(BTN_COMMON_INFO)/sizeof(BTNINFO));
    for(int i = 0; i < numItems; ++i)
    {
        ButtonDescWidget *w = new ButtonDescWidget();
        ui->vl_common->addWidget(w);
        w->setup(BTN_COMMON_INFO[i].id, BTN_COMMON_INFO[i].image, BTN_COMMON_INFO[i].type);
        _controls.push_back(QSharedPointer<ButtonDescWidget>(w));

        connect(w->getADSCheck(), SIGNAL(stateChanged(int)), this, SLOT(onADSChecked(int)));
    }

    int space = ui->vl_common->layout()->spacing();
    ui->panel->setMinimumHeight(numItems * (ButtonDescWidget::getHeight()+ space));
    ui->panel->setMaximumHeight(numItems * (ButtonDescWidget::getHeight()+ space));

    // PS4
    numItems = (sizeof(BTN_PS4_INFO)/sizeof(BTNINFO));
    for(int i = 0; i < numItems; ++i)
    {
        ButtonDescWidget *w = new ButtonDescWidget();
        ui->vl_ps4->addWidget(w);
        w->setup(BTN_PS4_INFO[i].id, BTN_PS4_INFO[i].image, BTN_PS4_INFO[i].type);
        _controls.push_back(QSharedPointer<ButtonDescWidget>(w));

        connect(w->getADSCheck(), SIGNAL(stateChanged(int)), this, SLOT(onADSChecked(int)));
    }

    // PS3
    numItems = (sizeof(BTN_PS3_INFO)/sizeof(BTNINFO));
    for(int i = 0; i < numItems; ++i)
    {
        ButtonDescWidget *w = new ButtonDescWidget();
        ui->vl_ps3->addWidget(w);
        w->setup(BTN_PS3_INFO[i].id, BTN_PS3_INFO[i].image, BTN_PS3_INFO[i].type);
        _controls.push_back(QSharedPointer<ButtonDescWidget>(w));

        connect(w->getADSCheck(), SIGNAL(stateChanged(int)), this, SLOT(onADSChecked(int)));
    }
}

void GameDefDialog::fillButtons()
{
    if(_game != NULL)
    {
        ui->edt_name->setText(_game->getName());
        ui->lbl_image->setPixmap(_game->getImage());
        ui->edt_desc->setPlainText(_game->getDescription());

        ui->chk_ps3->setChecked(_game->isConsolePS3());
        ui->chk_ps4->setChecked(_game->isConsolePS4());

qDebug() << "ADS btn id: " << _game->getADSButton();

        for(int i = 0; i < _controls.size(); ++i)
        {
            QSharedPointer<ButtonDescWidget> button = _controls.at(i);
            ButtonDesc *bd = _game->getButtonFromId(button->getId());
            if(bd != NULL)
            {
                if(button->getId() == _game->getADSButton())
                    button->setADS();

                button->setDescription(bd->getDescription());
            }
        }
    }
}

void GameDefDialog::on_btnselpic_clicked()
{
    QString fileName =
            QFileDialog::getOpenFileName(this,
                                         tr("Open File"),
                                         "",
                                         "Image Files (*.png *.jpg *.bmp *.xpm);;All Files (*.*)");
    if (!fileName.isEmpty())
    {
        ui->lbl_image->setPixmap(QPixmap(fileName));
    }
}

void GameDefDialog::on_btnok_clicked()
{
    if(_game != NULL)
    {
        _game->setName(ui->edt_name->text());
        _game->setDescription(ui->edt_desc->toPlainText());
        _game->setImage(*ui->lbl_image->pixmap());

        _game->setConsolePS3(ui->chk_ps3->isChecked());
        _game->setConsolePS4(ui->chk_ps4->isChecked());

qDebug() << "_controls.size(): " << _controls.size();

        for(int i = 0; i < _controls.size(); ++i)
        {
            QSharedPointer<ButtonDescWidget> button = _controls.at(i);
            ButtonDesc *bd = _game->getButtonFromId(button->getId());
            if(bd != NULL)
            {
                bd->setDescription(button->getDescription());
            }
            else
            {
                bd = new ButtonDesc();
                bd->setId(button->getId());
                bd->setDescription(button->getDescription());
                _game->buttons()->push_back(QSharedPointer<ButtonDesc>(bd));
            }
        }

qDebug() << "_game->buttons()->size(): " << _game->buttons()->size();
    }

    accept();
}

void GameDefDialog::onConsoleChecked(int state)
{
    QCheckBox *button = (QCheckBox *)sender();
    if(button == ui->chk_ps3)
    {
        ui->page_ps3->setEnabled(state == Qt::Checked);
    }
    else if(button == ui->chk_ps4)
    {
        ui->page_ps4->setEnabled(state == Qt::Checked);
    }
}

void GameDefDialog::onADSChecked(int state)
{
    QCheckBox *button = (QCheckBox *)sender();

    _game->setADSButton(0);

    if(state == Qt::Checked)
    {
        // Uncheck all other controls
        for(int i = 0; i < _controls.size(); ++i)
        {
            QSharedPointer<ButtonDescWidget> btn = _controls.at(i);
            if(btn->getADSCheck() != button)
                btn->resetADS();
        }

        int id = button->property("pskid").toInt();
        if(_game != NULL)
        {
qDebug() << "checkbox prop: " << id;
            _game->setADSButton(id);
        }
    }
}
