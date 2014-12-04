/*
    USB DFU Flasher PC part (cross-platform)
    Copyright (c) 2014, Alexey Kramarenko
    All rights reserved.
*/

#ifndef COMM_H
#define COMM_H

#include <QObject>
#include <QColor>
#include "common.h"
#include "error.h"

class ErrorProtocol: public Exception
{
public:
    ErrorProtocol() throw() :Exception() {str = (QObject::tr("Generic protocol error"));}
};

class ErrorProtocolInvalidResponse: public ErrorProtocol
{
public:
    ErrorProtocolInvalidResponse() throw() :ErrorProtocol() {str = (QObject::tr("Protocol invalid response"));}
};

class DFUD;

class Comm : public QObject
{
    Q_OBJECT
private:
    DFUD* dfud;

protected:
    void info(const QString& text, const QColor& color = Qt::black) {log(LOG_TYPE_DEFAULT, text, color);}
    void hint(const QString& text) {log(LOG_TYPE_HINT, text, Qt::black);}
    void warning(const QString& text) {log(LOG_TYPE_WARNING, text, Qt::black);}
    void error(const QString& text) {log(LOG_TYPE_ERROR, text, Qt::black);}
    void debug(const QString& text) {log(LOG_TYPE_DEBUG, text, Qt::black);}

    void cmdReq(unsigned char cmd, unsigned int param1, unsigned int param2, QByteArray data = QByteArray());

public:
    explicit Comm(QObject *parent = 0);
    virtual ~Comm();

    bool isActive();
    bool open();
    void close();

    void cmdVersion(int& loader, int& protocol);
    void cmdLeave();
    QByteArray cmdRead(unsigned int addr, unsigned int size);
    void cmdWrite(unsigned int addr, const QByteArray& buf);
    void cmdErase(unsigned int addr, unsigned int size);

    void flash(const QString& fileName, unsigned int addr, unsigned int size);

signals:
    void log(LOG_TYPE type, const QString& text, const QColor& color);


public slots:

};

#endif // COMM_H
