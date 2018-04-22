 #ifndef __SERIALETRXLIB_H__
	#define __SERIALETRXLIB_H__

//----------------------------------------------------------------------------------------------------------------------
//	SERIAL ETRX LIB
//----------------------------------------------------------------------------------------------------------------------
/*
	Essa função tem por objetivo responder a todos as necessidades relativas a comunicação entre o ETRX e o 
	firmware.
	
	Pré-requisitos:
		- 

	Pós-requisitos:
		- 

	Objetivos:
		- 
*/
//----------------------------------------------------------------------------------------------------------------------
//	FUSES and USES
//----------------------------------------------------------------------------------------------------------------------

	//#use rs232(baud = 19200, UART1 , stream = SERIAL1 , ERRORS )

//----------------------------------------------------------------------------------------------------------------------
//	DEFINIÇÕES
//----------------------------------------------------------------------------------------------------------------------
	
	#define		defNumCmdETRX				5
	#define 	defTAMANHO_BUFFER_ETRX 		40		//Define: o tamanho do BUFFER responsável por armazenar as mensagens capturadas provenientes do ETRX

//----------------------------------------------------------------------------------------------------------------------
//	ESTRUTURAS
//----------------------------------------------------------------------------------------------------------------------

	typedef struct _sCOMANDO
	{
		char *m_NomeDoComando;
		void (*m_Handler)( );
	} sCOMANDO;

//----------------------------------------------------------------------------------------------------------------------
//	ASSINATURA DE FUNÇÕES
//----------------------------------------------------------------------------------------------------------------------
	
	//<INTERFACE>
	void 	Lib_Serial				(	);
	void	AdicionaComando			( char *prmNome , void ( *prmHandler ) ( void ) );
	//int8	*PegaParametro			( int indice );
	//</INTERFACE>
/*
	void 	ProcessMsg				(	);
	void 	ExecuteCommand			(	);
	void 	GotoInitialState		(	);
	void 	GotoGetCommandState		(	);
	void 	GotoGetParametersState	(	);
	void 	VerifyFirstCharacter	( char prmCaracter );
	int1 	VerifyAntoherCommand	( int1 prmAllMatch , char prmCaracter );
	void 	VerifyCommand			( char prmCaracter );
	void 	VerifyParameters		( char prmCaracter );*/

//----------------------------------------------------------------------------------------------------------------------
//	VARIÁVEIS GLOBAIS
//----------------------------------------------------------------------------------------------------------------------
	
	static sCOMANDO			glbTabelaComandos [ ( defNumCmdETRX + defNumComandos ) ];		//Lista de comandos suportados pela aplicação
	static volatile int 	glbMsgState = 0;
	static volatile int 	glbMatchingCmd = -1;
	static volatile int 	glbMsgPosition = 0;
	static volatile int8	glbParameters[defTAMANHO_BUFFER_ETRX];

//----------------------------------------------------------------------------------------------------------------------
//	INTERRUPÇÕES
//----------------------------------------------------------------------------------------------------------------------

	//#INT_RDA
	void SerialMsg()
		{
		//disable_interrupts(INT_RDA);
		ProcessMsg();
		//enable_interrupts(INT_RDA);
		}

