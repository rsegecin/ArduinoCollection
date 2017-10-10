#include "SerialInterpreter.h"
#include "RTCTimer.h"
#include "Variable.h"

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
			/*if (RTCTimer.SetTime(SerialInterpreter.GetParameter(0)))
			{
				sprintf(lBuffer, "RTC has been set.");
			}
			else
			{
				sprintf(lBuffer, "Couldn't parse datetime.");
			}*/
			
			RTCTimer.SetTime(SerialInterpreter.GetParameter(0));

			sprintf(lBuffer, "Setting date %d:%d:%d and the date is %d/%d/%d %p",
				RTCTimer.TimeModel.tm_hour, RTCTimer.TimeModel.tm_min, RTCTimer.TimeModel.tm_sec,
				RTCTimer.TimeModel.tm_mday, RTCTimer.TimeModel.tm_mon, RTCTimer.TimeModel.tm_year, (void *) &RTCTimer.TimeModel);

			SerialInterpreter.Send(lBuffer);
			SerialInterpreter.ClearBuffer();
			break;
		}
		case SerialInterpreterClass::eSerialCommands::nPrint:
		{
			tm  t;
			RTCTimer.BreakTime(RTCTimer.MainTime, t);
			
			strftime(lBuffer, sizeof(lBuffer), "%d/%m/%Y %H:%M:%S", &t);

			/*sprintf(lBuffer, "Printing date %d:%d:%d and the date is %d/%d/%d %p",
				RTCTimer.STm.tm_hour, RTCTimer.STm.tm_min, RTCTimer.STm.tm_sec,
				RTCTimer.STm.tm_mday, RTCTimer.STm.tm_mon, RTCTimer.STm.tm_year, (void *)&RTCTimer.STm);*/

			SerialInterpreter.Send(lBuffer);
			
			SerialInterpreter.ClearBuffer();
			break;
		}
		default:
			break;
	}
}