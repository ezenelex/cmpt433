#include <stdio.h>
#include <stdlib.h>

#include <math.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <pthread.h>
#include <dirent.h>

#include "hal/sampler.h"
#include "hal/utilities.h"
#include "hal/display.h"




int main()
{
    /*
    setup_display_gpios();
    int i2cFileDesc = initI2cBus(I2CDRV_LINUX_BUS1, I2C_DEVICE_ADDRESS);

    // Set the direction of both 8-bit ports on I2C GPIO extender to be outputs
    writeI2cReg(i2cFileDesc, REG_DIRA, 0x00);
    writeI2cReg(i2cFileDesc, REG_DIRB, 0x00);

    number = 0;
    float photoresistor_value = 0;

    pthread_t display_thread;
    pthread_create( &display_thread, NULL, display_number, (void*)&i2cFileDesc);

    pthread_t photoresistor_thread;
    pthread_create( &photoresistor_thread, NULL, readPhotoresistor, (void*)&photoresistor_value);

    for(int i = 1; i < 100; i++) {
        number++;
        sleepForMs(100);
    }

    close(i2cFileDesc);

    done = true;
    pthread_join(display_thread, NULL);
    pthread_join(photoresistor_thread, NULL);
    */

    ///////////////// ACTUAL MAIN ///////////////////////    

    Display_setupGPIOs();
    int i2cFileDesc = Display_initI2cBus(I2CDRV_LINUX_BUS1, I2C_DEVICE_ADDRESS);
    


    // start "clock" thread that spawns a bunch of threads and sleeps for 1 second
    pthread_t xyz_thread;
    pthread_create( &xyz_thread, NULL, xyz, NULL);
    // start UDP thread
    // start LED thread
    // start seg display thread
    pthread_t display_thread;
    pthread_create( &display_thread, NULL, Display_start, (void*)&i2cFileDesc);

    // wait for something to trigger end of program, set "done" to true which tells all threads that they can return
    sleepForMs(10000);
    

    // join all threads
    Sampler_cleanup();
    Display_stop();
    pthread_join(xyz_thread, NULL);
    pthread_join(display_thread, NULL);


    // return 0
    return 0;

}

/*
void *display_number(void *i2cFileDesc) {
    int first_digit;
    int second_digit;

    // files to enable/disable digits
    FILE* f1;
    FILE* f2;

    while(!done) {
        first_digit = floor(number / 10);
        second_digit = number % 10;
        // turn off both digits
        f1 = fopen("/sys/class/gpio/gpio61/value", "w");
        f2 = fopen("/sys/class/gpio/gpio44/value", "w"); 
        fprintf(f1, "%d", 0);
        fprintf(f2, "%d", 0);
        fclose(f1);
        fclose(f2);
        // Drive I2C GPIO extender to display pattern for left digit
        writeI2cReg(*(int*)i2cFileDesc, REG_OUTA, digit_a[first_digit]);
        writeI2cReg(*(int*)i2cFileDesc, REG_OUTB, digit_b[first_digit]);
        // Turn on left digit via GPIO 61 set to a 1. Wait for a little time (5ms)
        f1 = fopen("/sys/class/gpio/gpio61/value", "w");
        fprintf(f1, "%d", 1);
        fclose(f1);
        sleepForMs(5);
        // turn off both digits
        f1 = fopen("/sys/class/gpio/gpio61/value", "w");
        f2 = fopen("/sys/class/gpio/gpio44/value", "w"); 
        fprintf(f1, "%d", 0);
        fprintf(f2, "%d", 0);
        fclose(f1);
        fclose(f2);
        // Drive I2C GPIO extender to display pattern for right digit
        writeI2cReg(*(int*)i2cFileDesc, REG_OUTA, digit_a[second_digit]);
        writeI2cReg(*(int*)i2cFileDesc, REG_OUTB, digit_b[second_digit]);
        // Turn on right digit via GPIO 44 set to a 1. Wait for a little time (5ms)
        f2 = fopen("/sys/class/gpio/gpio44/value", "w"); 
        fprintf(f2, "%d", 1);
        fclose(f2);
        sleepForMs(5);
    }
    return NULL;
}

*/


