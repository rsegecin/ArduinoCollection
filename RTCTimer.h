#pragma once

#ifndef _RTCTIMER_h
#define _RTCTIMER_h

#include <stdint.h>
#include <Arduino.h>
#include <Regexp.h>
#include "Communs.h"
//#include "SerialInterpreter.h"

typedef uint32_t tTime;

#define DEBUG_PIN PB1

#define SECS_PER_MIN		((tTime)(60UL))
#define SECS_PER_HOUR		((tTime)(3600UL))
#define SECS_PER_DAY		((tTime)(SECS_PER_HOUR * 24UL))
#define SECS_PER_MONTH_ODD	((tTime)(SECS_PER_DAY * 31UL))
#define SECS_PER_MONTH_EVEN	((tTime)(SECS_PER_DAY * 30UL))
#define SECS_PER_FEB		((tTime)(SECS_PER_DAY * 28UL))
#define SECS_PER_FEB_LEAP	((tTime)(SECS_PER_DAY * 29UL))

#define LEAP_YEAR(Y)	(((2000 + Y) > 0) && !((2000 + Y) % 4) && ( ((2000 + Y) % 100) || !((2000 + Y) % 400)))
static  const uint8_t	monthDays[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

struct sDateTime {
	int8_t		Seconds; 
	int8_t		Minutes; 
	int8_t		Hours;
	int8_t		DayOfMonth;
	int8_t		DayOfWeek;
	int8_t		Month; 
	int16_t		Year;
};

class RTCTimerClass
{
public:
	RTCTimerClass();

	uint32_t Time;
	sDateTime DateTime;
	void OnInterrupt();
	void DelayMili(uint32_t pMili);
	void DelayMili(uint32_t pMili, bool &pFlag);
	void DelayMili(uint32_t pMili, bool &pFlag, void(*doWhile)(void));
	void SetTime(tTime pTime); // Time in seconds since January 1st 2000
	bool SetTime(char pTimeString[]);
	sDateTime * ParseTime(char pTimeString[]);
	tTime MakeTime(sDateTime &t);
	void BreakTime(tTime &timeInput, sDateTime &tm);


private:
	//char lBuffer[DEF_MSG_SIZE];
	bool tmpTest;
	volatile uint32_t miliSeconds;
	volatile uint32_t tmpTicks;
};

extern RTCTimerClass RTCTimer;

#endif