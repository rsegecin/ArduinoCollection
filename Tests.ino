#include "SerialInterpreter.h"
#include "RTCTimer.h"
#include "MD5.h"

char lBuffer[DEF_MSG_SIZE];

#define NUMBER_OF_COMMANDS	4
sSerialCommand SerialCommands[NUMBER_OF_COMMANDS];
SerialInterpreterClass SerialInterpreter(SerialCommands, NUMBER_OF_COMMANDS);

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

	SerialCommands[0].Name = "setdate";
	SerialCommands[0].ExecFunction = SetTime;

	SerialCommands[1].Name = "parsedate";
	SerialCommands[1].ExecFunction = ParseDate;

	SerialCommands[2].Name = "print";
	SerialCommands[2].ExecFunction = PrintTime;

	SerialCommands[3].Name = "md5";
	SerialCommands[3].ExecFunction = PrintMD5;

	for (;;)
	{
		RTCTimer.DelayMili(1000, &DoWhatever);
		PrintTime();
	}
}

void DoWhatever()
{
	if (SerialInterpreter.MessageReady)
	{
		if (SerialInterpreter.ExecFunction != nullptr)
			SerialInterpreter.ExecFunction();
		SerialInterpreter.ClearBuffer();
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
}

void PrintDateTime(sDateTime datetime)
{
	sprintf(lBuffer, "printing: %i/%i/%i %i:%i:%i",
		datetime.DayOfMonth, datetime.Month, datetime.Year,
		datetime.Hours, datetime.Minutes, datetime.Seconds);
	SerialInterpreter.Send(lBuffer);
}

void PrintTime()
{
	sDateTime conv;
	RTCTimer.BreakTime(RTCTimer.Time, conv);

	sprintf(lBuffer, "now: %i/%i/%i %i:%i:%i",
		conv.DayOfMonth, conv.Month, conv.Year, conv.Hours, conv.Minutes, conv.Seconds);
	SerialInterpreter.Send(lBuffer);
}

void PrintMD5() {
	char Hash[33];

	MD5.MakeHash(Hash, SerialInterpreter.GetParameter(0));

	sprintf(lBuffer, "Hash of %s is %s", SerialInterpreter.GetParameter(0), Hash);
	SerialInterpreter.Send(lBuffer);
}