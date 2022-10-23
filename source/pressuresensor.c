#include "../header/includes.h"

// Normal task parameters.
static const int priorityMonitorPressure = 112;
static const char taskNameMonitorPressure[] = "tMonPress";
static const int priorityCalcPressure = 113;
static const char taskNameCalcPressure[] = "tCalPress";

// Shutdown task parameters.
static int taskShutdown;
static const int priorityShutdown = 101;
static const char taskNameShutdown[] = "tSdPress";

// Task delay.
static const int pressureCalcDelayTicks = 15;

// Pressure value.
static double currentPressurePSI = NORMAL_PRESSURE_PSI;

// Facade for initializing all tasks related to the pressure sensor.
void initializePressureSensor()
{
	TID_PRESSURESENSOR = taskSpawn(taskNameMonitorPressure, priorityMonitorPressure, DEFAULT_TASK_OPTIONS, DEFAULT_TASK_SIZE, monitorPressure, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	TID_CALCPRESSURE = taskSpawn(taskNameCalcPressure, priorityCalcPressure, DEFAULT_TASK_OPTIONS, DEFAULT_TASK_SIZE, calculatePressure, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	
	msgQSend(getMQPressure(), "INITIALIZED", MAX_MESSAGE_LENGTH, NO_WAIT, MSG_PRI_NORMAL);
}

// Facade for shutting down all tasks related to the pressure sensor.
void shutdownPressureSensor()
{
	taskShutdown = taskSpawn(taskNameShutdown, priorityShutdown, DEFAULT_TASK_OPTIONS, DEFAULT_TASK_SIZE, doShutdown, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

// Getter.
double getCurrentPressure()
{
	return currentPressurePSI;
}

// Shuts down all tasks related to the pressure sensor and notifies the printer.
static void doShutdown()
{
	taskDelete(TID_PRESSURESENSOR);
	taskDelete(TID_CALCPRESSURE);
	
	msgQSend(getMQPressure(), "pressure sensor shutdown", MAX_MESSAGE_LENGTH, NO_WAIT, MSG_PRI_NORMAL);
	
	taskDelete(taskShutdown);
}

/*
 * A naive approach to calculating pressure. A more real-life method would include:
 * 
 * Pressure as a function of the temperature (higher temperature, higher pressure)
 * Pressure as a function of the water level (I assume less water can allow for
 * higher pressure, as if the heater is just heating an empty tank, the hot air
 * will put more pressure on the tank than hot water would [I think])
 */
static void calculatePressure()
{
	// Chosen randomly, it simulates basic pressure increase	
	double someDivisor = 2.8;
	
	while (1)
	{
		// Bases current pressure on temperature and water level so it's "realistic"
		currentPressurePSI = (getCurrentTemperature() + getCurrentWaterLevel()) / someDivisor;
		
		addToPressure();
		subtractFromPressure();
						
		taskDelay(pressureCalcDelayTicks);
	}
}

/*
 * Monitors the pressure every tick and performs appropriate actions to keep the
 * pressure above the maximum specified by shutting off the heater and emptying
 * the tank.
 */
static void monitorPressure()
{
	while (1)
	{
		if (currentPressurePSI >= MAX_PRESS_PSI)
		{
			if (!isOutletOpen())
			{
				msgQSend(getMQPressure(), "maximum pressure reached", MAX_MESSAGE_LENGTH, NO_WAIT, MSG_PRI_NORMAL);
				emptyTank();
			}
		}
		else if (currentPressurePSI <= MIN_PRESS_PSI)
		{
			if (!isWaterHeaterOn())
			{
				msgQSend(getMQPressure(), "minimum pressure reached", MAX_MESSAGE_LENGTH, NO_WAIT, MSG_PRI_NORMAL);
				turnWaterHeaterOn();
			}
		}
		
		taskDelay(REALTIME_DELAY);
	}
}

// Adds additional pressure, given two circumstances
static void addToPressure()
{
	double extra = 1.5;
	
	// Specified: pressure increases when water reaches 2nd-to-highest sensor
	if (isWaterBuildingPressure() && !isOutletOpen())
	{
		currentPressurePSI += extra;
	}
	
	// Specified: pressure increases faster when temperature is "close to" 100C
	if (isPressureBuildingFast())
	{
		if (isWaterHeaterOn())
		{
			currentPressurePSI += extra;
		}
		else
		{
			currentPressurePSI -= extra;
		}
	}
}

// Subtracts from pressure if the outlet is open
static void subtractFromPressure()
{
	if (isOutletOpen())
	{
		double extra = 2.0;	
		/*
		 * Bending the laws of physics... real-life pressure won't go below a certain point, unless
		 * it's a vacuum, and this system is not. I could make a running formula that ensured this
		 * chosen minimum pressure value MIN_PRESS_PSI would not be crossed below, but, I hate math.
		 */
		if (currentPressurePSI <= MIN_PRESS_PSI)
		{
			currentPressurePSI = MIN_PRESS_PSI;
		}
		else
		{
			currentPressurePSI -= extra;				
		}
	}
}

static boolean isPressureBuildingFast()
{
	return getCurrentTemperature() >= PRESSURE_BUILD_TEMP 
			&& !isOutletOpen();
}
