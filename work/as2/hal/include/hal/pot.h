// pot.h

// This module controls the potentiometer.
// The function that does all the reading (POT_start()) runs in a thread.
// Each loop of the thread, it checks if the variable 'done' is true, then completes its last loop
// and joins the rest of the stopped threads.

#ifndef _POT_H_
#define _POT_H_

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "hal/utilities.h"

// Starts the infinite potentiometer reading loop.
// Only stops when POT_stop() is called, which simply sets 'done' to true
// which instructs the loop in this function to end.
void* POT_start();


// Sets 'done' to true, which stops POT_start()
void POT_stop();


// returns the current potentiometer reading
unsigned int POT_getReading();

#endif