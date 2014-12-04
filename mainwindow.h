/*
    USB DFU Flasher PC part (cross-platform)
    Copyright (c) 2014, Alexey Kramarenko
    All rights reserved.
*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>
#include "common.h"

class Comm;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
private:
    Ui::MainWindow *ui;
    Comm* comm;
    QFile logFile;
    bool newLine;

protected:
    void logToScreen(const QString& text, const QColor& color);
    void logToFile(const QString& text);
    void info(const QString& text, const QColor& color = Qt::black) {log(LOG_TYPE_DEFAULT, text, color);}
    void hint(const QString& text) {log(LOG_TYPE_HINT, text, Qt::black);}
    void warning(const QString& text) {log(LOG_TYPE_WARNING, text, Qt::black);}
    void error(const QString& text) {log(LOG_TYPE_ERROR, text, Qt::black);}
    void debug(const QString& text) {log(LOG_TYPE_DEBUG, text, Qt::black);}

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void log(LOG_TYPE type, const QString& text, const QColor& color);
private slots:
    void on_bFlash_clicked();
    void on_bSelectFile_clicked();
};

#endif // MAINWINDOW_H
