#include "SerialInterpreter.h"
#include "RTCTimer.h"

char lBuffer[DEF_MSG_SIZE];
sDateTime t;

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
	SerialInterpreter.AddCommand("print", SerialInterpreterClass::eSerialCommands::nPrint);
	
	for (;;)
	{
		if (SerialInterpreter.MessageReady)
		{
			SerialHandler();
		}
	}
}

void SerialHandler()
{
	switch (SerialInterpreter.MessageCommand)
	{
		case SerialInterpreterClass::eSerialCommands::nSetDate:
		{
			if (RTCTimer.SetTime(SerialInterpreter.GetParameter(0)))
			{
				sprintf(lBuffer, "RTC has been set.");
				RTCTimer.BreakTime(RTCTimer.Time, t);
			}
			else
			{
				sprintf(lBuffer, "Couldn't parse datetime %s.", SerialInterpreter.GetParameter(0));
			}

			SerialInterpreter.Send(lBuffer);
			SerialInterpreter.ClearBuffer();
			break;
		}
		case SerialInterpreterClass::eSerialCommands::nPrint:
		{			
			sprintf(lBuffer, "%i:%i:%i", t.Hours, t.Minutes, t.Seconds);
			SerialInterpreter.Send(lBuffer);
			
			sprintf(lBuffer, "%i %i %i", t.DayOfMonth, t.Month, t.Year);
			SerialInterpreter.Send(lBuffer);
			SerialInterpreter.ClearBuffer();
			break;
		}
		default:
			break;
	}
}