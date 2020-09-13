#ifndef BOARD_H
#define BOARD_H

#include <QLabel>
#include <QWidget>
#include <QFrame>
#include <QKeyEvent>
#include <QString>
#include <vector>
#include <QTimer>
#include <QParallelAnimationGroup>
#include <QPushButton>
#include "core/parameter.h"

#define CURRENT_UNIT 0
#define NEXT_UNIT 1

enum Direction{Up = 0,Left = 1,Down = 2,Right = 3};
enum Shape{I = 0,J = 1,L = 2,S = 3,Z = 4,O = 5,T = 6};
enum SpinDirection{ClockWise = -1,AntiClockWise = 1};

struct SingleSquare
{
    QPoint pos;
    QLabel *square = nullptr;
};

struct Unit
{
    SingleSquare unitOperated[4];
    QPoint spinCenter;
    SpinDirection spinDir;
    Shape shape;
    Direction dir;
};

class Board : public QWidget
{
    Q_OBJECT

public:
    Board(QWidget *parent);
    ~Board();

private:
    QLabel *squares[WIDTH][HEIGHT];
    /* Why not squares[HEIGHT][WIDTH]?
       To fit the representation of the position of square */
    Unit *currentUnit = nullptr;
    Unit *nextUnit = nullptr;
    std::vector<int> linesToDelete;
    QParallelAnimationGroup *animationOfFall = nullptr;
    QParallelAnimationGroup *animationOfFade = nullptr;
    int durationOfHalt = DURATION_OF_HALT;
    int remainingTime = 0;
    QTimer *timer = nullptr;
    QLabel *scoreValueLabel = nullptr;
    QLabel *scoreLabel = nullptr;
    int score = 0;
    int level = 1;              // speed of fall
    QFrame *leftArea = nullptr;
    QFrame *nextUnitArea = nullptr;
    QPushButton *pauseButton = nullptr;
    QPushButton *restartButton = nullptr;
    QLabel *shadowUnit[4] = {nullptr,nullptr,nullptr,nullptr};
    /* When some lines are being deleted, the keyboard events should be
     * ignored, because some members of the unitOperated are to be deleted. */
    bool keyboardEnabled = true;
    bool hasPaused = false;

    const QString color[7] = {"red","orange","yellow","green","blue","purple","black"};
    const int points[4] = {100,200,400,800};        // The scores when some lines are eliminated.

    void moveUnit(Direction dir);
    void generateUnit(Unit* &unit);
    bool getLinesToDelete(std::vector<int> lines);
    void deleteLines();
    int isPosValid(QPoint pos);
    void updateSquares();
    void fall();
    void placeUnit();   // Put the next unit into the board and generate a new unit
    void restart();
    void pause();
    void placeShadowUnit();

protected:
    void keyPressEvent(QKeyEvent *event);

signals:
    void moveTowards(Direction dir);
    void toBottom();
};

#endif // BOARD_H
