#include "board.h"
#include <QRandomGenerator>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <algorithm>

Board::Board(QWidget *parent)
    :QWidget(parent)
{
    //setStyleSheet("QLabel{margin:2px;background-color:black;}");
    setFixedSize(WIDTH * SIZEOFSQUARE,HEIGHT * SIZEOFSQUARE);
    connect(this,&Board::moveTowards,this,&Board::moveUnit);
    connect(this,&Board::toBottom,this,&Board::updateSquares);
    animationOfFall = new QParallelAnimationGroup;
    animationOfFade = new QParallelAnimationGroup;
    connect(animationOfFade,&QParallelAnimationGroup::finished,this,&Board::deleteLines);
    timer = new QTimer(this);
    connect(timer,&QTimer::timeout,this,&Board::fall);
    //connect(this,&Board::toBottom,timer,&QTimer::stop);
    /* The timer can also be stopped in the function updateSquares which will
     * be called after the toBottom signal is emitted. */
    for(int i = 0; i < WIDTH; i++)
        for(int j = 0; j < HEIGHT; j++)
            squares[i][j] = nullptr;
    generateUnit();
}

Board::~Board()
{}

void Board::keyPressEvent(QKeyEvent *event)
{
    //qDebug("key press");
    if(event->key() == Qt::Key_Up)emit moveTowards(Up);
    else if(event->key() == Qt::Key_Down)emit moveTowards(Down);
    else if(event->key() == Qt::Key_Left)emit moveTowards(Left);
    else if(event->key() == Qt::Key_Right)emit moveTowards(Right);
}

/*
 * rotation matrix  | cosx  -sinx |
 *                  | sinx  cosx  |
 * clockwise 90 degrees:| 0  1 |     anticlockwise 90 degrees:| 0 -1 |
 *                      | -1 0 |                              | 1  0 |
 * if the center of rotation is (x,y),the point to be rotated is (m,n),then
 * from (anticlockwise) | 0 -1 ||m-x|, we can get the vector rotated (y-n,m-x),
 *                      | 1  0 ||n-y|
 * thus we get the positon of the point,aka (x+y-n,m-x+y)
 * for clockwise,that is (x-y+n,x+y-m)
 */
void Board::moveUnit(Direction dir)
{
    QPoint newPos[4];
    if(dir == Up && shape == I)spinDir = static_cast<SpinDirection>(-spinDir);
    int x = spinCenter.x(),y = spinCenter.y();
    bool posValid = true;
    bool isBottom = false;
    for(int i = 0; i < 4; i++)
    {
        //oldPos[i] = unitOperated[i].pos;
        if(dir == Down)
        {
            posValid = isPosValid(newPos[i] = unitOperated[i].pos + QPoint(0,1));
            if(!posValid)
            {
                isBottom = true;
                break;
            }
        }
        else if(dir == Left)
        {
            posValid = isPosValid(newPos[i] = unitOperated[i].pos - QPoint(1,0));
            if(!posValid)break;
        }
        else if(dir == Right)
        {
            posValid = isPosValid(newPos[i] = unitOperated[i].pos + QPoint(1,0));
            if(!posValid)break;
        }
        else
        {
            if(shape == O)
                for(int i = 0; i < 4; i++)
                    newPos[i] = unitOperated[i].pos;
            /* It is difficult to decide the spinCenter of Shape O,
             * so the following algoritm does not fit Shape O.
             * Actually, it does not need to be rotated. */
            if(spinDir == AntiClockWise)
            {
                posValid = isPosValid(newPos[i] = QPoint(x + y - unitOperated[i].pos.y(),
                                        unitOperated[i].pos.x() - x + y));
                if(!posValid)break;
            }
            else
            {
                posValid = isPosValid(newPos[i] = QPoint(x - y + unitOperated[i].pos.y(),
                                         x + y - unitOperated[i].pos.x()));
                if(!posValid)break;
            }
        }
    }
    if(isBottom)
    {
        emit toBottom();
        return;
    }
    if(!posValid && shape == I && dir == Up)spinDir = static_cast<SpinDirection>(-spinDir);
    if(!posValid)return;
    for(int i = 0; i < 4; i++)
    {
        unitOperated[i].pos = newPos[i];
        unitOperated[i].square->setGeometry(newPos[i].x() * SIZEOFSQUARE,newPos[i].y() * SIZEOFSQUARE,
                                            SIZEOFSQUARE,SIZEOFSQUARE);
    }

    if(dir == Down)spinCenter += QPoint(0,1);
    else if(dir == Left)spinCenter -= QPoint(1,0);
    else if(dir == Right)spinCenter += QPoint(1,0);
}

/* This function can be modified to illustrate whether the unit
 * is by the border, to improve the rotation of units.
   If improved, the function moveUnit and generateUnit should be modified too. */
bool Board::isPosValid(QPoint pos)
{
    int x = pos.x(),y = pos.y();
    if(x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT || squares[x][y] != nullptr)
        return false;
    return true;
}

