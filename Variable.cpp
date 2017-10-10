#include "Variable.h"

VariableClass::VariableClass()
{
}


VariableClass::~VariableClass()
{
}

void VariableClass::MemberOne()
{
	sprintf(lBuffer, "time address one %p", (void *) &time);
	SerialInterpreter.Send(lBuffer);
}

void VariableClass::MemberTwo()
{
	sprintf(lBuffer, "time address two %p", (void *) &time);
	SerialInterpreter.Send(lBuffer);
}

VariableClass Variable;