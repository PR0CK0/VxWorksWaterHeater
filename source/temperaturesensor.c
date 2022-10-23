#include "../header/includes.h"

// Normal task parameters.
static const int priorityMonitorTemperature = 110;
static const char taskNameMonitorTemperature[] = "tMonTemp";
static const int priorityCalculateTemperature = 111;
static const char taskNameCalculateTemperature[] = "tCalcTemp";

// Shutdown task parameters.
static int taskShutdown;
static const int priorityShutdown = 100;
static const char taskNameShutdown[] = "tSdPress";

// Mutex semaphore.
static SEM_ID mtx;

// Task delay.
static const double temperatureCalcDelayTicks = 15;

// Temperature specifics.
static const double temperatureChangeRate = 0.2; // --------------- should be in heater
static double currentTemperature = NORMAL_TEMP_CELSIUS;
static double minimumTemperature = 40.0;
static double maximumTemperature = 90.0;

// Facade for initializing all tasks related to the temperature sensor.
void initializeTemperatureSensor()
{
	TID_TEMPERATURESENSOR = taskSpawn(taskNameMonitorTemperature, priorityMonitorTemperature, DEFAULT_TASK_OPTIONS, DEFAULT_TASK_SIZE, monitorTemperature, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	TID_CALCTEMPERATURE = taskSpawn(taskNameCalculateTemperature, priorityCalculateTemperature, DEFAULT_TASK_OPTIONS, DEFAULT_TASK_SIZE, calculateTemperature, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	
	mtx = semMCreate(SEM_Q_PRIORITY);
	
	msgQSend(getMQTemperature(), "INITIALIZED", MAX_MESSAGE_LENGTH, NO_WAIT, MSG_PRI_NORMAL);
}

// Facade for shutting down all tasks related to the temperature sensor.
void shutdownTemperatureSensor()
{
	taskShutdown = taskSpawn(taskNameShutdown, priorityShutdown, DEFAULT_TASK_OPTIONS, DEFAULT_TASK_SIZE, doShutdown, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

/*
 * Allows the user to set the tank's minimum temperature.
 * 
 * NOTE:
 * This function only works with integers being passed into it...
 * Usually you would only call it from the terminal.
 */
void setMinTemp(int temp)
{
	if (isTemperatureWithinAcceptableRange(temp) && temp < getMaxTemp())
	{
		minimumTemperature = temp;	

		char message[MAX_MESSAGE_LENGTH];
		sprintf(message, "minimum temperature set to: %d", temp);
				
		msgQSend(getMQTemperature(), message, MAX_MESSAGE_LENGTH, NO_WAIT, MSG_PRI_NORMAL);
	}
	else
	{
		char message[MAX_MESSAGE_LENGTH];
		sprintf(message, "setting the minimum temperature to %d- value unacceptable", temp);
		
		msgQSend(getMQExceptions(), message, MAX_MESSAGE_LENGTH, NO_WAIT, MSG_PRI_NORMAL);	
	}
}

/*
 * Allows the user to set the tank's maximum temperature.
 * 
 * NOTE:
 * This function only works with integers being passed into it.
 * Usually you would only call it from the terminal.
 */
void setMaxTemp(int temp)
{	
	if (isTemperatureWithinAcceptableRange(temp) && temp > getMinTemp())
	{
		maximumTemperature = temp;
		
		char message[MAX_MESSAGE_LENGTH];
		sprintf(message, "maximum temperature set to: %d", temp);
		
		msgQSend(getMQTemperature(), message, MAX_MESSAGE_LENGTH, NO_WAIT, MSG_PRI_NORMAL);
	}
	else
	{
		char message[MAX_MESSAGE_LENGTH];
		sprintf(message, "setting the maximum temperature to %d value unacceptable", temp);
		
		msgQSend(getMQExceptions(), message, MAX_MESSAGE_LENGTH, NO_WAIT, MSG_PRI_NORMAL);
	}
}

// Getter.
double getMinTemp()
{
	semTake(mtx, NO_WAIT);
		return minimumTemperature;
	semGive(mtx);
}

// Getter.
double getMaxTemp()
{
	semTake(mtx, NO_WAIT);
		return maximumTemperature;
	semGive(mtx);
}

// Getter.
double getCurrentTemperature()
{
	semTake(mtx, NO_WAIT);
		return currentTemperature;
	semGive(mtx);
}

/*
 * Shuts down the tasks related to the temperature sensor, turns the heater off,
 * and notifies the printer.
 */
static void doShutdown()
{
	taskDelete(TID_TEMPERATURESENSOR);
	taskDelete(TID_CALCTEMPERATURE);
	
	resetHeater();
	
	msgQSend(getMQTemperature(), "temperature sensor shutdown", MAX_MESSAGE_LENGTH, NO_WAIT, MSG_PRI_NORMAL);
	
	taskDelete(taskShutdown);
}

/*
 * A naive approach to calculating temperature... simply increases it by a small
 * amount if the heater is on, and decreases it by a small amount if the heater is
 * off. A more real-life method would include:
 * 
 * Temperature as a function of the water volume (more water, slower temperature
 * increase/decrease) and the number of inlets open (more inlets open, more cool
 * water entering the tank).
 */
static void calculateTemperature()
{	
	while (1)
	{
		if (!isWaterHeaterOn())
		{
			currentTemperature = currentTemperature - temperatureChangeRate;
		}
		else 
		{
			currentTemperature = currentTemperature + temperatureChangeRate;
		}
		
		taskDelay(temperatureCalcDelayTicks);
	}
}

/*
 * Monitors the temperature every tick and performs appropriate operations to
 * turn the heater on/off.
 */
static void monitorTemperature()
{
	while (1)
	{
		if (currentTemperature > getMaxTemp())
		{
			if (isWaterHeaterOn())
			{
				msgQSend(getMQTemperature(), "maximum temperature reached", MAX_MESSAGE_LENGTH, NO_WAIT, MSG_PRI_NORMAL);

				turnWaterHeaterOff();		
			}
		}
		else if (currentTemperature < getMinTemp())
		{
			if (!isWaterHeaterOn())
			{
				msgQSend(getMQTemperature(), "minimum temperature reached", MAX_MESSAGE_LENGTH, NO_WAIT, MSG_PRI_NORMAL);
				
				turnWaterHeaterOn();
			}
		} 
		
		taskDelay(REALTIME_DELAY);
	}
}

static boolean isTemperatureWithinAcceptableRange(double temp)
{
	return temp < MAX_TEMP_CELSIUS && temp > MIN_TEMP_CELSIUS;
}
