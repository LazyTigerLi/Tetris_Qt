#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "board.h"

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = 0);
    ~Widget();

private:
    Board *board = nullptr;
};

#endif // WIDGET_H
