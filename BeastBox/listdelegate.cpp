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

Ref:
    http://www.qtcentre.org/threads/27777-Customize-QListWidgetItem-how-to
*/
#include "listdelegate.h"
#include <QPen>
#include <QPainter>
#include <QDebug>
#include "profile.h"

int ListDelegate::HEIGHT = 125;

ListDelegate::ListDelegate(QObject *parent)
{
Q_UNUSED(parent)
}

ListDelegate::~ListDelegate()
{
}

void ListDelegate::paint ( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
    QRect r = option.rect;
    QRect rRGB = r;

    QPen backPen(QColor::fromRgb(0,0,0), 1, Qt::SolidLine);
    QPen linePen(QColor::fromRgb(22,22,22), 1, Qt::SolidLine);
    QPen fontPen(QColor::fromRgb(201,201,201), 1, Qt::SolidLine);
    QPen fdisPen(QColor::fromRgb(255,255,255), 1, Qt::SolidLine);
    QPen fdisPen1(QColor::fromRgb(60,60,60), 1, Qt::SolidLine);

    if(!(option.state & QStyle::State_Enabled))
    {
        fontPen.setColor(QColor::fromRgb(67,67,67));
        linePen.setColor(QColor::fromRgb(8,8,8));
    }

    if(option.state & QStyle::State_Selected)
    {
        if(option.state & QStyle::State_Enabled)
            painter->setBrush( QColor(77,0,0));
        else
            painter->setBrush( QColor(17,17,17));

        painter->drawRect(r);
    }
    else
    {
        painter->setBrush( (index.row() % 2) ? QColor(13,13,13) : QColor(3,3,3) );
        painter->drawRect(r);
    }

    //GET NAME, GAME, DESCRIPTION AND ICON
    QIcon ic = QIcon(qvariant_cast<QPixmap>(index.data(Qt::DecorationRole)));
    QString title = index.data(Qt::DisplayRole).toString();
    QString description = index.data(Qt::UserRole + 1).toString();
    QString game = index.data(Qt::UserRole + 2).toString();
    ConsoleType type = (ConsoleType)index.data(Qt::UserRole + 5).toInt();
    bool enabled = index.data(Qt::UserRole + 6).toBool();
    QColor c = index.data(Qt::UserRole + 3).value<QColor>();

    // PAINT PROFILE COLOR
    painter->setPen(linePen);
    rRGB.adjust(5, r.height()-20, 100-r.width(), -5);
    if(!(option.state & QStyle::State_Enabled))
        c.setAlpha(50);

    painter->setBrush(c);
    painter->drawRect(rRGB);

    //BORDER
    painter->setPen(linePen);
    painter->drawLine(r.topLeft(),r.topRight());
    painter->drawLine(r.topRight(),r.bottomRight());
    painter->drawLine(r.bottomLeft(),r.bottomRight());
    painter->drawLine(r.topLeft(),r.bottomLeft());

    painter->setPen(fontPen);

    int imageSpace = 110;

    // If no image, we provide one
    if(ic.isNull())
        ic = QPixmap(":/images/noimg.png");

    if (!ic.isNull())
    {
        QPainter::CompositionMode cm = painter->compositionMode();

        //ICON
        r = option.rect.adjusted(5, 5, 100-r.width(), -25);
        ic.paint(painter, r, Qt::AlignVCenter|Qt::AlignLeft);

        if(!(option.state & QStyle::State_Enabled))
        {
            painter->setCompositionMode(QPainter::CompositionMode_DestinationIn);
            painter->fillRect(r, QColor(255,255,255,150));
        }

        painter->setCompositionMode(cm);
//      imageSpace = 110;
    }

    //ENABLE
    if(!enabled)
    {
        if(!(option.state & QStyle::State_Enabled))
            painter->setPen(fdisPen1);
        else
            painter->setPen(fdisPen);

        r = rRGB;
        painter->setFont( QFont( "Lucida", 10, QFont::Bold));
        painter->drawText(r.left()+15, r.top(), r.width(), r.height(), Qt::AlignTop|Qt::AlignLeft, "DISABLED", &r);
        painter->setPen(fontPen);
    }

    //CONSOLE TAG
    QString text = "";

    if(type == CONSOLE_PS4)
        text = "PS4";
    else if(type == CONSOLE_PS3)
        text = "PS3";

    QFont fnt( "Lucida", 10, QFont::Black);
    painter->setPen(backPen);
    painter->setFont(fnt);

    QFontMetrics fm(fnt);
    int width = fm.width(text);

    r = QRect(imageSpace, option.rect.top()+10, width+7, 17);

    if(option.state & QStyle::State_Enabled)
        painter->fillRect(r, QColor(255,255,255,255));
    else
        painter->fillRect(r, QColor(22,22,22,255));

    painter->drawText(r.left()+3, r.top(), r.width(), r.height(), Qt::AlignTop|Qt::AlignLeft, text, &r);

    painter->setPen(fontPen);

    //NAME
    r = option.rect.adjusted(imageSpace + 40, 4, -5, -98);
    painter->setFont( QFont( "Lucida", 14, QFont::Normal ) );
    painter->drawText(r.left(), r.top(), r.width(), r.height(), Qt::AlignTop|Qt::AlignLeft, title, &r);

    //GAME
    r = option.rect.adjusted(imageSpace, 34, -5, -73);
    painter->setFont( QFont( "Lucida", 10, QFont::Bold ) );
    painter->drawText(r.left(), r.top(), r.width(), r.height(), Qt::TextWordWrap|Qt::AlignTop|Qt::AlignLeft, game, &r);

    //DESCRIPTION
    r = option.rect.adjusted(imageSpace, 56, -5, -5);
    painter->setFont( QFont( "Lucida", 10, QFont::Normal ) );
    painter->drawText(r.left(), r.top(), r.width(), r.height(), Qt::TextWordWrap|Qt::AlignTop|Qt::AlignLeft, description, &r);

    painter->setPen(Qt::NoPen);
}

QSize ListDelegate::sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
Q_UNUSED(option)
Q_UNUSED(index)

    return QSize(300, ListDelegate::HEIGHT);
}

