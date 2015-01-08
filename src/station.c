#include <reg51.h>
#include <intrins.h>
#include "defs.h"

#define TIME				8
#define DECODEKEYSIZE		32

#define MAXUSERSCOUNT		124

/*
#define keyNOKEY	0
#define keyNUM0		1
#define keyNUM1		2
#define keyNUM2		3
#define keyNUM3		4
#define keyNUM4		5
#define keyNUM5		6
#define keyNUM6		7
#define keyNUM7		8
#define keyNUM8		9
#define keyNUM9		10
#define keyF1		11
#define keyF2		12
*/

#define keyNOKEY		0
#define keyVOLUP		1
#define keyVOLDOWN		2
#define keyCHUP			3
#define keyCHDOWN		4
#define keyFCSCAN		5
#define keyNUM0			6
#define keyNUM1			7
#define keyNUM2			8
#define keyNUM3			9
#define keyNUM4			10
#define keyNUM5			11
#define keyNUM6			12
#define keyNUM7			13
#define keyNUM8			14
#define keyNUM9			15
#define keyNUM100		16
#define keyONOFF		17
#define keySLEEP		18
#define keyMUTE			19
#define keyRECALL		20
#define keyPC			21
#define keyPCMEMO		22
#define keyPCSCAN		23
#define keyFCMEMO		24

#define MINLONGPULSETIME    0x2F
#define MAXSHORTPULSETIME   0xFF
#define MINSHORTPULSETIME   0x2F
#define REPEADHOLDDELAY		512
// 512
// #define SMALLKEYSSKIPDELAY  512
#define CODESCOUNT			25
#define KEYSSKIPDELAY		256

#define MAXPULSETIME		0x1A * 4
#define MAXLONGPULSETIME	10000
#define TIMETHREASHOULD 	0x1A



#define KEYSPORT	P1
#define VIDEOOUT	P0

// #define KEYSDELAY	512
#define TRANSMITINTERVAL	2000

#define MARKADDRESS			0
#define USERSCOUNTADDRESS	8
#define USERSDATAADDRESS	16

sbit InfraInput = P3 ^ 2;
sbit TransOut = P2 ^ 2;
sbit SCL = P2 ^ 4;
sbit SDA = P2 ^ 3;
sbit Digit1 = P2 ^ 5;
sbit Digit2 = P2 ^ 6;
sbit Digit3 = P2 ^ 7;

typedef union tagINFRACODE{
	BYTE AData[4];
	DWORD Data;
} INFRACODE;

code BYTE CODEKEY[DECODEKEYSIZE] = { 'D', 'I', 'G', 'I', 'T', 'A', 'L', 'S', '-', 'H', 'A', 'M', 'A', '-', '(', '0', '3', '3', ')', ' ', '2', '3', '9', '4', '8', '9', '-', 'R', 'K', '&', 'M', 'A' };

code BYTE DECODE[10] = { 0xc0, // CODE 0
0xf9, // CODE 1
0xa4, // CODE 2
0xb0, // CODE 3
0x99, // CODE 4
0x92, // CODE 5
0x82, // CODE 6
0xf8, // CODE 7
0x80, // CODE 8
0x90  // CODE 9
};

code BYTE INFRACODES[CODESCOUNT] = { 0x00,// keyNOKEY       0
0x37,// keyVOLUP		1
0x0f,// keyVOLDOWN		2
0x2f,// keyCHUP		3
0x1f,// keyCHDOWN		4
0xff,// keyFCSCAN		5
0x9f,// keyNUM0        6
0xf7,// keyNUM1        7
0x77,// keyNUM2        8
0xb7,// keyNUM3        9
0xcf,// keyNUM4        10
0x4f,// keyNUM5        11
0x8f,// keyNUM6        12
0xef,// keyNUM7        13
0x6f,// keyNUM8        14
0xaf,// keyNUM9        15
0x5f,// keyNUM100      16
0x07,// keyONOFF		17
0xe7,// keySLEEP		18
0x17,// keyMUTE		19
0xdf,// keyRECALL		20
0xd7,// keyPC			21
0x57,// keyPCMEMO		22
0x97,// keyPCSCAN		23
0x7f// keyFCMEMO		24
};


