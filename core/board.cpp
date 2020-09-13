#include "core/board.h"
#include <QRandomGenerator>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <algorithm>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QVBoxLayout>

Board::Board(QWidget *parent)
    :QWidget(parent)
{
    setStyleSheet(".QFrame{border:1px solid black;}"
                  "QPushButton{font:9pt Arial Black;}"
                  "QLabel{font:9pt Arial Black;}");
    connect(this,&Board::moveTowards,this,&Board::moveUnit);
    connect(this,&Board::toBottom,this,&Board::updateSquares);
    animationOfFall = new QParallelAnimationGroup;
    animationOfFade = new QParallelAnimationGroup;
    connect(animationOfFade,&QParallelAnimationGroup::finished,this,&Board::deleteLines);
    connect(animationOfFall,&QParallelAnimationGroup::finished,this,&Board::placeUnit);
    timer = new QTimer(this);
    connect(timer,&QTimer::timeout,this,&Board::fall);
    //connect(this,&Board::toBottom,timer,&QTimer::stop);
    /* The timer can also be stopped in the function updateSquares which will
     * be called after the toBottom signal is emitted. */
    leftArea = new QFrame(this);
    leftArea->setFixedSize(WIDTH * SIZE_OF_SQUARE,HEIGHT * SIZE_OF_SQUARE);
    leftArea->setFocusPolicy(Qt::ClickFocus);
    leftArea->setFocus();
    nextUnitArea = new QFrame(this);
    nextUnitArea->setFixedSize(4 * SIZE_OF_SQUARE,4 * SIZE_OF_SQUARE);
    scoreLabel = new QLabel(tr("score"));
    scoreValueLabel = new QLabel(tr("0"));
    pauseButton = new QPushButton(tr("pause"),this);
    restartButton = new QPushButton(tr("restart"),this);
    connect(pauseButton,&QPushButton::clicked,this,&Board::pause);
    connect(restartButton,&QPushButton::clicked,this,&Board::restart);
    QHBoxLayout *scoreLayout = new QHBoxLayout;
    scoreLayout->addWidget(scoreLabel);
    scoreLayout->addWidget(scoreValueLabel);
    QVBoxLayout *rightLayout = new QVBoxLayout;
    rightLayout->addWidget(nextUnitArea);
    rightLayout->addLayout(scoreLayout);
    rightLayout->addWidget(pauseButton);
    rightLayout->addWidget(restartButton);
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->addWidget(leftArea);
    mainLayout->addLayout(rightLayout);
    setLayout(mainLayout);
    show();
    setFixedSize(size());

    for(int i = 0; i < WIDTH; i++)
        for(int j = 0; j < HEIGHT; j++)
            squares[i][j] = nullptr;
    for(int i = 0; i < 4; i++)
    {
        shadowUnit[i] = new QLabel(leftArea);
        shadowUnit[i]->setStyleSheet("background-color:rgba(0,0,0,0.1);");
        shadowUnit[i]->hide();
    }
    generateUnit(nextUnit);
    placeUnit();
}

Board::~Board()
{}

