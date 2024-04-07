#include "hal/joystick.h"
#include "hal/utilities.h"

void Joystick_setupGPIOs() {
    /*
    FILE *pFile;
    if(1==1) {
        runCommand("config-pin p8.14 gpio");
        DIR *dir = opendir("/sys/class/gpio/gpio26");
        // if gpio is not already exported then export it
        if(!dir) {
            pFile = fopen("/sys/class/gpio/export", "w");
            if(pFile == NULL) {
                printf("Couldn't open gpio26 export file");
                exit(1);
            }
            fprintf(pFile, "%d", JOYSTICK_UP_GPIO);
            fclose(pFile);
            sleepForMs(300);
        }
        pFile = fopen("/sys/class/gpio/gpio26/direction", "w");
        if(pFile == NULL) {
            printf("Couldn't open gpio26 direction file");
            exit(1);
        }    
        fprintf(pFile, "%s", "in");
        fclose(pFile);
        closedir(dir);
        sleepForMs(300);
    }
    if(1==1) {
        runCommand("config-pin p8.16 gpio");
        DIR *dir = opendir("/sys/class/gpio/gpio46");
        // if gpio is not already exported then export it
        if(!dir) {
            pFile = fopen("/sys/class/gpio/export", "w");
            if(pFile == NULL) {
                printf("Couldn't open gpio46 export file");
                exit(1);
            }
            fprintf(pFile, "%d", JOYSTICK_DOWN_GPIO);
            fclose(pFile);
            sleepForMs(300);
        }
        pFile = fopen("/sys/class/gpio/gpio46/direction", "w");
        if(pFile == NULL) {
            printf("Couldn't open gpio46 direction file");
            exit(1);
        }    
        fprintf(pFile, "%s", "in");
        fclose(pFile);
        closedir(dir);
        sleepForMs(300);
    }
    if(1==1) {
        runCommand("config-pin p8.15 gpio");
        DIR *dir = opendir("/sys/class/gpio/gpio47");
        // if gpio is not already exported then export it
        if(!dir) {
            pFile = fopen("/sys/class/gpio/export", "w");
            if(pFile == NULL) {
                printf("Couldn't open gpio47 export file");
                exit(1);
            }
            fprintf(pFile, "%d", JOYSTICK_RIGHT_GPIO);
            fclose(pFile);
            sleepForMs(300);
        }
        pFile = fopen("/sys/class/gpio/gpio47/direction", "w");
        if(pFile == NULL) {
            printf("Couldn't open gpio47 direction file");
            exit(1);
        }    
        fprintf(pFile, "%s", "in");
        fclose(pFile);
        closedir(dir);
        sleepForMs(300);
    }
    if(1==1) {
        runCommand("config-pin p8.18 gpio");
        DIR *dir = opendir("/sys/class/gpio/gpio65");
        // if gpio is not already exported then export it
        if(!dir) {
            pFile = fopen("/sys/class/gpio/export", "w");
            if(pFile == NULL) {
                printf("Couldn't open gpio65 export file");
                exit(1);
            }
            fprintf(pFile, "%d", JOYSTICK_LEFT_GPIO);
            fclose(pFile);
            sleepForMs(300);
        }
        pFile = fopen("/sys/class/gpio/gpio65/direction", "w");
        if(pFile == NULL) {
            printf("Couldn't open gpio65 direction file");
            exit(1);
        }    
        fprintf(pFile, "%s", "in");
        fclose(pFile);
        closedir(dir);
        sleepForMs(300);
    }
    if(1==1) {
        runCommand("config-pin p8.17 gpio");
        DIR *dir = opendir("/sys/class/gpio/gpio27");
        // if gpio is not already exported then export it
        if(!dir) {
            pFile = fopen("/sys/class/gpio/export", "w");
            if(pFile == NULL) {
                printf("Couldn't open gpio27 export file");
                exit(1);
            }
            fprintf(pFile, "%d", JOYSTICK_LEFT_GPIO);
            fclose(pFile);
            sleepForMs(300);
        }
        pFile = fopen("/sys/class/gpio/gpio27/direction", "w");
        if(pFile == NULL) {
            printf("Couldn't open gpio27 direction file");
            exit(1);
        }    
        fprintf(pFile, "%s", "in");
        fclose(pFile);
        closedir(dir);
        sleepForMs(300);
    }
    */
   runCommand("config-pin p8.15 pruin");
   sleepForMs(300);
   runCommand("config-pin p8.16 pruin");
   sleepForMs(300);
}

// Reads a joystick GPIO and returns 0 if pressed and 1 if not pressed
// The gpio argument of this function should be one of the #defined GPIOs in joystick.h
// Assumes Joystick_setupGPIOs() was called beforehand
bool Joystick_readGPIO(unsigned int gpio) {
    FILE *pFile;

    switch (gpio) {
        case JOYSTICK_UP_GPIO:
            pFile = fopen("/sys/class/gpio/gpio26/value", "r");
            break;
        case JOYSTICK_DOWN_GPIO:
            pFile = fopen("/sys/class/gpio/gpio46/value", "r");
            break;
        case JOYSTICK_LEFT_GPIO:
            pFile = fopen("/sys/class/gpio/gpio65/value", "r");
            break;
        case JOYSTICK_RIGHT_GPIO:
            pFile = fopen("/sys/class/gpio/gpio47/value", "r");
            break;
        case JOYSTICK_PUSHED_GPIO:
            pFile = fopen("/sys/class/gpio/gpio27/value", "r");
            break;
        default:
            printf("Trying to read invalid GPIO");
            exit(1);
    }
    
    if(pFile == NULL) {
        printf("Couldn't open GPIO value file");
        exit(1);
    }   

    char buff[2];
    fgets(buff, 2, pFile);
    fclose(pFile);
    return (bool)(buff[0] - 48);
}
