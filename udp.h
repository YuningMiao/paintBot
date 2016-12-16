#ifndef MYUDP_H
#define MYUDP_H

#include <QObject>
#include <QUdpSocket>
#include <QHostAddress>
#include <QString>

class MyUDP : public QObject
{
    Q_OBJECT
public:
    explicit MyUDP(QObject *parent = 0);
    void HelloUDP();
    void sendUDP(QString ins);
    quint64 value;
    QHostAddress addr;
signals:

public slots:
    void readyRead();

private:
    QUdpSocket *socket;
    QString address = "127.0.0.1";


};

namespace MyUDP {
    class robot: public MyUDP{};

}

#endif // MYUDP_H