void Board::keyPressEvent(QKeyEvent *event)
{
    if(!keyboardEnabled)return;
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
    if(dir == Up && currentUnit->shape == I)currentUnit->spinDir = static_cast<SpinDirection>(-currentUnit->spinDir);
    int x = currentUnit->spinCenter.x(),y = currentUnit->spinCenter.y();
    int posValid;
    bool isBottom = false;
    for(int i = 0; i < 4; i++)
    {
        if(dir == Down)
        {
            posValid = isPosValid(newPos[i] = currentUnit->unitOperated[i].pos + QPoint(0,1));
            if(posValid)
            {
                isBottom = true;
                break;
            }
        }
        else if(dir == Left)
        {
            posValid = isPosValid(newPos[i] = currentUnit->unitOperated[i].pos - QPoint(1,0));
            if(posValid)break;
        }
        else if(dir == Right)
        {
            posValid = isPosValid(newPos[i] = currentUnit->unitOperated[i].pos + QPoint(1,0));
            if(posValid)break;
        }
        else
        {
            if(currentUnit->shape == O)
                for(int i = 0; i < 4; i++)
                    newPos[i] = currentUnit->unitOperated[i].pos;
            /* It is difficult to decide the spinCenter of Shape O,
             * so the following algoritm does not fit Shape O.
             * Actually, it does not need to be rotated. */
            if(currentUnit->spinDir == AntiClockWise)
            {
                posValid = isPosValid(newPos[i] = QPoint(x + y - currentUnit->unitOperated[i].pos.y(),
                                        currentUnit->unitOperated[i].pos.x() - x + y));
                if(posValid)break;
            }
            else
            {
                posValid = isPosValid(newPos[i] = QPoint(x - y + currentUnit->unitOperated[i].pos.y(),
                                         x + y - currentUnit->unitOperated[i].pos.x()));
                if(posValid)break;
            }
        }
    }
    if(isBottom)
    {
        emit toBottom();
        return;
    }
    if(posValid && currentUnit->shape == I && dir == Up)
        currentUnit->spinDir = static_cast<SpinDirection>(-currentUnit->spinDir);
    if(posValid)return;
    for(int i = 0; i < 4; i++)
    {
        currentUnit->unitOperated[i].pos = newPos[i];
        currentUnit->unitOperated[i].square->setGeometry(newPos[i].x() * SIZE_OF_SQUARE,newPos[i].y() * SIZE_OF_SQUARE,
                                            SIZE_OF_SQUARE,SIZE_OF_SQUARE);
    }
    placeShadowUnit();

    if(dir == Down)currentUnit->spinCenter += QPoint(0,1);
    else if(dir == Left)currentUnit->spinCenter -= QPoint(1,0);
    else if(dir == Right)currentUnit->spinCenter += QPoint(1,0);
}

/* This function can be modified to illustrate whether the unit
 * is by the border, to improve the rotation of units.
   If improved, the function moveUnit should be modified too. */
/*  return value:
 *  0: no collision
 *  1: touch the left border
 *  2: touch the right border
 *  3: touch the bottom border
 *  4: collide with other squares */
int Board::isPosValid(QPoint pos)
{
    int x = pos.x(),y = pos.y();
    if(x < 0)return 1;
    if(x >= WIDTH)return 2;
    if(y >= HEIGHT)return 3;
    if(squares[x][y] != nullptr)return 4;
    return 0;
}

