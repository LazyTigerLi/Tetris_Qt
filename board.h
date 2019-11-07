#ifndef BOARD_H
#define BOARD_H

#include <QLabel>
#include <QWidget>
#include <QKeyEvent>
#include <QString>
#include <vector>
#include <QTimer>
#include <QParallelAnimationGroup>
#include "scale.h"

struct SingleSquare
{
    QPoint pos;
    QLabel *square = nullptr;
};

enum Direction{Up = 0,Left = 1,Down = 2,Right = 3};
enum Shape{I = 0,J = 1,L = 2,S = 3,Z = 4,O = 5,T = 6};
enum SpinDirection{ClockWise = -1,AntiClockWise = 1};

class Board : public QWidget
{
    Q_OBJECT

public:
    Board(QWidget *parent);
    ~Board();

private:
    SingleSquare unitOperated[4];
    QLabel *squares[WIDTH][HEIGHT];
    /* Why not squares[HEIGHT][WIDTH]?
       To fit the representation of the position of square */
    QPoint spinCenter;
    SpinDirection spinDir;
    Shape shape;
    Direction dir;
    std::vector<int> linesToDelete;
    QParallelAnimationGroup *animationOfFall = nullptr;
    QParallelAnimationGroup *animationOfFade = nullptr;
    QTimer *timer = nullptr;

    const QString color[7] = {"red","orange","yellow","green","blue","purple","black"};
    void moveUnit(Direction dir);
    void generateUnit();
    void getLinesToDelete(std::vector<int> lines);
    void deleteLines();
    bool isPosValid(QPoint pos);
    void updateSquares();
    void fall();

protected:
    void keyPressEvent(QKeyEvent *event);

signals:
    void moveTowards(Direction dir);
    void toBottom();
};

#endif // BOARD_H
