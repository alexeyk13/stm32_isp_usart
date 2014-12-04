/*
    USB DFU Flasher PC part (cross-platform)
    Copyright (c) 2014, Alexey Kramarenko
    All rights reserved.
*/

#ifndef ERROR_H
#define ERROR_H

#include <exception>
#include <QString>
#include <QObject>

/* base */
class Exception
{
protected:
    QString str;
public:
    Exception() throw() {str = (QObject::tr("Internal error"));}
    virtual ~Exception() throw() {}
    virtual QString what() throw() {return str;}
};

class ErrorNotActive: public Exception
{
public:
    ErrorNotActive() throw() :Exception() {str = (QObject::tr("Not active"));}
};

/* cancel class */
class ErrorCancel: public Exception
{
public:
    ErrorCancel() throw() :Exception() {str = (QObject::tr("Action cancelled"));}
};

/* file errors */
class ErrorFile: public Exception
{
public:
    ErrorFile() throw() :Exception() {str = (QObject::tr("Generic file error"));}
};

class ErrorFileNotFound: public ErrorFile
{
public:
    ErrorFileNotFound() throw() :ErrorFile() {str = (QObject::tr("File not found"));}
};

class ErrorFileOpen: public ErrorFile
{
public:
    ErrorFileOpen() throw() :ErrorFile() {str = (QObject::tr("File open error"));}
};

class ErrorFileCreate: public ErrorFile
{
public:
    ErrorFileCreate() throw() :ErrorFile() {str = (QObject::tr("File create error"));}
};

class ErrorFileWrite: public ErrorFile
{
public:
    ErrorFileWrite() throw() :ErrorFile() {str = (QObject::tr("File write error"));}
};

class ErrorFileRead: public ErrorFile
{
public:
    ErrorFileRead() throw() :ErrorFile() {str = (QObject::tr("File read error"));}
};

class ErrorFileCrc: public ErrorFile
{
public:
    ErrorFileCrc() throw() :ErrorFile() {str = (QObject::tr("File CRC error"));}
};

class ErrorFileCopy: public ErrorFile
{
public:
    ErrorFileCopy() throw() :ErrorFile() {str = (QObject::tr("File copy error"));}
};

class ErrorFileRemove: public ErrorFile
{
public:
    ErrorFileRemove() throw() :ErrorFile() {str = (QObject::tr("File remove error"));}
};

class ErrorFolderOpen: public ErrorFile
{
public:
    ErrorFolderOpen() throw() :ErrorFile() {str = (QObject::tr("Folder open error"));}
};

class ErrorFolderRemove: public ErrorFile
{
public:
    ErrorFolderRemove() throw() :ErrorFile() {str = (QObject::tr("Folder remove error"));}
};

/* network errors */
class ErrorNetwork: public Exception
{
public:
    ErrorNetwork() throw() :Exception() {str = (QObject::tr("Generic network error"));}
};

class ErrorNetworkNotAccessible: public ErrorNetwork
{
public:
    ErrorNetworkNotAccessible() throw() :ErrorNetwork() {str = (QObject::tr("Network is not accessible"));}
};

class ErrorNetworkAuthenticationRequired: public ErrorNetwork
{
public:
    ErrorNetworkAuthenticationRequired() throw() :ErrorNetwork() {str = (QObject::tr("Authentication required"));}
};

class ErrorNetworkForbidden: public ErrorNetwork
{
public:
    ErrorNetworkForbidden() throw() :ErrorNetwork() {str = (QObject::tr("Forbidden"));}
};

#endif // ERROR_H
