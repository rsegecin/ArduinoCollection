#pragma once
#include <Arduino.h>
#include <time.h>
#include "SerialInterpreter.h"

class VariableClass
{
public:
	VariableClass();
	~VariableClass();

	char lBuffer[DEF_MSG_SIZE];

	tm time;
	void MemberOne();
	void MemberTwo();
};

extern VariableClass Variable;