/*
code BYTE KEYCODES[13] = { 0xff, // keyNOKEY
0xd7, // keyNUM0
0xee, // keyNUM1
0xde, // keyNUM2
0xbe, // keyNUM3
0xed, // keyNUM4
0xdd, // keyNUM5
0xbd, // keyNUM6
0xeb, // keyNUM7
0xdb, // keyNUM8
0xbb, // keyNUM9
0xe7, // keyF1
0xb7, // keyF2
};
*/


unsigned char Video[3];

BYTE UserData[14];
WORD UserCode;

BYTE VideoCounter;
BYTE VideoState;
BYTE PressedKeyCode = keyNOKEY;
BYTE LastInfraKeyCode = keyNOKEY;
// WORD KeyDelayCounter;
WORD KeysTestDelayCounter = 0;
WORD ScanIntervalCounter;
WORD InRepeatDelayCounter = 0;

InfraRead0() interrupt 0 using 3
{
	register INFRACODE Data;
	register BYTE SearchData;
	register WORD LastPulseWidth;
	register BYTE I;

	Digit1 = 0;
	Digit2 = 0;
	Digit3 = 0;

	// Wait for end of LO pulse
	LastPulseWidth = 0;
	while ((InfraInput == 0) && (LastPulseWidth < MAXLONGPULSETIME)) LastPulseWidth++;
	if (LastPulseWidth >= MAXLONGPULSETIME) goto Finish;

	Data.Data = 0x00000000;

	// Wait for end of HI pulse
	LastPulseWidth = 0;
	while ((InfraInput == 1) && (LastPulseWidth < MAXLONGPULSETIME)) LastPulseWidth++;
	if (LastPulseWidth >= MAXLONGPULSETIME) goto Finish;

	for (I = 0; I < 32; I++){
		LastPulseWidth = 0;
		while ((InfraInput == 0) && (LastPulseWidth < MAXPULSETIME)) LastPulseWidth++;

		if (LastPulseWidth >= MAXPULSETIME) goto Finish;

		LastPulseWidth = 0;
		while ((InfraInput == 1) && (LastPulseWidth < MAXPULSETIME)) LastPulseWidth++;

		if (LastPulseWidth >= MAXPULSETIME){
			if (I == 0) goto RepeatCodeDetected;
			else goto Finish;
		}

		Data.Data = Data.Data << 1;
		if (LastPulseWidth > TIMETHREASHOULD) Data.Data = Data.Data | 0x00000001;
	}

	LastPulseWidth = 0;
	while ((InfraInput == 0) && (LastPulseWidth < MAXPULSETIME)) LastPulseWidth++;

	LastInfraKeyCode = keyNOKEY;

	if ((Data.AData[0] == 0x00) && (Data.AData[1] == 0xff)){
		SearchData = Data.AData[3];
		for (I = 0; I < CODESCOUNT; I++)
			if (SearchData == INFRACODES[I]){
				LastInfraKeyCode = I;
				InRepeatDelayCounter = REPEADHOLDDELAY;
				KeysTestDelayCounter = KEYSSKIPDELAY;
				EX0 = 0;
				break;
			};
	}

	PressedKeyCode = LastInfraKeyCode;
	goto Finish;

RepeatCodeDetected:

	if (InRepeatDelayCounter > 0){
		InRepeatDelayCounter = REPEADHOLDDELAY;
		KeysTestDelayCounter = KEYSSKIPDELAY;
		EX0 = 0;
		PressedKeyCode = LastInfraKeyCode;
	}
	else PressedKeyCode = keyNOKEY;

Finish:
	IE0 = 0; // clear another interrupt request created by series of pulses
}


