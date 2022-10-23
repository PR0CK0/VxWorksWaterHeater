#include "../header/includes.h"

// Mutex semaphore and watchdog.
static SEM_ID mtx;
static WDOG_ID wd;

// Watchdog delay.
static const int waitToForceHeaterBackOnTicks = 1600;

// State variable for water heater being on/off.
static boolean isOn = false;

// Initializes a mutex semaphore for use.
void setupHeater()
{
	mtx = semMCreate(SEM_Q_PRIORITY);
}

// Stops the watchdog and turns the heater off.
void resetHeater()
{
	turnWaterHeaterOff();
	
	wdCancel(wd);
	wdDelete(wd);
}

// Turns the water heater on if it is not already on.
void turnWaterHeaterOn()
{
	semTake(mtx, NO_WAIT);
		if (!isWaterHeaterOn())
		{
			msgQSend(getMQHeater(), "turning water heater on", MAX_MESSAGE_LENGTH, NO_WAIT, MSG_PRI_NORMAL);
			isOn = true;
		}
	semGive(mtx);
}

/*
 * Turns the water heater off if it is on.
 * 
 * Also, starts a watchdog when the heater is turned off.
 */
void turnWaterHeaterOff()
{	
	semTake(mtx, NO_WAIT);
		if (isWaterHeaterOn())
		{
			msgQSend(getMQHeater(), "turning water heater off", MAX_MESSAGE_LENGTH, NO_WAIT, MSG_PRI_NORMAL);
			isOn = false;
			
			wd = wdCreate();
			wdStart(wd, waitToForceHeaterBackOnTicks, enforceWatchdog);
		}
	semGive(mtx);
}

// Getter.
int isWaterHeaterOn()
{
	return isOn;
}

// Getter.
SEM_ID getHeaterSem()
{
	return mtx;
}

/*
 * Enforces a rule: after the specified delay, turns the water heater back on so
 * the water temperature cannot get too low.
 */
static void enforceWatchdog()
{
	msgQSend(getMQHeater(), "watchdog delay triggered- turning water heater back on", MAX_MESSAGE_LENGTH, NO_WAIT, MSG_PRI_NORMAL);
	
	turnWaterHeaterOn();	
	
	wdDelete(wd);
}
