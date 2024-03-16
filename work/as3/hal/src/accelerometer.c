
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <stdbool.h>
#include "hal/accelerometer.h"
#include "hal/utilities.h"
#include "hal/periodTimer.h"

#define I2C_ADDRESS 0x1C
#define I2C_BUS1 "/dev/i2c-1"
#define CTRL_REG1 0x2A





static int i2cFileDesc;
static bool stopping;

int16_t x;
int16_t y;
int16_t z;


// initialize I2C bus and enable the device
void Accelerometer_init() {
    stopping = false;
    FILE* pFile;
    // export GPIOs
    if(1==1) {
        runCommand("config-pin p9.18 gpio");
        DIR *dir = opendir("/sys/class/gpio/gpio4");
        // if gpio is not already exported then export it
        if(!dir) {
            pFile = fopen("/sys/class/gpio/export", "w");
            if(pFile == NULL) {
                printf("Couldn't open gpio4 export file");
                exit(1);
            }
            fprintf(pFile, "%d", 4);
            fclose(pFile);
            sleepForMs(300);
        }
        pFile = fopen("/sys/class/gpio/gpio4/direction", "w");
        if(pFile == NULL) {
            printf("Couldn't open gpio4 direction file");
            exit(1);
        }    
        fprintf(pFile, "%s", "in");
        fclose(pFile);
        closedir(dir);
        sleepForMs(300);
    }
    if(1==1) {
        runCommand("config-pin p9.17 gpio");
        DIR *dir = opendir("/sys/class/gpio/gpio5");
        // if gpio is not already exported then export it
        if(!dir) {
            pFile = fopen("/sys/class/gpio/export", "w");
            if(pFile == NULL) {
                printf("Couldn't open gpio5 export file");
                exit(1);
            }
            fprintf(pFile, "%d", 5);
            fclose(pFile);
            sleepForMs(300);
        }
        pFile = fopen("/sys/class/gpio/gpio5/direction", "w");
        if(pFile == NULL) {
            printf("Couldn't open gpio5 direction file");
            exit(1);
        }    
        fprintf(pFile, "%s", "in");
        fclose(pFile);
        closedir(dir);
        sleepForMs(300);
    }
    // ensure the pins are configured to i2c
    runCommand("config-pin P9_18 i2c");
    runCommand("config-pin P9_17 i2c");
    i2cFileDesc = open(I2C_BUS1, O_RDWR);
    int result = ioctl(i2cFileDesc, I2C_SLAVE, I2C_ADDRESS);
    if(result < 0) {
        perror("I2C: unable to set accelerometer to slave address");
        exit(1);
    }

    unsigned char buff[2];
    buff[0] = CTRL_REG1;    // register address for CTRL_REG1
    buff[1] = 1;            // enabling bit 0 turns on the device (0b00000001)
    int res = write(i2cFileDesc, buff, 2);
    if(res != 2) {
        perror("I2C: unable to enable accelerometer");
        exit(1);
    }

    return;
}

void Accelerometer_read(unsigned char regAddr) {

    // to read a register, must first write the address
    write(i2cFileDesc, &regAddr, sizeof(OUT_X_MSB));

    // i have no idea why the first byte i read is all 1's. I just read 7 bytes instead and ignore the first one and it works..? 
    char value[7];
    read(i2cFileDesc, &value, 7 * sizeof(char));

    // combine the values from the two registers from a certain direction to get the whole value
    x = value[1] << 8 | value[2];
    y = value[3] << 8 | value[4];
    z = value[5] << 8 | value[6];
    //printf("%d %d %d\n", x, y, z);

    Period_markEvent(PERIOD_EVENT_ACCELEROMETER_SAMPLED);
    return;
}

int16_t Accelerometer_getX() {
    return x;
}

int16_t Accelerometer_getY() {
    return y;
}

int16_t Accelerometer_getZ() {
    return z;
}

void Accelerometer_stop() {
    return;
}


