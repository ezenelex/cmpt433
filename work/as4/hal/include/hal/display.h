// display.h

// Module to setup and update the 14-segment display (uses a thread).
// It first requires the user the setupGPIOs() and initI2cBus() in that order.
// Then the display can be started with Display_start().
// The display's number can be changed with Display_setNumber().
// The caller stops the Display with Display_stop().

#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include <stdio.h>
#include <dirent.h>
#include <stdbool.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include "hal/utilities.h"

#define I2CDRV_LINUX_BUS0 "/dev/i2c-0"
#define I2CDRV_LINUX_BUS1 "/dev/i2c-1"
#define I2CDRV_LINUX_BUS2 "/dev/i2c-2"

// I2C registers for each digit of the 14-segment display. Basically write 0x00 to both of these to set all GPIOs in the expander to outputs
#define REG_DIRA 0x00
#define REG_DIRB 0x01
// I2C registers for ONE digit that store the shape of the digit. The first half of the shape is in 0x14, other in 0x15
#define REG_OUTA 0x14
#define REG_OUTB 0x15
// I2C device address of the 14-segment display
#define I2C_DEVICE_ADDRESS 0x20

// Call this in a thread, it will keep running in the background.
// This function constantly checks the variable 'number' and updates the display accordingly.
// (Assumes that Display_setupGPIOs() and Display_initI2cBus() have been called).
// Stop with Display_stop()
void *Display_start(void *i2cFileDesc);

// Stops the Display by setting the variable 'done' to true. The loop inside of Display_start()
// keeps checking if done is equal to true.
void Display_stop();

// Changes the number the display shows
void Display_setNumber(int num);

void Display_increment();
void Display_decrement();
int Display_getNumber();

// Sets up the required GPIOs for the display to work
void Display_setupGPIOs();

// Sets up the required I2C bus and pins for the display to work
int Display_initI2cBus(char* bus, int address);

// Writes values to the I2C bus to change what the display is showing. This function is only called by Display_start(), no need to ever call it yourself.
int Display_writeI2cRegister(int i2cFileDesc, unsigned char regAddr, unsigned char value);

#endif