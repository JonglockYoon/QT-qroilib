//
// Copyright (C) 2018
//
// interfaceport.cpp : 구현 파일입니다.
//
#include <QObject>
#include <QDebug>
#include <QTimerEvent>

#include "interfaceport.h"
#include "qextserialport.h"
#include "mainwindow.h"

CInterfacePort::CInterfacePort()
{
    m_pComSerial = nullptr;
    dScrewCurX = dScrewCurY = dScrewCurZ = 0.0;
}

CInterfacePort::~CInterfacePort()
{
    Close();
}

int CInterfacePort::Init(QString name)
{
    QString prefix = "";
#ifdef Q_OS_LINUX
    //prefix = "/dev/";
#endif

    QString strPort = prefix + name;
    if (strPort == "")
        return -1;
    PortSettings settings = {BAUD115200, DATA_8, PAR_NONE, STOP_1, FLOW_OFF, 10};
    m_timer.stop();
    m_pComSerial = new QextSerialPort(strPort, settings, QextSerialPort::Polling);
    m_timer.start(50, this);
    if (m_pComSerial->open(QIODevice::ReadWrite))
    {
        //connect(m_pComSerial,  SIGNAL(readyRead()),  this,   SLOT(onReadyRead()));
        qDebug() << "success port open";
    } else {
        QString str;
        str.sprintf("%s Port open failed", strPort.toStdString().c_str());
        qDebug() << str;
    }

    return 0;
}

void CInterfacePort::timerEvent(QTimerEvent *ev)
{
    if (ev->timerId() != m_timer.timerId()) {
      return;
    }

    // 50 ms마다 포트에서 들어오는 data를 읽는다.
    onReadyRead();

    vCheckSendCommandList();
}

void CInterfacePort::Close()
{
    if (m_pComSerial == nullptr) return;
    if (m_pComSerial->isOpen())
    {
        m_pComSerial->close();
    }
    delete m_pComSerial;
    m_pComSerial = nullptr;
}

void CInterfacePort::onReadyRead()
{
    if (m_pComSerial == nullptr)
        return;
    QByteArray bytes;
    int size = m_pComSerial->bytesAvailable();
    if (size <= 0)
        return;
    bytes.resize(size);
    m_pComSerial->read(bytes.data(), bytes.size());


    char *pBuff = bytes.data();
    qDebug() << "receive:" << QString(pBuff);

    static int state = 0;
    static int pos = 0;
    static int pos2 = 0;
    static char databuff[8192];
    static char csumbuff[64];
    for (int i = 0; i < size; i++)
    {
        switch (state) {
        case 0:
            if (pos == 0 && (pBuff[i] == 0x0d || pBuff[i] == 0x0a))
                break;
            databuff[pos] = pBuff[i];
            pos++;
            if (pos>2 && databuff[pos-2] == 0x04 && databuff[pos-1] == 0x04)
            {
                pos = pos2 = 0;
                memset(csumbuff, 0, sizeof(csumbuff));
                state = 1;
                break;
            }
            break;
        case 1:
            csumbuff[pos2] = pBuff[i];
            pos2++;
            if (pos2 > 16) {
                memset(databuff, 0, sizeof(databuff));
                pos = pos2 = 0;
                state = 0;
                break;
            }
            if (pos2>2 && csumbuff[pos2-2] == 0x04 && csumbuff[pos2-1] == 0x04)
            {
                unsigned long csum = 0;
                int len = strlen(databuff);
                for (int i=0; i<len; i++)
                    csum += databuff[i];
                qDebug() << "csum" << csum << atol(csumbuff);
                if (csum == atol(csumbuff)) {
                    DataProcess(len-2, databuff);
                }
                memset(databuff, 0, sizeof(databuff));
                pos = pos2 = 0;
                state = 0;
            }
            break;
        }
    }
}

