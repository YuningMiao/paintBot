#include <QtWidgets>
#include "robot.h"
//#include "main.cpp"



RobotPart::RobotPart(QGraphicsItem *parent)
   : QGraphicsObject(parent), color(Qt::lightGray), dragOver(false)
{
   setAcceptDrops(true);
}

void RobotPart::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
   if (event->mimeData()->hasColor()) {
       event->setAccepted(true);
       dragOver = true;
       update();
   } else {
       event->setAccepted(false);
   }
}

void RobotPart::dragLeaveEvent(QGraphicsSceneDragDropEvent *event)
{
   Q_UNUSED(event);
   dragOver = false;
   update();
}

void RobotPart::dropEvent(QGraphicsSceneDragDropEvent *event)
{
   dragOver = false;
   if (event->mimeData()->hasColor())
       color = qvariant_cast<QColor>(event->mimeData()->colorData());
   update();
}

RobotLink::RobotLink(QGraphicsItem *parent, int l)
   : RobotPart(parent)
{
   this->length = l;
}

QRectF RobotLink::boundingRect() const
{
   return QRectF(0, 0, 30, length * 30);
}

//defines the draw event for a robot link
void RobotLink::paint(QPainter *painter,
          const QStyleOptionGraphicsItem *option, QWidget *widget)
{
   Q_UNUSED(option);
   Q_UNUSED(widget);
   link = QRect(0, 0, 30, length + 30);
   axis = QRect(10, 10 + length, 10, 10);
   painter->setBrush(dragOver ? color.light(130) : color);
   painter->drawEllipse(link);
   painter->drawEllipse(axis);
}

RobotPainter::RobotPainter(QGraphicsItem *parent)
   : RobotPart(parent)
{
}

QRectF RobotPainter::boundingRect() const
{
   return QRectF(0, 0, 10, 10);
}

//paint event for robot painter
void RobotPainter::paint(QPainter *painter,
          const QStyleOptionGraphicsItem *option, QWidget *widget)
{
   Q_UNUSED(option);
   Q_UNUSED(widget);

   paintbrush = QRect(10,10,10,10);

   //draws paint spots
   for(int i=0; i<paintSpots.size(); i++)
   {
       QPointF localPaintSpot = this->mapFromScene(paintSpots[i].first);
       if(paintSpots[i].second == 0) painter->setBrush(Qt::red);
       QRect test = QRect(localPaintSpot.toPoint().x(), localPaintSpot.toPoint().y(),10, 10);
       painter->drawEllipse(test);
   }
   //draw the paintbrush
   painter->setBrush(Qt::black);
   painter->drawEllipse(paintbrush);
}

Robot::Robot(QGraphicsItem *parent)
   : RobotPart(parent)
{
   setFlag(ItemHasNoContents);

   //define and position robot components
   link1 = new RobotLink(this, 150);
   link2 = new RobotLink(this, 100);
   link3 = new RobotLink(this, 75);
   link2->setPos(0, -100);
   link3->setPos(0, -175);
   paintSprayer = new RobotPainter(this);
   paintSprayer->setPos(0,-175);
   QMatrix m =this->matrix();
}

QRectF Robot::boundingRect() const
{
   return QRectF();
}

void Robot::paint(QPainter *painter,
                 const QStyleOptionGraphicsItem *option, QWidget *widget)
{
   Q_UNUSED(painter);
   Q_UNUSED(option);
   Q_UNUSED(widget);
}

