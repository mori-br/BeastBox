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
#ifndef ABOUT_H
#define ABOUT_H

#include "version.h"
#include <QString>


const QString string = "<html>"
        "<head>"
        "<style>"
        "::-webkit-scrollbar {"
        "background-color: #000;"
        "}            "

        "::-webkit-scrollbar-thumb {"
        "background-color: #444;"
        "}            "

        "</style>"
        "</head>"

        "<body bgcolor=\"#000\" text=\"#1086e2\">"
        "<font face=\"verdana\" size=\"2\">"
        "<p>Copyright (C) 2014-2016 SOFTFACTORY Informática Ltda</p>"
        "<p>http://softfactory.com.br/</p>"
        "<h3>License</h3>"
        "<p>This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.</p>"
        "<p>The <b>"
        DEVICE_NAME
        "</b> software and firmware is being provided to you free of charge. You may use it only with official <b>"
        DEVICE_NAME
        "</b> hardware and for your own personal gaming use.</p><b>"
        "<p>You may absolutely not sell "
        DEVICE_NAME
        "</b> as part of any commercial product.\nYou may not redistribute any part of <b>"
        DEVICE_NAME
        "</b>.</p>"
        "<p>All distribution of <b>"
        DEVICE_NAME
        "</b> (software, firmware, hardware, and assembly instructions) can only come from softfactory's web site.</p>"
        "<p>In addition, no guarantee is made about the performance of the <b>"
        DEVICE_NAME
        "</b> hardware/software/firmware or the accuracy of the assembly instructions.</p>"
        "<p>The creators of <b>"
        DEVICE_NAME
        "</b> are not responsible for any injury (or worse) that you may incur when building your <b>"
        DEVICE_NAME
        "</b>.</p>"
        "<p>The creators of <b>"
        DEVICE_NAME
        "</b> are not responsible for supporting your use and/or building of your <b>"
        DEVICE_NAME
        "</b>.</p>"
        "<p>You taking action in using and/or building <b>"
        DEVICE_NAME
        "</b> means you understand and agree with the terms of use and disclaimer just covered.</p>"
        "<p>PS4™ and  PS3™ is a registered trademark of Sony Computer Entertainment.</p>"
        "</font>"
        " </body></html>";

#endif // ABOUT_H
