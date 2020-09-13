#ifndef SINGLE_PLAYER_PAGE_H
#define SINGLE_PLAYER_PAGE_H

#include <QWidget>

class SinglePlayerPage : public QWidget
{
    Q_OBJECT

public:
    SinglePlayerPage(QWidget *parent = 0);
    ~SinglePlayerPage();

private:
    QWidget *gameBoard = nullptr;

protected:
    void closeEvent(QCloseEvent *event) override;
// signals:
//    void closePage();
};

#endif // SINGLE_PLAYER_PAGE_H
