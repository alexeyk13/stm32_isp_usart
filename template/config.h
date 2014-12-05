/*
    USB DFU Flasher PC part (cross-platform)
    Copyright (c) 2014, Alexey Kramarenko
    All rights reserved.
*/

#ifndef CONFIG_H
#define CONFIG_H

#include <QString>

const QString LOG_FILE_NAME("file.log");
const QString LOG_DATE_FORMAT("dd.MM hh:mm:ss.zzz");

const int ACK_TIMEOUT_COUNT =                                       5000;
const int PAGE_SIZE =                                               128;
const int FLASH_BASE =                                              0x08000000;

const int PORT_DEFAULT_TIMEOUT =                                    5000;
const int REFRESH_RATE =                                            10;
const int NRETRY =                                                  3;

#endif // CONFIG_H
