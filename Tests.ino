#include "Utils.h"
#include "SerialInterpreter.h"
#include "Timer0.h"
#include "MD5pm.h"
#include "CalendarHelper.h"

char serialBuffer[DEF_MSG_SIZE];

#define NUMBER_OF_COMMANDS	4
sSerialCommand SerialCommands[NUMBER_OF_COMMANDS];
SerialInterpreterClass SerialInterpreter(SerialCommands, NUMBER_OF_COMMANDS);

ISR(TIMER1_COMPA_vect)
{
	Timer0.OnInterrupt();
}

void serialEvent()
{
	if (Serial.available())
	{
		char c = Serial.read();
		SerialInterpreter.OnInterrupt(c);
	}
}

void setup()
{
	sei();
	Serial.begin(115200);

	Serial.println("Welcome");

	SerialCommands[0].Name = "settime";
	SerialCommands[0].ExecFunction = SetTime;

	SerialCommands[1].Name = "print";
	SerialCommands[1].ExecFunction = PrintTime;

	SerialCommands[2].Name = "md5";
	SerialCommands[2].ExecFunction = PrintMD5;

	SerialCommands[3].Name = "hex2byte";
	SerialCommands[3].ExecFunction = HexToByte;
}

void loop()
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
    CalendarHelperClass::ParseStrDateTime(Timer0.Time, SerialInterpreter.GetParameter(0));
    sprintf(serialBuffer, "RTC has been set.");
    Serial.println(serialBuffer);
}

void PrintTime()
{
	sDateTime conv;
    CalendarHelperClass::ConvertToDateTime(conv, Timer0.Time);

	sprintf(serialBuffer, "now: %i/%i/%i %i:%i:%i",
		conv.Day, conv.Month, conv.Year, conv.Hour, conv.Minute, conv.Second);
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