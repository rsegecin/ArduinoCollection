#include "Timer0.h"

Timer0Class::Timer0Class()
{
	//set timer1 interrupt at 1kHz
	TCCR1A = 0;		// set entire TCCR1A register to 0
	TCCR1B = 0;		// same for TCCR1B
	TCNT1 = 0;		//initialize counter value to 0

					// set compare match register for 8khz increments
	OCR1A = 1999;	// = [(16*10^6) / (1000*8)] - 1 = (must be < 65536)

					// turn on CTC mode
	TCCR1B |= (1 << WGM12);
	// Set CS11 bits for 8 prescaler
	TCCR1B |= (1 << CS11);
	// enable timer compare interrupt
	TIMSK1 |= (1 << OCIE1A);

	set_output(DDRB, DEBUG_PIN);

	tmpTest = true;
}

void Timer0Class::OnInterrupt()
{
	if (tmpTest)
	{
		tmpTest = false;
		set_bit(PINB, DEBUG_PIN);
	}
	else
	{
		tmpTest = true;
		clear_bit(PINB, DEBUG_PIN);
	}

	miliToSec++;
	miliSeconds++;

	if (miliToSec >= 1000)
	{
		miliToSec = 0;
		Time++;
	}
}

void Timer0Class::DelayMili(uint32_t pMili)
{
	tmpTicks = miliSeconds + pMili;

	while (miliSeconds <= tmpTicks) { yield(); }
}

void Timer0Class::DelayMili(uint32_t pMili, bool &pFlag)
{
	tmpTicks = miliSeconds + pMili;

	while ((!pFlag) && (miliSeconds <= tmpTicks)) { yield(); }
}

void Timer0Class::DelayMili(uint32_t pMili, void(*doWhile)(void))
{
	tmpTicks = miliSeconds + pMili;

	while (miliSeconds <= tmpTicks)
	{
		doWhile();
	}
}

void Timer0Class::DelayMili(uint32_t pMili, bool &pFlag, void(*doWhile)(void))
{
	tmpTicks = miliSeconds + pMili;

	while ((!pFlag) && (miliSeconds <= tmpTicks))
	{
		doWhile();
	}
}

Timer0Class Timer0;