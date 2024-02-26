// Useful functions

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

long long getTimeInMs(void);

#endif