Timer0() interrupt 1 using 1
{
	if (VideoCounter > 0) VideoCounter--;
	else {
		switch (VideoState){
		case 0:
			Digit3 = 0;
			VIDEOOUT = Video[0];
			Digit1 = 1;
			VideoState = 1;
			break;
		case 1:
			Digit1 = 0;
			VIDEOOUT = Video[1];
			Digit2 = 1;
			VideoState = 2;
			break;
		case 2:
			Digit2 = 0;
			VIDEOOUT = Video[2];
			Digit3 = 1;
			VideoState = 0;
			break;
		default:
			VideoState = 0;
		}
		VideoCounter = 5;
	}

	// if (TimeCounter > 0) TimeCounter--;

	if (ScanIntervalCounter > 0) ScanIntervalCounter--;
	if (InRepeatDelayCounter > 0) InRepeatDelayCounter--;
	if (KeysTestDelayCounter > 1) KeysTestDelayCounter--;
	else if (KeysTestDelayCounter == 1){
		KeysTestDelayCounter--;
		IE0 = 0;
		EX0 = 1;
	}
	/*
	if (KeyDelayCounter > 0) KeyDelayCounter--;
	else {
	if (PressedKeyCode == keyNOKEY){
	BYTE Temp1;
	KEYSPORT = 0xf0;
	Temp1 = KEYSPORT;
	if (Temp1 != 0xf0){
	BYTE Temp2;
	KEYSPORT = 0x0f;
	Temp2 = KEYSPORT;
	Temp1 = ((Temp1 & 0xf0) | (Temp2 & 0x0f));
	if (Temp1 != 0xff){
	for (Temp2 = 0; Temp2 < 13; Temp2++) if (Temp1 == KEYCODES[Temp2]) break;
	if (Temp2 != 13) {
	PressedKeyCode = Temp2;
	KeyDelayCounter = KEYSDELAY;
	}
	else PressedKeyCode = keyNOKEY;
	}
	}
	else KeyDelayCounter = 0;
	}
	}
	*/
}

BOOL Start(void)
{
	// Send START, defined as high-to-low SDA with SCL high.
	// Return with SCL, SDA low.

	SDA = 1;
	SCL = 1;
	if (!SDA || !SCL) return FALSE; // Verify bus available.
	_nop_(); // enforce setup delay and cycle delay
	SDA = 0;
	_nop_(); // enforce hold delay
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	SCL = 0;
	return TRUE;
}

void Stop(void)
{
	// Send STOP, defined as low-to-high SDA with SCL high.
	// SCL expected low on entry. Return with SCL, SDA high.

	SDA = 0;
	_nop_(); // enforce SCL low and data setup
	_nop_();
	SCL = 1;
	_nop_(); // enforce setup delay
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	SDA = 1;
}

BOOL ShiftOut(BYTE AValue)
{
	// Shift out a byte to the AT24Cxx, most significant bit first.
	// SCL, SDA expected low on entry. Return with SCL low.
	BOOL Result;
	BYTE BitCounter;
	BYTE Value = AValue;

	for (BitCounter = 0; BitCounter < 8; BitCounter++){
		SDA = (Value & 0x80);
		_nop_(); // enforce SCL low and data setup
		SCL = 1; // raise clock
		_nop_(); // enforce SCL high
		_nop_();
		_nop_();
		_nop_();
		SCL = 0; // drop clock
		Value = Value << 1;
	};

	SDA = 1; // release SDA for ACK
	_nop_(); // enforce SCL low and tAA
	_nop_();
	SCL = 1; // raise ACK clock
	_nop_(); // enforce SCL high
	_nop_();
	_nop_();
	_nop_();
	Result = !SDA;
	SCL = 0; // drop ACK clock
	return (Result);
}


BYTE ShiftIn(void)
{
	// Shift in a byte from the AT24Cxx, most significant bit first.
	// SCL expected low on entry. Return with SCL low.

	BYTE Value = 0;
	BYTE BitCounter;

	SDA = 1; // make SDA an input
	for (BitCounter = 0; BitCounter < 8; BitCounter++){
		Value = Value << 1;
		_nop_(); // enforce SCL low and data setup
		_nop_();
		_nop_();
		SCL = 1; // raise clock
		_nop_(); // enforce SCL high
		_nop_();
		if (SDA == 1) Value |= 0x01;
		SCL = 0; // drop clock
	}

	return (Value);
}

