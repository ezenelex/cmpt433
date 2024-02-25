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


void *Display_start(void *i2cFileDesc);

void Display_stop();

void Display_setNumber(unsigned int number);

void Display_setupGPIOs();

int Display_initI2cBus(char* bus, int address);

int Display_writeI2cRegister(int i2cFileDesc, unsigned char regAddr, unsigned char value);

#endif