//----------------------------------------------------------------------------------------------------------------------
//	FUNÇÕES
//----------------------------------------------------------------------------------------------------------------------

	void Lib_Serial()
		{
		ClearBuffer();
		}
	
	void AdicionaComando(char *prmNome, void (*prmHandler)(void))
		{
		int num;
		num = strlen(glbTabelaComandos);
		
		glbTabelaComandos[num].m_NomeDoComando = prmNome;
		glbTabelaComandos[num].m_Handler = prmHandler;
		}

	int8 *PegaParametro(int indice)
		{
		}

	void ClearBuffer()
		{
		glbMsgState =  0;
		glbMatchingCmd = -1;
		glbMsgPosition = 0;
		glbMsgParam = 0;
				
		//memset ( glbParameters , 0x00 , defTAMANHO_BUFFER_ETRX );
		for (i = 0; i < BUFFER_SIZE; i++)
			{
			msgParameters[i] = 0x00;
			}
		}

	void ProcessMsg()
		{
		char lcAux;
		
		cin >> lcAux;
		//lcAux = fgetc(SERIAL1);
		
		if (glbMsgPosition > defTAMANHO_BUFFER_ETRX )
			{
			ClearBuffer();
			}

		switch (glbMsgState)
			{
			case 0:
				VerifyFirstCharacter(aux);
				break;
			case 1:
				VerifyCommand(aux);
				break;
			case 2:
				VerifyParameters(aux);
				break;
			default:
				break;
			}
		}

	void GotoInitialState()
		{
		glbMsgState = 0;
		glbMsgPosition = 0;
		glbMatchingCmd = -1;
		}
	
	void GotoGetCommandState()
		{
		glbMsgState = 1; // Estado para pegar os próximos caracteres
		glbMsgPosition = 0;
		glbMatchingCmd = 0;
		}

	void GotoGetParametersState()
		{
		glbMsgState = 2; // Estado para pegar parametros
		glbMsgPosition = 0; // Reseta a posição para a variável ser reusada no Estado de Parâmetros
		}

	void ExecuteCommand()
		{
		glbTabelaComandos[glbMatchingCmd].m_Handler();
		ClearBuffer();
		}
	
	void VerifyFirstCharacter(char prmCaracter)
	{	
		if (prmCaracter == 0x0A)
		{
			GotoGetCommandState();
		}
	}

	// Verifica se há outro comando na lista
	// Caso necessite uma comparação apenas das primeira letras dos comandos da lista 
	// o prmAllMatch será falso e o caractere será o caractere recebido no serial
	// Caso contrário será feito uma comparação exata de todos os caracteres dos comandos da lista
	int1 VerifyAntoherCommand(int1 prmAllMatch, char prmCaracter)
	{
		char * strCmd;
		char * strglbMatchingCmd;
		int num;
		int1 hasCmd = false;
		
		//strglbMatchingCmd = (char *)glbComandos[glbMatchingCmd];
		strglbMatchingCmd = (char *)glbTabelaComandos[glbMatchingCmd].m_NomeDoComando;

		for (num = glbMatchingCmd + 1; num < defNUMBER_OF_glbComandos; num++)
		{
			strCmd = (char *)glbTabelaComandos[num];
			if ((strncmp(strglbMatchingCmd, strCmd, glbMsgPosition) == 0) &&
				(((prmAllMatch == true) && (strlen(strCmd) == glbMsgPosition)) || 
				((prmAllMatch == false) && (prmCaracter == strCmd[glbMsgPosition]))))
			{
				glbMatchingCmd = num;
				hasCmd = true;
				break;
			}
		}

		return hasCmd;
	}

	void VerifyCommand(char prmCaracter)
	{
		char * strCmd;
		char * strglbMatchingCmd;
		int1 hasCmd;
		
		//strglbMatchingCmd = (char *)glbComandos[glbMatchingCmd];
		strglbMatchingCmd = (char *)glbTabelaComandos[glbMatchingCmd].m_NomeDoComando;
		
		if (prmCaracter == strglbMatchingCmd[glbMsgPosition])
		{
			glbMsgPosition++;
		}
		else 
		if (((prmCaracter == ',') || (prmCaracter == ':') || (prmCaracter == ';') || (prmCaracter == '=')) && 
			(glbMsgPosition == strlen(strglbMatchingCmd)))
		{
			GotoGetParametersState();
		}
		else 
		if ((prmCaracter == 0x0D) && (glbMsgPosition == strlen(strglbMatchingCmd)))
		{
			ExecuteCommand();
		}
		else 
		if ((prmCaracter == ',') || (prmCaracter == ':') || (prmCaracter == ';') || (prmCaracter == '=') || (prmCaracter == 0x0D))
		{
			hasCmd = VerifyAntoherCommand(true, NULL);

			if (hasCmd == true)
			{
				if (prmCaracter == 0x0D)
				{
					ExecuteCommand();
				}
				else
				{
					GotoGetParametersState();
				}
			}
			else
			{
				GotoInitialState();
			}
		}
		else
		{
			hasCmd = VerifyAntoherCommand(false, prmCaracter);

			if (hasCmd == true)
			{
				glbMsgPosition++;
			}
			else
			{
				GotoInitialState();
				VerifyFirstCharacter(prmCaracter);
			}
		}
	}

	void VerifyParameters(char prmCaracter)
	{
		if ((prmCaracter == ',') || (prmCaracter == ':') || (prmCaracter == ';') || (prmCaracter == '='))
		{
			glbParameters[glbMsgPosition] = 0x00;
			glbMsgPosition++;
		}
		else 
		if (prmCaracter == 0x0D)
		{
			glbParameters[glbMsgPosition] = 0x00;
			ExecuteCommand();
		}
		else
		{
			glbParameters[glbMsgPosition] = prmCaracter;
			glbMsgPosition++;
		}
	}


//----------------------------------------------------------------------------------------------------------------------
//	END OF FILE
//----------------------------------------------------------------------------------------------------------------------

#endif