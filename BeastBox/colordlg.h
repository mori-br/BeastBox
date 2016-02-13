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
#ifndef COLORDLG_H
#define COLORDLG_H

#include <QDialog>

namespace Ui {
class ColorDlg;
}

class ColorDlg : public QDialog
{
    Q_OBJECT

public:
    explicit ColorDlg(QColor *color, QWidget *parent = 0);
    ~ColorDlg();


private:
    Ui::ColorDlg *ui;

    QColor *_color;

    void changeProfileColor();

public slots:
    void handleSlideValue(int value);
    void handleSpinboxValue(int value);
    void buttonClicked();
};

#endif // COLORDLG_H
