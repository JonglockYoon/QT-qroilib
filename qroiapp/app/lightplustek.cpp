// lightplustek.cpp : 구현 파일입니다.
//
#include <QObject>
#include <QDebug>
#include <QTimerEvent>
#include <QThread>
#include "lightplustek.h"
#include "qextserialport.h"

CLightPlusTek::CLightPlusTek()
{
    m_pComSerial = NULL;
}

CLightPlusTek::~CLightPlusTek()
{
    Close();
}

int CLightPlusTek::Init(QString name)
{
    QString prefix = "";
#ifdef Q_OS_LINUX
    prefix = "/dev/";
#endif

    QString strPort = prefix + name;
    if (strPort == "")
        return -1;
    PortSettings settings = {BAUD38400, DATA_8, PAR_NONE, STOP_1, FLOW_OFF, 10};
#ifdef Q_OS_LINUX
    m_timer.stop();
    m_pComSerial = new QextSerialPort(strPort, settings, QextSerialPort::Polling);
    m_timer.start(50, this);
#else
    m_pComSerial = new QextSerialPort(strPort, settings, QextSerialPort::EventDriven);
#endif
    if (m_pComSerial->open(QIODevice::ReadWrite))
    {
        connect(m_pComSerial,  SIGNAL(readyRead()),  this,   SLOT(onReadyRead()));
        qDebug() << "success port open";
    } else {
        QString str;
        str.sprintf("%s Port open failed", strPort.toStdString().c_str());
        //qDebug() << str;//MSystem->DevLogSave(str.toLatin1().data());
    }

    SetOnOff(0x0);
    SetOnOff(0xFFFFFFFF);

    return 0;
}


void CLightPlusTek::timerEvent(QTimerEvent *ev)
{
    if (ev->timerId() != m_timer.timerId()) {
      return;
    }

    // 50 ms마다 포트에서 들어오는 data를 읽는다.
    onReadyRead();
}

void CLightPlusTek::Close()
{
    if (m_pComSerial == nullptr) return;
    if (m_pComSerial->isOpen())
    {
        m_pComSerial->close();
    }
    delete m_pComSerial;
    m_pComSerial = nullptr;
}

void CLightPlusTek::ControlBrightness(int nCh, int nVal)
{
    SetBrightness(nCh, nVal);
    QThread::msleep(10);
}

void CLightPlusTek::onReadyRead()
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

}


void CLightPlusTek::SetBrightness(int nCh, int nVal)
{
    if (m_pComSerial == NULL)
        return;
    if (!m_pComSerial->isOpen()) return;

    char buff[256];
    sprintf(buff, "setbrightness %d %d\x0d", nCh, nVal);
    int len = (int)strlen(buff);
    int ret = m_pComSerial->write(buff, len);

    QThread::msleep(20);
}

void CLightPlusTek::SetOnOff(int dwCh) // dwCh = 0xFFFFFFFF(모든 Ch On)
{
    if (m_pComSerial == NULL)
        return;
    if (!m_pComSerial->isOpen()) return;

    char buff[256];
    sprintf(buff, "setonex %x\x0d", dwCh);
    int len = (int)strlen(buff);
    int ret = m_pComSerial->write(buff, len);

    QThread::msleep(20);
}

