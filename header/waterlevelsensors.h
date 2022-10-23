#ifndef WATERLEVEL_H
#define WATERLEVEL_H

int TID_WATERLEVELSENSORS;
int TID_CALCWATERLEVEL;

void initializeWaterLevelSensors();
void shutdownWaterLevelSensors();
void adjustSensorLevels(int, int, int, int);

boolean isWaterBuildingPressure();
double getCurrentWaterLevel();
int getRateFactor();

static void doShutdown();
static void calculateWaterLevel();
static void monitorSensors();
static boolean isHighestSensorDetecting();
static boolean isTopMiddleSensorDetecting();
static boolean isBottomMiddleSensorDetecting();
static boolean isLowestSensorDetecting();

#endif
