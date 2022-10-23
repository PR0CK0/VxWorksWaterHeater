#include "../header/includes.h"

// Normal task parameters.
static const int priorityMonitorSensors = 114;
static const char taskNameMonitorSensors[] = "tMonWtrLvl";
static const int priorityCalcWaterLevel = 115;
static const char taskNameCalcWaterLevel[] = "tCalcWtrLvl";

// Shutdown task parameters.
static int taskShutdown;
static const int priorityShutdown = 102;
static const char taskNameShutdown[] = "tSdWtrLvl";

// Mutex semaphore.
static SEM_ID mtx;

// Task delay.
static const int waterCalcDelayTicks = 15;

// Water sensor specifics.
static const double sensorDetectionRange = 0.2;
static int highestSensorHeight = 40;
static int topMiddleSensorHeight = 35;
static int bottomMiddleSensorHeight = 20;
static int lowestSensorHeight = 15;

// Water specifics.
static const double waterChangeRate = 0.15; // ----------- should be in inlet/outlet (it's a physical property of the valves)
static double currentWaterLevel = MIN_WATER_LEVEL;
static int waterIntakeRateFactor = 0;

// Facade for initializing all tasks related to the water sensors.
void initializeWaterLevelSensors()
{
	TID_WATERLEVELSENSORS = taskSpawn(taskNameMonitorSensors, priorityMonitorSensors, DEFAULT_TASK_OPTIONS, DEFAULT_TASK_SIZE, monitorSensors, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	TID_CALCWATERLEVEL = taskSpawn(taskNameCalcWaterLevel, priorityCalcWaterLevel, DEFAULT_TASK_OPTIONS, DEFAULT_TASK_SIZE, calculateWaterLevel, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	
	mtx = semMCreate(SEM_Q_PRIORITY);
	
	msgQSend(getMQWaterLevel(), "INITIALIZED", MAX_MESSAGE_LENGTH, NO_WAIT, MSG_PRI_NORMAL);
}

// Facade for shutting down all tasks related to the water level sensors.
void shutdownWaterLevelSensors()
{
	taskShutdown = taskSpawn(taskNameShutdown, priorityShutdown, DEFAULT_TASK_OPTIONS, DEFAULT_TASK_SIZE, doShutdown, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

/*
 *  Allows the user to adjust the phyiscal levels (placements) of the water level sensors.
 *  Usually you would only call it from the terminal.
 */
void adjustSensorLevels(int pos1, int pos2, int pos3, int pos4)
{
	if (pos1 > MIN_WATER_LEVEL && pos1 < MAX_WATER_LEVEL && pos1 < pos2 && pos1 < pos3 && pos1 < pos4)
	{
		lowestSensorHeight = pos1;	
		
		char message[MAX_MESSAGE_LENGTH];
		sprintf(message, "lowest sensor height set to: %d", pos1);
		
		msgQSend(getMQWaterLevel(), message, MAX_MESSAGE_LENGTH, NO_WAIT, MSG_PRI_NORMAL);
	}
	else
	{
		msgQSend(getMQExceptions(), "setting the lowest sensor height (pos1)- value(s) wrong", MAX_MESSAGE_LENGTH, NO_WAIT, MSG_PRI_NORMAL);
		return;
	}
	
	if (pos2 > MIN_WATER_LEVEL && pos2 < MAX_WATER_LEVEL && pos2 > pos1 && pos2 < pos3 && pos2 < pos4)
	{
		bottomMiddleSensorHeight = pos2;
		
		char message[MAX_MESSAGE_LENGTH];
		sprintf(message, "bottom middle sensor height set to: %d", pos2);
		
		msgQSend(getMQWaterLevel(), message, MAX_MESSAGE_LENGTH, NO_WAIT, MSG_PRI_NORMAL);
	}
	else
	{
		msgQSend(getMQExceptions(), "setting the bottom middle sensor height (pos2)- value(s) wrong", MAX_MESSAGE_LENGTH, NO_WAIT, MSG_PRI_NORMAL);
		return;
	}
	
	if (pos3 > MIN_WATER_LEVEL && pos3 < MAX_WATER_LEVEL && pos3 > pos1 && pos3 > pos1 && pos3 < pos4)
	{
		topMiddleSensorHeight = pos3;
		
		char message[MAX_MESSAGE_LENGTH];
		sprintf(message, "top middle sensor height set to: %d", pos3);
		
		msgQSend(getMQWaterLevel(), message, MAX_MESSAGE_LENGTH, NO_WAIT, MSG_PRI_NORMAL);
	}
	else
	{
		msgQSend(getMQExceptions(), "setting the top middle sensor height (pos3)- value(s) wrong", MAX_MESSAGE_LENGTH, NO_WAIT, MSG_PRI_NORMAL);
		return;
	}
	
	if (pos4 > MIN_WATER_LEVEL && pos4 < MAX_WATER_LEVEL && pos4 > pos1 && pos4 > pos2 && pos4 > pos3)
	{
		highestSensorHeight = pos4;	
		
		char message[MAX_MESSAGE_LENGTH];
		sprintf(message, "highest sensor value set to: %d", pos4);
		
		msgQSend(getMQWaterLevel(), message, MAX_MESSAGE_LENGTH, NO_WAIT, MSG_PRI_NORMAL);
	}
	else
	{
		msgQSend(getMQExceptions(), "setting the highest sensor height (pos4)- value(s) wrong", MAX_MESSAGE_LENGTH, NO_WAIT, MSG_PRI_NORMAL);
		return;
	}
}

// To detect when pressure should start building up (as specified).
boolean isWaterBuildingPressure()
{
	semTake(mtx, NO_WAIT);
		double smallRange = 1.0;
		return currentWaterLevel >= topMiddleSensorHeight 
				&& !isHighestSensorDetecting();
	semGive(mtx);
}

// Getter.
double getCurrentWaterLevel()
{
	semTake(mtx, NO_WAIT);
		return currentWaterLevel;
	semGive(mtx);
}

// Getter.
int getRateFactor()
{
	semTake(mtx, NO_WAIT);
		return waterIntakeRateFactor;
	semGive(mtx);
}

/*
 * Shuts down all tasks related to the water level sensors, empties the tank and
 * notifies the printer.
 */
static void doShutdown()
{
	taskDelete(TID_WATERLEVELSENSORS);
	taskDelete(TID_CALCWATERLEVEL);
	
	resetOutlet();
		
	msgQSend(getMQWaterLevel(), "water level sensors shutdown", MAX_MESSAGE_LENGTH, NO_WAIT, MSG_PRI_NORMAL);
	
	taskDelete(taskShutdown);
}

/*
 * An approach to calculating the water level. The general idea: the more inlets open,
 * the more water that comes in in a given instant. I.e. having 2 inlets open means
 * twice the water influx rate, and having 2 inlets open with the outlet open means
 * the water influx rate is at unity, and having just the outlet open means a negative
 * one water influx rate.
 */
static void calculateWaterLevel()
{
	while (1)
	{
		if ((getInletsOpen() == 1 && !isOutletOpen()) || (getInletsOpen() == 2 && isOutletOpen()))
		{
			waterIntakeRateFactor = 1;
		}
		else if (getInletsOpen() == 2 && !isOutletOpen())
		{
			waterIntakeRateFactor = 2;
		}
		else if (getInletsOpen() == 0 && isOutletOpen())
		{
			waterIntakeRateFactor = -1;
		}
		else
		{
			waterIntakeRateFactor = 0;
		}
		
		currentWaterLevel = currentWaterLevel + (waterIntakeRateFactor * waterChangeRate);
				
		taskDelay(waterCalcDelayTicks);
	}
}

/*
 * Monitors the water level every tick and performs appropriate actios to keep
 * the water tank in between the minimum and maximum values by emptying and
 * filling the tank, where appropriate.
 */
static void monitorSensors()
{
	int currentlyDetectingSensor = 0;
	
	while (1)
	{
		if (getCurrentWaterLevel() > highestSensorHeight)
		{
			closeBothInlets();
		}
		else if (getCurrentWaterLevel() < lowestSensorHeight)
		{
			fillTank();
		}
		else if (isHighestSensorDetecting())
		{
			if (currentlyDetectingSensor != 4)
			{
				msgQSend(getMQWaterLevel(), "highest sensor reached", MAX_MESSAGE_LENGTH, NO_WAIT, MSG_PRI_URGENT);
				currentlyDetectingSensor = 4;
			}
		}
		else if (isTopMiddleSensorDetecting())
		{
			if (currentlyDetectingSensor != 3)
			{
				msgQSend(getMQWaterLevel(), "top middle sensor reached", MAX_MESSAGE_LENGTH, NO_WAIT, MSG_PRI_NORMAL);
				currentlyDetectingSensor = 3;				
			}
		}
		else if (isBottomMiddleSensorDetecting())
		{
			if (currentlyDetectingSensor != 2)
			{
				msgQSend(getMQWaterLevel(), "bottom middle sensor reached", MAX_MESSAGE_LENGTH, NO_WAIT, MSG_PRI_NORMAL);
				currentlyDetectingSensor = 2;				
			}
		}
		else if (isLowestSensorDetecting())
		{
			if (currentlyDetectingSensor != 1)
			{
				msgQSend(getMQWaterLevel(), "lowest sensor reached", MAX_MESSAGE_LENGTH, NO_WAIT, MSG_PRI_NORMAL);
				currentlyDetectingSensor = 1;				
			}
		}
		else
		{
			currentlyDetectingSensor = 0;
		}
		
		taskDelay(REALTIME_DELAY);
	}
}

// True if the water level is within the highest sensor's range.
static boolean isHighestSensorDetecting()
{
	return (currentWaterLevel >= highestSensorHeight - sensorDetectionRange) 
			&& (currentWaterLevel <= highestSensorHeight + sensorDetectionRange);
}

// True if the water level is within the top middle sensor's range.
static boolean isTopMiddleSensorDetecting()
{
	return (currentWaterLevel >= topMiddleSensorHeight - sensorDetectionRange) 
			&& (currentWaterLevel <= topMiddleSensorHeight + sensorDetectionRange);
}

// True if the water level is within the bottom middle sensor's range.
static boolean isBottomMiddleSensorDetecting()
{
	return (currentWaterLevel >= bottomMiddleSensorHeight - sensorDetectionRange) 
			&& (currentWaterLevel <= bottomMiddleSensorHeight + sensorDetectionRange);
}

// True if the water level is within the lowest sensor's range.
static boolean isLowestSensorDetecting()
{
	return (currentWaterLevel >= lowestSensorHeight - sensorDetectionRange) 
			&& (currentWaterLevel <= lowestSensorHeight + sensorDetectionRange);
}