void Board::generateUnit(Unit* &unit)
{   
    unit = new Unit;
    unit->shape = static_cast<Shape>(QRandomGenerator::global()->bounded(7));
    for(int i = 0; i < 4; i++)
    {
        unit->unitOperated[i].square = new QLabel;          // The parent will be set in placeUnit function.
        unit->unitOperated[i].square->setStyleSheet("margin:2px;background-color:" + color[unit->shape] + ";");
    }
    unit->dir = static_cast<Direction>(QRandomGenerator::global()->bounded(4));
    unit->spinDir = AntiClockWise;
    int mid = WIDTH / 2;
    if(unit->shape == I)
    {
        if(unit->dir == Up || unit->dir == Down)
        {
            unit->spinCenter = QPoint(mid,3);
            unit->spinDir = ClockWise;
            for(int i = 0; i < 4; i++)
                unit->unitOperated[i].pos = QPoint(mid,i);
        }
        else
        {
            unit->spinCenter = QPoint(mid,0);
            for(int i = 0; i < 4; i++)
                unit->unitOperated[i].pos = QPoint(mid + i,0);
        }
    }
    else if(unit->shape == J)
    {
        if(unit->dir == Up)                         //   O
        {                                           // * O
            unit->spinCenter = QPoint(mid,1);       // O O
            for(int i = 0; i < 3; i++)
                unit->unitOperated[i].pos = QPoint(mid + 1,i);
            unit->unitOperated[3].pos = QPoint(mid,2);
        }
        else if(unit->dir == Left)                  // O O O
        {                                           //   * O
            unit->spinCenter = QPoint(mid,1);
            for(int i = 0; i < 3; i++)
                unit->unitOperated[i].pos = QPoint(mid - 1 + i,0);
            unit->unitOperated[3].pos = QPoint(mid + 1,1);
        }
        else if(unit->dir == Down)                  // O O
        {                                           // O *
            unit->spinCenter = QPoint(mid,1);       // O
            for(int i = 0; i < 3; i++)
                unit->unitOperated[i].pos = QPoint(mid - 1,i);
            unit->unitOperated[3].pos = QPoint(mid,0);
        }
        else                                        // O *
        {                                           // O O O
            unit->spinCenter = QPoint(mid,0);
            for(int i = 0; i < 3; i++)
                unit->unitOperated[i].pos = QPoint(mid - 1 + i,1);
            unit->unitOperated[3].pos = QPoint(mid - 1,0);
        }
    }
    else if(unit->shape == L)
    {
        if(unit->dir == Up)                         // O
        {                                           // O *
            unit->spinCenter = QPoint(mid,1);       // O O
            for(int i = 0; i < 3; i++)
                unit->unitOperated[i].pos = QPoint(mid - 1,i);
            unit->unitOperated[3].pos = QPoint(mid,2);
        }
        else if(unit->dir == Left)                  //   * O
        {                                           // O O O
            unit->spinCenter = QPoint(mid,0);
            for(int i = 0; i < 3; i++)
                unit->unitOperated[i].pos = QPoint(mid - 1 + i,1);
            unit->unitOperated[3].pos = QPoint(mid + 1,0);
        }
        else if(unit->dir == Down)                  // O O
        {                                           // * O
            unit->spinCenter = QPoint(mid,1);       //   O
            for(int i = 0; i < 3; i++)
                unit->unitOperated[i].pos = QPoint(mid + 1,i);
            unit->unitOperated[3].pos = QPoint(mid,0);
        }
        else                                        // O O O
        {                                           // O *
            unit->spinCenter = QPoint(mid,1);
            for(int i = 0; i < 3; i++)
                unit->unitOperated[i].pos = QPoint(mid - 1 + i,0);
            unit->unitOperated[3].pos = QPoint(mid - 1,1);
        }
    }
    else if(unit->shape == S)
    {
        if(unit->dir == Up || unit->dir == Down)    // O
        {                                           // * O
            unit->spinCenter = QPoint(mid,1);       //   O
            unit->unitOperated[0].pos = QPoint(mid,0);
            unit->unitOperated[1].pos = QPoint(mid,1);
            unit->unitOperated[2].pos = QPoint(mid + 1,1);
            unit->unitOperated[3].pos = QPoint(mid + 1,2);
        }
        else if(unit->dir == Left || unit->dir == Right)    //   * O
        {                                                   // O O
            unit->spinCenter = QPoint(mid,0);
            unit->unitOperated[0].pos = QPoint(mid,0);
            unit->unitOperated[1].pos = QPoint(mid + 1,0);
            unit->unitOperated[2].pos = QPoint(mid - 1,1);
            unit->unitOperated[3].pos = QPoint(mid,1);
        }
    }
    else if(unit->shape == Z)
    {
        if(unit->dir == Up || unit->dir == Down)        //   O
        {                                               // O *
            unit->spinCenter = QPoint(mid,1);           // O
            unit->unitOperated[0].pos = QPoint(mid,0);
            unit->unitOperated[1].pos = QPoint(mid,1);
            unit->unitOperated[2].pos = QPoint(mid - 1,1);
            unit->unitOperated[3].pos = QPoint(mid - 1,2);
        }
        else if(unit->dir == Left || unit->dir == Right)    // O *
        {                                                   //   O O
            unit->spinCenter = QPoint(mid,0);
            unit->unitOperated[0].pos = QPoint(mid - 1,0);
            unit->unitOperated[1].pos = QPoint(mid,0);
            unit->unitOperated[2].pos = QPoint(mid,1);
            unit->unitOperated[3].pos = QPoint(mid + 1,1);
        }
    }
    else if(unit->shape == O)
    {
        // spinCenter can be any for Shape O,because my spin algorithm excludes Shape O.
        unit->unitOperated[0].pos = QPoint(mid,0);
        unit->unitOperated[1].pos = QPoint(mid + 1,0);
        unit->unitOperated[2].pos = QPoint(mid,1);
        unit->unitOperated[3].pos = QPoint(mid + 1,1);
    }
    else if(unit->shape == T)
    {
        if(unit->dir == Up)                         //   O
        {                                           // O * O
            unit->spinCenter = QPoint(mid,1);
            for(int i = 0; i < 3; i++)
                unit->unitOperated[i].pos = QPoint(mid - 1 + i,1);
            unit->unitOperated[3].pos = QPoint(mid,0);
        }
        else if(unit->dir == Left)                  //   O
        {                                           // O *
            unit->spinCenter = QPoint(mid,1);       //   O
            for(int i = 0; i < 3; i++)
                unit->unitOperated[i].pos = QPoint(mid,i);
            unit->unitOperated[3].pos = QPoint(mid - 1,1);
        }
        else if(unit->dir == Down)                  // O * O
        {                                           //   O
            unit->spinCenter = QPoint(mid,0);
            for(int i = 0; i < 3; i++)
                unit->unitOperated[i].pos = QPoint(mid - 1 + i,0);
            unit->unitOperated[3].pos = QPoint(mid,1);
        }
        else                                        // O
        {                                           // * O
            unit->spinCenter = QPoint(mid,1);       // O
            for(int i = 0; i < 3; i++)
                unit->unitOperated[i].pos = QPoint(mid,i);
            unit->unitOperated[3].pos = QPoint(mid + 1,1);
        }
    }
}

