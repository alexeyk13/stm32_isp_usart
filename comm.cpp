/*
    USB DFU Flasher PC part (cross-platform)
    Copyright (c) 2014, Alexey Kramarenko
    All rights reserved.
*/

#include "comm.h"
#include "config.h"
#include "error.h"
#include "proto.h"
#include <QFile>
#include <QtSerialPort/QSerialPortInfo>
#include <QtSerialPort/QSerialPort>
#include <QCoreApplication>
#include "delay.h"

Comm::Comm(QObject *parent) :
    QObject(parent)
{
    com = new QSerialPort();
}

Comm::~Comm()
{
    delete com;
}

QByteArray Comm::rx(unsigned int maxSize)
{
    QByteArray buf;
    while (static_cast<unsigned int>(buf.size()) < maxSize && com->waitForReadyRead(PORT_DEFAULT_TIMEOUT))
        buf.append(com->read(maxSize - buf.size()));
    if (buf.isEmpty())
        throw ErrorPortTimeout();
    return buf;
}

unsigned char Comm::rxChar()
{

    if (com->waitForReadyRead(PORT_DEFAULT_TIMEOUT))
    {
        unsigned char c;
        if (com->getChar(reinterpret_cast<char*>(&c)))
            return c;
    }
    throw ErrorPortTimeout();
}

void Comm::rxAck()
{
    unsigned char c = rxChar();
    if (c == ISP_NACK)
        throw ErrorProtocolNack();
    if (c != ISP_ACK)
        throw ErrorProtocolInvalidResponse();
}

void Comm::tx(const QByteArray &buf)
{
    char crc = buf.size() > 1 ? 0x00 : 0xff;
    foreach (char c, buf)
        crc ^= c;
    com->flush();
    com->write(buf);
    com->putChar(crc);
    rxAck();
}

void Comm::txAck()
{
    com->putChar(ISP_ACK);
}

void Comm::txReq(unsigned char cmd)
{
    QByteArray buf;
    buf.append(static_cast<char>(cmd));
    tx(buf);
}

void Comm::txAddr(unsigned int addr)
{
    QByteArray buf;
    buf.append(static_cast<char>((addr >> 24) & 0xff));
    buf.append(static_cast<char>((addr >> 16) & 0xff));
    buf.append(static_cast<char>((addr >> 8) & 0xff));
    buf.append(static_cast<char>((addr >> 0) & 0xff));
    tx(buf);
}

bool Comm::isActive()
{
    return com->isOpen();
}

void Comm::open(const QString &name, unsigned int speed)
{
    com->close();
    com->setPortName(name);
    if (!com->open(QIODevice::ReadWrite))
        throw ErrorPortOpen();
    com->setBaudRate(speed);
    com->setDataBits(QSerialPort::Data8);
    com->setStopBits(QSerialPort::OneStop);
    com->setParity(QSerialPort::EvenParity);
    com->setFlowControl(QSerialPort::NoFlowControl);

    hint(tr("Enter ISP mode and connect device...\n"));
    try
    {
        for (int i = 0; i < ACK_TIMEOUT_COUNT; ++i)
        {
            com->putChar(ISP_START_FRAME);
            if (com->waitForReadyRead(10))
            {
                char c;
                while (com->getChar(&c))
                {
                    if (c == ISP_ACK)
                    {
                        info(QObject::tr("Device ACK\n"));
                        return;
                    }
                    if (c == ISP_NACK)
                    {
                        info(QObject::tr("Device already connected\n"));
                        return;
                    }
                }
            }
            QCoreApplication::processEvents();
        }
        throw ErrorPortTimeout();
    }
    catch (...)
    {
        com->close();
        throw;
    }
}

void Comm::close()
{
    com->close();
}

QStringList Comm::ports()
{
    QStringList res;
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
        res << info.portName();
    return res;
}

unsigned char Comm::cmdGet()
{
    if (!com->isOpen())
        throw ErrorNotActive();

    txReq(ISP_GET);
    int len = rxChar();
    unsigned char version = rxChar();
    supportedCmds.clear();
    for (int i = 0; i < len; ++i)
        supportedCmds.append(rxChar());
    rxAck();

    return version;
}

unsigned char Comm::cmdGetVersion()
{
    if (!com->isOpen())
        throw ErrorNotActive();

    txReq(ISP_GET_VERSION);
    unsigned char version = rxChar();
    rxChar();
    rxChar();
    rxAck();

    return version;
}

unsigned short Comm::cmdGetID()
{
    unsigned short pid;
    if (!com->isOpen())
        throw ErrorNotActive();

    txReq(ISP_GET_ID);
    //len
    rxChar();
    pid = rxChar() << 8;
    pid |= rxChar();
    rxAck();

    return pid;
}

QByteArray Comm::cmdReadMemory(unsigned int addr, unsigned int size)
{
    QByteArray buf;
    txReq(ISP_READ_MEMORY);
    txAddr(addr);
    tx(QByteArray().append(static_cast<char>(size - 1)));

    buf = rx(size);
    if (static_cast<unsigned int>(buf.size()) < size)
        throw ErrorPortTimeout();

    return buf;
}

void Comm::dump(const QString &fileName, unsigned int addr, unsigned int size)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly))
        throw ErrorFileOpen();
    unsigned int i;
    try
    {
        info(QString(QObject::tr("Dumping 0x%1-0x%2")).arg(addr, 8, 16, QChar('0')).arg(addr + size, 8, 16, QChar('0')));
        for (i = 0; i * PAGE_SIZE < size; ++i)
        {
            for (int retry = 0;; ++retry)
            {
                try
                {
                    file.write(cmdReadMemory(i * PAGE_SIZE + addr, PAGE_SIZE));
                    break;
                }
                catch (...)
                {
                    if (retry < NRETRY)
                    {
                        info(QObject::tr("\n"));
                        warning(QString(QObject::tr("Retrain at: 0x%1")).arg(i * PAGE_SIZE + addr, 8, 16, QChar('0')));
                    }
                }
            }
            if (i && ((i % REFRESH_RATE) == 0))
                info(".");
        }
        info(QObject::tr("Ok!\n"));
        file.close();
    }
    catch (...)
    {
        info(QString(QObject::tr("\nFail! at 0x%1\n").arg(addr + i * PAGE_SIZE, 8, 16, QChar('0'))));
        file.close();
        throw;
    }
}

