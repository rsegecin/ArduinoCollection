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
			sDateTime conv;
			RTCTimer.BreakTime(RTCTimer.Time, conv);
			
			sprintf(lBuffer, "time now: %i:%i:%i",
				conv.Hours, conv.Minutes, conv.Seconds);
			SerialInterpreter.Send(lBuffer);

			SerialInterpreter.ClearBuffer();
			break;
		}
		default:
			break;
	}
}