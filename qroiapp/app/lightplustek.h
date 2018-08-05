#pragma once

#include <string>
#include <QObject>
#include <QBasicTimer>
#include <QTime>
#include "qextserialport.h"

using namespace std;

//class ROIDSHARED_EXPORT QextSerialPort;
class CLightPlusTek  : public QObject
{
public:
    CLightPlusTek();
    virtual ~CLightPlusTek();

    int Init(QString name);
    void Close();

public slots:
    void onReadyRead();

public:
    void DataProcess(int size, char *buffer);
public:
    QextSerialPort* m_pComSerial;

public:
    bool	m_bReply;

public:
    void SendSave();
    QBasicTimer m_timer;
    void timerEvent(QTimerEvent *ev);

public:
    void ControlBrightness(int nCh, int nVal);

    void OnDataArrival(int nComIndex, int size, char *buffer);
    void SetBrightness(int nCh, int nVal);
    void SetOnOff(int dwCh);// dwCh = 0xFFFFFFFF(모든 Ch On)
};
