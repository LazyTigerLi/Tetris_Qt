#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QPushButton>
#include "gui/single_player_page.h"

class HomePage : public QWidget
{
    Q_OBJECT

public:
    HomePage(QWidget *parent = 0);
    ~HomePage();

private:
    QPushButton *singleButton = nullptr;
    QPushButton *multiButton = nullptr;
    QPushButton *lanButton = nullptr;

    QWidget *singlePlayerPage = nullptr;
    QWidget *multiPlayerPage = nullptr;
    QWidget *lanPlayPage = nullptr;

    void createSinglePlayerPage();
    void createMultiPlayersPage();
    void createLanPlayPage();
};

#endif // WIDGET_H
