/*
    USB DFU Flasher PC part (cross-platform)
    Copyright (c) 2014, Alexey Kramarenko
    All rights reserved.
*/

#ifndef COMM_H
#define COMM_H

#include <QObject>
#include <QStringList>
#include <QColor>
#include <QVector>
#include "common.h"
#include "error.h"

const unsigned int ISP_MASS_ERASE =                             0xffff;
const unsigned int ISP_ERASE_BANK1 =                            0xfffe;
const unsigned int ISP_ERASE_BANK2 =                            0xfffd;

class QSerialPort;

class ErrorPort: public Exception
{
public:
    ErrorPort() throw() :Exception() {str = (QObject::tr("Generic port error"));}
};

class ErrorPortOpen: public Exception
{
public:
    ErrorPortOpen() throw() :Exception() {str = (QObject::tr("Port open error"));}
};

class ErrorPortTimeout: public Exception
{
public:
    ErrorPortTimeout() throw() :Exception() {str = (QObject::tr("Port timeout"));}
};

class ErrorProtocol: public Exception
{
public:
    ErrorProtocol() throw() :Exception() {str = (QObject::tr("Generic protocol error"));}
};

class ErrorProtocolTimeout: public ErrorProtocol
{
public:
    ErrorProtocolTimeout() throw() :ErrorProtocol() {str = (QObject::tr("Protocol timeout"));}
};

class ErrorProtocolInvalidResponse: public ErrorProtocol
{
public:
    ErrorProtocolInvalidResponse() throw() :ErrorProtocol() {str = (QObject::tr("Protocol invalid response"));}
};

class ErrorProtocolNack: public ErrorProtocol
{
public:
    ErrorProtocolNack() throw() :ErrorProtocol() {str = (QObject::tr("Protocol NACK"));}
};

class ErrorProtocolReadProtection: public ErrorProtocol
{
public:
    ErrorProtocolReadProtection() throw() :ErrorProtocol() {str = (QObject::tr("Device is READ protected"));}
};

class ErrorProtocolWriteProtection: public ErrorProtocol
{
public:
    ErrorProtocolWriteProtection() throw() :ErrorProtocol() {str = (QObject::tr("Device is WRITE protected"));}
};

class ErrorProtocolVerify: public ErrorProtocol
{
public:
    ErrorProtocolVerify() throw() :ErrorProtocol() {str = (QObject::tr("Page VERIFY failed"));}
};

class Comm : public QObject
{
    Q_OBJECT
private:
    QSerialPort* com;
    QVector<unsigned char> supportedCmds;

protected:
    void info(const QString& text, const QColor& color = Qt::black) {log(LOG_TYPE_DEFAULT, text, color);}
    void hint(const QString& text) {log(LOG_TYPE_HINT, text, Qt::black);}
    void warning(const QString& text) {log(LOG_TYPE_WARNING, text, Qt::black);}
    void error(const QString& text) {log(LOG_TYPE_ERROR, text, Qt::black);}
    void debug(const QString& text) {log(LOG_TYPE_DEBUG, text, Qt::black);}

    void ispStart();
    QByteArray rx(unsigned int maxSize);
    unsigned char rxChar();
    void rxAck();
    void tx(const QByteArray& buf);
    void txAck();
    void txReq(unsigned char cmd);
    void txAddr(unsigned int addr);
public:
    explicit Comm(QObject *parent = 0);
    virtual ~Comm();

    bool isActive();
    void open(const QString& name, unsigned int speed);
    void close();

    QStringList ports();


    unsigned char cmdGet();
    unsigned char cmdGetVersion();
    unsigned short cmdGetID();
    QByteArray cmdReadMemory(unsigned int addr, unsigned int size);
    void cmdGo(unsigned int addr);
    void cmdWriteMemory(unsigned int addr, const QByteArray& data);
    void cmdEraseMemory(unsigned int page);
    void cmdEraseMemoryEx(unsigned int page);
    void cmdReadoutProtect();
    void cmdReadoutUnProtect();

    void dump(const QString& fileName, unsigned int addr, unsigned int size);
    void erase(unsigned int addr, unsigned int size);
    void flash(const QByteArray& data, unsigned int addr, bool verify = true);
    void flash(const QString& fileName, unsigned int addr, bool verify = true);
signals:
    void log(LOG_TYPE type, const QString& text, const QColor& color);


public slots:

};

#endif // COMM_H
