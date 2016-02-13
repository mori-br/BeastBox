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
#include "colordlg.h"
#include "ui_colordlg.h"

ColorDlg::ColorDlg(QColor *color, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ColorDlg)
{
    _color = color;

    // Remove question mark from window title
    Qt::WindowFlags flags = windowFlags();
    Qt::WindowFlags helpFlag = Qt::WindowContextHelpButtonHint;
    flags = flags & (~helpFlag);
    setWindowFlags(flags);

    ui->setupUi(this);

    connect(ui->sb_red, SIGNAL(valueChanged(int)), this, SLOT(handleSpinboxValue(int)));
    connect(ui->sb_green, SIGNAL(valueChanged(int)), this, SLOT(handleSpinboxValue(int)));
    connect(ui->sb_blue, SIGNAL(valueChanged(int)), this, SLOT(handleSpinboxValue(int)));

    connect(ui->dial_r, SIGNAL(valueChanged(int)), this, SLOT(handleSlideValue(int)));
    connect(ui->dial_g, SIGNAL(valueChanged(int)), this, SLOT(handleSlideValue(int)));
    connect(ui->dial_b, SIGNAL(valueChanged(int)), this, SLOT(handleSlideValue(int)));

    connect(ui->btn_ok, SIGNAL(clicked()), this, SLOT(buttonClicked()));
    connect(ui->btn_cancel, SIGNAL(clicked()), this, SLOT(buttonClicked()));

    changeProfileColor();
}

ColorDlg::~ColorDlg()
{
    delete ui;
}

void ColorDlg::handleSlideValue(int value)
{
    QDial *dial = (QDial *)sender();
    if(dial == ui->dial_b)
    {
        _color->setBlue(value);
        ui->sb_blue->setValue(value);
    }
    else if(dial == ui->dial_g)
    {
        _color->setGreen(value);
        ui->sb_green ->setValue(value);
    }
    else if(dial == ui->dial_r)
    {
        _color->setRed(value);
        ui->sb_red->setValue(value);
    }

    changeProfileColor();
}



void ColorDlg::handleSpinboxValue(int value)
{
    QSpinBox *spin = (QSpinBox *)sender();
    if(spin == ui->sb_red)
    {
        _color->setRed(value);
        ui->dial_r->setValue(value);
    }
    else if(spin == ui->sb_green)
    {
        _color->setGreen(value);
        ui->dial_g->setValue(value);
    }
    else if(spin == ui->sb_blue)
    {
        _color->setBlue(value);
        ui->dial_b->setValue(value);
    }

    changeProfileColor();
}

void ColorDlg::changeProfileColor()
{
    ui->dial_r->setValue(_color->red());
    ui->dial_g->setValue(_color->green());
    ui->dial_b->setValue(_color->blue());

    ui->sb_red->setValue(_color->red());
    ui->sb_green->setValue(_color->green());
    ui->sb_blue->setValue(_color->blue());

    QPalette palette = ui->lbl_color->palette();
    palette.setColor(ui->lbl_color->backgroundRole(), *_color);
    ui->lbl_color->setPalette(palette);
    ui->lbl_color->repaint();
}

void ColorDlg::buttonClicked()
{
    QToolButton *button = (QToolButton *)sender();
    if(button == ui->btn_cancel)
        emit reject();
    if(button == ui->btn_ok)
        emit accept();
}
