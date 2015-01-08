#include <reg52.h>
#include <intrins.h>
#include "defs.h"

sbit Test = P3 ^ 1;
sbit Digit1 = P1 ^ 7;
sbit Digit2 = P1 ^ 6;
sbit Digit3 = P1 ^ 5;
sbit Volume = P1 ^ 0;
sbit Mute = P3 ^ 7;
sbit TunerData = P3 ^ 0;
sbit TunerClock = P2 ^ 1;
sbit OnOffKey = P0 ^ 0;
sbit ChUpKey = P0 ^ 1;
sbit ChDownKey = P0 ^ 2;
sbit VolUpKey = P0 ^ 3;
sbit VolDownKey = P0 ^ 4;
sbit KeysCommon = P1 ^ 4;
sbit Relay = P2 ^ 7;
sbit InfraInput = P3 ^ 2;
sbit CommInput = P3 ^ 3;
sbit SCL = P1 ^ 1;
sbit SDA = P1 ^ 2;

#define DISPLAY			P0

#define CODESCOUNT		25

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



#define TIMETHREASHOULD     0x15
//0x20
#define MAXPULSETIME        0x80
//0x20
#define MAXLONGPULSETIME    0xFF
//0xF0
#define MINLONGPULSETIME    0x2F
#define MAXSHORTPULSETIME   0xFF
#define MINSHORTPULSETIME   0x2F
#define REPEADHOLDDELAY     35
//50
//#define BIGKEYSSKIPDELAY	40

#define KEYSSKIPDELAY		20
//20
#define SMALLKEYSSKIPDELAY  15
//15
#define DISPLAYPERIODLO   	0x00
#define DISPLAYPERIODHI		0xfb
#define VOLUMECOUNTERRELOAD	32
#define VOLUMETIMERRELOAD	(-200)
//(-50)
#define VOLUMESTEP			1
#define MAXCHANELNO			107
#define MINCHANELNO			2
#define STATEDELAY          1024

#define DECODEKEYSIZE		32
#define FCADDRESS			16
#define MCADDRESS			32
#define SERIALNOADDRESS		48
#define CFGSADDRESS			64

typedef union tagINFRACODE{
	BYTE AData[4];
	DWORD Data;
} INFRACODE;

typedef struct tacCFGS{
	BYTE ChanelNo;
	BYTE PrevChanel;
	BYTE VolumeSize;
} CFGS;


CFGS Configs;

BYTE VolumeCounter;
BYTE FChanels[14];
BYTE MChanels[14];

BYTE PressedKeyCode = keyNOKEY;
BYTE LastInfraKeyCode = keyNOKEY;
WORD KeysTestDelayCounter = 0;
WORD InRepeatDelayCounter = 0;
BYTE ScanState;
BOOL Power = 0;
WORD StateDelay = 0;
WORD DelayCounter = 0;
WORD ResciveIdleCounter = 0;
BOOL NewDataRescived = 0;
BOOL BadData = 0;

