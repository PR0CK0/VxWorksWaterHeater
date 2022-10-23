#include "../header/includes.h"

/*
	Hey, Yevgeniy. Some things:
	
	- All code you read is mine. My teammates helped with nothing in this regard.
	
	- When turning the system off, in the last output to the console, the pressure
	  value printed sometimes gets mangled. No idea why. Doesn't affect anything.
	  
	- The math for the pressure sensor is really simulated, in that it does not
	  portray real-life very well, but it does the job. I followed the requirements
	  that pressure increases when the water-level reaches the 2nd-highest sensor
	  and increases more when temperature is "near" 100C, but also made it so that
	  pressure increases along with temperature/water-level increase, so it's more
	  dynamic. You can check all of the values used in constants.h.
	  
	- If you want to test all cases and see temperature and pressure hit
	  their max values, just change the default maxes in the constants.h file.
	  
	- Message queue use is in printer.c, however each module does use the MQs defined
	  there. Semaphore use is in temperaturesensor, waterlevelsensor, outlet, inlets
	  and heater. Watchdogs are used in heater and outlet.
	  
	- Read the comments in shutdown_readme.c if you'd like.
	  
	- Sorry if I missed any of the requirements. I am liable to miss some things, 
	  as I was a one-man team, on a team of five :(
	  
	Thanks for all of your help in the lab,
	TP
*/

void turnOn();
void turnOff();
void aa();
void ab();
void ac();
void ad();

// Facade for turning the system on.
void turnOn()
{
	initializePrinter();
	initializeTimestamping();
	setupInlets();
	setupOutlet();
	setupHeater();
	initializeTemperatureSensor();
	initializePressureSensor();
	initializeWaterLevelSensors();
}

// Facade for turning the system off.
void turnOff()
{
	shutdownTemperatureSensor();
	shutdownPressureSensor();
	shutdownWaterLevelSensors();
	shutdownTimestamping();
}

// Test case for okay water sensor changes.
void aa()
{
	adjustSensorLevels(10, 20, 25, 45);
}

// Test case for bad water sensor changes.
void ab()
{
	adjustSensorLevels(50, 40, 30, 20);
}

// Test case for okay temperature level changes.
void ac()
{
	setMaxTemp(98);
	setMinTemp(40);
}

// Test case for bad temperature level changes.
void ad()
{
	setMaxTemp(1000);
	setMinTemp(-1);
}
