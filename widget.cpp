#include "widget.h"
#include <QHBoxLayout>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    board = new Board(this);
    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(board);
    setLayout(mainLayout);
    board->setFocus();
}

Widget::~Widget()
{

}