void Robot::paintCircle()
{
   int color = 0;
   QPointF a,b;

   //potential expansion to allow for more colors
   switch(color)
   {
       case 0:
               b = this->mapFromItem(this->paintSprayer, this->paintSprayer->paintbrush.x(), this->paintSprayer->paintbrush.y());
               this->paintSprayer->addPaintSpot(std::make_pair(b,0));
               paintSprayer->update();
               link1->update();
               link2->update();
               link3->update();
               break;

       default:qDebug() << "black";
               break;
   }
   client1.sendUDP("p");
}
void Robot::moveUp1(qreal dx, qreal dy)
{
   QPointF current = paintSprayer->mapToScene(paintSprayer->paintbrush.center().x(), paintSprayer->paintbrush.center().y());
   qreal x = current.x() - link2->mapToScene(link2->axis.center()).x();
   qreal y = -((current.y() - link2->mapToScene(link2->axis.center()).y()));
   qreal desiredX = dx;
   qreal desiredY = dy;
   qDebug() << x << ", " << y;
   qreal c2 = (qPow(desiredX,2)+qPow(desiredY,2)-qPow(100,2)-qPow(75,2))/(2*100*75);
   qreal s2 = qSqrt(1-qPow(c2,2));
   qreal k1 = 100 + 75*c2;
   qreal k2 = 75*s2;
   qreal theta2 = qAtan2(s2,c2);
   qreal theta1 = qAtan2(desiredY,x) - qAtan2(k2,k1);
   qreal a3x = link3->mapToScene(link3->axis.center()).x()-link2->mapToScene(link2->axis.center()).x();
   qreal a3y = -(link3->mapToScene(link3->axis.center()).y()-link2->mapToScene(link2->axis.center()).y());
   qDebug() << a3x << ", " << a3y;
   qreal currentTheta1=0;
   if(a3x != 0) currentTheta1= qAtan(a3y/a3x);
   qreal currentTheta2 = 0 - currentTheta1;
   if(x-a3x !=0) currentTheta2 = qAtan((y-a3y)/(x-a3x)) - currentTheta1;
   qDebug() << currentTheta1 << ' ' << currentTheta2;

   if ((qPow(100-75,2) <= (qPow(desiredX,2)+qPow(desiredY,2)))&&((qPow(desiredX,2)+qPow(desiredY,2))<=(qPow(100+75,2))))
   {
       //return everything to center frame
       QPoint link2center = link2->axis.center();

       QPointF link2centerScene = link2->mapToScene(QPointF(link2center.x(),link2center.y()));

       QPointF link2inLink3 = link3->mapFromScene(link2centerScene);

       QPointF l3scene = link3->mapToScene(link3->axis.center());
       qreal angle2 = qAtan((current.y()-l3scene.y())/(current.x()-l3scene.x()));

       //lines up link3 with link2
       QPoint a = link3->axis.center();
       QTransform axis3Transform2;
       axis3Transform2.translate(a.x(), a.y());
       if(((current.x()>=l3scene.x())&&l3scene.x()>0)||((current.x()<l3scene.x())&&l3scene.x()<0)) axis3Transform2.rotate(180/3.1415*(-angle2 - currentTheta1));
       else axis3Transform2.rotate(180/3.14159265359*(-angle2 - currentTheta1)+180);
       axis3Transform2.translate(-a.x(), -a.y());
       link3->setTransform(axis3Transform2, true);
       paintSprayer->setTransform(axis3Transform2, true);

       //lines up all links
       qreal angleT = qAtan(l3scene.y()-link2center.y())/(l3scene.x()-link2center.x());
       QTransform axis2Transform;
       axis2Transform.translate(link2center.x(), link2center.y());
       axis2Transform.rotate(-180/3.14159265359*angleT);
       axis2Transform.translate(-link2center.x(), -link2center.y());
       link2->setTransform(axis2Transform);
       link3->setTransform(axis2Transform);
       paintSprayer->setTransform(axis2Transform);

       link1->resetTransform();
       link2->resetTransform();
       link3->resetTransform();
       paintSprayer->resetTransform();
       //set new position based off theta1
       QTransform axis2Transform2;
       axis2Transform2.translate(link2center.x(), link2center.y());
       axis2Transform2.rotate(90-180/3.14159265359*theta1);
       axis2Transform2.translate(-link2center.x(), -link2center.y());
       link2->setTransform(axis2Transform2,true);

       link2inLink3 = link3->mapFromScene(link2centerScene);

       QTransform axis3Transform3;
       axis3Transform3.translate(link2inLink3.x(), link2inLink3.y());
       axis3Transform3.rotate(90-180/3.14159265359*theta1);
       axis3Transform3.translate(-link2inLink3.x(), -link2inLink3.y());
       link3->setTransform(axis3Transform3,true);
       paintSprayer->setTransform(axis3Transform3,true);

       //sets new position based off theta2
       QPoint bb = link3->axis.center();
       QTransform axis3Transform;
       axis3Transform.translate(bb.x(), bb.y());
       if(paintSprayer->mapToScene(paintSprayer->paintbrush.center()).y() < link2->mapToScene(link2->axis.center()).y()) axis3Transform.rotate(-180/3.14159265359*theta2);
       else {qDebug() << "here";axis3Transform.rotate(-180/3.14159265359*theta2);}
       axis3Transform.translate(-bb.x(), -bb.y());
       link3->setTransform(axis3Transform, true);
       paintSprayer->setTransform(axis3Transform, true);



   }
   else
   {
       //if not too high or low or too far left or right
       while(-(link2->mapToScene(link2->axis.center()).y() - paintSprayer->mapToScene(paintSprayer->paintbrush.center()).y())<=175)
       {
           if(link2->mapToScene(link2->axis.center()).x()<x)
           {
               qDebug() << "Move right";
               QPointF loc1 = link1->pos();
               link1->setPos(QPointF(loc1.toPoint().x() + 10, loc1.toPoint().y()));
               QPointF loc2 = link2->pos();
               link2->setPos(QPointF(loc2.toPoint().x() + 10, loc2.toPoint().y()));
               QPointF loc3 = link3->pos();
               link3->setPos(QPointF(loc3.toPoint().x() + 10, loc3.toPoint().y()));
               QPointF loc4 = paintSprayer->pos();
               paintSprayer->setPos(QPointF(loc4.toPoint().x() + 10, loc4.toPoint().y()));

           }
           else
           {
               qDebug() << "Move left";
               QPointF loc1 = link1->pos();
               link1->setPos(QPointF(loc1.toPoint().x() - 10, loc1.toPoint().y()));
               QPointF loc2 = link2->pos();
               link2->setPos(QPointF(loc2.toPoint().x() - 10, loc2.toPoint().y()));
               QPointF loc3 = link3->pos();
               link3->setPos(QPointF(loc3.toPoint().x() - 10, loc3.toPoint().y()));
               QPointF loc4 = paintSprayer->pos();
               paintSprayer->setPos(QPointF(loc4.toPoint().x() - 10, loc4.toPoint().y()));
               this->moveUp1(desiredX+10, desiredY);

           }
       }
   }

}
void Robot::moveUp()
{
   bool repeating = false;
   bool stuck = false;
   QPointF current = paintSprayer->mapToScene(paintSprayer->paintbrush.center().x(), paintSprayer->paintbrush.center().y());
   qreal x = current.x() - link2->mapToScene(link2->axis.center()).x();
   qreal y = -((current.y() - link2->mapToScene(link2->axis.center()).y()));
   qreal desiredX = x;
   qreal desiredY = y + 5;

    REPEAT:
    if (repeating == true) desiredX = 0;
    if (stuck == true) desiredX = -30;
    repeating = false;
    stuck = false;
   qDebug() << x << ", " << y;
   qreal c2 = (qPow(desiredX,2)+qPow(desiredY,2)-qPow(100,2)-qPow(75,2))/(2*100*75);
   qreal s2 = qSqrt(1-qPow(c2,2));
   qreal k1 = 100 + 75*c2;
   qreal k2 = 75*s2;
   qreal theta2 = qAtan2(s2,c2);
   qreal theta1 = qAtan2(desiredY,desiredX) - qAtan2(k2,k1);
   qreal a3x = link3->mapToScene(link3->axis.center()).x()-link2->mapToScene(link2->axis.center()).x();
   qreal a3y = -(link3->mapToScene(link3->axis.center()).y()-link2->mapToScene(link2->axis.center()).y());
   qDebug() << a3x << ", " << a3y;
   qreal currentTheta1=0;
   if(a3x != 0) currentTheta1= qAtan(a3y/a3x);
   qreal currentTheta2 = 0 - currentTheta1;
   if(x-a3x !=0) currentTheta2 = qAtan((y-a3y)/(x-a3x)) - currentTheta1;
   qDebug() << currentTheta1 << ' ' << currentTheta2;

   if ((qPow(100-75,2) <= (qPow(desiredX,2)+qPow(desiredY,2)))&&((qPow(desiredX,2)+qPow(desiredY,2))<=(qPow(100+75,2))))
   {
       //return everything to center frame
       QPoint link2center = link2->axis.center();

       QPointF link2centerScene = link2->mapToScene(QPointF(link2center.x(),link2center.y()));

       QPointF link2inLink3 = link3->mapFromScene(link2centerScene);

       QPointF l3scene = link3->mapToScene(link3->axis.center());
       qreal angle2 = qAtan((current.y()-l3scene.y())/(current.x()-l3scene.x()));

       //lines up link3 with link2
       QPoint a = link3->axis.center();
       QTransform axis3Transform2;
       axis3Transform2.translate(a.x(), a.y());
       if(((current.x()>=l3scene.x())&&l3scene.x()>0)||((current.x()<l3scene.x())&&l3scene.x()<0)) axis3Transform2.rotate(180/3.1415*(-angle2 - currentTheta1));
       else axis3Transform2.rotate(180/3.14159265359*(-angle2 - currentTheta1)+180);
       axis3Transform2.translate(-a.x(), -a.y());
       link3->setTransform(axis3Transform2, true);
       paintSprayer->setTransform(axis3Transform2, true);

       //lines up all links
       qreal angleT = qAtan(l3scene.y()-link2center.y())/(l3scene.x()-link2center.x());
       QTransform axis2Transform;
       axis2Transform.translate(link2center.x(), link2center.y());
       axis2Transform.rotate(-180/3.14159265359*angleT);
       axis2Transform.translate(-link2center.x(), -link2center.y());
       link2->setTransform(axis2Transform);
       link3->setTransform(axis2Transform);
       paintSprayer->setTransform(axis2Transform);

       link1->resetTransform();
       link2->resetTransform();
       link3->resetTransform();
       paintSprayer->resetTransform();
       //set new position based off theta1
       QTransform axis2Transform2;
       axis2Transform2.translate(link2center.x(), link2center.y());
       axis2Transform2.rotate(90-180/3.14159265359*theta1);
       axis2Transform2.translate(-link2center.x(), -link2center.y());
       link2->setTransform(axis2Transform2,true);

       link2inLink3 = link3->mapFromScene(link2centerScene);

       QTransform axis3Transform3;
       axis3Transform3.translate(link2inLink3.x(), link2inLink3.y());
       axis3Transform3.rotate(90-180/3.14159265359*theta1);
       axis3Transform3.translate(-link2inLink3.x(), -link2inLink3.y());
       link3->setTransform(axis3Transform3,true);
       paintSprayer->setTransform(axis3Transform3,true);

       //sets new position based off theta2
       QPoint bb = link3->axis.center();
       QTransform axis3Transform;
       axis3Transform.translate(bb.x(), bb.y());
       if(paintSprayer->mapToScene(paintSprayer->paintbrush.center()).y() < link2->mapToScene(link2->axis.center()).y()) axis3Transform.rotate(-180/3.14159265359*theta2);
       else {qDebug() << "here";axis3Transform.rotate(-180/3.14159265359*theta2);}
       axis3Transform.translate(-bb.x(), -bb.y());
       link3->setTransform(axis3Transform, true);
       paintSprayer->setTransform(axis3Transform, true);



   }
   else
   {
       //if not too high or low or too far left or right
       if(qAbs(link2->mapToScene(link2->axis.center()).y() - paintSprayer->mapToScene(paintSprayer->paintbrush.center()).y())<=171)
       {
           qDebug() << qAbs(link2->mapToScene(link2->axis.center()).y() - paintSprayer->mapToScene(paintSprayer->paintbrush.center()).y());
           if(qAbs(link2->mapToScene(link2->axis.center()).y() - paintSprayer->mapToScene(paintSprayer->paintbrush.center()).y())<=35)
           {
               qDebug() << "Problem!";
               QPointF loc1 = link1->pos();
               link1->setPos(QPointF(loc1.toPoint().x() + 30, loc1.toPoint().y()));
               QPointF loc2 = link2->pos();
               link2->setPos(QPointF(loc2.toPoint().x() + 30, loc2.toPoint().y()));
               QPointF loc3 = link3->pos();
               link3->setPos(QPointF(loc3.toPoint().x() + 30, loc3.toPoint().y()));
               QPointF loc4 = paintSprayer->pos();
               paintSprayer->setPos(QPointF(loc4.toPoint().x() + 30, loc4.toPoint().y()));
               stuck = true;
               goto REPEAT;

           }
           else if(link2->mapToScene(link2->axis.center()).x()<x)
           {
               qDebug() << "Move right";
               QPointF loc1 = link1->pos();
               link1->setPos(QPointF(loc1.toPoint().x() + desiredX, loc1.toPoint().y()));
               QPointF loc2 = link2->pos();
               link2->setPos(QPointF(loc2.toPoint().x() + desiredX, loc2.toPoint().y()));
               QPointF loc3 = link3->pos();
               link3->setPos(QPointF(loc3.toPoint().x() + desiredX, loc3.toPoint().y()));
               QPointF loc4 = paintSprayer->pos();
               paintSprayer->setPos(QPointF(loc4.toPoint().x() + desiredX, loc4.toPoint().y()));
               repeating = true;
               goto REPEAT;
           }
           else
           {
               qDebug() << "Move left";
               QPointF loc1 = link1->pos();
               link1->setPos(QPointF(loc1.toPoint().x() + desiredX, loc1.toPoint().y()));
               QPointF loc2 = link2->pos();
               link2->setPos(QPointF(loc2.toPoint().x() + desiredX, loc2.toPoint().y()));
               QPointF loc3 = link3->pos();
               link3->setPos(QPointF(loc3.toPoint().x() + desiredX, loc3.toPoint().y()));
               QPointF loc4 = paintSprayer->pos();
               paintSprayer->setPos(QPointF(loc4.toPoint().x() + desiredX, loc4.toPoint().y()));
               repeating = true;
               goto REPEAT;

           }
       }
   }
   client1.sendUDP(QString("mu"));

}