//
// json 파싱을 해서 목적하는 작업을 실행하는 함수이다.
//
void CInterfacePort::DataProcess(int size, char *buffer)
{
    QString str = buffer;
    QByteArray jsonstr(str.toUtf8(), size);
    QJsonDocument doc = QJsonDocument::fromJson(jsonstr);
    QJsonObject a = doc.object();

    QJsonObject ts = a["time-stamp"].toObject();
    QString date = ts["date"].toString();
    QString time = ts["time"].toString();
    QString command = a["command"].toString();
    QString machine = a["machine"].toString();
    QString type = a["type"].toString();

    if (before_date == date && before_time == time &&
        before_type == type && before_machine == machine) // 같은 packet이 연달아 들어올경우 무시한다.
        return;

    before_date = date;
    before_time = time;
    before_type = type;
    before_machine = machine;
    int lr = 0;
    if (machine == "left")
        lr = 0;
    else
        lr = 1;

    if (type == "ack") { // ack 처리.
        vDeleteSendCommandList(command.toLatin1().data(), machine.toLatin1().data());
        return;
    }

    QString strLog = QString("RecvData: %1 %2").arg(command).arg(machine);
    theMainWindow->DevLogSave(strLog.toLocal8Bit().constData());

    if (command == "readscrewpos") // 티칭 data response.
    {
        MInfoTeachManager*	pInfo = theMainWindow->m_pInfoTeachManager;
        pInfo->m_vecScrewTable[lr].clear();
        pInfo->m_vecScrewTable[lr].resize(MAX_SCREW);
        QString strLog = QString("response %1 %2").arg(command).arg(machine);
        theMainWindow->DevLogSave(strLog.toLocal8Bit().constData());
        QJsonArray b = a["data"].toArray();
        int n = b.size();
        for (int i=0; i<n; i++) {
            QJsonObject c = b[i].toObject();

            int pt = c["point"].toInt();
            double x= c["x"].toDouble();
            double y= c["y"].toDouble();
            double z= c["z"].toDouble();

            if (pt >= 0 && pt < MAX_SCREW)
            {
                SCREWITEM *pitem = &pInfo->m_vecScrewTable[lr][pt];
                pitem->point = pt;
                pitem->x = x;
                pitem->y = y;
                pitem->z = z;
                pitem->dx = 0;
                pitem->dy = 0;

                QString str = QString("%1,%2,%3").arg(x).arg(y).arg(z);
                QString str1 = QString("%1,%2").arg(0).arg(0);
                int lr = 0;
                if (machine == "left")
                    lr = 0;
                else
                    lr = 1;
                theMainWindow->mPositionsDock->Add(lr, str, str1);
            }
        }
    }
    if (command == "position") { // 현재위치 broadcasting.
        QJsonObject b = a["data"].toObject();
        double x = b["x"].toDouble();
        double y = b["y"].toDouble();
        double z = b["z"].toDouble();
        qDebug() << "position" << x << y << z;

        if (theMainWindow->mTeachDock->m_bFeducialTeachSelect[0] )
            theMainWindow->mTeachDock->setFeducialVisionPos(x, y, z);
        else if (theMainWindow->mTeachDock->m_bFeducialTeachSelect[1])
            theMainWindow->mTeachDock->setFeducialScrewPos(x, y, z);
        else {
            double x1, y1, z1;
            int n = theMainWindow->mPositionsDock->GetSelectedRow(lr, x1, y1, z1);
            if (n >= 0) {
                theMainWindow->mPositionsDock->setSelectedRowValue(lr, x, y, z);
            }
        }
    }
    if (command == "align") { // align 요구.
        QJsonObject b = a["data"].toObject();
        int pt = b["point"].toInt() - 1;
        double x = b["x"].toDouble();
        double y = b["y"].toDouble();
        qDebug() << "align" << pt << x << y;

        SendAck(command, machine);

        theMainWindow->m_pTrsAlign[lr]->point = pt; // 현재위치한point
        //theMainWindow->m_pTrsAlign[lr]->dX = x;
        //theMainWindow->m_pTrsAlign[lr]->dY = y;
        theMainWindow->m_pTrsAlign[lr]->bDoAlign = true;
        theMainWindow->m_pTrsAlign[lr]->bAllTeachingMode = false;

    }
    if (command == "move") { // move complete.
        QJsonObject b = a["data"].toObject();
        double x = b["x"].toDouble();
        double y = b["y"].toDouble();
        double z = b["z"].toDouble();
        qDebug() << "move" << x << y << z;
        bMoveComplete[lr] = true;
        theMainWindow->mTeachDock->DisplayCurPointTextString();
    }
    if (command == "loadcsv") { // loadcsv response.
        QJsonObject b = a["data"].toObject();
        double x = b["x"].toDouble();
        double y = b["y"].toDouble();
        double z = b["z"].toDouble();
        qDebug() << "loadcsv" << x << y << z;
        dScrewCurX = x;
        dScrewCurY = y;
        dScrewCurZ = z;
    }
}

