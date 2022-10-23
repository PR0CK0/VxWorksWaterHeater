#ifndef PRINTER_H
#define PRINTER_H

int TID_PRINTERALERT;
int TID_PRINTERCONSISTENT;

void initializePrinter();
void printAlertData();
void printConsistentData();

MSG_Q_ID getMQWaterLevel();
MSG_Q_ID getMQPressure();
MSG_Q_ID getMQTemperature();
MSG_Q_ID getMQOutlet();
MSG_Q_ID getMQInlets();
MSG_Q_ID getMQHeater();
MSG_Q_ID getMQExceptions();
MSG_Q_ID getMQTimestamp();

static void doShutdown();

#endif
