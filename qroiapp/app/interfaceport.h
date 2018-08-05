#pragma once

#include <string>
#include <QObject>
#include <QList>
#include <QBasicTimer>
#include <QTime>
#include <QMutex>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

#include "qextserialport.h"

#define MAKETIME(T) ((unsigned int)((T) * 1000))

typedef struct _tagSendData {
    unsigned long len;
    char *pSendData;
    char command[32];
    char machine[16];
    short reply;
    int retry;
    qint64 sendTime;
} SENDDATA;

using namespace std;

//class ROIDSHARED_EXPORT QextSerialPort;
class CInterfacePort : public QObject
{
    Q_OBJECT

public:
    CInterfacePort();
    virtual ~CInterfacePort();

    int Init(QString name);
	void Close();

    void WriteData(char *pBuff, int len);
public slots:
    void onReadyRead();

public:
    void DataProcess(int size, char *buffer);

public:
    QextSerialPort* m_pComSerial;
    bool	m_bReply;
    double dScrewCurX;
    double dScrewCurY;
    double dScrewCurZ;
    bool bMoveComplete[2];

public:
    QBasicTimer m_timer;
    void timerEvent(QTimerEvent *ev);

    int vCheckSendCommandList();
    int vDeleteSendCommandList(char *command, char *machine);
    int vSendCommand(unsigned char *pData, int nLen, short reply, char *command, char *machine);
    void vSendData(char *pbuff, int nLen);

    void SendAck(QString command, QString machine);
    void Move(QString machine, double x, double y, double z);
    void WriteScrewPos(QString machine);
    void ReadScrewPos(QString machine);
    void ResponseAlignScrewPos(QString machine);
    void LoadCSV(QString machine);

private:
    QMutex	m_sync;
    QList<SENDDATA*> sendCommandList;
    QString before_date;
    QString before_time;
    QString before_type;
    QString before_machine;

};
