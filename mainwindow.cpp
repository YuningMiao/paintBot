#include "mainwindow.h"
#include <QtWidgets>
#include <math.h>
#include "main.cpp"


class GraphicsView : public QGraphicsView
{
public:
    GraphicsView(QGraphicsScene *scene) : QGraphicsView(scene)
    {
    }

protected:
    virtual void resizeEvent(QResizeEvent *) Q_DECL_OVERRIDE
    {
    }
};


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{

    QGraphicsScene *scene2 = new QGraphicsScene(0,0,1000,800);
    QGraphicsScene *scene = new QGraphicsScene(-340,-260,700, 500);

    robot = new Robot;
    scene->addItem(robot);

    QGraphicsView *view = new QGraphicsView(scene);
    view2 = new QGraphicsView(scene2);
    view->setRenderHint(QPainter::Antialiasing);
    view->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);

    layout = new QGridLayout();

    // Joint Layout Buttons
    arm1counterButton = new QPushButton("Left", view2);
    arm1clockButton = new QPushButton("Right", view2);
    arm2counterButton = new QPushButton("Counterclockwise", view2);
    arm2clockButton = new QPushButton("Clockwise", view2);
    arm3counterButton = new QPushButton("Counterclockwise", view2);
    arm3clockButton = new QPushButton("Clockwise", view2);

    arm1MovementLabel = new QLabel("Arm 1 Movement:");
    arm2MovementLabel = new QLabel("Arm 2 Movement:");
    arm3MovementLabel = new QLabel("Arm 3 Movement:");

    //server buttons
    server = new QRadioButton("server", this);
    client = new QRadioButton("client", this);
    delay = new QCheckBox("Delay", this);


    // World Layout Buttons
    upButton = new QPushButton("Up", view2);
    leftButton = new QPushButton("Left", view2);
    rightButton = new QPushButton("Right", view2);
    downButton = new QPushButton("Down", view2);
    upButton->setVisible(false);
    leftButton->setVisible(false);
    rightButton->setVisible(false);
    downButton->setVisible(false);

    paintButton = new QPushButton("Paint!", view2);
    jointModeControlButton = new QPushButton("Joint\nmode\nControl", view2);
    worldModeControlButton = new QPushButton("World\nmode\nControl", view2);
    jointModeControlButton->setMinimumHeight(300);
    worldModeControlButton->setMinimumHeight(300);

    QSignalMapper* signalMapper = new QSignalMapper (robot) ;

    // Connect Joint mode Control buttons
    GraphicsView::connect(arm1counterButton, SIGNAL(pressed()),robot, SLOT(rotateCounterAboutAxis1()));
    GraphicsView::connect(arm1clockButton, SIGNAL(pressed()),robot, SLOT(rotateAboutAxis1()));
    GraphicsView::connect(arm2counterButton, SIGNAL(pressed()),robot, SLOT(rotateCounterAboutAxis2()));
    GraphicsView::connect(arm2clockButton, SIGNAL(pressed()),robot, SLOT(rotateAboutAxis2()));
    GraphicsView::connect(arm3counterButton, SIGNAL(pressed()),robot, SLOT(rotateCounterAboutAxis3()));
    GraphicsView::connect(arm3clockButton, SIGNAL(pressed()),robot, SLOT(rotateAboutAxis3()));
    GraphicsView::connect(paintButton, SIGNAL(pressed()),robot, SLOT(paintCircle()));

    // Connect World mode Control buttons and keys
    GraphicsView::connect(upButton, SIGNAL(pressed()),robot, SLOT(moveUp()));
    GraphicsView::connect(leftButton, SIGNAL(pressed()),robot, SLOT(rotateCounterAboutAxis1()));
    GraphicsView::connect(rightButton, SIGNAL(pressed()),robot, SLOT(rotateAboutAxis1()));
    GraphicsView::connect(downButton, SIGNAL(pressed()),robot, SLOT(moveDown()));
    QShortcut *shortcutUp = new QShortcut(Qt::Key_Up, this);
    QObject::connect(shortcutUp, SIGNAL(activated()), robot, SLOT(moveUp()));
    QShortcut *shortcutLeft = new QShortcut(Qt::Key_Left, this);
    QObject::connect(shortcutLeft, SIGNAL(activated()), robot, SLOT(rotateCounterAboutAxis1()));
    QShortcut *shortcutRight = new QShortcut(Qt::Key_Right, this);
    QObject::connect(shortcutRight, SIGNAL(activated()), robot, SLOT(rotateAboutAxis1()));
    QShortcut *shortcutDown = new QShortcut(Qt::Key_Down, this);
    QObject::connect(shortcutDown, SIGNAL(activated()), robot, SLOT(moveDown()));

    // Connect controller buttons
    GraphicsView::connect(jointModeControlButton, SIGNAL(pressed()), this, SLOT(setJointModeControl()));
    GraphicsView::connect(worldModeControlButton, SIGNAL(pressed()), this, SLOT(setWorldModeControl()));

    // Adds all widgets to Layout. Some will not be visible, depending on the mode control.
    layout->setColumnStretch(0, 1);
    layout->setColumnStretch(4, 1);
    layout->addWidget(view,0,0,2,5);

    // Joint Layout buttons
    layout->addWidget(arm1MovementLabel, 2, 2, Qt::AlignRight);
    layout->addWidget(arm1counterButton, 2, 3,1,1,  Qt::AlignRight);
    layout->addWidget(arm1clockButton, 2, 4,1,1,  Qt::AlignLeft);
    layout->addWidget(arm2MovementLabel, 3, 2,  Qt::AlignRight);
    layout->addWidget(arm2counterButton, 3, 3, 1,1, Qt::AlignRight);
    layout->addWidget(arm2clockButton, 3, 4,1,1, Qt::AlignLeft);
    layout->addWidget(arm3MovementLabel, 4, 2, Qt::AlignRight);
    layout->addWidget(arm3clockButton, 4, 4,1,1, Qt::AlignLeft);
    layout->addWidget(arm3counterButton, 4, 3,1,1, Qt::AlignRight);

    //server client Layout buttons
    layout->addWidget(server, 3,5,1,1, Qt::AlignRight);
    layout->addWidget(client, 3,4,1,1, Qt::AlignRight);
    layout->addWidget(delay, 4,5,1,1, Qt::AlignRight);



    // World Layout buttons
    layout->addWidget(upButton, 2, 3,1,1, Qt::AlignLeft);
    layout->addWidget(leftButton, 3, 2,1,1, Qt::AlignRight);
    layout->addWidget(rightButton, 3, 4,1,1, Qt::AlignLeft);
    layout->addWidget(downButton, 3, 3,1,1, Qt::AlignRight);

    layout->addWidget(paintButton,3,0,1,1,Qt::AlignCenter);
    layout->addWidget(jointModeControlButton, 0, 5, Qt::AlignTop);
    layout->addWidget(worldModeControlButton, 1, 5, Qt::AlignTop);

    upButton->setShortcut(Qt::Key_W);
        downButton->setShortcut(Qt::Key_S);
        leftButton->setShortcut(Qt::Key_A);
        rightButton->setShortcut(Qt::Key_D);
        paintButton->setShortcut(Qt::Key_P);

        paintButton->setAutoRepeat(true);
        downButton->setAutoRepeat(true);
        upButton->setAutoRepeat(true);
        leftButton->setAutoRepeat(true);
        rightButton->setAutoRepeat(true);
    view2->setBackgroundBrush(QColor(220, 220, 220));
    view2->setWindowTitle("TAMU Paint Bot");
    view2->setLayout(layout);
    view2->show();
}

// Set control to Joint mode
void MainWindow::setJointModeControl() {
    arm1counterButton->setVisible(true);
    arm1clockButton->setVisible(true);
    arm2counterButton->setVisible(true);
    arm2clockButton->setVisible(true);
    arm3counterButton->setVisible(true);
    arm3clockButton->setVisible(true);
    arm1MovementLabel->setVisible(true);
    arm2MovementLabel->setVisible(true);
    arm3MovementLabel->setVisible(true);
    upButton->setVisible(false);
    leftButton->setVisible(false);
    rightButton->setVisible(false);
    downButton->setVisible(false);
}

// Set control to World mode
void MainWindow::setWorldModeControl() {
    arm1counterButton->setVisible(false);
    arm1clockButton->setVisible(false);
    arm2counterButton->setVisible(false);
    arm2clockButton->setVisible(false);
    arm3counterButton->setVisible(false);
    arm3clockButton->setVisible(false);
    arm1MovementLabel->setVisible(false);
    arm2MovementLabel->setVisible(false);
    arm3MovementLabel->setVisible(false);
    upButton->setVisible(true);
    leftButton->setVisible(true);
    rightButton->setVisible(true);
    downButton->setVisible(true);
}
