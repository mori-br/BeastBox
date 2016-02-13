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
#include "ui_dialog.h"
#include "listdelegate.h"
#include "gamedef.h"
#include "../common/ps.h"

#include <QFileDialog>
#include <QDebug>
#include <QList>
#include <QMessageBox>
#include <QTime>

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);

    // Remove question mark from window title
    Qt::WindowFlags flags = windowFlags();
    Qt::WindowFlags helpFlag = Qt::WindowContextHelpButtonHint;
    flags = flags & (~helpFlag);
    setWindowFlags(flags);

    ui->listGames->setItemDelegate(new ListDelegate(ui->listGames));

    connect(ui->listGames, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(editGame(QListWidgetItem*)));
}

Dialog::~Dialog()
{
    delete ui;
}


void Dialog::on_btnadd_clicked()
{
    GameDef *gameDef = new GameDef();

    GameDefDialog dlg(gameDef, this);
    int res = dlg.exec();

    if(res == QDialog::Accepted )
    {
        gameDef->calcCRC32();
        GameStream::instance()->games()->push_back(QSharedPointer<GameDef>(gameDef));
        int sel = GameStream::instance()->games()->size()-1;
        _fillGameList(sel);
    }
    else if(res == QDialog::Rejected )
    {
        delete gameDef;
    }
}

void Dialog::on_btnedit_clicked()
{
    QList<QListWidgetItem *> selected = ui->listGames->selectedItems();
    if(selected.size() > 0)
        editGame(selected.at(0));
}

void Dialog::on_btndel_clicked()
{
    QList<QListWidgetItem *> selected = ui->listGames->selectedItems();
    if(selected.size() > 0)
    {
        int idx = selected.at(0)->data(Qt::UserRole + 2).toInt();

        QString s = tr("Do you really want to delete the selected game?");

        if(QMessageBox::question(this,
                                 tr("Game Deletion"),
                                 "<font color=\"#FFF\" size=\"5\">"+s+"</font>",
                                 QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes)
        {
            GameStream::instance()->games()->removeAt(idx);
            _fillGameList();
        }
    }
}

void Dialog::on_btnclone_clicked()
{
    QList<QListWidgetItem *> selected = ui->listGames->selectedItems();
    if(selected.size() > 0)
   {
        int idx = selected.at(0)->data(Qt::UserRole + 2).toInt();
        QSharedPointer<GameDef> p = GameStream::instance()->games()->at(idx);
        GameDef *gameDef = p->clone();
        gameDef->calcCRC32();
        GameStream::instance()->games()->push_back(QSharedPointer<GameDef>(gameDef));
        int sel = GameStream::instance()->games()->size()-1;
        _fillGameList(sel);
    }
}

void Dialog::on_btnopen_clicked()
{
    QString fileName =
                QFileDialog::getOpenFileName(this,
                                             tr("Open File"),
                                             "",
                                             "Games Definition Files (*.gdf);;All Files (*.*)");
    if (!fileName.isEmpty())
    {
        LoadError err;
        if(!GameStream::instance()->load(fileName, &err))
        {
            QMessageBox* msg = new QMessageBox();
            msg->setWindowTitle(tr("Loading File Error"));

            QString s;
            if(err == LOAD_ERROR_SIGNATURE)
                s = tr("The file %1 have an invalid signature").arg(fileName);
            else if(err == LOAD_ERROR_VERSION)
                s = tr("The file %1 have an invalid version").arg(fileName);
            else
                s = tr("Unknown error opening %1").arg(fileName);

            msg->setText(s);
            msg->show();

            return;
        }

        _fillGameList();
    }
}

void Dialog::on_btnsave_clicked()
{
    if(GameStream::instance()->games()->size() > 0)
    {
        QString fileName = GameStream::instance()->getFilename();
        if(fileName.isEmpty())
        {
            fileName =
                QFileDialog::getSaveFileName(this,
                                             tr("Save File"),
                                             "",
                                             "Games Definition Files (*.gdf);;All Files (*.*)");
        }

        if(!fileName.isEmpty())
        {
            GameStream::instance()->save(fileName);
        }
    }
}

void Dialog::_fillGameList(int selected)
{
    ui->listGames->clear();

    for(int i = 0; i < GameStream::instance()->games()->size(); ++i)
    {
        QSharedPointer<GameDef> p = GameStream::instance()->games()->at(i);

        QListWidgetItem *item = new QListWidgetItem();

        item->setData(Qt::DisplayRole, p->getName());
        item->setData(Qt::UserRole + 1, p->getDescription());
        item->setData(Qt::DecorationRole, p->getImage());
        item->setData(Qt::UserRole + 2, i);
        item->setData(Qt::UserRole + 3, p->isConsolePS3());
        item->setData(Qt::UserRole + 4, p->isConsolePS4());

        ui->listGames->addItem(item);
    }

    int count = GameStream::instance()->games()->size();

    if(selected < 0 &&  count > 0)
        ui->listGames->setCurrentRow(0);
    else if(selected >= 0)
        ui->listGames->setCurrentRow(selected);
}

void Dialog::editGame(QListWidgetItem *item)
{
    QString s = item->data(Qt::DisplayRole).toString();

    int idx = item->data(Qt::UserRole + 2).toInt();
    QSharedPointer<GameDef> p = GameStream::instance()->games()->at(idx);

    GameDefDialog dlg(p.data(), this);
    int res = dlg.exec();

    if(res == QDialog::Accepted )
    {
        p->calcCRC32();
        _fillGameList();
        ui->listGames->setCurrentRow(idx);
    }
}
