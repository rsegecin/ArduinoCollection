#pragma once

#ifndef _RTCTIMER_h
#define _RTCTIMER_h

#include <stdint.h>
#include <Arduino.h>
#include <Regexp.h>
#include <time.h>
#include "Communs.h"
#include "SerialInterpreter.h"

#define DEBUG_PIN PB1

#define SECS_PER_MIN	((time_t)(60UL))
#define SECS_PER_HOUR	((time_t)(3600UL))
#define SECS_PER_DAY	((time_t)(SECS_PER_HOUR * 24UL))
#define DAYS_PER_WEEK	((time_t)(7UL))
#define SECS_PER_WEEK	((time_t)(SECS_PER_DAY * DAYS_PER_WEEK))
#define SECS_PER_YEAR	((time_t)(SECS_PER_WEEK * 52UL))

#define LEAP_YEAR(Y)	(((2000 + Y) > 0) && !((2000 + Y) % 4) && ( ((2000 + Y) % 100) || !((2000 + Y) % 400)))
static  const uint8_t	monthDays[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

class RTCTimerClass
{
public:
	RTCTimerClass();

	struct tm TimeModel;
	volatile time_t MainTime;
	void OnInterrupt();
	void DelayMili(uint32_t pMili);
	void DelayMili(uint32_t pMili, bool &pFlag);
	void DelayMili(uint32_t pMili, bool &pFlag, void(*doWhile)(void));
	//void SetTime(time_t pTime); // Time in seconds since January 1st 2000
	bool SetTime(char pTimeString[]);
	struct tm * ParseTime(char pTimeString[]);
	time_t MakeTime(tm &t);
	void BreakTime(time_t timeInput, tm &tm);


private:

	char lBuffer[DEF_MSG_SIZE];
	bool tmpTest;
	volatile uint32_t miliSeconds;
	volatile uint32_t tmpTicks;
};

extern RTCTimerClass RTCTimer;

#endif