void Robot::moveDown()
{
    bool repeating = false;
    bool stuck = false;
    QPointF current = paintSprayer->mapToScene(paintSprayer->paintbrush.center().x(), paintSprayer->paintbrush.center().y());
    qreal x = current.x() - link2->mapToScene(link2->axis.center()).x();
    qreal y = -((current.y() - link2->mapToScene(link2->axis.center()).y()));
    qreal desiredX = x;
    qreal desiredY = y - 5;

     REPEAT:
     if (repeating == true) desiredX = 0;
     if (stuck == true) desiredX = -30;
     repeating = false;
     stuck = false;

    qDebug() << x << ", " << y;
    qreal c2 = (qPow(desiredX,2)+qPow(desiredY,2)-qPow(100,2)-qPow(75,2))/(2*100*75);
    qreal s2 = qSqrt(1-qPow(c2,2));
    qreal k1 = 100 + 75*c2;
    qreal k2 = 75*s2;
    qreal theta2 = qAtan2(s2,c2);
    qreal theta1 = qAtan2(desiredY,desiredX) - qAtan2(k2,k1);
    qreal a3x = link3->mapToScene(link3->axis.center()).x()-link2->mapToScene(link2->axis.center()).x();
    qreal a3y = -(link3->mapToScene(link3->axis.center()).y()-link2->mapToScene(link2->axis.center()).y());
    qDebug() << a3x << ", " << a3y;
    qreal currentTheta1=0;
    if(a3x != 0) currentTheta1= qAtan(a3y/a3x);
    qreal currentTheta2 = 0 - currentTheta1;
    if(x-a3x !=0) currentTheta2 = qAtan((y-a3y)/(x-a3x)) - currentTheta1;
    qDebug() << currentTheta1 << ' ' << currentTheta2;

    if ((qPow(100-75,2) <= (qPow(desiredX,2)+qPow(desiredY,2)))&&((qPow(desiredX,2)+qPow(desiredY,2))<=(qPow(100+75,2))))
    {
        //return everything to center frame
        QPoint link2center = link2->axis.center();

        QPointF link2centerScene = link2->mapToScene(QPointF(link2center.x(),link2center.y()));

        QPointF link2inLink3 = link3->mapFromScene(link2centerScene);

        QPointF l3scene = link3->mapToScene(link3->axis.center());
        qreal angle2 = qAtan((current.y()-l3scene.y())/(current.x()-l3scene.x()));

        //lines up link3 with link2
        QPoint a = link3->axis.center();
        QTransform axis3Transform2;
        axis3Transform2.translate(a.x(), a.y());
        if(((current.x()>=l3scene.x())&&l3scene.x()>0)||((current.x()<l3scene.x())&&l3scene.x()<0)) axis3Transform2.rotate(180/3.1415*(-angle2 - currentTheta1));
        else axis3Transform2.rotate(180/3.14159265359*(-angle2 - currentTheta1)+180);
        axis3Transform2.translate(-a.x(), -a.y());
        link3->setTransform(axis3Transform2, true);
        paintSprayer->setTransform(axis3Transform2, true);

        //lines up all links
        qreal angleT = qAtan(l3scene.y()-link2center.y())/(l3scene.x()-link2center.x());
        QTransform axis2Transform;
        axis2Transform.translate(link2center.x(), link2center.y());
        axis2Transform.rotate(-180/3.14159265359*angleT);
        axis2Transform.translate(-link2center.x(), -link2center.y());
        link2->setTransform(axis2Transform);
        link3->setTransform(axis2Transform);
        paintSprayer->setTransform(axis2Transform);

        link1->resetTransform();
        link2->resetTransform();
        link3->resetTransform();
        paintSprayer->resetTransform();
        //set new position based off theta1
        QTransform axis2Transform2;
        axis2Transform2.translate(link2center.x(), link2center.y());
        axis2Transform2.rotate(90-180/3.14159265359*theta1);
        axis2Transform2.translate(-link2center.x(), -link2center.y());
        link2->setTransform(axis2Transform2,true);

        link2inLink3 = link3->mapFromScene(link2centerScene);

        QTransform axis3Transform3;
        axis3Transform3.translate(link2inLink3.x(), link2inLink3.y());
        axis3Transform3.rotate(90-180/3.14159265359*theta1);
        axis3Transform3.translate(-link2inLink3.x(), -link2inLink3.y());
        link3->setTransform(axis3Transform3,true);
        paintSprayer->setTransform(axis3Transform3,true);

        //sets new position based off theta2
        QPoint bb = link3->axis.center();
        QTransform axis3Transform;
        axis3Transform.translate(bb.x(), bb.y());
        if(paintSprayer->mapToScene(paintSprayer->paintbrush.center()).y() < link2->mapToScene(link2->axis.center()).y()) axis3Transform.rotate(-180/3.14159265359*theta2);
        else {qDebug() << "here";axis3Transform.rotate(-180/3.14159265359*theta2);}
        axis3Transform.translate(-bb.x(), -bb.y());
        link3->setTransform(axis3Transform, true);
        paintSprayer->setTransform(axis3Transform, true);



    }
    else
    {
        //if not too high or low or too far left or right
        if(qAbs(link2->mapToScene(link2->axis.center()).y() - paintSprayer->mapToScene(paintSprayer->paintbrush.center()).y())<=170)
        {
            qDebug() << qAbs(link2->mapToScene(link2->axis.center()).y() - paintSprayer->mapToScene(paintSprayer->paintbrush.center()).y());
            if(qAbs(link2->mapToScene(link2->axis.center()).y() - paintSprayer->mapToScene(paintSprayer->paintbrush.center()).y())<=35)
            {
                qDebug() << "Problem!";
                QPointF loc1 = link1->pos();
                link1->setPos(QPointF(loc1.toPoint().x() + 30, loc1.toPoint().y()));
                QPointF loc2 = link2->pos();
                link2->setPos(QPointF(loc2.toPoint().x() + 30, loc2.toPoint().y()));
                QPointF loc3 = link3->pos();
                link3->setPos(QPointF(loc3.toPoint().x() + 30, loc3.toPoint().y()));
                QPointF loc4 = paintSprayer->pos();
                paintSprayer->setPos(QPointF(loc4.toPoint().x() + 30, loc4.toPoint().y()));
                stuck = true;
                goto REPEAT;

            }
            else if(link2->mapToScene(link2->axis.center()).x()<x)
            {
                qDebug() << "Move right";
                QPointF loc1 = link1->pos();
                link1->setPos(QPointF(loc1.toPoint().x() + desiredX, loc1.toPoint().y()));
                QPointF loc2 = link2->pos();
                link2->setPos(QPointF(loc2.toPoint().x() + desiredX, loc2.toPoint().y()));
                QPointF loc3 = link3->pos();
                link3->setPos(QPointF(loc3.toPoint().x() + desiredX, loc3.toPoint().y()));
                QPointF loc4 = paintSprayer->pos();
                paintSprayer->setPos(QPointF(loc4.toPoint().x() + desiredX, loc4.toPoint().y()));
                repeating = true;
                goto REPEAT;
            }
            else
            {
                qDebug() << "Move left";
                QPointF loc1 = link1->pos();
                link1->setPos(QPointF(loc1.toPoint().x() + desiredX, loc1.toPoint().y()));
                QPointF loc2 = link2->pos();
                link2->setPos(QPointF(loc2.toPoint().x() + desiredX, loc2.toPoint().y()));
                QPointF loc3 = link3->pos();
                link3->setPos(QPointF(loc3.toPoint().x() + desiredX, loc3.toPoint().y()));
                QPointF loc4 = paintSprayer->pos();
                paintSprayer->setPos(QPointF(loc4.toPoint().x() + desiredX, loc4.toPoint().y()));
                repeating = true;
                goto REPEAT;

            }
        }
    }
    client1.sendUDP(QString("md"));
}

