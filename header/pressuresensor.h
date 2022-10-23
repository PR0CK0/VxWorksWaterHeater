#ifndef PRESSURE_H
#define PRESSURE_H

int TID_PRESSURESENSOR;
int TID_CALCPRESSURE;

void initializePressureSensor();
void shutdownPressureSensor();

double getCurrentPressure();

static void doShutdown();
static void calculatePressure();
static void monitorPressure();
static void addToPressure();
static void subtractFromPressure();
static boolean isPressureBuildingFast();

#endif
