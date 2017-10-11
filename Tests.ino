#include "SerialInterpreter.h"
#include "RTCTimer.h"

char lBuffer[DEF_MSG_SIZE];

ISR(TIMER1_COMPA_vect)
{
	RTCTimer.OnInterrupt();
}

ISR(USART_RX_vect)
{
	SerialInterpreter.OnInterrupt();
}

int main(void)
{
	sei();

	SerialInterpreter.Send("Welcome");

	SerialInterpreter.AddCommand("setdate", SerialInterpreterClass::eSerialCommands::nSetDate);
	SerialInterpreter.AddCommand("parsedate", SerialInterpreterClass::eSerialCommands::nParseDate);
	SerialInterpreter.AddCommand("print", SerialInterpreterClass::eSerialCommands::nPrint);

	for (;;)
	{
		RTCTimer.DelayMili(1000, &doWhatever);
		PrintTime();
	}
}

void doWhatever()
{
	if (SerialInterpreter.MessageReady)
	{
		SerialHandler();
	}
}

void SerialHandler()
{
	switch (SerialInterpreter.MessageCommand)
	{
	case SerialInterpreterClass::eSerialCommands::nSetDate:
	{
		SetTime();
		break;
	}
	case SerialInterpreterClass::eSerialCommands::nParseDate:
	{
		ParseDate();
		break;
	}
	case SerialInterpreterClass::eSerialCommands::nPrint:
	{
		PrintTime();
		break;
	}
	default:
		break;
	}
}

void SetTime()
{
	if (RTCTimer.SetTime(SerialInterpreter.GetParameter(0)))
	{
		sprintf(lBuffer, "RTC has been set.");
		SerialInterpreter.Send(lBuffer);
		PrintDateTime(RTCTimer.DateTime);
	}
	else
	{
		sprintf(lBuffer, "Couldn't parse datetime %s.", SerialInterpreter.GetParameter(0));
		SerialInterpreter.Send(lBuffer);
	}

	SerialInterpreter.ClearBuffer();
}

void ParseDate()
{
	static sDateTime datetime;

	if (RTCTimer.ParseTime(datetime, SerialInterpreter.GetParameter(0)))
	{
		sprintf(lBuffer, "Parsing.");
		SerialInterpreter.Send(lBuffer);
		PrintDateTime(datetime);
	}
	else
	{
		sprintf(lBuffer, "Couldn't parse datetime %s.", SerialInterpreter.GetParameter(0));
		SerialInterpreter.Send(lBuffer);
	}

	SerialInterpreter.ClearBuffer();
}

void PrintDateTime(sDateTime datetime)
{
	sprintf(lBuffer, "printing: %i/%i/%i %i:%i:%i",
		datetime.DayOfMonth, datetime.Month, datetime.Year,
		datetime.Hours, datetime.Minutes, datetime.Seconds);
	SerialInterpreter.Send(lBuffer);
	SerialInterpreter.ClearBuffer();
}

void PrintTime()
{
	sDateTime conv;
	RTCTimer.BreakTime(RTCTimer.Time, conv);

	sprintf(lBuffer, "now: %i/%i/%i %i:%i:%i",
		conv.DayOfMonth, conv.Month, conv.Year, conv.Hours, conv.Minutes, conv.Seconds);
	SerialInterpreter.Send(lBuffer);
	SerialInterpreter.ClearBuffer();
}