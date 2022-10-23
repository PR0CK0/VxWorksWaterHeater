#ifndef WATERHEATER_H
#define WATERHEATER_H

void setupHeater();
void resetHeater();
void turnWaterHeaterOn();
void turnWaterHeaterOff();

int isWaterHeaterOn();
SEM_ID getHeaterSem();

static void enforceWatchdog();

#endif
