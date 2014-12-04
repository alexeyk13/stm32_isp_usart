/*
    USB DFU Flasher PC part (cross-platform)
    Copyright (c) 2014, Alexey Kramarenko
    All rights reserved.
*/

#ifndef DELAY_H
#define DELAY_H

#include <qthread.h>

class sleeper : public QThread
{
public:
	static void sleep_s(unsigned long s) {
		QThread::sleep(s);
	}
	static void sleep_ms(unsigned long ms) {
		QThread::msleep(ms);
	}
	static void sleep_us(unsigned long us) {
		QThread::usleep(us);
	}
};

//sleep common (Qt doesn't have one)
static inline void sleep_s(unsigned long s) {sleeper::sleep_s(s);}
static inline void sleep_ms(unsigned long ms) {sleeper::sleep_ms(ms);}
static inline void sleep_us(unsigned long us) {sleeper::sleep_us(us);}


#endif // DELAY_H
