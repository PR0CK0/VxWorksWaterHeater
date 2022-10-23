#include "../header/includes.h"

// Mutex semaphore and watchdog.
static SEM_ID mtx;
static WDOG_ID wd;

// Watchdog delay.
static const int waitToRefillTankTicks = 2100;

// State variable for outlet being opened/closed.
static boolean outletOpen = false;

// Initializes a mutex semaphore for use.
void setupOutlet()
{
	mtx = semMCreate(SEM_Q_PRIORITY);
}

// Stops the watchdog and empties the tank.
void resetOutlet()
{
	emptyTank();

	wdCancel(wd);
	wdDelete(wd);
}

// Opens the outlet if not already open.
void openOutlet()
{
	semTake(mtx, NO_WAIT);
		if (!isOutletOpen())
		{
			msgQSend(getMQOutlet(), "opening outlet", MAX_MESSAGE_LENGTH, NO_WAIT, MSG_PRI_NORMAL);
			
			outletOpen = true;
		}
	semGive(mtx);
}

// Closes the outlet if it is open.
void closeOutlet()
{
	semTake(mtx, NO_WAIT);
		if (isOutletOpen())
		{
			msgQSend(getMQOutlet(), "closing outlet", MAX_MESSAGE_LENGTH, NO_WAIT, MSG_PRI_NORMAL);
			
			outletOpen = false;
		}
	semGive(mtx);
}

/*
 * Empties the tank (reducing the water level), by opening the outlet
 * and closing any open inlets.
 * 
 * Also, starts a watchdog once the outlet opens.
 */
void emptyTank()
{	
	semTake(mtx, NO_WAIT);
		if (!isOutletOpen())
		{
			msgQSend(getMQOutlet(), "emptying tank", MAX_MESSAGE_LENGTH, NO_WAIT, MSG_PRI_NORMAL);
			
			openOutlet();
			
			wd = wdCreate();
			wdStart(wd, waitToRefillTankTicks, enforceWatchdog);
		}
	semGive(mtx);
	
	semTake(getInletSem(), NO_WAIT);
		closeBothInlets();
	semGive(getInletSem());
}

// Getter.
boolean isOutletOpen()
{
	return outletOpen;
}

// Getter.
SEM_ID getOutletSem()
{
	return mtx;
}

/*
 * Enforces a rule: after the specified delay, fill the tank back up so the
 * tank cannot empty all the way.
 */
static void enforceWatchdog()
{
	msgQSend(getMQOutlet(), "watchdog delay triggered- refilling tank", MAX_MESSAGE_LENGTH, NO_WAIT, MSG_PRI_NORMAL);
	
	fillTank();
	
	wdDelete(wd);
}
