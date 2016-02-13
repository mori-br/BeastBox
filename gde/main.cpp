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
#include <QApplication>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    // look up e.g. :/translations/app.xx_XX.qm
    if (translator.load(QLocale(), QLatin1String("app"), QLatin1String("."), QLatin1String(":/translations")))
        a.installTranslator(&translator);

// Force locale
//if (translator.load(QLocale(QLocale::Portuguese, QLocale::Brazil), QLatin1String("app"), QLatin1String("."), QLatin1String(":/translations")))
//    a.installTranslator(&translator);


    Dialog w;
    w.show();

    return a.exec();
}
