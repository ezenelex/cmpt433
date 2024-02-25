// Sample button module 
// Part of the Hardware Abstraction Layer (HAL) 

#define JOYSTICK_UP_GPIO 26
#define JOYSTICK_DOWN_GPIO 46
#define JOYSTICK_LEFT_GPIO 65
#define JOYSTICK_RIGHT_GPIO 47

#ifndef _BUTTON_H_
#define _BUTTON_H_

#include <stdbool.h>

void runCommnad(char* command);
void sleepForMs(long long delayInMs);
void control_led(unsigned int led_num, bool brightness);
void setup_led();
void setup_joystick_gpios();
bool read_gpio(unsigned int gpio);

#endif