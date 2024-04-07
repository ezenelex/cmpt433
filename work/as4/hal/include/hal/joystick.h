#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <dirent.h>

#define JOYSTICK_UP_GPIO 26
#define JOYSTICK_DOWN_GPIO 46
#define JOYSTICK_LEFT_GPIO 65
#define JOYSTICK_RIGHT_GPIO 47
#define JOYSTICK_PUSHED_GPIO 27

void Joystick_setupGPIOs();

bool Joystick_readGPIO(unsigned int gpio);

#endif