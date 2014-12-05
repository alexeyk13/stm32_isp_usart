/*
    USB DFU Flasher PC part (cross-platform)
    Copyright (c) 2014, Alexey Kramarenko
    All rights reserved.
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "comm.h"
#include <QTextStream>
#include <QDateTime>
#include "config.h"
#include "error.h"
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    logFile(LOG_FILE_NAME),
    newLine(true)
{
    ui->setupUi(this);
    if (logFile.open(QIODevice::Append | QIODevice::Text))
        logToFile(tr("System started\n"));
    else
        logToScreen(tr("Can't create log\n"), Qt::darkYellow);
    comm = new Comm(this);
    connect(comm, SIGNAL(log(LOG_TYPE,QString,QColor)), this, SLOT(log(LOG_TYPE,QString,QColor)));
    info(tr("Application started\n"));

    ui->ePort->addItems(comm->ports());
    ui->eSpeed->addItem("1200");
    ui->eSpeed->addItem("2400");
    ui->eSpeed->addItem("4800");
    ui->eSpeed->addItem("9600");
    ui->eSpeed->addItem("14400");
    ui->eSpeed->addItem("19200");
    ui->eSpeed->addItem("28800");
    ui->eSpeed->addItem("38400");
    ui->eSpeed->addItem("57600");
    ui->eSpeed->addItem("115200");
    ui->eSpeed->setCurrentText("115200");
}

MainWindow::~MainWindow()
{
    delete comm;
    logFile.close();
    delete ui;
}

void MainWindow::logToScreen(const QString &text, const QColor &color)
{
    ui->log->setDisabled(true);
    ui->log->moveCursor(QTextCursor::End);
    QString html(QString("<font color=#%1>%2</font>").arg(static_cast<unsigned long>(color.rgb()) & 0xffffff, 6, 16, QChar('0')).arg(text));
    html.replace("\n", "<br>");
    ui->log->textCursor().insertHtml(html);
    ui->log->setDisabled(false);
    QCoreApplication::processEvents();
}

void MainWindow::logToFile(const QString &text)
{
    if (logFile.isOpen())
    {
        QTextStream out(&logFile);
        if (newLine)
            out << QDateTime::currentDateTime().toString(LOG_DATE_FORMAT) << ' ';
        out << text;
        newLine = text.contains('\n');
    }
}

void MainWindow::log(LOG_TYPE type, const QString &text, const QColor &color)
{
    switch (type)
    {
    case LOG_TYPE_DEFAULT:
        logToScreen(text, color);
        logToFile(text);
        break;
    case LOG_TYPE_HINT:
        logToScreen(text, Qt::darkCyan);
        logToFile(tr("Hint: ") + text);
        break;
    case LOG_TYPE_WARNING:
        logToScreen(text, Qt::darkYellow);
        logToFile(tr("Warning: ") + text);
        break;
    case LOG_TYPE_ERROR:
        logToScreen(text, Qt::red);
        logToFile(tr("Error: ") + text);
        break;
    case LOG_TYPE_DEBUG:
#ifndef QT_NO_DEBUG
        logToScreen(text, Qt::lightGray);
#endif
        logToFile("> " + text);
        break;
    }
}

void MainWindow::on_bFlash_clicked()
{
    unsigned int addr = ui->eAddress->text().toInt(0, 16);
    unsigned int size = ui->eSize->text().toInt(0, 16);
    try
    {
        comm->open(ui->ePort->currentText(), ui->eSpeed->currentText().toInt());
        try
        {
            comm->erase(addr, size);
            comm->flash(ui->eFile->text(), addr, true);
            comm->close();
        }
        catch (...)
        {
            comm->close();
            throw;
        }
    }
    catch (Exception& e)
    {
        error(e.what() + "\n");
    }
    catch (...)
    {
        error(tr("Unhandled exception\n"));
    }
}

void MainWindow::on_bSelectFile_clicked()
{
    QString name(QFileDialog::getOpenFileName(this, tr("Open File"),"",tr("Firmsware (*.bin)")));
    if (!name.isEmpty())
        ui->eFile->setText(name);
}

void MainWindow::on_bDump_clicked()
{
    unsigned int addr = ui->eAddress->text().toInt(0, 16);
    unsigned int size = ui->eSize->text().toInt(0, 16);
    try
    {
        comm->open(ui->ePort->currentText(), ui->eSpeed->currentText().toInt());
        try
        {
            comm->dump(ui->eFile->text(), addr, size);
            comm->close();
        }
        catch (...)
        {
            comm->close();
            throw;
        }
    }
    catch (Exception& e)
    {
        error(e.what() + "\n");
    }
    catch (...)
    {
        error(tr("Unhandled exception\n"));
    }
}

void MainWindow::on_bReadProtect_clicked()
{
    try
    {
        comm->open(ui->ePort->currentText(), ui->eSpeed->currentText().toInt());
        try
        {
            info(tr("Read protecting\n"));
            comm->cmdReadoutProtect();
            hint(tr("Read protection complete. Device is reset\n"));
        }
        catch (...)
        {
            comm->close();
            throw;
        }
    }
    catch (Exception& e)
    {
        error(e.what() + "\n");
    }
    catch (...)
    {
        error(tr("Unhandled exception\n"));
    }
}

void MainWindow::on_eMassErase_clicked()
{
    try
    {
        comm->open(ui->ePort->currentText(), ui->eSpeed->currentText().toInt());
        try
        {
            info(tr("Mass erasing\n"));
            comm->cmdEraseMemoryEx(ISP_MASS_ERASE);
            hint(tr("Mass erase complete. Device is reset\n"));
        }
        catch (...)
        {
            comm->close();
            throw;
        }
    }
    catch (Exception& e)
    {
        error(e.what() + "\n");
    }
    catch (...)
    {
        error(tr("Unhandled exception\n"));
    }
}
