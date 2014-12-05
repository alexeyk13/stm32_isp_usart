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

void Comm::req(unsigned char cmd)
{
    QByteArray buf(sizeof(REQ), 0);
    REQ* req = reinterpret_cast<REQ*>(buf.data());
    req->cmd = cmd;
    req->crc = cmd ^ 0xff;
    com->flush();
    com->write(buf);
    rxAck();
}

bool Comm::isActive()
{
    return com->isOpen();
}

void Comm::open(const QString &name)
{
    com->close();
    com->setPortName(name);
    if (!com->open(QIODevice::ReadWrite))
        throw ErrorPortOpen();
    com->setBaudRate(PORT_BAUD);
    com->setDataBits(QSerialPort::Data8);
    com->setStopBits(QSerialPort::OneStop);
    com->setParity(QSerialPort::NoParity);
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

    req(ISP_GET);
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

    req(ISP_GET_VERSION);
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

    req(ISP_GET_ID);
    //len
    rxChar();
    pid = rxChar() << 8;
    pid |= rxChar();
    rxAck();

    return pid;
}

