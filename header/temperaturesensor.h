#ifndef TEMPERATURE_H
#define TEMPERATURE_H

int TID_TEMPERATURESENSOR;
int TID_CALCTEMPERATURE;

void shutdownTemperatureSensor();
void setMinTemp(int);
void setMaxTemp(int);

double getCurrentTemperature();
double getMinTemp();
double getMaxTemp();

static void doShutdown();
static void calculateTemperature();
static void monitorTemperature();
static boolean isTemperatureWithinAcceptableRange();

#endif
