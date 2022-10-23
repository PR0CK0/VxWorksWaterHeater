/* --------------- TASK/CONSTRUCT ------------------ */
// Task constants.
#define DEFAULT_TASK_OPTIONS 0x100
#define DEFAULT_TASK_SIZE 2000
#define REALTIME_DELAY 1

// Constants for message queue use.
#define MAX_MESSAGE_LENGTH 100
#define MAX_MESSAGES 10

/* --------------- TANK-SPECIFIC ------------------ */
// Number of water inlets on the tank.
#define NUMBER_OF_INLETS 2

/* --------------- TEMPERATURE ------------------ */
// Min/max acceptable temperature values (different from the user-specified ones).
#define MIN_TEMP_CELSIUS 10
#define MAX_TEMP_CELSIUS 100

// Value for standard starting temperature (made-up).
#define NORMAL_TEMP_CELSIUS 30

/* 
 * Constant threshold at which pressure builds faster. Required.
 * Specified as: "pressure builds faster when temperature is kept close to 100C".
 * Ambiguous requirement, so chose a 20C degree buffer as "close to" arbitrarily.
 */
#define PRESSURE_BUILD_TEMP 80

/* --------------- PRESSURE ------------------ */
// Minimum and maximum pressure values. 
#define MIN_PRESS_PSI 20
#define MAX_PRESS_PSI 50

// Value for standard room pressure (made-up).
#define NORMAL_PRESSURE_PSI 30

/* ----------------- WATER-LEVEL --------------------- */
/*
 * The maximum tank size specified is 50L and the minimum is 0L. We give a buffer 
 * room of 1 L to ensure that the tank is never going to be too full.
 */
#define TANK_SIZE_LITERS 50
#define MAX_WATER_LEVEL 49
#define MIN_WATER_LEVEL 0
