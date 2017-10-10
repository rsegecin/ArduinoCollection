#include "RTCTimer.h"

RTCTimerClass::RTCTimerClass()
{
	//set timer1 interrupt at 1kHz
	TCCR1A = 0;		// set entire TCCR1A register to 0
	TCCR1B = 0;		// same for TCCR1B
	TCNT1 = 0;		//initialize counter value to 0

					// set compare match register for 8khz increments
	OCR1A = 999;	// = [(16*10^6) / (1000*8)] - 1 = (must be < 65536)

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

	miliSeconds++;

	//if (miliSeconds >= 1000)
	//	mainTime++;
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

void RTCTimerClass::DelayMili(uint32_t pMili, bool &pFlag, void(*doWhile)(void))
{
	tmpTicks = miliSeconds + pMili;

	while ((!pFlag) && (miliSeconds <= tmpTicks))
	{
		doWhile();
	}
}

//void RTCTimerClass::SetTime(time_t pTime)
//{
//	MainTime = pTime;
//}

bool RTCTimerClass::SetTime(char pTimeString[])
{
	struct tm * pointer = ParseTime(pTimeString);

	//if (TimeModel.tm_year != -1)
	//{
	//	sprintf(lBuffer, "Metallica %d:%d:%d and the date is %d/%d/%d %p %p",
	//		TimeModel.tm_hour, TimeModel.tm_min, TimeModel.tm_sec,
	//		TimeModel.tm_mday, TimeModel.tm_mon, TimeModel.tm_year, (void *)&TimeModel, (void *)pointer);

	//	SerialInterpreter.Send(lBuffer);


	//	sprintf(lBuffer, "All night long");
	//	SerialInterpreter.Send(lBuffer);

	//	/*
	//	time_t t = MakeTime(STm);

	//	if (t != -1)
	//	{
	//		MainTime = t;
	//		return true;
	//	}*/
	//}

	MainTime = 0;

	return false;
}

struct tm * RTCTimerClass::ParseTime(char pTimeString[])
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

		memset(&TimeModel, 0x00, sizeof(struct tm));

		if (msDate.level == 3)
		{
			msDate.GetCapture(cap, 0);
			TimeModel.tm_year = atoi(cap) - 2000;
			msDate.GetCapture(cap, 1);
			TimeModel.tm_mon = atoi(cap);
			msDate.GetCapture(cap, 2);
			TimeModel.tm_mday = atoi(cap);

			strcpy_P(tmpBuffer, TimeISORegex);
			MatchState msTime(strTime);
			msTime.Match(tmpBuffer);

			if (msTime.level == 3)
			{
				msTime.GetCapture(cap, 0);
				TimeModel.tm_hour = atoi(cap);
				msTime.GetCapture(cap, 1);
				TimeModel.tm_min = atoi(cap);
				msTime.GetCapture(cap, 2);
				TimeModel.tm_sec = atoi(cap);

				sprintf(lBuffer, "The time is %d:%d:%d and the date is %d/%d/%d %p",
					TimeModel.tm_hour, TimeModel.tm_min, TimeModel.tm_sec,
					TimeModel.tm_mday, TimeModel.tm_mon, TimeModel.tm_year, (void **)&TimeModel);

				SerialInterpreter.Send(lBuffer);

				allRight = true;
			}
		}
	}

	if (!allRight)
		TimeModel.tm_year = -1;

	TimeModel.tm_isdst = -1;

	return &TimeModel;
}


time_t RTCTimerClass::MakeTime(tm &pTm)
{
	int i;
	uint32_t seconds;

	// seconds from 1970 till 1 jan 00:00:00 of the given year
	seconds = pTm.tm_year * (SECS_PER_DAY * 365);
	for (i = 0; i < pTm.tm_year; i++)
	{
		if (LEAP_YEAR(i))
		{
			seconds += SECS_PER_DAY;   // add extra days for leap years
		}
	}

	// add days for this year, months start from 1
	for (i = 1; i < pTm.tm_mon; i++)
	{
		if ((i == 2) && LEAP_YEAR(pTm.tm_year))
		{
			seconds += SECS_PER_DAY * 29;
		}
		else
		{
			seconds += SECS_PER_DAY * monthDays[i - 1];  //monthDay array starts from 0
		}
	}

	seconds += (pTm.tm_mday - 1) * SECS_PER_DAY;
	seconds += pTm.tm_hour * SECS_PER_HOUR;
	seconds += pTm.tm_min * SECS_PER_MIN;
	seconds += pTm.tm_sec;
	return (time_t)seconds;
}

void RTCTimerClass::BreakTime(time_t timeInput, tm &tm)
{
	uint8_t year;
	uint8_t month, monthLength;
	uint32_t time;
	unsigned long days;

	time = (uint32_t)timeInput;
	tm.tm_sec = time % 60;
	time /= 60; // now it is minutes
	tm.tm_min = time % 60;
	time /= 60; // now it is hours
	tm.tm_hour = time % 24;
	time /= 24; // now it is days
	tm.tm_wday = ((time + 4) % 7) + 1;  // Sunday is day 1 

	year = 0;
	days = 0;
	while ((unsigned)(days += (LEAP_YEAR(year) ? 366 : 365)) <= time)
	{
		year++;
	}
	tm.tm_year = year; // year is offset from 1970 

	days -= LEAP_YEAR(year) ? 366 : 365;
	time -= days; // now it is days in this year, starting at 0

	days = 0;
	month = 0;
	monthLength = 0;
	for (month = 0; month < 12; month++)
	{
		if (month == 1)
		{
			if (LEAP_YEAR(year))
			{
				monthLength = 29;
			}
			else
			{
				monthLength = 28;
			}
		}
		else {
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

	tm.tm_mon = month + 1;  // jan is month 1  
	tm.tm_mday = time + 1;     // day of month
}


RTCTimerClass RTCTimer;