#include "gui/homepage.h"
#include "gui/single_player_page.h"
#include <QVBoxLayout>

HomePage::HomePage(QWidget *parent)
    : QWidget(parent)
{
    // setAttribute(Qt::WA_DeleteOnClose);
    // may deconstruct twice
    setWindowFlags(Qt::Dialog);
    setStyleSheet("QPushButton{font:9pt Arial Black}");
    singleButton = new QPushButton(tr("Single Player"), this);
    multiButton = new QPushButton(tr("Multiple Players"), this);
    lanButton = new QPushButton(tr("Via LAN"), this);
    connect(singleButton, &QPushButton::clicked, this,
            &HomePage::createSinglePlayerPage);
    connect(multiButton, &QPushButton::clicked, this,
            &HomePage::createMultiPlayersPage);
    connect(lanButton, &QPushButton::clicked, this,
            &HomePage::createLanPlayPage);
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(singleButton);
    mainLayout->addWidget(multiButton);
    mainLayout->addWidget(lanButton);
}

HomePage::~HomePage()
{}

void HomePage::createSinglePlayerPage()
{
    hide();
    singlePlayerPage = new SinglePlayerPage(this);
    singlePlayerPage->show();
    // connect(singlePlayerPage, SIGNAL(closePage()), this, SLOT(show()));
}

void HomePage::createMultiPlayersPage()
{

}

void HomePage::createLanPlayPage()
{

}