void ACK(void)
{
	// Clock out an acknowledge bit (low).
	// SCL expected low on entry. Return with SCL, SDA low.

	SDA = 0; // ACK bit
	_nop_(); // enforce SCL low and data setup
	_nop_();
	SCL = 1; // raise clock
	_nop_(); // enforce SCL high
	_nop_();
	_nop_();
	_nop_();
	SCL = 0; // drop clock
}


void NAK(void)
{
	// Clock out a negative acknowledge bit (high).
	// SCL expected low on entry. Return with SCL low, SDA high.

	SDA = 1; // NAK bit
	_nop_(); // enforce SCL low and data setup
	_nop_();
	SCL = 1; // raise clock
	_nop_(); // enforce SCL high
	_nop_();
	_nop_();
	_nop_();
	SCL = 0; // drop clock
}

BOOL WriteByte(BYTE APage, BYTE Address, BYTE AData)
{
	// AT24Cxx Byte Write function.
	// Does not wait for write cycle to complete.

	if (!Start()) goto Error2; // abort if bus not available
	if (!ShiftOut((APage << 1) | 0xa0)) goto Error1; // abort if no acknowledge
	if (!ShiftOut(Address)) goto Error1; // abort if no acknowledge
	if (!ShiftOut(AData)) goto Error1; // abort if no acknowledge
	Stop();
	return TRUE;
Error1:
	Stop();
Error2:
	return FALSE;
}


BOOL WriteBuffer(BYTE APage, BYTE Address, void *ABuffer, BYTE ASize)
{
	// AT24Cxx Byte Write function.
	// Does not wait for write cycle to complete.

	BYTE *BufferPtr = ABuffer;
	BYTE Counter;
	if (!Start()) goto Error2; // abort if bus not available
	if (!ShiftOut((APage << 1) | 0xa0)) goto Error1; // abort if no acknowledge
	if (!ShiftOut(Address)) goto Error1; // abort if no acknowledge

	for (Counter = 0; Counter < ASize; Counter++){
		if (!ShiftOut(*BufferPtr)) goto Error1; // abort if no acknowledge
		BufferPtr++;
	};

	Stop();
	return TRUE;
Error1:
	Stop();
Error2:
	return FALSE;
}

BOOL ReadByte(BYTE APage, BYTE Address, BYTE *AData)
{
	if (!Start()) goto Error2; // abort if bus not available
	if (!ShiftOut((APage << 1) | 0xa0)) goto Error1; // abort if no acknowledge
	if (!ShiftOut(Address)) goto Error1; // abort if no acknowledge
	if (!Start()) goto Error2; // abort if bus not available
	if (!ShiftOut((APage << 1) | 0xa1)) goto Error1; // abort if no acknowledge
	*AData = ShiftIn(); // receive data byte
	NAK(); // do not acknowledge byte
	Stop();
	return TRUE;
Error1:
	Stop();
Error2:
	return FALSE;
}

BOOL ReadBuffer(BYTE APage, BYTE Address, void *ABuffer, BYTE ASize)
{
	BYTE *BufferPtr = ABuffer;
	BYTE Counter;

	if (!Start()) goto Error2; // abort if bus not available
	if (!ShiftOut((APage << 1) | 0xa0)) goto Error1; // abort if no acknowledge
	if (!ShiftOut(Address)) goto Error1; // abort if no acknowledge
	if (!Start()) goto Error2; // abort if bus not available
	if (!ShiftOut((APage << 1) | 0xa1)) goto Error1; // abort if no acknowledge

	*BufferPtr = ShiftIn(); // receive data byte
	BufferPtr++;
	for (Counter = 1; Counter < ASize; Counter++){
		ACK();
		*BufferPtr = ShiftIn(); // receive data byte
		BufferPtr++;
	}
	NAK();
	Stop();
	return TRUE;
Error1:
	Stop();
Error2:
	return FALSE;
}

