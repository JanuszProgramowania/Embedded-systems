/******************************************************************************
 *
 * Copyright:
 *    (C) 2000 - 2007 Embedded Artists AB
 *
 * Description:
 *    Main program for LPC2148 Education Board test program
 *
 *****************************************************************************/


#include "../pre_emptive_os/api/osapi.h"
#include "../pre_emptive_os/api/general.h"
#include <printf_P.h>
#include <ea_init.h>
#include <lpc2xxx.h>
#include <consol.h>

#define PROC1_STACK_SIZE 2048
#define LCDPROC_STACK_SIZE 2048
#define TEMPSENSPROC_STACK_SIZE 2048
#define ADCPROC_STACK_SIZE 2048
#define FANPROC_STACK_SIZE 2048
#define INIT_STACK_SIZE  400

static tU8 proc1Stack[PROC1_STACK_SIZE];
static tU8 lcdProcStack[LCDPROC_STACK_SIZE];
static tU8 tempSensProcStack[TEMPSENSPROC_STACK_SIZE];
static tU8 adcProcStack[ADCPROC_STACK_SIZE];
static tU8 fanProcStack[FANPROC_STACK_SIZE];
static tU8 initStack[INIT_STACK_SIZE];
static tU8 pid1;
static tU8 lcdProcid;
static tU8 tempSensProcid;
static tU8 adcProcid;
static tU8 fanProcid;

static void proc1(void* arg);
static void lcdProc(void* arg);
static void tempSensProc(void* arg);
static void adcProc(void* arg);
static void fanProc(void* arg);
static void initProc(void* arg);

volatile tU32 msClock = 0;

void testLcd(void);
void printTempInfoInit();
void printTempInfoRepeat(tU32 *cTemp, tU32 *dTemp, tU32 *modeFlag);
void analogueInputInit();
tU32 getAnalogueInput(tU8 channel);
void testI2C(void);
tU32 readCurrentTemp();
void testMotor(void);
void runMotorAtSpeed(tU32 *speed);
static void adjustMotorSpeedToTemp();
static tU32 currentTemp = 0;
static tU32 desiredTemp = 0;
static tU32 fanSpeed = 300;
static tU32 autoModeFlag = 1;
/*****************************************************************************
 *
 * Description:
 *    The first function to execute 
 *
 ****************************************************************************/
int
main(void)
{
  tU8 error;
  tU8 pid;
  
  osInit();
  osCreateProcess(initProc, initStack, INIT_STACK_SIZE, &pid, 1, NULL, &error);
  osStartProcess(pid, &error);
  
  osStart();
  return 0;
}

/*****************************************************************************
 *
 * Description:
 *    A process entry function 
 *
 * Params:
 *    [in] arg - This parameter is not used in this application. 
 *
 ****************************************************************************/
static void
proc1(void* arg)
{
	tU8 error;

	osCreateProcess(lcdProc, lcdProcStack, LCDPROC_STACK_SIZE, &lcdProcid, 3, NULL, &error);
	osStartProcess(lcdProcid, &error);

	osCreateProcess(tempSensProc, tempSensProcStack, TEMPSENSPROC_STACK_SIZE, &tempSensProcid, 3, NULL, &error);
	osStartProcess(tempSensProcid, &error);

	osCreateProcess(adcProc, adcProcStack, ADCPROC_STACK_SIZE, &adcProcid, 3, NULL, &error);
	osStartProcess(adcProcid, &error);

	osCreateProcess(fanProc, fanProcStack, FANPROC_STACK_SIZE, &fanProcid, 3, NULL, &error);
	osStartProcess(fanProcid, &error);

//	tU32 pattern;
//	IODIR0  |= 0x00000100;
//	IOSET0   = 0x00000100;
	IODIR0 &= ~(1<<14);
	for(;;)
	{
		adjustMotorSpeedToTemp();
		if(!(IOPIN0 & (1<<14))){
			//printf("yep\n");
			if(autoModeFlag){
				autoModeFlag = 0;
			}
			else{
				autoModeFlag = 1;
			}
			//printf("Flag: %d\n", autoModeFlag);
			while(!(IOPIN0 & (1<<14)))
				osSleep(5);
		}
		else{
			//printf("\rnope\n");
		}
//		// Blink on P0.8
//		pattern = 0x00000100;
//		IOCLR0 = pattern;
//		osSleep(20);
//		IOSET0 = pattern;
		osSleep(3);
	}
}

static void lcdProc(void* arg){
	printTempInfoInit();
	for(;;){
		printTempInfoRepeat(&currentTemp, &desiredTemp, &autoModeFlag);
		osSleep(10);
	}
}

static void tempSensProc(void* arg){
	for(;;){
		currentTemp = readCurrentTemp();
		osSleep(50);
	}
}

static void adcProc(void* arg){
	analogueInputInit();
	tU32 tmp;
	for(;;){
		tmp = getAnalogueInput(2);
		tmp *= 300;
		tmp /= 1023;
		tmp += 100;
		tmp -= tmp%5;
		desiredTemp = tmp;

		if(!autoModeFlag){
			tmp = getAnalogueInput(1);
			tmp *= 300;
			tmp /= 1023;
			fanSpeed = tmp;
		}
		osSleep(10);
	}
}

static void fanProc(void* arg){
	for(;;){
		//testMotor();
		runMotorAtSpeed(&fanSpeed);
		osSleep(50);
	}
}

static void
initProc(void* arg)
{
  tU8 error;

  eaInit();
  osCreateProcess(proc1, proc1Stack, PROC1_STACK_SIZE, &pid1, 3, NULL, &error);
  osStartProcess(pid1, &error);

  osDeleteProcess();
}

/*****************************************************************************
 *
 * Description:
 *    The timer tick entry function that is called once every timer tick
 *    interrupt in the RTOS. Observe that any processing in this
 *    function must be kept as short as possible since this function
 *    execute in interrupt context.
 *
 * Params:
 *    [in] elapsedTime - The number of elapsed milliseconds since last call.
 *
 ****************************************************************************/

static void adjustMotorSpeedToTemp(){
	if(!autoModeFlag) return;
	int diff = currentTemp - desiredTemp;
	if(diff>0){
		if(diff>100){
			fanSpeed=300; //max speed
		}
		else{
			fanSpeed = diff*3;
		}
	}
	else{
		fanSpeed = 0;
	}
}

void
appTick(tU32 elapsedTime)
{
  msClock += elapsedTime;
}
