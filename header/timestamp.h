#ifndef TIMESTAMP_H
#define TIMESTAMP_H

int TID_TICKS;
int TID_TIMESTAMP;

void initializeTimestamping();
void shutdownTimestamping();

int getTimeInSecs();
int getTimeInNano();
int getCurrentTicks();

static void doShutdown();
static void tickMaster();
static void timeStamper();

#endif