void Delay(INT ATime)
{
	TF1 = 0;
	TL1 = *(BYTE *)(((BYTE *)(&ATime)) + 0);
	TH1 = *(BYTE *)(((BYTE *)(&ATime)) + 1);
	TR1 = 1;
	while (!TF1);
}

void Transmit(void)
{
	BOOL PrevEA = EA;
	EA = 0;
	Digit1 = 0;
	Digit2 = 0;
	Digit3 = 0;


	TransOut = 0;
	Delay(-(TIME * 3));

	TransOut = 1;
	Delay(-(TIME));

	TransOut = 0;
	Delay(-(TIME));

	TransOut = 1;
	Delay(-(TIME / 2));

	{
		register BYTE I;
		register WORD BitMask = 0x8000;

		for (I = 0; I < 16; I++){
			TransOut = 0;
			Delay(-(TIME / 8));
			TransOut = 1;
			if ((UserCode & BitMask) == 0) Delay(-(TIME / 8));
			else Delay(-(TIME / 2));
			TransOut = 0;
			BitMask >>= 1;
		}
	}

	Delay(-(TIME / 2));

	{
		register BYTE I;
		register BYTE J;
		for (J = 0; J < 14; J++){
			register BYTE BitMask = 0x80;
			register BYTE Data = UserData[J];
			for (I = 0; I < 8; I++){
				TransOut = 0;
				Delay(-(TIME / 8)); // TIME / 2
				TransOut = 1;
				if ((Data & BitMask) == 0) Delay(-(TIME / 8)); // TIME / 8
				else Delay(-(TIME / 2)); // TIME / 2
				TransOut = 0;
				BitMask >>= 1;
			}
			Delay(-(TIME / 8));
		}
	}

	TransOut = 0;
	Delay(-(TIME / 2));
	TransOut = 1;

	EA = PrevEA;
}

void CodeBuffer(void *ABuffer, BYTE ASize, BOOL ADir)
{
	BYTE I;
	BYTE *BufferPtr = ABuffer;
	for (I = 0; I < ASize; I++){
		register BYTE SubKey;
		SubKey = CODEKEY[I & 0x1f] + I;
		if (ADir) *BufferPtr += SubKey;
		else *BufferPtr -= SubKey;
		BufferPtr++;
	}
}

BOOL RetryReadBuffer(WORD Address, void *ABuffer, BYTE ASize)
{
	register BYTE I;
	for (I = 0; I < 8; I++){
		Delay(100);
		if (ReadBuffer(((Address & 0xff00) >> 8), (Address & 0x00ff), ABuffer, ASize)) {
			CodeBuffer(ABuffer, ASize, FALSE);
			return TRUE;
		}
	};
	return FALSE;
}

BOOL RetryWriteBuffer(WORD Address, void *ABuffer, BYTE ASize)
{
	register BYTE I;
	BOOL Result = FALSE;
	CodeBuffer(ABuffer, ASize, TRUE);
	for (I = 0; I < 8; I++){
		Delay(100);
		if (WriteBuffer(((Address & 0xff00) >> 8), (Address & 0x00ff), ABuffer, ASize)) {
			Result = TRUE;
			break;
		}
	};
	CodeBuffer(ABuffer, ASize, FALSE);
	return Result;
}

void DisplayNumber(WORD AValue, BYTE ACursorMask, BYTE ADotMask)
{
	register BYTE Mask = 0x01;
	register BYTE DigitNo;

	for (DigitNo = 0; DigitNo < 3; DigitNo++){
		if (AValue > 0) {
			Video[DigitNo] = DECODE[(WORD)(AValue % 10)];
			AValue = AValue / 10;
		}
		else {
			if (DigitNo == 0) Video[0] = DECODE[0];
			else Video[DigitNo] = 0xff;
		}
		if (Mask & ACursorMask) Video[DigitNo] = 0xbf;
		if (Mask & ADotMask) Video[DigitNo] = (Video[DigitNo] & 0x7f);
		Mask = Mask << 1;
	}
}

void GetUserData(WORD AUserCode)
{
	WORD UserDataAddress = USERSDATAADDRESS + (AUserCode << 4);

	RetryReadBuffer(UserDataAddress, &UserData, 14);
}

