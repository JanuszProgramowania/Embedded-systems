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

static void
delay37us(void)
{
	volatile tU32 i;
	for(i=0; i<5; i++)
    asm volatile (" nop"); //delay 15 ns x 2500 = about 37 us delay
}

/*****************************************************************************
 *
 * Description:
 *    A process entry function.
 *
 ****************************************************************************/
void
testMotor(void)
{
  static tU32 stepmotorSteps[4] = {0x00201000, 0x00200000, 0x00000000, 0x00001000};  //P0.21 and P0.12 are used to contol the stepper motor
  static tU8 stepmotorIndex = 0;

  IODIR0 |= 0x00201000;
  IOCLR0  = stepmotorSteps[0];

  for(;;)
  {
    //update to new step (backwards)
    if (stepmotorIndex == 0)
      stepmotorIndex = 3;
    else
      stepmotorIndex--;

    //output new step
    IOCLR0 = stepmotorSteps[0];
    IOSET0 = stepmotorSteps[stepmotorIndex];
    osSleep(5);
  }
}

void runMotorAtSpeed(tU32 *speed){
	  static tU32 stepmotorSteps[4] = {0x00201000, 0x00200000, 0x00000000, 0x00001000};  //P0.21 and P0.12 are used to contol the stepper motor
	  static tU8 stepmotorIndex = 0;
	  tU32 i, j;


	  IODIR0 |= 0x00201000;
	  IOCLR0  = stepmotorSteps[0];
	  tU32 delay = 0;
	  for(;;)
	  {
		if(*speed>0){
			//update to new step (backwards)
			if (stepmotorIndex == 0)
			  stepmotorIndex = 3;
			else
			  stepmotorIndex--;

			//output new step
			IOCLR0 = stepmotorSteps[0];
			IOSET0 = stepmotorSteps[stepmotorIndex];
			osSleep(1);
			delay = 300-(*speed);
			for(i=0;i<delay;i++){
				for(j=0;j<delay;j++)
					delay37us();
			}
		}
		else{
			osSleep(10);
		}
	  }
}
