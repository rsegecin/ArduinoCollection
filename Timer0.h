#pragma once

#ifndef _Timer0_h
#define _Timer0_h

#include <Arduino.h>
#include "Communs.h"

#define DEBUG_PIN 7

class Timer0Class
{
public:
	Timer0Class();

	uint32_t Time;
	void OnInterrupt();
	void DelayMili(uint32_t pMili);
	void DelayMili(uint32_t pMili, bool &pFlag);
	void DelayMili(uint32_t pMili, void(*doWhile)(void));
	void DelayMili(uint32_t pMili, bool &pFlag, void(*doWhile)(void));

private:
	//char lBuffer[DEF_MSG_SIZE];
	bool tmpTest;
	volatile uint32_t miliSeconds;
	volatile uint32_t miliToSec;
	volatile uint32_t tmpTicks;
};

extern Timer0Class Timer0;

#endif