void SetUserData(WORD AUserIndex)
{
	WORD UserDataAddress = USERSDATAADDRESS + (AUserIndex << 4);

	RetryWriteBuffer(UserDataAddress, &UserData, 14);
}

BOOL EditNumber(WORD *AValue, WORD AMinValue, WORD AMaxValue, BYTE ADotsMask)
{
	BOOL Result = FALSE;
	WORD Value = 0;
	WORD CursorMask = 0x07;
	BOOL Mode = 1;

	while (1){
		DisplayNumber(Value, CursorMask, ADotsMask);
		PressedKeyCode = keyNOKEY;
		while (PressedKeyCode == keyNOKEY);
	ReEnter:
		if (Mode){
			if ((PressedKeyCode >= keyNUM0) && (PressedKeyCode <= keyNUM9)){
				if (Value > 99) Value = Value % 100;
				Value = Value * 10 + (PressedKeyCode - keyNUM0);
				CursorMask = 0x00;
			}
			else if (PressedKeyCode == keyRECALL){
				if ((Value >= AMinValue) && (Value <= AMaxValue)){
					*AValue = Value;
					Result = TRUE;
					break;
				};
			}
			else if (PressedKeyCode == keyONOFF){
				Result = FALSE;
				break;
			}
			else if ((PressedKeyCode == keyCHUP) || (PressedKeyCode == keyCHUP)) {
				if (Value < AMinValue) Value = AMinValue;
				else if (Value > AMaxValue) Value = AMaxValue;
				CursorMask = 0x00;
				Mode = 0;
			}
		}
		else {
			if (PressedKeyCode == keyCHUP) {
				if (AMaxValue > Value) Value++;
			}
			else if (PressedKeyCode == keyCHDOWN) {
				if (AMinValue < Value) Value--;
			}
			else if ((PressedKeyCode >= keyNUM0) && (PressedKeyCode <= keyNUM9)){
				Value = 0;
				Mode = 1;
				goto ReEnter;
			}
			else if ((PressedKeyCode == keyRECALL) || (PressedKeyCode == keyONOFF)){
				Mode = 1;
				goto ReEnter;
			}
		}
	}

	PressedKeyCode = keyNOKEY;

	return Result;
}

/*
BOOL EditNumber( WORD *AValue, WORD AMinValue, WORD AMaxValue, BYTE ADotsMask)
{
BOOL Result = FALSE;
WORD Value = 0;
WORD CursorMask = 0x07;

while (1){
DisplayNumber( Value, CursorMask, ADotsMask);
PressedKeyCode = keyNOKEY;
while (PressedKeyCode == keyNOKEY);
if ((PressedKeyCode >= keyNUM0) && (PressedKeyCode <= keyNUM9)){
if (Value > 99) Value = Value % 100;
Value = Value * 10 + (PressedKeyCode - keyNUM0);
CursorMask = 0x00;
}
else if (PressedKeyCode == keyPC){
Result = TRUE;
break;
}
else if (PressedKeyCode == keyRECALL){
Result = FALSE;
break;
}
}

PressedKeyCode = keyNOKEY;

if (Result){
if ((Value >= AMinValue) && ( Value <= AMaxValue)) *AValue = Value;
else Result = FALSE;
}

return Result;
}
*/

BOOL IsMasked(BYTE AChanelNo, BOOL AChange)
{
	register BYTE Temp1 = (AChanelNo >> 3);
	register BYTE Temp2 = (0x01 << (AChanelNo & 0x07));
	if (AChange) UserData[Temp1] ^= Temp2;
	return (UserData[Temp1] & Temp2);
}

