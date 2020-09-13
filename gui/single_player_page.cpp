#include "gui/single_player_page.h"
#include "core/board.h"
#include <QDebug>

SinglePlayerPage::SinglePlayerPage(QWidget *parent)
    : QWidget(parent)
{
    setWindowFlags(Qt::Dialog);
    setAttribute(Qt::WA_DeleteOnClose);
    // if not set this, deconstruction will not be implemented when closing the widget
    setWindowFlag(Qt::Window);      // make child windows stand alone with parent
    gameBoard = new Board(this);
}

SinglePlayerPage::~SinglePlayerPage()
{}

void SinglePlayerPage::closeEvent(QCloseEvent *event)
{
    parentWidget()->show();
    QWidget::closeEvent(event);
}