code unsigned char DECODE[] = { 0xc0, // CODE 0
0xf9, // CODE 1
0xa4, // CODE 2
0xb0, // CODE 3
0x99, // CODE 4
0x92, // CODE 5
0x82, // CODE 6
0xf8, // CODE 7
0x80, // CODE 8
0x90, // CODE 9
0xff  // CODE BLACK
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

// 110000100 + (0 + CHANELS1) + (CHANELS2 + 0) + 110011100 + (CHANELS3 + 0) + 0?

code DWORD CPUSERIAL = 0x00000005;

code BYTE MEMSETUPCODE[8] = { keyNUM1, keyNUM1, keyNUM2, keyNUM2, keyNUM3, keyNUM3, keyNUM4, keyNUM4 };

code BYTE CODEKEY[DECODEKEYSIZE] = { 'D', 'I', 'G', 'I', 'T', 'A', 'L', 'S', '-', 'H', 'A', 'M', 'A', '-', '(', '0', '3', '3', ')', ' ', '2', '3', '9', '4', '8', '9', '-', 'R', 'K', '&', 'M', 'A' };

code BYTE CHANELS1[106] = {
	208, 208, 48, 216, 216, 56, 56, 248, 248, 4, 4, 200, 40, 40, 232, 232, //      1
	24, 24, 24, 4, 196, 196, 36, 36, 228, 228, 228, 20, 20, 212, 212, 52, //     17
	52, 244, 244, 12, 12, 204, 204, 44, 44, 236, 236, 28, 28, 220, 220, 60, //     33
	60, 252, 252, 2, 2, 194, 194, 34, 34, 226, 226, 18, 18, 210, 210, 50, //     49
	50, 242, 242, 10, 10, 202, 202, 42, 42, 234, 234, 26, 26, 218, 218, 58, //     65
	58, 250, 250, 6, 6, 198, 198, 38, 38, 230, 230, 48, 240, 240, 8, 8, //     81
	200, 200, 22, 22, 214, 214, 54, 54, 246, 246
};

code BYTE CHANELS2[106] = {
	78, 71, 74, 70, 75, 66, 77, 68, 73, 64, 78, 71, 74, 67, 76, 69, //      1
	72, 65, 79, 71, 74, 67, 76, 69, 72, 65, 79, 70, 75, 70, 71, 70, //     17
	71, 70, 71, 70, 71, 70, 71, 70, 71, 70, 71, 70, 71, 70, 71, 70, //     33
	71, 70, 71, 70, 71, 70, 71, 70, 71, 70, 71, 70, 71, 70, 71, 70, //     49
	71, 70, 71, 70, 71, 70, 71, 70, 71, 70, 71, 70, 71, 70, 71, 70, //     65
	71, 70, 71, 70, 71, 70, 71, 70, 71, 70, 71, 67, 76, 69, 72, 65, //     81
	64, 78, 70, 71, 70, 71, 70, 71, 70, 71
};

code BYTE CHANELS3[106] = {
	37, 37, 37, 41, 41, 41, 41, 41, 41, 41, 41, 37, 37, 37, 37, 37, //      1
	37, 37, 37, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, //     17
	41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, //     33
	41, 41, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, //     49
	44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, //     65
	44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 37, 37, 37, 37, 37, //     81
	37, 37, 44, 44, 44, 44, 44, 44, 44, 44
};

unsigned char Video[3];

void DisplayNumber(WORD AValue, BYTE ACursorMask, BYTE ADotMask)
{
	register BYTE Mask = 0x01;
	register BYTE DigitNo;
	if (Power){
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
	else {
		Video[0] = 0xff;
		Video[1] = 0xff;
		Video[2] = 0xff;
	}
}

InfraRead0() interrupt 0 using 3
{
	register INFRACODE Data;
	register BYTE SearchData;
	register BYTE LastPulseWidth;
	register BYTE I;

	if (KeysTestDelayCounter > 0) goto Finish;

	Digit1 = 0;
	Digit2 = 0;
	Digit3 = 0;

	// Wait for end of LO pulse
	LastPulseWidth = 0;
	while ((InfraInput == 0) && (LastPulseWidth < MAXLONGPULSETIME)) LastPulseWidth++;
	if ((LastPulseWidth >= MAXLONGPULSETIME) || (LastPulseWidth <= MINLONGPULSETIME)) goto Mask;

	Data.Data = 0x00000000;

	// Wait for end of HI pulse
	LastPulseWidth = 0;
	while ((InfraInput == 1) && (LastPulseWidth < MAXLONGPULSETIME)) LastPulseWidth++;
	if ((LastPulseWidth >= MAXSHORTPULSETIME) || (LastPulseWidth <= MINSHORTPULSETIME)) goto Mask;

	for (I = 0; I < 32; I++){
		LastPulseWidth = 0;
		while ((InfraInput == 0) && (LastPulseWidth < MAXPULSETIME)) LastPulseWidth++;

		if (LastPulseWidth >= MAXPULSETIME) goto Mask;

		LastPulseWidth = 0;
		while ((InfraInput == 1) && (LastPulseWidth < MAXPULSETIME)) LastPulseWidth++;

		if (LastPulseWidth >= MAXPULSETIME){
			if (I == 0) goto RepeatCodeDetected;
			else goto Mask;
		}

		Data.Data = Data.Data << 1;
		if (LastPulseWidth > TIMETHREASHOULD) Data.Data = Data.Data | 0x00000001;
	}

	LastPulseWidth = 0;
	while ((InfraInput == 0) && (LastPulseWidth < MAXPULSETIME)) LastPulseWidth++;

	LastInfraKeyCode = keyNOKEY;

	if ((Data.AData[0] == 0x00) && (Data.AData[1] == 0xff)){
		SearchData = Data.AData[3];
		for (I = 0; I<CODESCOUNT; I++)
			if (SearchData == INFRACODES[I]){
				LastInfraKeyCode = I;
				KeysTestDelayCounter = KEYSSKIPDELAY;
				InRepeatDelayCounter = REPEADHOLDDELAY;
				break;
			};
	}

	PressedKeyCode = LastInfraKeyCode;
	if (LastInfraKeyCode > keyFCSCAN) LastInfraKeyCode = keyNOKEY;
	goto Mask;
	/*
	if (LastInfraKeyCode == keyONOFF) LastInfraKeyCode = keyNOKEY;
	goto Mask;
	*/

RepeatCodeDetected:

	if (InRepeatDelayCounter > 0){
		KeysTestDelayCounter = SMALLKEYSSKIPDELAY;
		InRepeatDelayCounter = REPEADHOLDDELAY;
		PressedKeyCode = LastInfraKeyCode;
	}
	else PressedKeyCode = keyNOKEY;
	goto Mask;

Mask:
	KeysTestDelayCounter = SMALLKEYSSKIPDELAY;
	InRepeatDelayCounter = REPEADHOLDDELAY;
Finish:
	IE0 = 0; // clear another interrupt request created by series of pulses
}

VolTimer0() interrupt 1 using 1
{
	if (VolumeCounter > 0) VolumeCounter--;
	else VolumeCounter = VOLUMECOUNTERRELOAD;

	Volume = (VolumeCounter >= Configs.VolumeSize);
}

ComRead1() interrupt 2 using 1
{
	WORD MaxWidth = 0xffff;
	WORD Threshould;
	register WORD PulseWidth;
	EA = 0;
	Volume = 1;
	Digit1 = 0;
	Digit2 = 0;
	Digit3 = 0;

	for (PulseWidth = 0; (CommInput == 0) && (PulseWidth < MaxWidth); PulseWidth++);
	if (PulseWidth >= MaxWidth) goto Finish;

	for (PulseWidth = 0; (CommInput == 1) && (PulseWidth < MaxWidth); PulseWidth++);
	if (PulseWidth >= MaxWidth) goto Finish;

	for (; (CommInput == 0) && (PulseWidth < MaxWidth); PulseWidth++);
	if (PulseWidth >= MaxWidth) goto Finish;

	MaxWidth = PulseWidth;
	Threshould = PulseWidth >> 3;

	for (PulseWidth = 0; (CommInput == 1) && (PulseWidth < MaxWidth); PulseWidth++);
	if (PulseWidth >= MaxWidth) goto Finish;

	{
		register WORD Address = CPUSERIAL;
		register BYTE I;
		for (I = 0; I < 16; I++){
			for (PulseWidth = 0; (CommInput == 0) && (PulseWidth < MaxWidth); PulseWidth++);
			if (PulseWidth >= MaxWidth) goto Finish;
			for (PulseWidth = 0; (CommInput == 1) && (PulseWidth < MaxWidth); PulseWidth++);
			if (PulseWidth >= MaxWidth) goto Finish;
			if (PulseWidth > Threshould) {
				if ((Address & 0x01) == 0){
					EX1 = 0;
					ResciveIdleCounter = 500;
					goto Finish;
				}
			}
			else {
				if ((Address & 0x01) != 0){
					EX1 = 0;
					ResciveIdleCounter = 500;
					goto Finish;
				}
			}
			Address >>= 1;
		}

		{
			register BYTE Data;
			register BYTE J;

			NewDataRescived = 1;
			BadData = 1;
			for (J = 0; J < 14; J++){
				Data = 0;
				for (I = 0; I < 8; I++){
					for (PulseWidth = 0; (CommInput == 0) && (PulseWidth < MaxWidth); PulseWidth++);
					if (PulseWidth >= MaxWidth) goto Finish;
					for (PulseWidth = 0; (CommInput == 1) && (PulseWidth < MaxWidth); PulseWidth++);
					if (PulseWidth >= MaxWidth) goto Finish;
					Data <<= 1;
					if (PulseWidth > Threshould) Data |= 0x01;
				}
				MChanels[J] = Data;
			}
			BadData = 0;
		}
		/*
		else {
		EX1 = 0;
		ResciveIdleCounter = 500;
		}
		*/
	}

	for (PulseWidth = 0; (CommInput == 0) && (PulseWidth < MaxWidth); PulseWidth++);

	if (PulseWidth >= MaxWidth) goto Finish;

Finish:
	IE1 = 0;
	EA = 1;
}

DisplayTimer1() interrupt 3 using 1
{
	if (DelayCounter > 0) DelayCounter--;

	switch (ScanState){
	case 0:
		Digit3 = 0;
		DISPLAY = Video[0];
		Digit1 = 1;
		ScanState = 1;
		break;
	case 1:
		Digit1 = 0;
		DISPLAY = Video[1];
		Digit2 = 1;
		ScanState = 2;
		break;
	case 2:
		Digit2 = 0;
		DISPLAY = Video[2];
		Digit3 = 1;
		ScanState = 3;
		break;
	default:
		Digit3 = 0;
		if (KeysTestDelayCounter > 0){
			EX0 = 0;
			KeysTestDelayCounter--;
		};
		if (InRepeatDelayCounter > 0) InRepeatDelayCounter--;

		if (PressedKeyCode == keyNOKEY){
			if (KeysTestDelayCounter == 0){
				IE0 = 0;
				EX0 = 1;
				KeysCommon = 0;
				OnOffKey = 1;
				ChUpKey = 1;
				ChDownKey = 1;
				VolUpKey = 1;
				VolDownKey = 1;

				if (ChUpKey == 0){
					PressedKeyCode = keyCHUP;
					KeysTestDelayCounter = KEYSSKIPDELAY;
				}
				else if (ChDownKey == 0){
					PressedKeyCode = keyCHDOWN;
					KeysTestDelayCounter = KEYSSKIPDELAY;
				}
				else if (VolUpKey == 0){
					PressedKeyCode = keyVOLUP;
					KeysTestDelayCounter = KEYSSKIPDELAY;
				}
				else if (VolDownKey == 0){
					PressedKeyCode = keyVOLDOWN;
					KeysTestDelayCounter = KEYSSKIPDELAY;
				}
				else if (OnOffKey == 0){
					PressedKeyCode = keyONOFF;
					KeysTestDelayCounter = KEYSSKIPDELAY;
				}

				KeysCommon = 1;
			};
		};

		ScanState = 0;
	};

	/* set timer period for next scan step */
	if (StateDelay > 0) StateDelay--;
	else if (Power == 0){

		if (Video[2] == 0x7f) {
			Video[2] = 0xff;
			StateDelay = 320;
		}
		else {
			Video[2] = 0x7f;
			StateDelay = 32;
		}
	}

	if (ResciveIdleCounter > 1) ResciveIdleCounter--;
	else if (ResciveIdleCounter == 1){
		IE1 = 0;
		EX1 = 1;
		// AcceptedAddress = 0;
		ResciveIdleCounter = 0;
	}


	TL1 = DISPLAYPERIODLO;
	TH1 = DISPLAYPERIODHI;
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

void Delay(WORD AInterval)
{
	DelayCounter = AInterval;
	while (DelayCounter > 0);
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

BOOL RetryReadBuffer(BYTE Address, void *ABuffer, BYTE ASize)
{
	register BYTE I;
	for (I = 0; I < 8; I++){
		Delay(100);
		if (ReadBuffer(0, Address, ABuffer, ASize)) {
			CodeBuffer(ABuffer, ASize, FALSE);
			return TRUE;
		}
	};
	return FALSE;
}

BOOL RetryWriteBuffer(BYTE Address, void *ABuffer, BYTE ASize)
{
	register BYTE I;
	BOOL Result = FALSE;
	CodeBuffer(ABuffer, ASize, TRUE);
	for (I = 0; I < 8; I++){
		Delay(100);
		if (WriteBuffer(0, Address, ABuffer, ASize)) {
			Result = TRUE;
			break;
		}
	};
	CodeBuffer(ABuffer, ASize, FALSE);
	return Result;
}

BOOL IsFavorate(BYTE AChanelNo, BOOL AChange)
{
	register BYTE Temp1 = (AChanelNo >> 3);
	register BYTE Temp2 = (0x01 << (AChanelNo & 0x07));
	if (AChange) FChanels[Temp1] ^= Temp2;
	return (FChanels[Temp1] & Temp2);
}

BOOL IsMasked(BYTE AChanelNo, BOOL AChange)
{
	register BYTE Temp1 = (AChanelNo >> 3);
	register BYTE Temp2 = (0x01 << (AChanelNo & 0x07));
	if (AChange) MChanels[Temp1] |= Temp2;
	return (MChanels[Temp1] & Temp2);
}

BYTE FindNearerUnMasked(BYTE AChanelNo, BOOL ADir)
{
	if ((AChanelNo >= MINCHANELNO) && (AChanelNo <= MAXCHANELNO) && (IsMasked(AChanelNo, FALSE))){
		register BYTE ScanChanel = AChanelNo;
		do {
			if (!IsMasked(ScanChanel, FALSE)) break;
			if (ADir){
				ScanChanel++;
				if (ScanChanel > MAXCHANELNO) ScanChanel = MINCHANELNO;
			}
			else {
				ScanChanel--;
				if (ScanChanel < MINCHANELNO) ScanChanel = MAXCHANELNO;
			}
		} while (ScanChanel != AChanelNo);

		if (ScanChanel != AChanelNo) return ScanChanel;
		else return 0;
	}
	else if ((AChanelNo >= MINCHANELNO) && (AChanelNo <= MAXCHANELNO)) return AChanelNo;
	else return 0;
}

// 1 1000 0100 + (0 + CHANELS1) + (CHANELS2 + 0) + 1 1001 1100 + (CHANELS3 + 0) + 0?

void SendTunerData(WORD Data, BYTE Length)
{
	register BYTE Counter;
	register WORD Mask = 0x0001;
	for (Counter = 0; Counter < Length; Counter++){
		TunerData = (Data & Mask);
		_nop_();
		_nop_();
		TunerClock = 1;
		Mask = Mask << 1;
		TunerClock = 0;
		_nop_();
		_nop_();
	}
}

void ChangeChanel(BYTE AChanel)
{
	if ((AChanel > MAXCHANELNO) || (AChanel < MINCHANELNO)) return;
	EA = 0;
	TunerData = 0;
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	TunerData = 1;
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	TunerData = 0;
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	TunerClock = 0;
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	SendTunerData(0x0043, 10);
	SendTunerData(CHANELS1[AChanel - MINCHANELNO], 8);
	SendTunerData(CHANELS2[AChanel - MINCHANELNO], 8);
	SendTunerData(0x00E6, 10);
	SendTunerData(CHANELS3[AChanel - MINCHANELNO], 8);
	SendTunerData(0x0000, 1);
	TunerData = 0;
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	TunerClock = 1;
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	TunerData = 1;
	EA = 1;
}

void EditChanelNo(void)
{
	BYTE State = 0;
	BYTE NewChanelNo = 0;
	DisplayNumber(NewChanelNo, 0x07, 0x00);
	while (State != 3){
		StateDelay = STATEDELAY;
		while (PressedKeyCode == keyNOKEY) if (StateDelay == 0) goto Finish;
		switch (State){
		case 0:
			if (PressedKeyCode == keyNUM100){
				NewChanelNo = 100;
				DisplayNumber(NewChanelNo, 0x01, 0x00);
				State = 2;
			}
			else if ((PressedKeyCode >= keyNUM0) && (PressedKeyCode <= keyNUM9)){
				NewChanelNo = ((PressedKeyCode - keyNUM0) * 10);
				DisplayNumber(NewChanelNo, 0x01, 0x00);
				State = 2;
			}
			else goto Finish;
			break;
		case 1:
			if ((PressedKeyCode >= keyNUM0) && (PressedKeyCode <= keyNUM9)){
				NewChanelNo = NewChanelNo + ((PressedKeyCode - keyNUM0) * 10);
				DisplayNumber(NewChanelNo, 0x01, 0x00);
				State = 2;
			}
			else goto Finish;
			break;
		case 2:
			if ((PressedKeyCode >= keyNUM0) && (PressedKeyCode <= keyNUM9)){
				NewChanelNo = (NewChanelNo + (PressedKeyCode - keyNUM0));
				DisplayNumber(NewChanelNo, 0x00, 0x00);
				State = 3;
			}
			else goto Finish;
			break;
		}
		PressedKeyCode = keyNOKEY;
	}

	if ((NewChanelNo >= MINCHANELNO) && (NewChanelNo <= MAXCHANELNO) && (!IsMasked(NewChanelNo, FALSE))){
		Configs.PrevChanel = Configs.ChanelNo;
		Configs.ChanelNo = NewChanelNo;
	}
	ChangeChanel(Configs.ChanelNo);
Finish:;
}

void ChangeVol(void)
{
	StateDelay = STATEDELAY;
	do {
		PressedKeyCode = keyNOKEY;
		DisplayNumber(Configs.VolumeSize, 0x00, 0x00);
		Video[2] = 0xc7;
		while (PressedKeyCode == keyNOKEY) if (StateDelay == 0) goto Finish;
		StateDelay = STATEDELAY;
		switch (PressedKeyCode){
		case keyVOLUP:
			if (Configs.VolumeSize <= (VOLUMECOUNTERRELOAD - VOLUMESTEP)) Configs.VolumeSize += VOLUMESTEP;
			break;
		case keyVOLDOWN:
			if (Configs.VolumeSize >= VOLUMESTEP) Configs.VolumeSize -= VOLUMESTEP;
			break;
		default:
			goto Finish;
		};
	} while (StateDelay > 0);
Finish:;
}

void MemSetup(void)
{
	DisplayNumber(0, 0x07, 0x00);
	{
		BYTE KeyNo = 0;

		while (KeyNo < 8) {
			while (PressedKeyCode == keyNOKEY);
			if (PressedKeyCode == MEMSETUPCODE[KeyNo]) KeyNo++;
			else KeyNo = 0;
			DisplayNumber(0, 0x07, 0x07);
			Delay(200);
			DisplayNumber(0, 0x07, 0x00);
			PressedKeyCode = keyNOKEY;
		};
	}

	DisplayNumber(0, 0x00, 0x00);
	RetryWriteBuffer(FCADDRESS, FChanels, 14);
	RetryWriteBuffer(MCADDRESS, MChanels, 14);
	RetryWriteBuffer(CFGSADDRESS, &Configs, 3);
	{
		DWORD MemSerial = CPUSERIAL;
		RetryWriteBuffer(SERIALNOADDRESS, &MemSerial, 4);
	}

}

main(){
	Power = 0;
	Relay = Power;
	Mute = 0;
	Video[0] = 0xff;
	Video[1] = 0xff;
	Video[2] = 0xff;

	IP = 0x0e;				 /* set Int0 to be with high priorety */
	IT0 = 1;				 /* set Int0 to be trigged by hi->lo transition */
	IT1 = 1;

	TL0 = VOLUMETIMERRELOAD;
	TH0 = VOLUMETIMERRELOAD;            /* set timer period */
	TL1 = DISPLAYPERIODLO;
	TH1 = DISPLAYPERIODHI;            /* set timer period */

	TMOD = 0x12;      		 /* select mode 2 */
	TR0 = 1;                 /* start timer 0 */
	TR1 = 1;                 /* start timer 1 */
	ET0 = 1;                 /* enable timer 0 interrupt */
	ET1 = 1;                 /* enable timer 1 interrupt */
	EX0 = 1;                 /* enable int0 interrupt */
	EX1 = 1;                 /* enable int1 interrupt */
	EA = 1;                  /* global interrupt enable */

	// AcceptedAddress = 0;

	Configs.ChanelNo = 2;
	Configs.PrevChanel = 2;
	Configs.VolumeSize = VOLUMECOUNTERRELOAD / 2;

	{
		register BYTE I;
		for (I = 0; I < 14; I++){
			FChanels[I] = 0x00;
			MChanels[I] = 0xFF;
		};
	}

	{
		register BYTE I;
		DWORD MemSerial;

		RetryReadBuffer(SERIALNOADDRESS, &MemSerial, 4);
		if (MemSerial == CPUSERIAL) {
			RetryReadBuffer(FCADDRESS, FChanels, 14);
			RetryReadBuffer(MCADDRESS, MChanels, 14);
			RetryReadBuffer(CFGSADDRESS, &Configs, 3);
		}
		else {
			Power = 1;
			Relay = Power;
			MemSetup();
		}
	}

	Configs.ChanelNo = FindNearerUnMasked(Configs.ChanelNo, TRUE);
	Configs.PrevChanel = Configs.ChanelNo;

	while (1){
		DisplayNumber(Configs.ChanelNo, 0x00, IsFavorate(Configs.ChanelNo, FALSE));
		while (PressedKeyCode == keyNOKEY){
			if (NewDataRescived == 1){
				if (BadData == 1) RetryReadBuffer(MCADDRESS, MChanels, 14);
				else {
					RetryWriteBuffer(MCADDRESS, MChanels, 14);
					if (Configs.ChanelNo < MINCHANELNO) Configs.ChanelNo = MINCHANELNO;
					Configs.ChanelNo = FindNearerUnMasked(Configs.ChanelNo, TRUE);
					ChangeChanel(Configs.ChanelNo);
					DisplayNumber(Configs.ChanelNo, 0x00, IsFavorate(Configs.ChanelNo, FALSE));
				}
				BadData = 0;
				NewDataRescived = 0;
			}
		}
		if (Power == 1) {
			switch (PressedKeyCode){
			case keyONOFF:
				Power = ~Power;
				Relay = Power;
				PressedKeyCode = keyNOKEY;
				break;
			case keyVOLUP:
				ChangeVol();
				break;
			case keyVOLDOWN:
				ChangeVol();
				break;
			case keyCHUP:
				if (Configs.ChanelNo != 0) {
					if (Configs.ChanelNo < MAXCHANELNO){
						Configs.PrevChanel = Configs.ChanelNo;
						Configs.ChanelNo++;
					}
					else Configs.ChanelNo = MINCHANELNO;
					Configs.ChanelNo = FindNearerUnMasked(Configs.ChanelNo, TRUE);
					ChangeChanel(Configs.ChanelNo);
				}
				PressedKeyCode = keyNOKEY;
				break;
			case keyCHDOWN:
				if (Configs.ChanelNo != 0) {
					if (Configs.ChanelNo > MINCHANELNO){
						Configs.PrevChanel = Configs.ChanelNo;
						Configs.ChanelNo--;
					}
					else Configs.ChanelNo = MAXCHANELNO;
					Configs.ChanelNo = FindNearerUnMasked(Configs.ChanelNo, FALSE);
					ChangeChanel(Configs.ChanelNo);
				}
				PressedKeyCode = keyNOKEY;
				break;
			case keyRECALL:
			{
				register BYTE TempChanelNo = Configs.ChanelNo;
				Configs.ChanelNo = Configs.PrevChanel;
				Configs.PrevChanel = TempChanelNo;
			}
			Configs.ChanelNo = FindNearerUnMasked(Configs.ChanelNo, TRUE);
			ChangeChanel(Configs.ChanelNo);
			PressedKeyCode = keyNOKEY;
			break;
			case keyMUTE:
				Mute = ~Mute;
				PressedKeyCode = keyNOKEY;
				break;
			case keyFCMEMO:
				if (Configs.ChanelNo != 0) IsFavorate(Configs.ChanelNo, TRUE);
				PressedKeyCode = keyNOKEY;
				break;
			case keyFCSCAN:
				if (Configs.ChanelNo != 0) {
					register BYTE ScanChanel = Configs.ChanelNo + 1;

					while (ScanChanel <= MAXCHANELNO){
						if (IsFavorate(ScanChanel, FALSE) && (!IsMasked(ScanChanel, FALSE))) break;
						ScanChanel++;
					}

					if (ScanChanel > MAXCHANELNO){
						ScanChanel = MINCHANELNO;
						while (ScanChanel < Configs.ChanelNo){
							if (IsFavorate(ScanChanel, FALSE) && (!IsMasked(ScanChanel, FALSE))) break;
							ScanChanel++;
						}
					};
					if (ScanChanel != Configs.ChanelNo){
						Configs.ChanelNo = ScanChanel;
						ChangeChanel(Configs.ChanelNo);
					}
					Configs.ChanelNo = ScanChanel;
				}
				PressedKeyCode = keyNOKEY;
				break;
			case keyPCMEMO:
				IsMasked(Configs.ChanelNo, TRUE);
				Configs.ChanelNo = FindNearerUnMasked(Configs.ChanelNo, TRUE);
				PressedKeyCode = keyNOKEY;
				break;
			case keyPC:
				RetryWriteBuffer(FCADDRESS, FChanels, 14);
				RetryWriteBuffer(MCADDRESS, MChanels, 14);
				RetryWriteBuffer(CFGSADDRESS, &Configs, 3);
				PressedKeyCode = keyNOKEY;
				break;
			case keyPCSCAN:
			{
				DWORD MemSerial = 0xffffffff;
				RetryWriteBuffer(SERIALNOADDRESS, &MemSerial, 4);
			}
			break;
			case keySLEEP:
			{
				register BYTE I;
				for (I = 0; I < 14; I++){
					FChanels[I] = 0x00;
					MChanels[I] = 0x00;
				};
			}

			Configs.ChanelNo = 2;
			Configs.PrevChanel = 2;
			Configs.VolumeSize = VOLUMECOUNTERRELOAD / 2;
			PressedKeyCode = keyNOKEY;
			break;
			default:
				if ((PressedKeyCode >= keyNUM0) && (PressedKeyCode <= keyNUM100)) EditChanelNo();
				else PressedKeyCode = keyNOKEY;
			}
		}
		else {
			if (PressedKeyCode == keyONOFF){
				Power = ~Power;
				Relay = Power;
				StateDelay = 64;
				while (StateDelay != 0);
				ChangeChanel(Configs.ChanelNo);
			}
			PressedKeyCode = keyNOKEY;
		}
	};
}
