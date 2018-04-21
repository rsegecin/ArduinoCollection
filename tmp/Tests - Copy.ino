#include "SerialInterpreter.h"
#include "RTCTimer.h"
#include "MD5.h"
#include <AESLib.h>
#include "AES.h"

char lBuffer[DEF_MSG_SIZE];

#define NUMBER_OF_COMMANDS	6
sSerialCommand SerialCommands[NUMBER_OF_COMMANDS];
SerialInterpreterClass SerialInterpreter(SerialCommands, NUMBER_OF_COMMANDS);

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

	SerialCommands[0].Name = "setdate";
	SerialCommands[0].ExecFunction = SetTime;

	SerialCommands[1].Name = "parsedate";
	SerialCommands[1].ExecFunction = ParseDate;

	SerialCommands[2].Name = "print";
	SerialCommands[2].ExecFunction = PrintTime;

	SerialCommands[3].Name = "md5";
	SerialCommands[3].ExecFunction = PrintMD5;

	SerialCommands[4].Name = "Encrypt";
	SerialCommands[4].ExecFunction = Encrypt;

	SerialCommands[5].Name = "crypt";
	SerialCommands[5].ExecFunction = crypt;
	
	crypt();

	for (;;)
	{
		RTCTimer.DelayMili(1000, &DoWhatever);
		//PrintTime();
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
		sprintf(lBuffer, "RTC has been set.");
		SerialInterpreter.Send(lBuffer);
		PrintDateTime(RTCTimer.DateTime);
	}
	else
	{
		sprintf(lBuffer, "Couldn't parse datetime %s.", SerialInterpreter.GetParameter(0));
		SerialInterpreter.Send(lBuffer);
	}
}

void ParseDate()
{
	static sDateTime datetime;

	if (RTCTimer.ParseTime(datetime, SerialInterpreter.GetParameter(0)))
	{
		sprintf(lBuffer, "Parsing.");
		SerialInterpreter.Send(lBuffer);
		PrintDateTime(datetime);
	}
	else
	{
		sprintf(lBuffer, "Couldn't parse datetime %s.", SerialInterpreter.GetParameter(0));
		SerialInterpreter.Send(lBuffer);
	}
}

void PrintDateTime(sDateTime datetime)
{
	sprintf(lBuffer, "printing: %i/%i/%i %i:%i:%i",
		datetime.DayOfMonth, datetime.Month, datetime.Year,
		datetime.Hours, datetime.Minutes, datetime.Seconds);
	SerialInterpreter.Send(lBuffer);
}

void PrintTime()
{
	sDateTime conv;
	RTCTimer.BreakTime(RTCTimer.Time, conv);

	sprintf(lBuffer, "now: %i/%i/%i %i:%i:%i",
		conv.DayOfMonth, conv.Month, conv.Year, conv.Hours, conv.Minutes, conv.Seconds);
	SerialInterpreter.Send(lBuffer);
}

void PrintMD5() {
	char Hash[129];

	MD5.MakeHash(Hash, SerialInterpreter.GetParameter(0));

	sprintf(lBuffer, "Hash of %s is %s", SerialInterpreter.GetParameter(0), Hash);
	SerialInterpreter.Send(lBuffer);
}

void ByteToHexString(char * hexStrParam, unsigned char * byteArrayParam, unsigned int byteArrayLength)
{
	unsigned char num;

	for (int i = 0, u = 0; i < byteArrayLength; i++, u++)
	{
		num = byteArrayParam[i] >> 4;
		hexStrParam[u] = (char)pgm_read_byte(HEX_VALUES + num);
		num = byteArrayParam[i] & 0xf;
		hexStrParam[++u] = (char)pgm_read_byte(HEX_VALUES + num);
	}
}

