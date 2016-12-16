#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets>
#include "robot.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    Robot *robot;
    QCheckBox* delay;
signals:

public slots:

private slots:
    void setJointModeControl();
    void setWorldModeControl();

private:
    QPushButton* paintButton;

    // Joint mode control buttons
    QPushButton* arm1counterButton;
    QPushButton* arm1clockButton;
    QPushButton* arm2counterButton;
    QPushButton* arm2clockButton;
    QPushButton* arm3counterButton;
    QPushButton* arm3clockButton;

    // World mode control buttons
    QPushButton* upButton;
    QPushButton* leftButton;
    QPushButton* rightButton;
    QPushButton* downButton;

    // server buttons
    QRadioButton* server;
    QRadioButton* client;



    // Mode control change buttons
    QPushButton* jointModeControlButton;
    QPushButton* worldModeControlButton;

    QLabel* arm1MovementLabel;
    QLabel* arm2MovementLabel;
    QLabel* arm3MovementLabel;

    QGraphicsView* view2;

    QGridLayout* layout;

};

#endif // MAINWINDOW_H