BOOL EditChanels()
{
	BOOL Changed = FALSE;
	BYTE ChanelNo = 2;
	BYTE I;
	while (1){
		DisplayNumber(ChanelNo, 0x00, 0x02 | ((BYTE)IsMasked(ChanelNo, FALSE)));
		PressedKeyCode = keyNOKEY;
		while (PressedKeyCode == keyNOKEY);
		switch (PressedKeyCode) {
		case keyCHUP:
			ChanelNo++;
			if (ChanelNo > 107) ChanelNo = 2;
			break;
		case keyCHDOWN:
			ChanelNo--;
			if (ChanelNo < 2) ChanelNo = 107;
			break;
		case keyPCMEMO:
			IsMasked(ChanelNo, TRUE);
			Changed = TRUE;
			break;
		case keyFCSCAN:
			for (I = 0; I < 14; I++) UserData[I] = 0x00;
			Changed = TRUE;
			break;
		case keyFCMEMO:
			for (I = 0; I < 14; I++) UserData[I] = 0xff;
			Changed = TRUE;
			break;
		case keyRECALL:
			return Changed;
			break;
		case keyONOFF:
			return FALSE;
			break;
		}
	}
}

/*
BOOL EditChanels()
{
BOOL Changed = FALSE;
BYTE ChanelNo = 2;
while (1){
DisplayNumber( ChanelNo, 0x00, 0x02 | ((BYTE)IsMasked( ChanelNo, FALSE)));
PressedKeyCode = keyNOKEY;
while (PressedKeyCode == keyNOKEY);
switch (PressedKeyCode) {
case keyCHUP:
ChanelNo ++;
if (ChanelNo > 107) ChanelNo = 2;
break;
case keyCHDOWN:
ChanelNo --;
if (ChanelNo < 2) ChanelNo = 107;
break;
case keyPCMEMO:
IsMasked( ChanelNo, TRUE);
Changed = TRUE;
break;
case keyFCSCAN:
{
register BYTE I;
for ( I = 0; I < 14; I++) UserData[I] = 0x00;
}
Changed = TRUE;
break;
case keyFCMEMO:
{
register BYTE I;
for ( I = 0; I < 14; I++) UserData[I] = 0xff;
}
Changed = TRUE;
break;
case keyRECALL:
return Changed;
case keyONOFF:
return FALSE;
}
}
}
*/

void ClearData(void)
{
	BYTE I;
	for (I = 0; I < 14; I++) UserData[I] = 0xff;
	for (UserCode = 0; UserCode < MAXUSERSCOUNT; UserCode++) {
		DisplayNumber(UserCode, 0x00, 0x07);
		SetUserData(UserCode);
	}
}

main()
{
	// KeyDelayCounter = 0;
	VideoCounter = 0;
	VideoState = 0;
	PressedKeyCode = keyNOKEY;
	// KeyDelayCounter = 0;
	ScanIntervalCounter = 0;

	IP = 0x01;
	IT0 = 1;

	TMOD = 0x12;
	TL0 = (-250);
	TH0 = (-250);
	TR0 = 1;
	ET0 = 1;
	EX0 = 1;
	EA = 1;

	DisplayNumber(0, 0x07, 0x07);

	Delay(1000);

	if (PressedKeyCode == keyRECALL){
		WORD MemMark = 12345;
		ClearData();
		RetryWriteBuffer(MARKADDRESS, &MemMark, 2);
	}
	else {
		WORD MemMark;
		RetryReadBuffer(MARKADDRESS, &MemMark, 2);
		if (MemMark != 12345){
			ClearData();
			MemMark = 12345;
			RetryWriteBuffer(MARKADDRESS, &MemMark, 2);
		}
	};

	DisplayNumber(0, 0x07, 0x00);

	while (1){
		PressedKeyCode = keyNOKEY;
		while (PressedKeyCode == keyNOKEY){
			if (ScanIntervalCounter == 0){
				DisplayNumber(UserCode, 0x00, 0x00);
				Transmit();
				ScanIntervalCounter = TRANSMITINTERVAL;
				UserCode++;
				if (UserCode >= MAXUSERSCOUNT) UserCode = 0;
				GetUserData(UserCode);
			}
		};

		if (PressedKeyCode == keyONOFF){
			if (EditNumber(&UserCode, 0, MAXUSERSCOUNT - 1, 0x01)){
				GetUserData(UserCode);
				if (EditChanels()) SetUserData(UserCode);
				else GetUserData(UserCode);
			}
		}
	}
}