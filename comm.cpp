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

void Comm::ispStart()
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

    try
    {
        hint(tr("Enter ISP mode and connect device...\n"));
        ispStart();
        unsigned char version;
        unsigned short pid;
        version = cmdGet();
        info(QString(tr("ISP loader version: %1.%2\n")).arg(version >> 4).arg(version & 0xf));
        pid = cmdGetID();
        info(QString(tr("PID: 0x%1\n")).arg(pid, 4, 16, QChar('0')));
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
    try
    {
        txReq(ISP_READ_MEMORY);
    }
    catch (ErrorProtocolNack)
    {
        throw ErrorProtocolReadProtection();
    }

    txAddr(addr);
    tx(QByteArray().append(static_cast<char>(size - 1)));

    buf = rx(size);
    if (static_cast<unsigned int>(buf.size()) < size)
        throw ErrorPortTimeout();

    return buf;
}

void Comm::cmdGo(unsigned int addr)
{
    txReq(ISP_GO);
    txAddr(addr);
    com->close();
}

void Comm::cmdWriteMemory(unsigned int addr, const QByteArray &data)
{
    try
    {
        txReq(ISP_WRITE_MEMORY);
    }
    catch (ErrorProtocolNack)
    {
        throw ErrorProtocolWriteProtection();
    }

    txAddr(addr);
    tx(QByteArray().append(static_cast<char>(data.size() - 1)).append(data));
}

void Comm::cmdEraseMemory(unsigned int page)
{
    try
    {
        txReq(ISP_ERASE_MEMORY);
    }
    catch (ErrorProtocolNack)
    {
        throw ErrorProtocolWriteProtection();
    }
    QByteArray buf;
    if (page != ISP_MASS_ERASE)
        buf.append(static_cast<char>(0x00));
    buf.append(static_cast<char>(page & 0xff));
    tx(buf);
    //device will reset
    if (page == ISP_MASS_ERASE)
        com->close();
}

void Comm::cmdEraseMemoryEx(unsigned int page)
{
    try
    {
        txReq(ISP_ERASE_MEMORY_EX);
    }
    catch (ErrorProtocolNack)
    {
        throw ErrorProtocolWriteProtection();
    }
    QByteArray buf;
    if (page != ISP_ERASE_BANK1 && page != ISP_ERASE_BANK2 && page != ISP_MASS_ERASE)
    {
        buf.append(static_cast<char>(0 >> 8));
        buf.append(static_cast<char>(0 & 0xff));
    }
    buf.append(static_cast<char>(page >> 8));
    buf.append(static_cast<char>(page & 0xff));
    tx(buf);
    //device will reset
    if (page == ISP_MASS_ERASE)
        com->close();
}

void Comm::cmdReadoutProtect()
{
    txReq(ISP_READOUT_PROTECT);
    rxAck();
    com->close();
}

void Comm::cmdReadoutUnProtect()
{
    txReq(ISP_READOUT_UNPROTECT);
    rxAck();
    com->close();
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
                        continue;
                    }
                    throw;
                }
            }
            if (i && ((i % REFRESH_RATE) == 0))
                info(".");
        }
        info(QObject::tr(".Ok!\n"));
        file.close();
    }
    catch (...)
    {
        info(QString(QObject::tr(".Fail! at 0x%1\n").arg(addr + i * PAGE_SIZE, 8, 16, QChar('0'))));
        file.close();
        throw;
    }
}

void Comm::erase(unsigned int addr, unsigned int size)
{
    unsigned int i;
    try
    {
        info(QString(QObject::tr("Erasing 0x%1-0x%2")).arg(addr, 8, 16, QChar('0')).arg(addr + size, 8, 16, QChar('0')));
        for (i = 0; i * PAGE_SIZE < size; ++i)
        {
            for (int retry = 0;; ++retry)
            {
                try
                {
                    if (supportedCmds.contains(ISP_ERASE_MEMORY_EX))
                        cmdEraseMemoryEx(((i * PAGE_SIZE + addr) - FLASH_BASE) / PAGE_SIZE);
                    else
                        cmdEraseMemory(((i * PAGE_SIZE + addr) - FLASH_BASE) / PAGE_SIZE);
                    break;
                }
                catch (...)
                {
                    if (retry < NRETRY)
                    {
                        info(QObject::tr("\n"));
                        warning(QString(QObject::tr("Retrain at: 0x%1")).arg(i * PAGE_SIZE + addr, 8, 16, QChar('0')));
                        continue;
                    }
                    throw;
                }
            }
            if (i && ((i % REFRESH_RATE) == 0))
                info(".");
        }
        info(QObject::tr(".Ok!\n"));
    }
    catch (...)
    {
        info(QString(QObject::tr(".Fail! at 0x%1\n").arg(addr + i * PAGE_SIZE, 8, 16, QChar('0'))));
        throw;
    }
}

void Comm::flash(const QByteArray &data, unsigned int addr, bool verify)
{
    unsigned int i;
    try
    {
        info(QString(QObject::tr("Flashing")));
        for (i = 0; i * PAGE_SIZE < static_cast<unsigned int>(data.size()); ++i)
        {
            QByteArray chunk(data.mid(i * PAGE_SIZE, PAGE_SIZE));
            if (static_cast<unsigned int>(chunk.size()) < PAGE_SIZE)
                chunk += QByteArray(PAGE_SIZE - chunk.size(), 0x0);
            for (int retry = 0;; ++retry)
            {
                try
                {
                    cmdWriteMemory(i * PAGE_SIZE + addr, chunk);
                    break;
                }
                catch (...)
                {
                    if (retry < NRETRY)
                    {
                        info(QObject::tr("\n"));
                        warning(QString(QObject::tr("Retrain at: 0x%1")).arg(i * PAGE_SIZE + addr, 8, 16, QChar('0')));
                        continue;
                    }
                    throw;
                }
            }
            if (verify)
            {
                for (int retry = 0;; ++retry)
                {
                    try
                    {
                        if (chunk != cmdReadMemory(i * PAGE_SIZE + addr, PAGE_SIZE))
                            throw ErrorProtocolVerify();
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
            }
            if (i && ((i % REFRESH_RATE) == 0))
                info(".");
        }
        info(QObject::tr(".Ok!\n"));
    }
    catch (...)
    {
        info(QString(QObject::tr(".Fail! at 0x%1\n").arg(addr + i * PAGE_SIZE, 8, 16, QChar('0'))));
        throw;
    }
}

void Comm::flash(const QString &fileName, unsigned int addr, bool verify)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
        throw ErrorFileOpen();
    QByteArray data(file.readAll());
    file.close();
    flash(data, addr, verify);
}

