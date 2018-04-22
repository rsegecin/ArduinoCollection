#include "stdafx.h"
#include "conio.h"
#include <cstdlib>
#include <stdio.h>
#include <string.h>
#include <cstdlib>
#include <iostream>
#include "SerialInterpreter.h"

using namespace std;

#define NUMBER_OF_COMMANDS	4
sSerialCommand SerialCommands[NUMBER_OF_COMMANDS];
SerialInterpreter Serial(SerialCommands, NUMBER_OF_COMMANDS);

void SetTime()
{
    printf("\nSetTime,%s\n", Serial.GetParameter(0));
}

void PrintTime()
{
    printf("\nPrintTime.\n");
}

void PrintMD5()
{
    printf("\nPrintMD5 1:%s 2:%s.\n", Serial.GetParameter(0), Serial.GetParameter(1));
}

void HexToByte()
{
    printf("\nHexToByte\n");
}

int main()
{
    char lcAux;
            
    SerialCommands[0].Name = "settime";
    SerialCommands[0].ExecFunction = SetTime;

    SerialCommands[1].Name = "print";
    SerialCommands[1].ExecFunction = PrintTime;

    SerialCommands[2].Name = "md5";
    SerialCommands[2].ExecFunction = PrintMD5;

    SerialCommands[3].Name = "hex2byte";
    SerialCommands[3].ExecFunction = HexToByte;

    while (true)
    {
        cin >> lcAux;
        Serial.OnInterrupt(lcAux);

        if (Serial.MessageReady == true)
        {
            if (Serial.ExecFunction != nullptr)
                Serial.ExecFunction();
            Serial.ClearBuffer();
        }
    }

    return 0;
}
