#ifndef OUTLETVALVE_H
#define OUTLETVALVE_H

void setupOutlet();
void resetOutlet();
void openOutlet();
void closeOutlet();
void emptyTank();

int getOutletsOpen();
SEM_ID getOutletSem();

static void enforceWatchdog();

#endif
