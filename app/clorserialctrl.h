#ifndef CLORSERIALCTRL_H
#define CLORSERIALCTRL_H

#include <QObject>
#include <QtSerialPort/QSerialPort>
#include "clightsequence.h"
#include <QTimer>

class CLORSerialCtrl : public QObject
{
    Q_OBJECT
public:
    explicit CLORSerialCtrl( QObject* parent = nullptr );
    ~CLORSerialCtrl();

    bool setPortParams( const QString& name, qint32 baudRate );

    bool isOpen() const;

public slots:
    void playStarted( std::weak_ptr<CLightSequence> currentSequense );

private:

    void writeHeartbeatData();
    void setIntensity( const Channel& channel, double intensity );

private:

    QSerialPort m_serial;

    std::list<std::shared_ptr<QMetaObject::Connection>> m_sequenseConncetion;

    QTimer* m_timer;


};

#endif // CLORSERIALCTRL_H
