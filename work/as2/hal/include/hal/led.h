#ifndef _LED_H_
#define _LED_H_

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "hal/utilities.h"
#include "hal/pot.h"

void* LED_start();

void LED_setup();
 
void LED_stop();

unsigned int LED_getFreq();

unsigned int LED_getPeriod();

#endif