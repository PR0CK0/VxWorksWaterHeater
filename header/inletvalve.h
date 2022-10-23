#ifndef INLETVALVE_H
#define INLETVALVE_H

void setupInlets();
void openInlet();
void closeInlet();
void closeBothInlets();
void fillTank();

int getInletsOpen();
SEM_ID getInletSem();

#endif