void Board::generateUnit()
{   
    shape = static_cast<Shape>(QRandomGenerator::global()->bounded(7));
    for(int i = 0; i < 4; i++)
    {
        unitOperated[i].square = new QLabel(this);
        unitOperated[i].square->setStyleSheet("margin:2px;background-color:" + color[shape] + ";");
    }
    dir = static_cast<Direction>(QRandomGenerator::global()->bounded(4));
    spinDir = AntiClockWise;
    int mid = WIDTH / 2;
    if(shape == I)
    {
        if(dir == Up || dir == Down)
        {
            spinCenter = QPoint(mid,3);
            spinDir = ClockWise;
            for(int i = 0; i < 4; i++)
                unitOperated[i].pos = QPoint(mid,i);
        }
        else
        {
            spinCenter = QPoint(mid,0);
            for(int i = 0; i < 4; i++)
                unitOperated[i].pos = QPoint(mid + i,0);
        }
    }
    else if(shape == J)
    {
        if(dir == Up)                       //   O
        {                                   // * O
            spinCenter = QPoint(mid,1);     // O O
            for(int i = 0; i < 3; i++)
                unitOperated[i].pos = QPoint(mid + 1,i);
            unitOperated[3].pos = QPoint(mid,2);
        }
        else if(dir == Left)                // O O O
        {                                   //   * O
            spinCenter = QPoint(mid,1);
            for(int i = 0; i < 3; i++)
                unitOperated[i].pos = QPoint(mid - 1 + i,0);
            unitOperated[3].pos = QPoint(mid + 1,1);
        }
        else if(dir == Down)                // O O
        {                                   // O *
            spinCenter = QPoint(mid,1);     // O
            for(int i = 0; i < 3; i++)
                unitOperated[i].pos = QPoint(mid - 1,i);
            unitOperated[3].pos = QPoint(mid,0);
        }
        else                                // O *
        {                                   // O O O
            spinCenter = QPoint(mid,0);
            for(int i = 0; i < 3; i++)
                unitOperated[i].pos = QPoint(mid - 1 + i,1);
            unitOperated[3].pos = QPoint(mid - 1,0);
        }
    }
    else if(shape == L)
    {
        if(dir == Up)                       // O
        {                                   // O *
            spinCenter = QPoint(mid,1);     // O O
            for(int i = 0; i < 3; i++)
                unitOperated[i].pos = QPoint(mid - 1,i);
            unitOperated[3].pos = QPoint(mid,2);
        }
        else if(dir == Left)                //   * O
        {                                   // O O O
            spinCenter = QPoint(mid,0);
            for(int i = 0; i < 3; i++)
                unitOperated[i].pos = QPoint(mid - 1 + i,1);
            unitOperated[3].pos = QPoint(mid + 1,0);
        }
        else if(dir == Down)                // O O
        {                                   // * O
            spinCenter = QPoint(mid,1);     //   O
            for(int i = 0; i < 3; i++)
                unitOperated[i].pos = QPoint(mid + 1,i);
            unitOperated[3].pos = QPoint(mid,0);
        }
        else                                // O O O
        {                                   // O *
            spinCenter = QPoint(mid,1);
            for(int i = 0; i < 3; i++)
                unitOperated[i].pos = QPoint(mid - 1 + i,0);
            unitOperated[3].pos = QPoint(mid - 1,1);
        }
    }
    else if(shape == S)
    {
        if(dir == Up || dir == Down)        // O
        {                                   // * O
            spinCenter = QPoint(mid,1);     //   O
            unitOperated[0].pos = QPoint(mid,0);
            unitOperated[1].pos = QPoint(mid,1);
            unitOperated[2].pos = QPoint(mid + 1,1);
            unitOperated[3].pos = QPoint(mid + 1,2);
        }
        else if(dir == Left || dir == Right)    //   * O
        {                                       // O O
            spinCenter = QPoint(mid,0);
            unitOperated[0].pos = QPoint(mid,0);
            unitOperated[1].pos = QPoint(mid + 1,0);
            unitOperated[2].pos = QPoint(mid - 1,1);
            unitOperated[3].pos = QPoint(mid,1);
        }
    }
    else if(shape == Z)
    {
        if(dir == Up || dir == Down)        //   O
        {                                   // O *
            spinCenter = QPoint(mid,1);     // O
            unitOperated[0].pos = QPoint(mid,0);
            unitOperated[1].pos = QPoint(mid,1);
            unitOperated[2].pos = QPoint(mid - 1,1);
            unitOperated[3].pos = QPoint(mid - 1,2);
        }
        else if(dir == Left || dir == Right)    // O *
        {                                       //   O O
            spinCenter = QPoint(mid,0);
            unitOperated[0].pos = QPoint(mid - 1,0);
            unitOperated[1].pos = QPoint(mid,0);
            unitOperated[2].pos = QPoint(mid,1);
            unitOperated[3].pos = QPoint(mid + 1,1);
        }
    }
    else if(shape == O)
    {
        // spinCenter can be any for Shape O,because my spin algorithm excludes Shape O.
        unitOperated[0].pos = QPoint(mid,0);
        unitOperated[1].pos = QPoint(mid + 1,0);
        unitOperated[2].pos = QPoint(mid,1);
        unitOperated[3].pos = QPoint(mid + 1,1);
    }
    else if(shape == T)
    {
        if(dir == Up)                       //   O
        {                                   // O * O
            spinCenter = QPoint(mid,1);
            for(int i = 0; i < 3; i++)
                unitOperated[i].pos = QPoint(mid - 1 + i,1);
            unitOperated[3].pos = QPoint(mid,0);
        }
        else if(dir == Left)                //   O
        {                                   // O *
            spinCenter = QPoint(mid,1);        //   O
            for(int i = 0; i < 3; i++)
                unitOperated[i].pos = QPoint(mid,i);
            unitOperated[3].pos = QPoint(mid - 1,1);
        }
        else if(dir == Down)                // O * O
        {                                   //   O
            spinCenter = QPoint(mid,0);
            for(int i = 0; i < 3; i++)
                unitOperated[i].pos = QPoint(mid - 1 + i,0);
            unitOperated[3].pos = QPoint(mid,1);
        }
        else                                // O
        {                                   // * O
            spinCenter = QPoint(mid,1);        // O
            for(int i = 0; i < 3; i++)
                unitOperated[i].pos = QPoint(mid,i);
            unitOperated[3].pos = QPoint(mid + 1,1);
        }
    }

    for(int i = 0; i < 4; i++)
    {
        unitOperated[i].square->setGeometry(unitOperated[i].pos.x() * SIZEOFSQUARE,unitOperated[i].pos.y() * SIZEOFSQUARE,
                                            SIZEOFSQUARE,SIZEOFSQUARE);
        unitOperated[i].square->show();
    }

    timer->start(1000);
}