/* This function is called when the unit is at the bottom. */
void Board::updateSquares()
{
    timer->stop();
    std::vector<int> lines;
    for(int i = 0; i < 4; i++)
    {
        int x = currentUnit->unitOperated[i].pos.x(), y = currentUnit->unitOperated[i].pos.y();
        squares[x][y] = currentUnit->unitOperated[i].square;
        //currentUnit->unitOperated[i].square = nullptr;
        if(std::find(lines.begin(),lines.end(),y) == lines.end())lines.push_back(y);
    }
    delete currentUnit;             // No need to delete the pointer currentUnit->unitOperated[i].square
    currentUnit = nullptr;
    if(!getLinesToDelete(lines))placeUnit();
}

// The return value represents whether there are lines to be deleted.
bool Board::getLinesToDelete(std::vector<int> lines)
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
    if(linesToDelete.empty())return false;
    score += points[linesToDelete.size() - 1];
    scoreValueLabel->setText(QString::number(score));
    if(score >= level * SCORE_OF_UPGRADE)
    {
        level++;
        durationOfHalt *= 0.8;
    }
    keyboardEnabled = false;
    animationOfFade->clear();
    for(int i = 0; i < linesToDelete.size(); i++)
    {
        //qDebug("%d",linesToDelete[i]);
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
    return true;
}

void Board::deleteLines()
{
    //qDebug("delete lines");
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
                    animation->setEndValue(squares[k][j]->pos() + QPoint(0,(linesToDelete.size() - i) * SIZE_OF_SQUARE));
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
    if(timer->interval() != durationOfHalt)timer->start(durationOfHalt);
    moveUnit(Down);
}

