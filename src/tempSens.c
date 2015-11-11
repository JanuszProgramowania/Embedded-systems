/******************************************************************************
 *
 * Copyright:
 *    (C) 2000 - 2005 Embedded Artists AB
 *
 *****************************************************************************/


#include "../pre_emptive_os/api/osapi.h"
#include "../pre_emptive_os/api/general.h"
#include <printf_P.h>
#include <lpc2xxx.h>
#include <consol.h>
#include <string.h>
#include "eeprom.h"

#define MAX_LENGTH 14

tU32 readCurrentTemp(){
	i2cInit();
	tU8 data[3];
	if (1 == lm75Read(0x90, &data[0], 3))
	{
	tU16 temp;
	temp = (((tU16)data[0]<<8) + (tU16)data[1]) >> 7;
	return (temp*5);
	}
	else
	printf("\nFAILED to read from LM75!!!!!!!!!!!!!!!!!\n");
	return 0;
}