void Encrypt() {
	uint8_t key[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
	uint8_t iv[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
	char data[16];
	aes_context ctx;
	char auxBuffer[129];

	memset(data, 0x00, 16);
	memcpy(data, SerialInterpreter.GetParameter(0), strlen(SerialInterpreter.GetParameter(0)));

	ctx = aes128_cbc_enc_start((const uint8_t*)key, iv);
	aes128_cbc_enc_continue(ctx, data, 16);
	aes128_cbc_enc_finish(ctx);

	memset(auxBuffer, 0x00, 129);
	ByteToHexString(auxBuffer, (unsigned char *)data, sizeof(data));
	sprintf(lBuffer, "encrypted-cbc: %s", auxBuffer);
	SerialInterpreter.Send(lBuffer);

	ctx = aes128_cbc_dec_start((const uint8_t*)key, iv);
	aes128_cbc_dec_continue(ctx, data, 16);
	aes128_cbc_dec_finish(ctx);

	sprintf(lBuffer, "decrypted-cbc: %s", data);
	SerialInterpreter.Send(lBuffer);

	memset(auxBuffer, 0x00, 129);
	ByteToHexString(auxBuffer, (unsigned char *)iv, sizeof(iv));
	sprintf(lBuffer, "iv: %s", auxBuffer);
	SerialInterpreter.Send(lBuffer);
}

#define CBLOCK (2 * N_BLOCK)

void crypt() {
	AES aes;
	uint8_t key[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
	uint8_t iv[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
	byte tmp_iv[N_BLOCK];

	byte data[CBLOCK];
	byte ciphed[CBLOCK];
	byte deciphed[CBLOCK];

	char auxBuffer[129];

	SerialInterpreter.Send("Encrypting");

	memset(data, 0x00, CBLOCK);
	memcpy(data, "rinaldi massuda segecin", 23);
	
	int i, j = 0;

	for (i = 15; i >= 0; i--)
	{
		if (data[i] == 0)
			j++;
		else
			break;
	}

	memset(data + (i + 1), j, j);

	sprintf(lBuffer, "", i, j);
	SerialInterpreter.Send(lBuffer);

	ByteToHexString(auxBuffer, (unsigned char *)data, sizeof(data));
	sprintf(lBuffer, "i: %i j: %i, data: %s ", i, j, auxBuffer);
	SerialInterpreter.Send(lBuffer);

	memcpy(tmp_iv, iv, 16);
	aes.set_key(key, 128);
	byte test = aes.cbc_encrypt(data, ciphed, (CBLOCK / N_BLOCK), tmp_iv);

	if (test == SUCCESS)
	{
		SerialInterpreter.Send("Success");

		ByteToHexString(auxBuffer, (unsigned char *)ciphed, sizeof(ciphed));
		auxBuffer[32] = 0x00;
		sprintf(lBuffer, "encrypted-cbc: %s CBLOCK: %i", auxBuffer, CBLOCK);
		SerialInterpreter.Send(lBuffer);

		ByteToHexString(auxBuffer, (unsigned char *)tmp_iv, sizeof(tmp_iv));
		auxBuffer[32] = 0x00;
		sprintf(lBuffer, "tmp_iv: %s", auxBuffer);
		SerialInterpreter.Send(lBuffer);

		memcpy(tmp_iv, iv, 16);
		memset(deciphed, 0x00, 16);
		test = aes.cbc_decrypt(ciphed, deciphed, CBLOCK / N_BLOCK, tmp_iv);

		if (test == SUCCESS)
		{
			sprintf(lBuffer, "dencrypted-cbc: %s", deciphed);
			SerialInterpreter.Send(lBuffer);
		}
		else
		{
			SerialInterpreter.Send("Failure");
		}
	}
	else
	{
		SerialInterpreter.Send("Failure");
	}
}

////real iv = iv x2 ex: 01234567 = 0123456701234567
//unsigned long long int my_iv = 36753562;
//
//#define BLOCK_SIZE 17
//
//void crypt() {
//	AES aes;
//	byte key[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
//	byte iv[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
//	byte tmp_iv[N_BLOCK];
//	byte cipher[BLOCK_SIZE];
//	byte check[BLOCK_SIZE];
//	char auxBuffer[129];
//
//	byte plain[BLOCK_SIZE];
//	
//	memset(plain, 0x00, BLOCK_SIZE);
//	memcpy(plain, (uint8_t*)"rinaldi", 7);
//	
//	memcpy(tmp_iv, iv, 16);
//
//	sprintf(lBuffer, "sizeof %i", sizeof(tmp_iv));
//	SerialInterpreter.Send(lBuffer);
//
//	memset(auxBuffer, 0x00, 129);
//	ByteToHexString(auxBuffer, (unsigned char *)tmp_iv, sizeof(tmp_iv));
//	sprintf(lBuffer, "1 iv: %s", auxBuffer);
//	SerialInterpreter.Send(lBuffer);
//	
//	aes.do_aes_encrypt(plain, BLOCK_SIZE, cipher, key, 128, tmp_iv);
//
//	memset(auxBuffer, 0x00, 129);
//	ByteToHexString(auxBuffer, (unsigned char *)tmp_iv, sizeof(tmp_iv));
//	sprintf(lBuffer, "2 iv: %s size: %i pad: %i", auxBuffer, aes.get_size(), aes.get_pad());
//	SerialInterpreter.Send(lBuffer);
//	
//	memcpy(tmp_iv, iv, 16);
//
//	memset(auxBuffer, 0x00, 129);
//	ByteToHexString(auxBuffer, (unsigned char *)tmp_iv, sizeof(tmp_iv));
//	sprintf(lBuffer, "3 iv: %s", auxBuffer);
//	SerialInterpreter.Send(lBuffer);
//
//	aes.do_aes_decrypt(cipher, BLOCK_SIZE, check, key, 128, tmp_iv);
//	
//	memset(auxBuffer, 0x00, 129);
//	ByteToHexString(auxBuffer, (unsigned char *)tmp_iv, sizeof(tmp_iv));
//	sprintf(lBuffer, "4 iv: %s", auxBuffer);
//	SerialInterpreter.Send(lBuffer);
//
//	memset(auxBuffer, 0x00, 129);
//	ByteToHexString(auxBuffer, (unsigned char *)cipher, sizeof(cipher));
//	sprintf(lBuffer, "cipher: %s", auxBuffer);
//	SerialInterpreter.Send(lBuffer);
//
//	memset(auxBuffer, 0x00, 129);
//	memcpy(auxBuffer, check, 129);
//	sprintf(lBuffer, "check: %s", auxBuffer);
//	SerialInterpreter.Send(lBuffer);
//
//	memset(auxBuffer, 0x00, 129);
//	ByteToHexString(auxBuffer, (unsigned char *)tmp_iv, sizeof(tmp_iv));
//	sprintf(lBuffer, "iv: %s", auxBuffer);
//	SerialInterpreter.Send(lBuffer);
//}