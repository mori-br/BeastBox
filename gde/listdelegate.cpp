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

ListDelegate::ListDelegate(QObject *parent)
{
Q_UNUSED(parent)
}

ListDelegate::~ListDelegate()
{
}

void ListDelegate::paint ( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
Q_UNUSED(option)

    QRect r = option.rect;

    QPen backPen(QColor::fromRgb(0,0,0), 1, Qt::SolidLine);
    QPen linePen(QColor::fromRgb(22,22,22), 1, Qt::SolidLine);
    QPen fontPen(QColor::fromRgb(201,201,201), 1, Qt::SolidLine);

    if(option.state & QStyle::State_Selected)
    {
        painter->setBrush( QColor(77,0,0));
        painter->drawRect(r);
    }
    else
    {
        painter->setBrush( (index.row() % 2) ? QColor(13,13,13) : QColor(3,3,3) );
        painter->drawRect(r);
    }

    //BORDER
    painter->setPen(linePen);
    painter->drawLine(r.topLeft(),r.topRight());
    painter->drawLine(r.topRight(),r.bottomRight());
    painter->drawLine(r.bottomLeft(),r.bottomRight());
    painter->drawLine(r.topLeft(),r.bottomLeft());

    painter->setPen(fontPen);

    //GET TITLE, DESCRIPTION AND ICON
    QIcon ic = QIcon(qvariant_cast<QPixmap>(index.data(Qt::DecorationRole)));
    QString title = index.data(Qt::DisplayRole).toString();
    QString description = index.data(Qt::UserRole + 1).toString();
    bool type3 = index.data(Qt::UserRole + 3).toBool();
    bool type4 = index.data(Qt::UserRole + 4).toBool();

    int imageSpace = 0;
    if (!ic.isNull())
    {
        //ICON
        r = option.rect.adjusted(5, 5, -5, -5);
        ic.paint(painter, r, Qt::AlignVCenter|Qt::AlignLeft);
        imageSpace = 110;
    }

    //CONSOLE TAG
    if(type4 || type3)
    {
        QString text("ALL");

        if(type4 && !type3)
            text = "PS4";
        if(type3 && !type4)
            text = "PS3";

        QFont fnt( "Lucida", 10, QFont::Black);
        painter->setPen(backPen);
        painter->setFont(fnt);

        QFontMetrics fm(fnt);
        int width = fm.width(text);

        r = QRect(imageSpace, option.rect.top()+10, width+7, 17);

        painter->fillRect(r, QColor(255,255,255,255));
        painter->drawText(r.left()+3, r.top(), r.width(), r.height(), Qt::AlignTop|Qt::AlignLeft, text, &r);
        painter->setPen(fontPen);
    }

    //TITLE
    //    r = option.rect.adjusted(imageSpace, 0, -10, -70);
    r = option.rect.adjusted(imageSpace + 40, 0, -10, -70);

    painter->setFont( QFont( "Lucida", 14, QFont::Normal ) );
    painter->drawText(r.left(), r.top(), r.width(), r.height(), Qt::AlignBottom|Qt::AlignLeft, title, &r);

    //DESCRIPTION
    r = option.rect.adjusted(imageSpace, 35, -10, -15);
    painter->setFont( QFont( "Lucida", 10, QFont::Normal ) );
    painter->drawText(r.left(), r.top(), r.width(), r.height(), Qt::TextWordWrap|Qt::AlignTop|Qt::AlignLeft, description, &r);

    painter->setPen(Qt::NoPen);
}

QSize ListDelegate::sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
Q_UNUSED(option)
Q_UNUSED(index)

    return QSize(300, 100); // very dumb value
}

