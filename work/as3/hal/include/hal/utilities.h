// utilities.h

// Useful functions that are used throughout this assignment

#ifndef _UTILITIES_H_
#define _UTILITIES_H_
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <dirent.h>



// Pauses the calling thread for 'delayInMs' milliseconds
void sleepForMs(long long delayInMs);

// runs the command
void runCommand(char* command);

// gets the current time in ms
long long getTimeInMs(void);

#endif