void CInterfacePort::WriteData(char *pBuff, int nLen)
{
    char temp[16];
    char buff[4096];
    QMutexLocker ml(&m_sync);

    if (!m_pComSerial)
        return;
    if (!m_pComSerial->isOpen())
        return;


    memset(buff, 0, sizeof(buff));
    strcpy(buff, pBuff);

    strcat(buff, "\x04");
    strcat(buff, "\x04");

    int csum = 0;
    int len = strlen(buff);
    for (int i=0; i<len; i++)
        csum += buff[i];
    sprintf(temp, "%d", csum);
    strcat(buff, temp);

    strcat(buff, "\x04");
    strcat(buff, "\x04");
    len = strlen(buff);

    m_pComSerial->write((const char *)buff, len);
}


void CInterfacePort::SendAck(QString command, QString machine)
{
    QJsonObject json;
    QJsonObject ts;

    QString date = QDateTime::currentDateTime().toString("yyyy-MM-dd");
    QString time = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");

    ts["date"] = QString(date);
    ts["time"] = QString(time);
    json["time-stamp"] = ts;
    json["type"] = QString("ack");
    json["command"] = QString(command);
    json["machine"] = QString(machine);

    //qDebug() << json;
    QJsonDocument doc(json);
    QString strJson(doc.toJson(QJsonDocument::Compact));

    vSendCommand((unsigned char*)strJson.toLatin1().data(), strJson.length(), 0, nullptr, nullptr); // 응답필요없음
}


//
// 슬리브 이동요청
//
void CInterfacePort::Move(QString machine, double x, double y, double z)
{
    QString command = "move";
    QJsonObject json;
    QJsonObject ts;
    QJsonObject data;

    QString date = QDateTime::currentDateTime().toString("yyyy-MM-dd");
    QString time = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");

    ts["date"] = QString(date);
    ts["time"] = QString(time);
    json["time-stamp"] = ts;
    json["type"] = QString("request");
    json["command"] = QString(command);
    json["machine"] = QString(machine);
    int lr = 0;
    if (machine == "left")
        lr = 0;
    else
        lr = 1;

    data["x"] = x;
    data["y"] = y;
    json["data"] = data;

    //qDebug() << json;
    QJsonDocument doc(json);
    QString strJson(doc.toJson(QJsonDocument::Compact));

    bMoveComplete[lr] = false;
    vSendCommand((unsigned char*)strJson.toLatin1().data(), strJson.length(), 1, command.toLatin1().data(), machine.toLatin1().data()); // ack 필요.
#ifdef SIMULATION
    QThread::msleep(200);
    bMoveComplete[lr] = true;
#endif
}