/* This function is called when the unit is at the bottom. */
void Board::updateSquares()
{
    timer->stop();
    std::vector<int> lines;
    for(int i = 0; i < 4; i++)
    {
        int x = unitOperated[i].pos.x(), y = unitOperated[i].pos.y();
        squares[x][y] = unitOperated[i].square;
        unitOperated[i].square = nullptr;
        if(std::find(lines.begin(),lines.end(),y) == lines.end())lines.push_back(y);
    }
    getLinesToDelete(lines);
    generateUnit();
}

void Board::getLinesToDelete(std::vector<int> lines)
{
    linesToDelete.clear();
    for(int i = 0; i < lines.size(); i++)
    {
        bool filled = true;
        for(int j = 0; j < WIDTH; j++)
            if(squares[j][lines[i]] == nullptr)
            {
                filled = false;
                break;
            }
        if(filled)linesToDelete.push_back(lines[i]);
    }
    if(linesToDelete.empty())return;
    //qDebug("%d",linesToDelete.size());
    animationOfFade->clear();
    for(int i = 0; i < linesToDelete.size(); i++)
    {
        qDebug("%d",linesToDelete[i]);
        for(int j = 0; j < WIDTH; j++)
        {
            QGraphicsOpacityEffect *opacityEffect = new QGraphicsOpacityEffect(squares[j][linesToDelete[i]]);
            squares[j][linesToDelete[i]]->setGraphicsEffect(opacityEffect);
            QPropertyAnimation *animation = new QPropertyAnimation(opacityEffect,"opacity");
            animation->setStartValue(1);
            animation->setEndValue(0);
            animation->setDuration(500);
            animationOfFade->addAnimation(animation);
        }
    }
    animationOfFade->start();
}

void Board::deleteLines()
{
    for(int i = 0; i < linesToDelete.size(); i++)
    {
        for(int j = 0; j < WIDTH; j++)
        {
            delete squares[j][linesToDelete[i]];
            squares[j][linesToDelete[i]] = nullptr;
        }
    }
    std::sort(linesToDelete.begin(),linesToDelete.end());
    linesToDelete.insert(linesToDelete.begin(),-1);     // To make the following loop concise
    animationOfFall->clear();
    for(int i = linesToDelete.size() - 1; i >= 1; i--)
    {
        for(int j = linesToDelete[i] - 1; j > linesToDelete[i - 1]; j--)
        {
            for(int k = 0; k < WIDTH; k++)
            {
                if(squares[k][j])
                {
                    QPropertyAnimation *animation = new QPropertyAnimation(squares[k][j],"pos");
                    animation->setEasingCurve(QEasingCurve(QEasingCurve::OutElastic));
                    animation->setStartValue(squares[k][j]->pos());
                    animation->setEndValue(squares[k][j]->pos() + QPoint(0,(linesToDelete.size() - i) * SIZEOFSQUARE));
                    animation->setDuration(500);
                    animationOfFall->addAnimation(animation);
                }
                // No matther whether squares[k][j] is nullptr, this should be implemented.
                squares[k][j + linesToDelete.size() - i] = squares[k][j];
            }
        }
    }
    animationOfFall->start();
}

void Board::fall()
{
    moveUnit(Down);
}
