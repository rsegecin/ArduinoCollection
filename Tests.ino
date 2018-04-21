#include "Utils.h"
#include "SerialInterpreter.h"
#include "RTCTimer.h"
#include "MD5pm.h"

char serialBuffer[DEF_MSG_SIZE];

#define NUMBER_OF_COMMANDS	5
sSerialCommand SerialCommands[NUMBER_OF_COMMANDS];
SerialInterpreterClass SerialInterpreter(SerialCommands, NUMBER_OF_COMMANDS);

ISR(TIMER1_COMPA_vect)
{
	RTCTimer.OnInterrupt();
}

void serialEvent()
{
	if (Serial.available())
	{
		char c = Serial.read();
		SerialInterpreter.OnInterrupt(c);
		//Serial.print(c);
	}
}

void setup()
{
	sei();
	Serial.begin(115200);

	Serial.println("Welcome");

	SerialCommands[0].Name = "setdate";
	SerialCommands[0].ExecFunction = SetTime;

	SerialCommands[1].Name = "parsedate";
	SerialCommands[1].ExecFunction = ParseDate;

	SerialCommands[2].Name = "print";
	SerialCommands[2].ExecFunction = PrintTime;

	SerialCommands[3].Name = "md5";
	SerialCommands[3].ExecFunction = PrintMD5;

	SerialCommands[4].Name = "hex2byte";
	SerialCommands[4].ExecFunction = HexToByte;
}

void loop()
{
	//RTCTimer.DelayMili(1000, &DoWhatever);
	//PrintTime();
	if (SerialInterpreter.MessageReady)
	{
		if (SerialInterpreter.ExecFunction != nullptr)
			SerialInterpreter.ExecFunction();
		SerialInterpreter.ClearBuffer();
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
		sprintf(serialBuffer, "RTC has been set.");
		Serial.println(serialBuffer);
		PrintDateTime(RTCTimer.DateTime);
	}
	else
	{
		sprintf(serialBuffer, "Couldn't parse datetime %s.", SerialInterpreter.GetParameter(0));
		Serial.println(serialBuffer);
	}
}

void ParseDate()
{
	static sDateTime datetime;

	if (RTCTimer.ParseTime(datetime, SerialInterpreter.GetParameter(0)))
	{
		sprintf(serialBuffer, "Parsing.");
		Serial.println(serialBuffer);
		PrintDateTime(datetime);
	}
	else
	{
		sprintf(serialBuffer, "Couldn't parse datetime %s.", SerialInterpreter.GetParameter(0));
		Serial.println(serialBuffer);
	}
}

void PrintDateTime(sDateTime datetime)
{
	sprintf(serialBuffer, "printing: %i/%i/%i %i:%i:%i",
		datetime.DayOfMonth, datetime.Month, datetime.Year,
		datetime.Hours, datetime.Minutes, datetime.Seconds);
	Serial.println(serialBuffer);
}

void PrintTime()
{
	sDateTime conv;
	RTCTimer.BreakTime(RTCTimer.Time, conv);

	sprintf(serialBuffer, "now: %i/%i/%i %i:%i:%i",
		conv.DayOfMonth, conv.Month, conv.Year, conv.Hours, conv.Minutes, conv.Seconds);
	Serial.println(serialBuffer);
}

void PrintMD5() {
	byte hash[16];
	char strHash[33];

	MD5pm.MakeMD5((byte *) SerialInterpreter.GetParameter(0), strlen(SerialInterpreter.GetParameter(0)), hash);
	Utils.ByteToHexString(strHash, hash, 16);
	
	sprintf(serialBuffer, "Hash of %s is %s", SerialInterpreter.GetParameter(0), strHash);
	Serial.println(serialBuffer);
}

void HexToByte() {
	byte buffer[DEF_MSG_SIZE / 2];
	char aux[DEF_MSG_SIZE];

	uint16_t length = strlen(SerialInterpreter.GetParameter(0));

	Utils.HexToByteArray(buffer, SerialInterpreter.GetParameter(0), length);	
	Utils.ByteToHexString(aux, buffer, length / 2);

	sprintf(serialBuffer, "Byte array is %s", aux);
	Serial.println(serialBuffer);
}