//
// Vision에서 티칭한 결과를 체결기로 보낸다.
//
void CInterfacePort::WriteScrewPos(QString machine)
{
    QString command = "writescrewpos";
    QJsonObject json;
    QJsonObject ts;
    QJsonObject data;

    QString date = QDateTime::currentDateTime().toString("yyyy-MM-dd");
    QString time = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");

    ts["date"] = QString(date);
    ts["time"] = QString(time);
    json["time-stamp"] = ts;
    json["type"] = QString("request");
    json["command"] = QString(command);
    json["machine"] = QString(machine);

    QJsonArray array;
    int lr = 0;
    if (machine == "left")
        lr = 0;
    else
        lr = 1;
    MInfoTeachManager*	pInfo = theMainWindow->m_pInfoTeachManager;
    int size = pInfo->m_vecScrewTable[lr].size();
    for (int j = 0; j < size; j++)
    {
        SCREWITEM *pitem = &pInfo->m_vecScrewTable[lr][j];
        data["point"] = pitem->point;
        data["x"] = pitem->x;
        data["y"] = pitem->y;
        data["z"] = pitem->z;
        array.push_back(data);
    }
    json["data"] = array;

    QJsonDocument doc(json);
    QString strJson(doc.toJson(QJsonDocument::Compact));
    vSendCommand((unsigned char*)strJson.toLatin1().data(), strJson.length(), 1, command.toLatin1().data(), machine.toLatin1().data()); // ack 필요.
}

//
// 티칭Table요청
//
void CInterfacePort::ReadScrewPos(QString machine)
{
    QString command = "readscrewpos";
    QJsonObject json;
    QJsonObject ts;

    QString date = QDateTime::currentDateTime().toString("yyyy-MM-dd");
    QString time = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");

    ts["date"] = QString(date);
    ts["time"] = QString(time);
    json["time-stamp"] = ts;
    json["type"] = QString("request");
    json["command"] = QString(command);
    json["machine"] = QString(machine);

    QJsonDocument doc(json);
    QString strJson(doc.toJson(QJsonDocument::Compact));

    vSendCommand((unsigned char*)strJson.toLatin1().data(), strJson.length(), 1, command.toLatin1().data(), machine.toLatin1().data()); // ack 필요.
}

//
// "aling"요청의 결과
//
void CInterfacePort::ResponseAlignScrewPos(QString machine)
{
    QString command = "align";
    QJsonObject json;
    QJsonObject ts;
    QJsonObject data;

    QString date = QDateTime::currentDateTime().toString("yyyy-MM-dd");
    QString time = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");

    ts["date"] = QString(date);
    ts["time"] = QString(time);
    json["time-stamp"] = ts;
    json["type"] = QString("response");
    json["command"] = QString(command);
    json["machine"] = QString(machine);

    QJsonArray array;
    int lr = 0;
    if (machine == "left")
        lr = 0;
    else
        lr = 1;
    MInfoTeachManager*	pInfo = theMainWindow->m_pInfoTeachManager;
    int size = pInfo->m_vecScrewTable[lr].size();
    for (int j = 0; j < size; j++)
    {
        SCREWITEM *pitem = &pInfo->m_vecScrewTable[lr][j];
        data["point"] = pitem->point;
        data["x"] = pitem->x + pitem->dx;
        data["y"] = pitem->y + pitem->dy;
        data["z"] = pitem->z;
        array.push_back(data);
    }
    json["data"] = array;

    QJsonDocument doc(json);
    QString strJson(doc.toJson(QJsonDocument::Compact));
    vSendCommand((unsigned char*)strJson.toLatin1().data(), strJson.length(), 1, command.toLatin1().data(), machine.toLatin1().data()); // ack 필요.
}

