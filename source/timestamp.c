#include "../header/includes.h"

// Normal task parameters.
static const int priorityTicks = 180;
static const char taskNameTicks[] = "tTickTimer";
static const int priorityTimestamp = 181;
static const char taskNameTimestamp[] = "tTimestamp";

// Shutdown task parameters.
static int taskShutdown;
static const int priorityShutdown = 103;
static const char taskNameShutdown[] = "tSdTiming";

// Global variable for keeping track of the number of ticks since program start.
static int ticks = 0;

// Structure to hold second/nanosecond values since program start.
static struct timespec_1
{
	time_t tv_sec;
	time_t tv_nsec;
}tstamp;

// Facade for initializing all tasks related to the timestamping work.
void initializeTimestamping()
{
	TID_TICKS = taskSpawn(taskNameTicks, priorityTicks, DEFAULT_TASK_OPTIONS, DEFAULT_TASK_SIZE, tickMaster, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	TID_TIMESTAMP = taskSpawn(taskNameTimestamp, priorityTimestamp, DEFAULT_TASK_OPTIONS, DEFAULT_TASK_SIZE, timeStamper, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

// Facade for shutting down all tasks related to the timestamping work.
void shutdownTimestamping()
{
	taskShutdown = taskSpawn(taskNameShutdown, priorityShutdown, DEFAULT_TASK_OPTIONS, DEFAULT_TASK_SIZE, doShutdown, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

// Getter.
int getTimeInSecs()
{
	return tstamp.tv_sec;
}

// Getter.
int getTimeInNano()
{
	return tstamp.tv_nsec;
}

// Getter.
int getCurrentTicks()
{
	return ticks;
}

// Shuts down the tasks related to the timestamping work and notifies the printer.
static void doShutdown()
{
	taskDelete(TID_TICKS);
	taskDelete(TID_TIMESTAMP);
	
	msgQSend(getMQTimestamp(), "timestamp shutdown", MAX_MESSAGE_LENGTH, NO_WAIT, MSG_PRI_NORMAL);
}

// Increases the tick count by one every tick.
static void tickMaster()
{
	while (1)
	{
		ticks++;
		
		taskDelay(REALTIME_DELAY);
	}
}

// Updates the time for the structure tstamp in seconds and nanoseconds every tick.
static void timeStamper()
{
	tstamp.tv_sec = 0;
	tstamp.tv_nsec = 0;
	clock_settime(CLOCK_REALTIME, &tstamp);

	while (1)
	{
		clock_gettime(CLOCK_REALTIME, &tstamp);
		taskDelay(REALTIME_DELAY);
	}
}
