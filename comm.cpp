/*
    USB DFU Flasher PC part (cross-platform)
    Copyright (c) 2014, Alexey Kramarenko
    All rights reserved.
*/

#include "comm.h"
#include "usb/dfud.h"
#include "config.h"
#include "error.h"
#include "proto.h"
#include <QFile>

Comm::Comm(QObject *parent) :
    QObject(parent)
{
    dfud = new DFUD();
}

Comm::~Comm()
{
    delete dfud;
}

void Comm::cmdReq(unsigned char cmd, unsigned int param1, unsigned int param2, QByteArray data)
{
    QByteArray buf(sizeof(PROTO_REQ), 0);
    PROTO_REQ* proto = reinterpret_cast<PROTO_REQ*>(buf.data());
    proto->cmd = cmd;
    proto->data_size = data.size();
    proto->param1 = param1;
    proto->param2 = param2;
    dfud->write(buf + data);
}

bool Comm::isActive()
{
    return dfud->isActive();
}

bool Comm::open()
{
    if (!dfud->open(VID, PID))
        return false;
    return true;
}

void Comm::close()
{
    dfud->close();
}

void Comm::cmdVersion(int& loader, int& protocol)
{
    cmdReq(PROTO_CMD_VERSION, 0, 0);
    QByteArray buf(dfud->read());
    if (static_cast<unsigned int>(buf.size()) < sizeof(PROTO_VERSION_RESP))
        throw ErrorProtocolInvalidResponse();
    PROTO_VERSION_RESP* version = reinterpret_cast<PROTO_VERSION_RESP*>(buf.data());
    loader = version->loader;
    protocol = version->protocol;
}

void Comm::cmdLeave()
{
    cmdReq(PROTO_CMD_LEAVE, 0, 0);
}

QByteArray Comm::cmdRead(unsigned int addr, unsigned int size)
{
    cmdReq(PROTO_CMD_READ, addr, size);
    QByteArray buf(dfud->read());
    if (static_cast<unsigned int>(buf.size()) < size)
        throw ErrorProtocolInvalidResponse();
    return buf;
}

void Comm::cmdWrite(unsigned int addr, const QByteArray &buf)
{
    cmdReq(PROTO_CMD_WRITE, addr, buf.size(), buf);
}

void Comm::cmdErase(unsigned int addr, unsigned int size)
{
    cmdReq(PROTO_CMD_ERASE, addr, size);
}

void Comm::flash(const QString &fileName, unsigned int addr, unsigned int size)
{
    unsigned int i;
    QFile fwFile(fileName);
    if (!fwFile.open(QIODevice::ReadOnly))
        throw ErrorFileOpen();
    QByteArray fw(fwFile.readAll());
    fwFile.close();

    info(QString(tr("Erasing: 0x%1-0x%2")).arg(addr, 8, 16, QChar('0')).arg(addr + size, 8, 16, QChar('0')));
    for (i = 0; i * DFU_BLOCK_SIZE < size; ++i)
    {
        cmdErase(i * DFU_BLOCK_SIZE + addr, DFU_BLOCK_SIZE);
        if (i && ((i % REFRESH_RATE) == 0))
            info(".");
    }
    info("\n");

    info(QString(tr("Flashing: 0x%1")).arg(addr, 8, 16, QChar('0')));
    for (i = 0; i * DFU_BLOCK_SIZE < static_cast<unsigned int>(fw.size()); ++i)
    {
        QByteArray chunk(fw.mid(i * DFU_BLOCK_SIZE, DFU_BLOCK_SIZE));
        if (static_cast<unsigned int>(chunk.size()) < DFU_BLOCK_SIZE)
            chunk += QByteArray(DFU_BLOCK_SIZE - chunk.size(), 0x0);
        cmdWrite(i * DFU_BLOCK_SIZE + addr, chunk);
        if (i && ((i % REFRESH_RATE) == 0))
            info(".");
    }
    info("\n");
}
