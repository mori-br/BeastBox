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

#include <QLogger/QLogger.h>
#include <QStyleFactory>

#include <../common/profiledata.h>

using namespace QLogger;


#if defined(STATIC_QT5) && defined(Q_OS_WIN32)
  #include <QtPlugin>
  Q_IMPORT_PLUGIN (QWindowsIntegrationPlugin);
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setStyle(QStyleFactory::create("cleanlooks"));

    //a.setStyle("fusion");
    //a.setStyle("windows");
    //a.setStyle("motif");
    //a.setStyle("cde");
    //a.setStyle("plastique");
    //a.setStyle("windowsxp");
    //a.setStyle("macintosh");

    //a.setStyle(QStyleFactory::create("Fusion"));

    a.setStyleSheet("QToolTip { color: #fff; background-color: #111; border-radius: 3px; border: 1px solid #333; font: 14px; }");

    // startup de log engine
    QLoggerManager *manager = QLoggerManager::getInstance();
    manager->addDestination("file.log", "Default", DebugLevel);
    QLog_Debug("Default", QString("%1 started").arg( a.applicationDisplayName() ));

    QTranslator translator;
    // look up e.g. :/translations/app.xx_XX.qm
    if (translator.load(QLocale(), QLatin1String("app"), QLatin1String("."), QLatin1String(":/translations")))
        a.installTranslator(&translator);

//Force Locale
//if (translator.load(QLocale(QLocale::Portuguese, QLocale::Brazil), QLatin1String("app"), QLatin1String("."), QLatin1String(":/translations")))
//    a.installTranslator(&translator);


QLog_Debug("Default", QString("TranslationData %1").arg( sizeof(TranslationData) ));
QLog_Debug("Default", QString("PROFILE_SIZE %1").arg( PROFILE_SIZE ));
QLog_Debug("Default", QString("ProfileData %1").arg( sizeof(ProfileData) ));


    Dialog w;

    w.show();

    return a.exec();
}
