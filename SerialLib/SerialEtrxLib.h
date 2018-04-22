#include "stdafx.h"
#include "conio.h"
#include <cstdlib>
#include <stdio.h>
#include <string.h>
#include <cstdlib>
#include <iostream>

using namespace std;

#define		defNumComandos				4
#define 	defTAMANHO_BUFFER_ETRX 		40

struct sSerialCommand
{
    char *Name;
    void(*ExecFunction)(void) = nullptr;
};

void 	Lib_Serial();
void	AdicionaComando(char *pNome, void(*pHandler) (void));
char	*PegaParametro(int indice);

sSerialCommand			glbTabelaComandos[(defNumComandos)];		//Lista de comandos suportados pela aplicação
volatile bool	MsgReady = 0;		//Serial message's flag
volatile int 	ProcessingState = 0;
volatile int 	MatchingCmd = -1;
volatile int 	MsgPosition = 0;
volatile int 	NumComandos = 0;
char			MessageBuffer[defTAMANHO_BUFFER_ETRX];
volatile bool flagD;

void AdicionaComando(char *pNome, void(*pHandler)(void))
{
    glbTabelaComandos[NumComandos].Name = pNome;
    glbTabelaComandos[NumComandos].ExecFunction = pHandler;
    NumComandos++;
}

char *PegaParametro(int indice)
{
    int num;
    int parametros = 0;
    char *retorno = NULL;

    for (num = 0; num < defTAMANHO_BUFFER_ETRX; num++)
    {
        if (MessageBuffer[num] != 0x00)
        {
            parametros++;
            if (parametros == indice)
            {
                retorno = &MessageBuffer[num];
            }
            while ((MessageBuffer[num] != 0x00) && (num < defTAMANHO_BUFFER_ETRX)) { num++; }
        }
    }

    return retorno;
}

void ClearBuffer()
{
    int i;

    MsgReady = false;
    ProcessingState = 0;
    MatchingCmd = -1;
    MsgPosition = 0;

    //memset ( glbParameters , 0x00 , defTAMANHO_BUFFER_ETRX );
    for (i = 0; i < defTAMANHO_BUFFER_ETRX; i++)
    {
        MessageBuffer[i] = 0x00;
    }
}

void SetInitialState()
{
    //enable_interrupts(INT_RDA);
    MsgReady = false;
    ProcessingState = 0;
    MsgPosition = 0;
    MatchingCmd = -1;
}

void SetCommandState()
{
    ProcessingState = 1; // Estado para pegar os próximos caracteres
    MsgPosition = 0;
    MatchingCmd = 0;
}

void SetParametersState()
{
    ProcessingState = 2; // Estado para pegar parametros
    MsgPosition = 0; // Reseta a posição para a variável ser reusada no Estado de Parâmetros
}

void SetDoneState()
{
    ProcessingState = -1; // Estado de espera
    MsgReady = true;
}

void ExecuteCommand()
{
    glbTabelaComandos[MatchingCmd].ExecFunction();
    ClearBuffer();
}

void VerifyFirstCharacter(char pChar)
{
    if (pChar == '$')
    {
        flagD = true;
    }
    else if ((flagD == true) && (pChar == '%'))
    {
        SetCommandState();
        flagD = false;
    }
    else
    {
        flagD = false;
    }
}

// Verifica se há outro comando na lista
// Caso necessite uma comparação apenas das primeira letras dos comandos da lista 
// o pAllMatch será falso e o caractere será o caractere recebido no serial
// Caso contrário será feito uma comparação exata de todos os caracteres dos comandos da lista
bool CommandExists(bool pAllMatch, char pCaracter)
{
    char * strglbMatchingCmd;
    char * strCmd;
    int num;
    bool hasCmd = false;

    //strglbMatchingCmd = (char *)glbComandos[glbMatchingCmd];
    strglbMatchingCmd = (char *)glbTabelaComandos[MatchingCmd].Name;

    for (num = MatchingCmd + 1; num < (defNumComandos); num++)
    {
        strCmd = (char *)glbTabelaComandos[num].Name;
        if ((strncmp(strglbMatchingCmd, strCmd, MsgPosition) == 0) &&
            (((pAllMatch == true) && (strlen(strCmd) == MsgPosition)) ||
            ((pAllMatch == false) && (pCaracter == strCmd[MsgPosition]))))
        {
            MatchingCmd = num;
            hasCmd = true;
            break;
        }
    }

    return hasCmd;
}

void ProcessCommand(char pChar)
{
    char * strglbMatchingCmd;
    bool hasCmd;

    strglbMatchingCmd = (char *)glbTabelaComandos[MatchingCmd].Name;

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
                VerifyFirstCharacter(pChar);
            }
        }
    }
}

void ProcessParameters(char pCaracter)
{
    if ((pCaracter == ',') || (pCaracter == ';') || (pCaracter == '='))
    {
        MessageBuffer[MsgPosition] = 0x00;
        MsgPosition++;
    }
    else
    {
        if (pCaracter == '$')
        {
            flagD = true;
        }
        else if ((flagD == true) && (pCaracter == '%'))
        {
            MessageBuffer[MsgPosition] = 0x00;
            SetDoneState();
            flagD = false;
        }
        else
        {
            flagD = false;
            MessageBuffer[MsgPosition] = pCaracter;
            MsgPosition++;
        }
    }
}

void ProcessMsg()
{
    char lcAux;

    cin >> lcAux;
    //lcAux = fgetc(SERIAL1);

    if (MsgReady == false)
    {
        if (MsgPosition > defTAMANHO_BUFFER_ETRX)
        {
            ClearBuffer();
        }

        switch (ProcessingState)
        {
        case 0:
            VerifyFirstCharacter(lcAux);
            break;
        case 1:
            ProcessCommand(lcAux);
            break;
        case 2:
            ProcessParameters(lcAux);
            break;
        default:
            break;
        }
    }
}

//#INT_RDA
void SerialMsg()
{
    //disable_interrupts(INT_RDA);
    ProcessMsg();
    //if (glbMsgReady == false)
    //enable_interrupts(INT_RDA);
}

void Lib_Serial()
{
    ClearBuffer();
}

