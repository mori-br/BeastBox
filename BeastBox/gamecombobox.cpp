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
#include "gamecombobox.h"

#include <QStylePainter>

GameComboBox::GameComboBox(QWidget *parent) :
    QComboBox(parent)
{
}

void GameComboBox::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPen fontMarkedPen(Qt::white, 1, Qt::SolidLine);
    QPen fontMarkedPenDis(QColor::fromRgb(67,67,67), 1, Qt::SolidLine);

    QStylePainter painter(this);

    QStyleOptionComboBox opt;
    initStyleOption(&opt);
    painter.drawComplexControl(QStyle::CC_ComboBox, opt);

    QIcon ic = QIcon(qvariant_cast<QPixmap>(itemData(currentIndex(), Qt::DecorationRole)));
    QString title = itemData(currentIndex(), Qt::DisplayRole).toString();
    QString description = itemData(currentIndex(), Qt::UserRole + 1).toString();

    int imageSpace = 10;
    QRect r = rect();
    if (!ic.isNull())
    {
        QPainter::CompositionMode cm = painter.compositionMode();

        QRect r = rect().adjusted(5, 5, -5, -5);
        ic.paint(&painter, r, Qt::AlignVCenter|Qt::AlignLeft);
        imageSpace = 110;

        if(!(opt.state & QStyle::State_Enabled))
        {
            painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
            painter.fillRect(r, QColor(255,255,255,150));
        }

        painter.setCompositionMode(cm);
    }

    if(!(opt.state & QStyle::State_Enabled))
        painter.setPen(fontMarkedPenDis);
    else
        painter.setPen(fontMarkedPen);

    r = rect().adjusted(imageSpace, 0, -10, -70);
    painter.setFont( QFont( "Lucida", 13, QFont::Normal ) );
    painter.drawText(r.left(), r.top(), r.width(), r.height(), Qt::AlignBottom|Qt::AlignLeft, title, &r);

    r = rect().adjusted(imageSpace, 35, -20, -15);
    painter.setFont( QFont( "Lucida", 10, QFont::Normal ) );
    painter.drawText(r.left(), r.top(), r.width(), r.height(), Qt::TextWordWrap|Qt::AlignTop|Qt::AlignLeft, description, &r);
    painter.setPen(Qt::NoPen);
}