// LoadCSV실행중이라는것을 체결기로 알린다.
// 체결기는 티칭 모드로 전환 되면서 현재 위치를 broadcasting 한다.
void CInterfacePort::LoadCSV(QString machine)
{
    QString command = "loadcsv";
    QJsonObject json;
    QJsonObject ts;

    QString date = QDateTime::currentDateTime().toString("yyyy-MM-dd");
    QString time = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");

    ts["date"] = QString(date);
    ts["time"] = QString(time);
    json["time-stamp"] = ts;
    json["type"] = QString("request");
    json["command"] = QString(command);
    json["machine"] = QString(machine);

    QJsonDocument doc(json);
    QString strJson(doc.toJson(QJsonDocument::Compact));

    vSendCommand((unsigned char*)strJson.toLatin1().data(), strJson.length(), 1, command.toLatin1().data(), machine.toLatin1().data()); // ack 필요.
}


//
// 송신파트
//

//
// 전송할 문자열을 sendCommandList에 담는다.
// vCheckSendCommandList() 함수에서 전송된다.
// reply가 0가 아니면 ack신호를 받아야한다.
//
int CInterfacePort::vSendCommand(unsigned char *pData, int nLen, short reply, char *command, char *machine)
{
    WriteData((char*)pData, nLen);

    QString strLog = QString("WriteData: %1 %2").arg(command).arg(machine);
    theMainWindow->DevLogSave(strLog.toLocal8Bit().constData());

    QDateTime time = QDateTime::currentDateTime();
    qint64 t = time.currentMSecsSinceEpoch();

    if (reply != 0) // ack를 기다리는 packet이면.
    {
        SENDDATA *pSendDataQue = (SENDDATA *)malloc(sizeof(SENDDATA));
        if (pSendDataQue)
            memset(pSendDataQue, 0, sizeof(SENDDATA));
        if (pSendDataQue)
        {
            pSendDataQue->pSendData = (char *)malloc(nLen+1);
            if (pSendDataQue->pSendData)
            {
                memcpy(pSendDataQue->pSendData, pData, nLen);
                pSendDataQue->pSendData[nLen] = 0x0;
                pSendDataQue->len = nLen;
                pSendDataQue->reply = reply;
                if (command)
                    strcpy(pSendDataQue->command, command);
                if (machine)
                    strcpy(pSendDataQue->machine, machine);
                pSendDataQue->sendTime = t;
                pSendDataQue->retry = 0;
                sendCommandList.append(pSendDataQue);
            } else {
                free(pSendDataQue);
            }
        }
        else {
            return -1;
        }
    }


    return 0;
}

//
// sendCommandList List에 내용이 있으면 꺼집어내어 상태측으로 전송한다.
// 수시로 호출될수 있다.
//
int CInterfacePort::vCheckSendCommandList()
{
    QDateTime time = QDateTime::currentDateTime();
    qint64 t = time.currentMSecsSinceEpoch();

    foreach(SENDDATA* pElement, sendCommandList)
    {
        if (pElement == NULL) break;
        if ((t - pElement->sendTime) > MAKETIME(1.5)) // 1.5초를 기다려도 응답이 없으면 재전송.
        {
            if (pElement->retry < 2) // 최대 3번 보낸다.
            {
                pElement->retry++;
                pElement->sendTime = t;
                qDebug() << ("vCheckSendCommandList\n");
                WriteData((char*)pElement->pSendData, pElement->len);
            } else { // 전송대기열에서 지움.
                if (pElement->pSendData) free(pElement->pSendData);
                free(pElement);
                sendCommandList.removeOne(pElement);
            }
            break; // 동일 loop에서는 한개 packet만 처리.
        }
    }

    return 0;
}

int CInterfacePort::vDeleteSendCommandList(char *command, char *machine)
{
    foreach(SENDDATA* pElement, sendCommandList)
    {
        if (pElement == NULL) break;

        if (strcmp(pElement->command, command) == 0 && strcmp(pElement->machine, machine) == 0)
        {
            if (pElement->pSendData) free(pElement->pSendData);
            free(pElement);
            sendCommandList.removeOne(pElement);
            qDebug() << "vDeleteSendCommandList" << command;
            break;
        }
    }
    return 0;
}