void Robot::rotateAboutAxis1()
{
   QPointF loc1 = link1->pos();
   link1->setPos(QPointF(loc1.toPoint().x() + 4, loc1.toPoint().y()));
   QPointF loc2 = link2->pos();
   link2->setPos(QPointF(loc2.toPoint().x() + 4, loc2.toPoint().y()));
   QPointF loc3 = link3->pos();
   link3->setPos(QPointF(loc3.toPoint().x() + 4, loc3.toPoint().y()));
   QPointF loc4 = paintSprayer->pos();
   paintSprayer->setPos(QPointF(loc4.toPoint().x() + 4, loc4.toPoint().y()));
   client1.sendUDP(QString("r1"));
}

void Robot::rotateAboutAxis2()
{
   QPoint link2center = link2->axis.center();

   QPointF link2centerScene = link2->mapToScene(QPointF(link2center.x(),link2center.y()));

   QPointF link2inLink3 = link3->mapFromScene(link2centerScene);

   QTransform axis2Transform;
   axis2Transform.translate(link2center.x(), link2center.y());
   axis2Transform.rotate(5);
   axis2Transform.translate(-link2center.x(), -link2center.y());

   QTransform axis3Transform;
   axis3Transform.translate(link2inLink3.x(), link2inLink3.y());
   axis3Transform.rotate(5);
   axis3Transform.translate(-link2inLink3.x(), -link2inLink3.y());

   link2->setTransform(axis2Transform, true);
   link3->setTransform(axis3Transform, true);
   paintSprayer->setTransform(axis3Transform, true);
   client1.sendUDP(QString("r2"));
}

