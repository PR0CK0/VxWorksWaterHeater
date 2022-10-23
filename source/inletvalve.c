#include "../header/includes.h"

// Mutex semaphore.
static SEM_ID mtx;

// Number of inlets open at any given time.
static int inletsOpen = 0;

// Sets up a mutex semaphore for use.
void setupInlets()
{
	mtx = semMCreate(SEM_Q_PRIORITY);
}

// Opens an inlet.
void openInlet()
{
	semTake(mtx, NO_WAIT);
		if (getInletsOpen() < NUMBER_OF_INLETS)
		{
			msgQSend(getMQInlets(), "opening inlet", MAX_MESSAGE_LENGTH, NO_WAIT, MSG_PRI_NORMAL);
			
			inletsOpen++;
		}
	semGive(mtx);
}

// Closes an inlet.
void closeInlet()
{
	semTake(mtx, NO_WAIT);
		if (getInletsOpen() > 0)
		{
			msgQSend(getMQInlets(), "closing inlet", MAX_MESSAGE_LENGTH, NO_WAIT, MSG_PRI_NORMAL);
			
			inletsOpen--;
		}
	semGive(mtx);
}

// Opens both inlets.
void openBothInlets()
{
	semTake(mtx, NO_WAIT);
		while (getInletsOpen() < NUMBER_OF_INLETS)
		{
			openInlet();
		}
	semGive(mtx);
}

// Closes both inlets.
void closeBothInlets()
{
	semTake(mtx, NO_WAIT);
		while (getInletsOpen() > 0)
		{
			closeInlet();
		}
	semGive(mtx);
}

// Fills the tank by closing the outlet and opening both inlets.
void fillTank()
{	
	semTake(getOutletSem(), NO_WAIT);
		if (isOutletOpen())
		{
			msgQSend(getMQInlets(), "filling tank", MAX_MESSAGE_LENGTH, NO_WAIT, MSG_PRI_NORMAL);
			
			closeOutlet();
		}
	semGive(getOutletSem());
	
	semTake(mtx, NO_WAIT);
		openBothInlets();
	semGive(mtx);
}

// Getter.
int getInletsOpen()
{
	return inletsOpen;
}

// Getter.
SEM_ID getInletSem()
{
	return mtx;
}
