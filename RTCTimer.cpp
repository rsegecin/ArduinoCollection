#include "RTCTimer.h"

RTCTimerClass::RTCTimerClass()
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

void RTCTimerClass::OnInterrupt()
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

void RTCTimerClass::DelayMili(uint32_t pMili)
{
	tmpTicks = miliSeconds + pMili;

	while (miliSeconds <= tmpTicks) { yield(); }
}

void RTCTimerClass::DelayMili(uint32_t pMili, bool &pFlag)
{
	tmpTicks = miliSeconds + pMili;

	while ((!pFlag) && (miliSeconds <= tmpTicks)) { yield(); }
}

void RTCTimerClass::DelayMili(uint32_t pMili, void(*doWhile)(void))
{
	tmpTicks = miliSeconds + pMili;

	while (miliSeconds <= tmpTicks)
	{
		doWhile();
	}
}

void RTCTimerClass::DelayMili(uint32_t pMili, bool &pFlag, void(*doWhile)(void))
{
	tmpTicks = miliSeconds + pMili;

	while ((!pFlag) && (miliSeconds <= tmpTicks))
	{
		doWhile();
	}
}

void RTCTimerClass::SetTime(tTime pTime)
{
	Time = pTime;
}

bool RTCTimerClass::SetTime(char pTimeString[])
{
	ParseTime(DateTime, pTimeString);

	if (DateTime.Year != -1)
	{
		Time = MakeTime(DateTime);

		return true;
	}

	Time = 0;

	return false;
}

bool RTCTimerClass::ParseTime(sDateTime &pDateTime, char pTimeString[])
{
	String strDateTime(pTimeString);
	int iT = strDateTime.indexOf('T');
	int iZ = strDateTime.indexOf('Z');
	bool allRight = false;

	if ((iT == 10) && ((iZ == 19) || (iZ == 23)))
	{
		char strDate[11];
		char strTime[9];
		strDateTime.substring(0, iT).toCharArray(strDate, 11);
		strDateTime.substring(iT + 1, iZ).toCharArray(strTime, 9);

		char tmpBuffer[20];
		strcpy_P(tmpBuffer, DateISORegex);
		MatchState msDate(strDate);
		msDate.Match(tmpBuffer);

		char cap[4];

		memset(&pDateTime, 0x00, sizeof(sDateTime));

		if (msDate.level == 3)
		{
			msDate.GetCapture(cap, 0);
			pDateTime.Year = atoi(cap);
			msDate.GetCapture(cap, 1);
			pDateTime.Month = atoi(cap);
			msDate.GetCapture(cap, 2);
			pDateTime.DayOfMonth = atoi(cap);

			strcpy_P(tmpBuffer, TimeISORegex);
			MatchState msTime(strTime);
			msTime.Match(tmpBuffer);

			if (msTime.level == 3)
			{
				msTime.GetCapture(cap, 0);
				pDateTime.Hours = atoi(cap);
				msTime.GetCapture(cap, 1);
				pDateTime.Minutes = atoi(cap);
				msTime.GetCapture(cap, 2);
				pDateTime.Seconds = atoi(cap);

				allRight = true;
			}
		}
	}

	return allRight;
}

tTime RTCTimerClass::MakeTime(sDateTime &pTm)
{
	int i;
	uint32_t seconds;
	int16_t year = pTm.Year - BASE_YEAR;

	// seconds from 2000 till 1 jan 00:00:00 of the given year
	seconds = year * SECS_PER_YEAR;
	for (i = 0; i < year; i++)
	{
		if (LEAP_YEAR(i))
		{
			seconds += SECS_PER_DAY;
		}
	}

	// add days for this actual year passed on datetime, months start from 1
	for (i = 1; i < pTm.Month; i++)
	{
		switch (monthDays[i - 1])
		{
		case 28:
			if (LEAP_YEAR(year))
				seconds += SECS_FEB_LEAP;
			else
				seconds += SECS_FEB;
			break;
		case 30:
			seconds += SECS_PER_MONTH_EVEN;
			break;
		case 31:
			seconds += SECS_PER_MONTH_ODD;
			break;
		default:
			break;
		}
	}

	seconds += (pTm.DayOfMonth - 1) * SECS_PER_DAY;
	seconds += pTm.Hours * SECS_PER_HOUR;
	seconds += pTm.Minutes * SECS_PER_MIN;
	seconds += pTm.Seconds;

	return (tTime)seconds;
}

void RTCTimerClass::BreakTime(tTime &timeInput, sDateTime &tm)
{
	uint8_t year;
	uint8_t month, monthLength;
	uint32_t time;
	uint16_t days;

	/*sprintf(lBuffer, "RTC has been set %lu.", (uint32_t)RTCTimer.Time);
	SerialInterpreter.Send(lBuffer);*/
	
	time = (uint32_t)timeInput;
	tm.Seconds = time % 60;
	time /= 60; // now it is minutes
	tm.Minutes = time % 60;
	time /= 60; // now it is hours
	tm.Hours = time % 24;
	time /= 24; // now it is days
	tm.DayOfWeek = ((time + 4) % 7) + 1;  // Sunday is day 1 

	year = 0;
	days = 0;
	for (;;)
	{
		days += (LEAP_YEAR(year) ? 366 : 365);
		if (days < time)
			year++;
		else
			break;
	}
	tm.Year = year + BASE_YEAR; // year is offset from 2000 

	days -= (LEAP_YEAR(year) ? 366 : 365);
	time -= days; // now it is days in this year, starting at 0

	days = 0;
	month = 0;
	monthLength = 0;

	for (month = 0; month < 12; month++)
	{
		if ((month == 1) && (LEAP_YEAR(year)))
		{
			monthLength = 29;
		}
		else
		{
			monthLength = monthDays[month];
		}

		if (time >= monthLength)
		{
			time -= monthLength;
		}
		else
		{
			break;
		}
	}

	tm.Month = month + 1;  // jan is month 1  
	tm.DayOfMonth = time + 1;     // day of month
}

RTCTimerClass RTCTimer;