void Robot::rotateAboutAxis3()
{
   QPoint a = link3->axis.center();
   QTransform axis3Transform;
   axis3Transform.translate(a.x(), a.y());
   axis3Transform.rotate(5);
   axis3Transform.translate(-a.x(), -a.y());
   link3->setTransform(axis3Transform, true);
   paintSprayer->setTransform(axis3Transform, true);
   client1.sendUDP(QString("r3"));
}

void Robot::rotateCounterAboutAxis1()
{
   QPointF loc1 = link1->pos();
   link1->setPos(QPointF(loc1.toPoint().x() - 4, loc1.toPoint().y()));
   QPointF loc2 = link2->pos();
   link2->setPos(QPointF(loc2.toPoint().x() - 4, loc2.toPoint().y()));
   QPointF loc3 = link3->pos();
   link3->setPos(QPointF(loc3.toPoint().x() - 4, loc3.toPoint().y()));
   QPointF loc4 = paintSprayer->pos();
   paintSprayer->setPos(QPointF(loc4.toPoint().x() - 4, loc4.toPoint().y()));
   client1.sendUDP(QString("rc1"));
}

void Robot::rotateCounterAboutAxis2()
{
   QPoint link2center = link2->axis.center();

   QPointF link2centerScene = link2->mapToScene(QPointF(link2center.x(),link2center.y()));

   QPointF link2inLink3 = link3->mapFromScene(link2centerScene);

   QTransform axis2Transform;
   axis2Transform.translate(link2center.x(), link2center.y());
   axis2Transform.rotate(-5);
   axis2Transform.translate(-link2center.x(), -link2center.y());

   QTransform axis3Transform;
   axis3Transform.translate(link2inLink3.x(), link2inLink3.y());
   axis3Transform.rotate(-5);
   axis3Transform.translate(-link2inLink3.x(), -link2inLink3.y());

   link2->setTransform(axis2Transform, true);
   link3->setTransform(axis3Transform, true);
   paintSprayer->setTransform(axis3Transform, true);
   client1.sendUDP(QString("rc2"));
}

void Robot::rotateCounterAboutAxis3()
{
   QPoint a = link3->axis.center();
   QTransform axis3Transform;
   axis3Transform.translate(a.x(), a.y());
   axis3Transform.rotate(-5);
   axis3Transform.translate(-a.x(), -a.y());
   link3->setTransform(axis3Transform, true);
   paintSprayer->setTransform(axis3Transform, true);
   client1.sendUDP(QString("rc3"));
}
