// led.h

// This module controls the LED on PWN OB, corresponding to P9.21
// The LED_start() function runs an infinite loop in a thread that constantly
// updates the frequency of the PWM signal driving the light by reading the potetiometer
// value with POT_getReading()

// The LED_start() thread stops when LED_stop() is called, which simply sets 'done' to true

#ifndef _LED_H_
#define _LED_H_

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "hal/utilities.h"
#include "hal/pot.h"

// Runs in a thread
// Infinite loop that keeps checking the current potentiometer value and updating the frequency and duty
// Cycle of the LED accordingly.
// (Assumes LED_setup() was previously run)
void* LED_start();

// Sets up the LED pin P9.21 for PWM use.
// Puts in some initial values for the PWM period and duty cycle, and enables it.
void LED_setup();
 
// Stops LED_start() thread by setting 'done' to true
void LED_stop();

// Self explanatory
unsigned int LED_getFreq();

// Self explanatory
unsigned int LED_getPeriod();

#endif