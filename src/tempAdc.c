#include "../pre_emptive_os/api/osapi.h"
#include "../pre_emptive_os/api/general.h"
#include <printf_P.h>
#include <lpc2xxx.h>
#include <consol.h>
#include "../startup/config.h"


/******************************************************************************
 * Defines and typedefs
 *****************************************************************************/
#define CRYSTAL_FREQUENCY FOSC
#define PLL_FACTOR        PLL_MUL
#define VPBDIV_FACTOR     PBSD

#define AIN0 0
#define AIN1 1
#define AIN2 2
#define AIN3 3
#define AIN4 4
#define AIN5 5
#define AIN6 6
#define AIN7 7

#define VREF_IN_MV 2475


/*****************************************************************************
 *
 * Description:
 *    xxx
 *
 ****************************************************************************/
static void
delay37us(void)
{
	volatile tU32 i;

	for(i=0; i<2500; i++)
    asm volatile (" nop"); //delay 15 ns x 2500 = about 37 us delay
}

/*****************************************************************************
 *
 * Description:
 *    Start a conversion of one selected analogue input and return
 *    10-bit result.
 *
 * Params:
 *    [in] channel - analogue input channel to convert.
 *
 * Return:
 *    10-bit conversion result
 *
 ****************************************************************************/
tU32 getAnalogueInput(tU8 channel)
{
	//start conversion now (for selected channel)
	ADCR = (ADCR & 0xFFFFFF00) | (1 << channel) | (1 << 24);

	//wait til done
	while((ADDR & 0x80000000) == 0)
	  ;

	//get result and adjust to 10-bit integer
	return (ADDR>>6) & 0x3FF;
}

void analogueInputInit(){
  tU32 integerResult;
  PINSEL1 &= ~0x0f0c0000;
  PINSEL1 |=  0x05080000;

  //initialize ADC
  ADCR = (1 << 0)                             |  //SEL = 1, dummy channel #1
		 ((CRYSTAL_FREQUENCY *
		   PLL_FACTOR /
		   VPBDIV_FACTOR) / 4500000 - 1) << 8 |  //set clock division factor, so ADC clock is 4.5MHz
		 (0 << 16)                            |  //BURST = 0, conversions are SW controlled
		 (0 << 17)                            |  //CLKS  = 0, 11 clocks = 10-bit result
		 (1 << 21)                            |  //PDN   = 1, ADC is active
		 (1 << 24)                            |  //START = 1, start a conversion now
		 (0 << 27);							                 //EDGE  = 0, not relevant when start=1

  //short delay and dummy read
  osSleep(1);
  integerResult = ADDR;
}
