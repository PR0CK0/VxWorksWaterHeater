#include "../header/includes.h"

/*
	Hey, Yevgeniy. I included this module just to show that an effort
	was put forth to create a more cohesive (and less redundant) system
	shutdown process.
	
	What I did works (and so does this), but I liked my old method better.
	
	You can also see that I employed some error checking in this module, 
	but did not in the working modules. This is because the error checking
	code had so much overhead that the system startup's responsiveness went
	to crap and the program would crash on startup. So, without error
	checking (when spawning tasks, etc.), the program works fine and is fast.
	
	-TP
*/

// Shutdown task parameters.
static int taskShutdown;
static const int priorityShutdown = 100;
static const char taskNameShutdown[] = "tShutdown";

// Spawns a task for shutting down the entire system.
//void shutdownSystem()
//{
//	if (taskShutdown = taskSpawn(taskNameShutdown, priorityShutdown, DEFAULT_TASK_OPTIONS, DEFAULT_TASK_SIZE, 
//				doShutdown, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) == ERROR)
//	{
//		msgQSend(getMQExceptions(), "ERROR initializing shutdown task... that really sucks", MAX_MESSAGE_LENGTH, NO_WAIT, MSG_PRI_NORMAL);
//	}
//}
//
//// Performs all actions needed to shutdown the system properly.
//static void doShutdown()
//{
//	shutdownTask(TID_WATERLEVELSENSORS);
//	shutdownTask(TID_CALCWATERLEVEL);
//	shutdownTask(TID_PRESSURESENSOR);
//	shutdownTask(TID_CALCPRESSURE);
//	shutdownTask(TID_TEMPERATURESENSOR);
//	shutdownTask(TID_CALCTEMPERATURE);
//	shutdownTask(TID_TICKS);
//	shutdownTask(TID_TIMESTAMP);
//	
//	msgQSend(getMQWaterLevel(), "water level sensors shutdown", MAX_MESSAGE_LENGTH, NO_WAIT, MSG_PRI_NORMAL);
//	msgQSend(getMQPressure(), "pressure sensor shutdown", MAX_MESSAGE_LENGTH, NO_WAIT, MSG_PRI_NORMAL);	
//	msgQSend(getMQTemperature(), "temperature sensor shutdown", MAX_MESSAGE_LENGTH, NO_WAIT, MSG_PRI_NORMAL);
//	
//	resetOutlet();
//	resetHeater();
//			
//	shutdownTask(TID_PRINTERCONSISTENT);
//	shutdownTask(TID_PRINTERALERT);
//	
//	logMsg("\n\n--- SYSTEM SHUTDOWN COMPLETE ---\n\n", 0, 0, 0, 0, 0, 0);
//	
//	taskDelete(taskShutdown);
//}
//
//// Shuts down individual tasks and checks for errors in doing so.
//static void shutdownTask(int tid)
//{
//	if (taskDelete(tid) == ERROR)
//	{
//		char message[MAX_MESSAGE_LENGTH];
//		sprintf(message, "error shutting down task: %s", taskName(tid));
//		
//		msgQSend(getMQExceptions(), message, MAX_MESSAGE_LENGTH, NO_WAIT, MSG_PRI_NORMAL);
//	}
//}
