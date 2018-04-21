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

#define BASE_YEAR				2000UL
#define SECS_PER_MIN			60UL
#define SECS_PER_HOUR			3600UL
#define SECS_PER_DAY			SECS_PER_HOUR * 24UL
#define SECS_PER_YEAR			SECS_PER_DAY * 365UL
#define SECS_FEB				SECS_PER_DAY * 28UL
#define SECS_FEB_LEAP			SECS_PER_DAY * 29UL
#define SECS_PER_MONTH_EVEN		SECS_PER_DAY * 30UL
#define SECS_PER_MONTH_ODD		SECS_PER_DAY * 31UL

#define LEAP_YEAR(Y)	(((BASE_YEAR + Y) > 0) && !((BASE_YEAR + Y) % 4) && ( ((BASE_YEAR + Y) % 100) || !((BASE_YEAR + Y) % 400)))
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
	void DelayMili(uint32_t pMili, void(*doWhile)(void));
	void DelayMili(uint32_t pMili, bool &pFlag, void(*doWhile)(void));
	void SetTime(tTime pTime); // Time in seconds since January 1st 2000
	bool SetTime(char pTimeString[]);
	bool ParseTime(sDateTime &pDateTime, char pTimeString[]);
	tTime MakeTime(sDateTime &t);
	void BreakTime(tTime &timeInput, sDateTime &tm);


private:
	//char lBuffer[DEF_MSG_SIZE];
	bool tmpTest;
	volatile uint32_t miliSeconds;
	volatile uint32_t miliToSec;
	volatile uint32_t tmpTicks;
};

extern RTCTimerClass RTCTimer;

#endif