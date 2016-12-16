#ifndef ROBOT_H
#define ROBOT_H


#include <QGraphicsItem>
#include <vector>
#include "udp.h"
QT_BEGIN_NAMESPACE
class QGraphicsSceneMouseEvent;
class QParallelAnimationGroup;
QT_END_NAMESPACE

//base class for robot parts - all defined-classes are derived form RobotPart
class RobotPart : public QGraphicsObject
{
Q_OBJECT
public:
    RobotPart(QGraphicsItem *parent = 0);
    MyUDP client1;

protected:
    void dragEnterEvent(QGraphicsSceneDragDropEvent *event) Q_DECL_OVERRIDE;
    void dragLeaveEvent(QGraphicsSceneDragDropEvent *event) Q_DECL_OVERRIDE;
    void dropEvent(QGraphicsSceneDragDropEvent *event) Q_DECL_OVERRIDE;

    QColor color;
    bool dragOver;
};

//defines the paint sprayer and the location of the paint spots
class RobotPainter : public RobotPart
{
Q_OBJECT
public:
    RobotPainter(QGraphicsItem *parent = 0);
    QRect paintbrush; //location of paint sprayer
    std::vector<std::pair<QPointF,int> > paintSpots; //vector of paint spots
    QRectF boundingRect() const Q_DECL_OVERRIDE;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0) Q_DECL_OVERRIDE;
    inline void addPaintSpot(std::pair<QPointF, int> p ) {paintSpots.push_back(p);} //adds paint spot to vector
};

//defines the links and axes
class RobotLink : public RobotPart
{
Q_OBJECT
    int length; //holds practical link length (distance between axes)

public:
    RobotLink(QGraphicsItem *parent = 0, int l=0);
    QRect link;
    QRect axis;
    QMatrix m;
    QRectF boundingRect() const Q_DECL_OVERRIDE;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0) Q_DECL_OVERRIDE;
    void rotateClockwise();
};

//defines robot as combination of parts
class Robot : public RobotPart
{
Q_OBJECT
    RobotLink *link1; //first link and axis
    RobotLink *link2; //second link and axis
    RobotLink *link3; //third link and axis
    RobotPainter *paintSprayer; //paint sprayer and paint spots
public:
    Robot(QGraphicsItem *parent = 0);
    QRectF boundingRect() const Q_DECL_OVERRIDE;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0) Q_DECL_OVERRIDE;
public slots:
    void rotateAboutAxis1(); //move right on axis 1
    void rotateAboutAxis2(); //rotate clockwise on axis 2
    void rotateAboutAxis3(); //rotate clockwise on axis 3
    void rotateCounterAboutAxis1(); //move left on axis 1
    void rotateCounterAboutAxis2(); //rotate counterclockwise on axis 2
    void rotateCounterAboutAxis3(); //rotate counterclockwise on axis 3
    void paintCircle(); //update vector of paint circles
    void moveUp(); //moves position up using inverse kinematics
    void moveUp1(qreal a, qreal b); //moves position up using inverse kinematics
    void moveDown(); //moves position down using inverse kinematics
};

#endif
