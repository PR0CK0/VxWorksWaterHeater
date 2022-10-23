#include "../header/includes.h"

// Normal task parameters.
static const int priorityPrinterAlert = 190;
static const char taskNamePrinterAlert[] = "tPrintAlert";
static const int priorityPrinterConsistent = 191;
static const char taskNamePrinterConsistent[] = "tPrintConst";

// Shutdown task parameters.
static int taskShutdown;
static const int priorityShutdown = 104;
static const char taskNameShutdown[] = "tSdPrinter";

// Task delays.
static const int consistentDelayTicks = 110;

/*
 * All message queues. One for each sensor (temp, press, wtr lvl), one for
 * each actuator (inlet, outlet, heater), one for exceptions and one for
 * the timestamping.
 */
static MSG_Q_ID mqTemperatureSensor;
static MSG_Q_ID mqPressureSensor;
static MSG_Q_ID mqWaterLevelSensors;
static MSG_Q_ID mqOutlet;
static MSG_Q_ID mqInlets;
static MSG_Q_ID mqHeater;
static MSG_Q_ID mqTimestamp;
static MSG_Q_ID mqExceptions;

// Facade for initializing all of the message queues used and all tasks related to the printer.
void initializePrinter()
{	
	mqTemperatureSensor = msgQCreate(MAX_MESSAGES, MAX_MESSAGE_LENGTH, MSG_Q_FIFO);
	mqPressureSensor = msgQCreate(MAX_MESSAGES, MAX_MESSAGE_LENGTH, MSG_Q_FIFO);
	mqWaterLevelSensors = msgQCreate(MAX_MESSAGES, MAX_MESSAGE_LENGTH, MSG_Q_FIFO);
	mqOutlet = msgQCreate(MAX_MESSAGES, MAX_MESSAGE_LENGTH, MSG_Q_FIFO);
	mqInlets = msgQCreate(MAX_MESSAGES, MAX_MESSAGE_LENGTH, MSG_Q_FIFO);
	mqHeater = msgQCreate(MAX_MESSAGES, MAX_MESSAGE_LENGTH, MSG_Q_FIFO);
	mqTimestamp = msgQCreate(MAX_MESSAGES, MAX_MESSAGE_LENGTH, MSG_Q_FIFO);
	mqExceptions = msgQCreate(MAX_MESSAGES, MAX_MESSAGE_LENGTH, MSG_Q_FIFO);
	
	TID_PRINTERALERT = taskSpawn(taskNamePrinterAlert, priorityPrinterAlert, DEFAULT_TASK_OPTIONS, DEFAULT_TASK_SIZE, printAlertData, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	TID_PRINTERCONSISTENT = taskSpawn(taskNamePrinterConsistent, priorityPrinterConsistent, DEFAULT_TASK_OPTIONS, DEFAULT_TASK_SIZE, printConsistentData, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

/*
 * Prints any new data on the message queues by checking them every tick.
 * 
 * One dependency: the timestamp task(s) never send messages except when the shutdown process is occuring.
 * When this message is received on the timestamp message queue, the tasks related to the printer are
 * shut down and the program terminates.
 * 
 * This is coupled, I know. But the regular timestamp tasks never have a case where they put something on
 * the message queue
 */
void printAlertData()
{
	char message[MAX_MESSAGE_LENGTH];
	
	while (1)
	{
		if (msgQReceive(mqTemperatureSensor, message, MAX_MESSAGE_LENGTH, 0) != ERROR)
		{
			logMsg("\t--- TEMP SENSOR: %s at %d ticks, %ds, %dns\n", message, getCurrentTicks(), getTimeInSecs(), getTimeInNano(), 0, 0);
		}
		
		if (msgQReceive(mqPressureSensor, message, MAX_MESSAGE_LENGTH, 0) != ERROR)
		{
			logMsg("\t--- PRESS SENSOR: %s at %d ticks, %ds, %dns\n", message, getCurrentTicks(), getTimeInSecs(), getTimeInNano(), 0, 0);		
		}
		
		if (msgQReceive(mqWaterLevelSensors, message, MAX_MESSAGE_LENGTH, 0) != ERROR)
		{
			logMsg("\t--- WTR LVL SENSORS: %s at %d ticks, %ds, %dns\n", message, getCurrentTicks(), getTimeInSecs(), getTimeInNano(), 0, 0);			
		}
		
		if (msgQReceive(mqOutlet, message, MAX_MESSAGE_LENGTH, 0) != ERROR)
		{
			logMsg("\t--- OUTLET: %s at %d ticks, %ds, %dns\n", message, getCurrentTicks(), getTimeInSecs(), getTimeInNano(), 0, 0);		
		}
		
		if (msgQReceive(mqInlets, message, MAX_MESSAGE_LENGTH, 0) != ERROR)
		{
			logMsg("\t--- INLETS: %s at %d ticks, %ds, %dns\n", message, getCurrentTicks(), getTimeInSecs(), getTimeInNano(), 0, 0);			
		}
		
		if (msgQReceive(mqHeater, message, MAX_MESSAGE_LENGTH, 0) != ERROR)
		{
			logMsg("\t--- HEATER: %s at %d ticks, %ds, %dns\n", message, getCurrentTicks(), getTimeInSecs(), getTimeInNano(), 0, 0);			
		}
		
		if (msgQReceive(mqExceptions, message, MAX_MESSAGE_LENGTH, 0) != ERROR)
		{
			logMsg("\t--- EXCEPTION: %s at %d ticks, %ds, %dns\n", message, getCurrentTicks(), getTimeInSecs(), getTimeInNano(), 0, 0);			
		}
		
		if (msgQReceive(mqTimestamp, message, MAX_MESSAGE_LENGTH, 0) != ERROR)
		{
			logMsg("\t--- TIMESTAMP: %s at %d ticks, %ds, %dns\n", message, getCurrentTicks(), getTimeInSecs(), getTimeInNano(), 0, 0);
			taskShutdown = taskSpawn(taskNameShutdown, priorityShutdown, 0x100, 2000, doShutdown, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		}
		
		taskDelay(REALTIME_DELAY);
	}
}

// Every specified period, prints the temperature, pressure and water level values.
void printConsistentData()
{
	while (1)
	{
		double waterLevel = getCurrentWaterLevel();
		int *ptrWtrLvl = (int*)&waterLevel;
		
		double temp = getCurrentTemperature();
		int *ptrTemp = (int*)&temp;
		
		double pressure = getCurrentPressure();
		int *ptrPress = (int*)&pressure;
		
		logMsg("\tWTR LVL: %.1lfL - TEMP: %.1lfC - PRESS: %.1lfPSI\n", ptrWtrLvl[0], ptrWtrLvl[1], ptrTemp[0], ptrTemp[1], ptrPress[0], ptrPress[1]);
		
		taskDelay(consistentDelayTicks);
	}
}

// Getter.
MSG_Q_ID getMQTemperature()
{
	return mqTemperatureSensor;
}

// Getter.
MSG_Q_ID getMQPressure()
{
	return mqPressureSensor;
}

// Getter.
MSG_Q_ID getMQWaterLevel()
{
	return mqWaterLevelSensors;
}

// Getter.
MSG_Q_ID getMQOutlet()
{
	return mqOutlet;
}

// Getter.
MSG_Q_ID getMQInlets()
{
	return mqInlets;
}

// Getter.
MSG_Q_ID getMQHeater()
{
	return mqHeater;
}

// Getter.
MSG_Q_ID getMQExceptions()
{
	return mqExceptions;
}

// Getter.
MSG_Q_ID getMQTimestamp()
{
	return mqTimestamp;
}

// Performs the final shutdown operation by shutting down all printer-related tasks.
static void doShutdown()
{	
	taskDelete(TID_PRINTERCONSISTENT);
	taskDelete(TID_PRINTERALERT);
	
	logMsg("\n\n\t--- FULL SYSTEM SHUTDOWN COMPLETE ---\n\n", 0, 0, 0, 0, 0, 0);
	
	taskDelete(taskShutdown);
}
