#include "stdafx.h"
#include "SerialInterpreter.h"

SerialInterpreter::SerialInterpreter(sSerialCommand * pSerialCommands, int pNumberOfCommands)
{
    SerialCommands = pSerialCommands;
    NumberOfCommands = pNumberOfCommands;
    
    ClearBuffer();
}

void SerialInterpreter::ClearBuffer(void)
{
    SetInitialState();

    memset(MessageBuffer, 0x00, DEF_MSG_SIZE);
}

char * SerialInterpreter::GetParameter(unsigned char pIndex)
{
    unsigned char num;
    unsigned char parameters = 0;
    char *ret_aux = NULL;

    for (num = 0; num < DEF_MSG_SIZE; num++)
    {
        if (MessageBuffer[num] != 0x00)
        {
            if (parameters == pIndex)
            {
                ret_aux = &MessageBuffer[num];
            }

            while ((MessageBuffer[num] != 0x00) && (num < DEF_MSG_SIZE)) { num++; }

            parameters++;
        }
    }

    return ret_aux;
}

void SerialInterpreter::SetInitialState()
{
    MessageReady = false;
    ProcessingState = (Preamble) ? eSerialState::Ready : eSerialState::PCommand;
    MatchingCmd = 0;
    MsgPosition = 0;
}

void SerialInterpreter::SetCommandState()
{
    ProcessingState = eSerialState::PCommand;
    MatchingCmd = 0;
    MsgPosition = 0;
}

void SerialInterpreter::SetParametersState()
{
    ProcessingState = eSerialState::PParameters;
    MsgPosition = 0;
}

void SerialInterpreter::SetDoneState()
{
    ProcessingState = eSerialState::Done;
    MessageReady = true;
    ExecFunction = SerialCommands[MatchingCmd].ExecFunction;
}

void SerialInterpreter::OnInterrupt(char pChar)
{
    if (MessageReady == false)
    {
        switch (ProcessingState)
        {
        case 0:
            CheckPreamble(pChar);
            break;
        case 1:
            ProcessCommand(pChar);
            break;
        case 2:
            ProcessParameters(pChar);
            break;
        default:
            break;
        }
    }
}

void SerialInterpreter::SetPreamble(bool pValue)
{
    Preamble = pValue;
    SetInitialState();
}

void SerialInterpreter::CheckPreamble(char pChar)
{
    if (pChar == '$')
    {
        flagD = true;
    }
    else if ((flagD == true) && (pChar == '%'))
    {
        flagDA = true;
        flagD = false;
    }
    else
    {
        flagD = false;

        if (flagDA == true)
        {
            flagDA = false;
            SetCommandState();
            ProcessCommand(pChar);
        }
    }
}

// Checks if the string Name of the current matching command matches in full with another command Name on the list
// Or checks if the string Name of the current matching command matches partially with another command Name on the list
// It also automatically sets MatchingCmd with the matched command
bool SerialInterpreter::CommandExists(bool pAllMatch, char pChar)
{
    char * strMatchingCmd;
    char * strCmd;
    int num;
    bool hasCmd = false;

    strMatchingCmd = SerialCommands[MatchingCmd].Name;

    for (num = MatchingCmd + 1; num < NumberOfCommands; num++)
    {
        strCmd = SerialCommands[num].Name;
        if ((strncmp(strMatchingCmd, strCmd, MsgPosition) == 0) &&
            (((pAllMatch == true) && (strlen(strCmd) == MsgPosition)) ||
            ((pAllMatch == false) && (pChar == strCmd[MsgPosition]))))
        {
            MatchingCmd = num;
            hasCmd = true;
            break;
        }
    }

    return hasCmd;
}

void SerialInterpreter::ProcessCommand(char pChar)
{
    char * strglbMatchingCmd;
    bool hasCmd;

    strglbMatchingCmd = SerialCommands[MatchingCmd].Name;

    if (pChar == strglbMatchingCmd[MsgPosition])
    {
        MsgPosition++;
    }
    else
    {
        if (pChar == '$')
        {
            flagD = true;
        }
        else if ((flagD == true) && (pChar == '%'))
        {
            flagD = false;

            if (MsgPosition == strlen(strglbMatchingCmd))
            {
                SetDoneState();
            }
            else
            {
                hasCmd = CommandExists(true, NULL);

                if (hasCmd == true)
                {
                    SetDoneState();
                }
                else
                {
                    SetInitialState();
                }
            }
        }
        else if (((pChar == ',') || (pChar == ';') || (pChar == '=')))
        {
            flagD = false;

            if (MsgPosition == strlen(strglbMatchingCmd))
            {
                SetParametersState();
            }
            else
            {
                hasCmd = CommandExists(true, NULL);

                if (hasCmd == true)
                {
                    SetParametersState();
                }
                else
                {
                    SetInitialState();
                }
            }
        }
        else
        {
            flagD = false;

            hasCmd = CommandExists(false, pChar);

            if (hasCmd == true)
            {
                MsgPosition++;
            }
            else
            {
                SetInitialState();
                CheckPreamble(pChar);
            }
        }
    }
}

void SerialInterpreter::ProcessParameters(char pChar)
{
    if ((pChar == ',') || (pChar == ';') || (pChar == '='))
    {
        MessageBuffer[MsgPosition] = 0x00;
        MsgPosition++;
    }
    else
    {
        if (pChar == '$')
        {
            flagD = true;
        }
        else if ((flagD == true) && (pChar == '%'))
        {
            MessageBuffer[MsgPosition] = 0x00;
            SetDoneState();
            flagD = false;
        }
        else
        {
            flagD = false;
            MessageBuffer[MsgPosition] = pChar;
            MsgPosition++;
        }
    }
}
