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
#include <stdint.h>
#include "common.h"
#include "error.h"

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

class Comm : public QObject
{
    Q_OBJECT
private:
    QSerialPort* com;
    QVector<uint8_t> supportedCmds;

protected:
    void info(const QString& text, const QColor& color = Qt::black) {log(LOG_TYPE_DEFAULT, text, color);}
    void hint(const QString& text) {log(LOG_TYPE_HINT, text, Qt::black);}
    void warning(const QString& text) {log(LOG_TYPE_WARNING, text, Qt::black);}
    void error(const QString& text) {log(LOG_TYPE_ERROR, text, Qt::black);}
    void debug(const QString& text) {log(LOG_TYPE_DEBUG, text, Qt::black);}

    unsigned char rxChar();
    void rxAck();
    void req(unsigned char cmd);
public:
    explicit Comm(QObject *parent = 0);
    virtual ~Comm();

    bool isActive();
    void open(const QString& name);
    void close();

    QStringList ports();


    unsigned char cmdGet();
    unsigned char cmdGetVersion();
    unsigned short cmdGetID();
signals:
    void log(LOG_TYPE type, const QString& text, const QColor& color);


public slots:

};

#endif // COMM_H
