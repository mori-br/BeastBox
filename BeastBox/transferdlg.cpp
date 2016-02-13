#include "transferdlg.h"

#include <QVBoxLayout>
#include <QStyle>

TransferDlg::TransferDlg(QWidget *parent)
    : QWidget(parent)
{
    setWindowModality(Qt::WindowModal);
    setWindowFlags(Qt::Dialog|Qt::CustomizeWindowHint|Qt::FramelessWindowHint|Qt::NoDropShadowWindowHint);
    setStyleSheet("background-color:#000;border-style: outset;border-width: 1px;border-color:#888;");

    QVBoxLayout* layoutV = new QVBoxLayout(this);
    layoutV->setSpacing(15);

    _lbl = new QLabel("Device not connected.", this);
    _lbl->setStyleSheet("color:#FFF;border-width: 0px;font:12px;padding:6px;");
    layoutV->addWidget(_lbl);

    _pb = new QProgressBar(this);
    layoutV->addWidget(_pb);

    QRect rect = QStyle::alignedRect(parent->layoutDirection(),
                                     Qt::AlignHCenter|Qt::AlignVCenter,
                                     QSize(parent->width()-100, parent->height()/6),
                                     parent->geometry());

    setGeometry(rect);
}

void TransferDlg::onSetupProgressBar(int range)
{
    if(_pb != NULL)
    {
        _pb->setRange(0, range);
        _pb->setValue(0);
    }
}

void TransferDlg::onUpdateTransferStatus(int pos, const QString &text)
{
    if(_pb != NULL)
        _pb->setValue(pos);

    if(_lbl != NULL)
        _lbl->setText(text);
}