void Board::placeUnit()         // Called after a new unit is generated.
{
    currentUnit = nextUnit;
    nextUnit = nullptr;         // necessary
    for(int i = 0; i < 4; i++)
    {
        if(isPosValid(currentUnit->unitOperated[i].pos))
        {
            QMessageBox::information(this,tr("Game Over!"),tr("You've lost!"),
                                     QMessageBox::Ok);
            keyboardEnabled = false;
            pauseButton->setDisabled(true);
            timer->stop();
            return;
        }
        currentUnit->unitOperated[i].square->setParent(leftArea);
        currentUnit->unitOperated[i].square->setGeometry(
                    currentUnit->unitOperated[i].pos.x() * SIZE_OF_SQUARE,
                    currentUnit->unitOperated[i].pos.y() * SIZE_OF_SQUARE,
                    SIZE_OF_SQUARE,SIZE_OF_SQUARE);
        currentUnit->unitOperated[i].square->show();
    }
    generateUnit(nextUnit);
    placeShadowUnit();

    std::vector<int> xPos,yPos;
    for(int i = 0; i < 4; i++)
    {
        xPos.emplace_back(nextUnit->unitOperated[i].pos.x());
        yPos.emplace_back(nextUnit->unitOperated[i].pos.y());
    }
    int minX = *std::min_element(xPos.begin(),xPos.end());
    int minY = *std::min_element(yPos.begin(),yPos.end());
    for(int i = 0; i < 4; i++)
    {
        nextUnit->unitOperated[i].square->setParent(nextUnitArea);
        if(nextUnit->shape != I)
            nextUnit->unitOperated[i].square->setGeometry(
                    (nextUnit->unitOperated[i].pos.x() - minX + 1) * SIZE_OF_SQUARE,
                    (nextUnit->unitOperated[i].pos.y() - minY + 1) * SIZE_OF_SQUARE,
                    SIZE_OF_SQUARE,SIZE_OF_SQUARE);
        else
        {
            if(nextUnit->dir == Up || nextUnit->dir == Down)
                nextUnit->unitOperated[i].square->setGeometry(SIZE_OF_SQUARE,
                        i * SIZE_OF_SQUARE,SIZE_OF_SQUARE,SIZE_OF_SQUARE);
            else
                nextUnit->unitOperated[i].square->setGeometry(i * SIZE_OF_SQUARE,
                        SIZE_OF_SQUARE,SIZE_OF_SQUARE,SIZE_OF_SQUARE);
        }
        nextUnit->unitOperated[i].square->show();
    }
    keyboardEnabled = true;
    timer->start(durationOfHalt);
}

void Board::restart()
{
    leftArea->setFocus();
    if(currentUnit)
    {
        for(int i = 0; i < 4; i++)
            delete currentUnit->unitOperated[i].square;
        currentUnit = nullptr;
    }
    if(nextUnit)
    {
        for(int i = 0; i < 4; i++)
            delete nextUnit->unitOperated[i].square;
        delete nextUnit;
        nextUnit = nullptr;
    }
    score = 0;
    scoreValueLabel->setText(tr("0"));
    for(int i = 0; i < WIDTH; i++)
        for(int j = 0; j < HEIGHT; j++)
        {
            delete squares[i][j];
            squares[i][j] = nullptr;
        }
    durationOfHalt = DURATION_OF_HALT;
    generateUnit(nextUnit);
    placeUnit();
}

void Board::pause()
{
    pauseButton->setDisabled(true);
    if(hasPaused)
    {
        leftArea->setFocus();
        timer->start(remainingTime);
        pauseButton->setText(tr("pause"));
        keyboardEnabled = true;
        hasPaused = false;
    }
    else
    {
        remainingTime = timer->remainingTime();
        timer->stop();
        pauseButton->setText(tr("continue"));
        keyboardEnabled = false;
        hasPaused = true;
    }
    pauseButton->setEnabled(true);
}

void Board::placeShadowUnit()       // Called after the currentUnit is placed or moved
{
    int minDistanceToBottom = HEIGHT;
    for(int i = 0; i < 4; i++)
    {
        int d = 0;
        for(int j = currentUnit->unitOperated[i].pos.y() + 1; j < HEIGHT; j++)
        {
            if(squares[currentUnit->unitOperated[i].pos.x()][j])break;
            d++;
        }
        if(d < minDistanceToBottom)minDistanceToBottom = d;
    }
    for(int i = 0; i < 4; i++)
    {
        shadowUnit[i]->setGeometry(QRect((QPoint(currentUnit->unitOperated[i].pos) + QPoint(0,minDistanceToBottom))
                                         * SIZE_OF_SQUARE,QSize(SIZE_OF_SQUARE,SIZE_OF_SQUARE)));
        shadowUnit[i]->show();
    }
}
