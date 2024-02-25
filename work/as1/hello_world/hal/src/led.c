#include "hal/led.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <dirent.h>

void runCommnad(char* command){
    // Execute the shell command (output into pipe)
    FILE *pipe = popen(command, "r");
    // Ignore output of the command; but consume it
    // so we don't get an error when closing the pipe.
    char buffer[1024];
    while (!feof(pipe) && !ferror(pipe)) {
        if (fgets(buffer, sizeof(buffer), pipe) == NULL)
            break;
        // printf("--> %s", buffer); // Uncomment for debugging
    }
    // Get the exit code from the pipe; non-zero is an error:
    int exitCode = WEXITSTATUS(pclose(pipe));
    if (exitCode != 0) {
        perror("Unable to execute command:");
        printf(" command: %s\n", command);
        printf(" exit code: %d\n", exitCode);
    }
}

void sleepForMs(long long delayInMs) {
    const long long NS_PER_MS = 1000 * 1000;
    const long long NS_PER_SECOND = 1000000000;
    long long delayNs = delayInMs * NS_PER_MS;
    int seconds = delayNs / NS_PER_SECOND;
    int nanoseconds = delayNs % NS_PER_SECOND;
    struct timespec reqDelay = {seconds, nanoseconds};
    nanosleep(&reqDelay, (struct timespec *) NULL);    
}

void control_led(unsigned int led_num, bool brightness) {
    FILE *pFile;
    switch(led_num) {
        case 0:
            pFile = fopen("/sys/class/leds/beaglebone:green:usr0/brightness", "w");
            break;
        case 1:
            pFile = fopen("/sys/class/leds/beaglebone:green:usr1/brightness", "w");
            break;
        case 2:
            pFile = fopen("/sys/class/leds/beaglebone:green:usr2/brightness", "w");
            break;
        case 3:
            pFile = fopen("/sys/class/leds/beaglebone:green:usr3/brightness", "w");
            break;
        default:
            printf("Trying to modify LED %u - invalid LED\n", led_num);
            exit(1); 
    }

    if(pFile == NULL) {
        printf("Opening LED %u brightness file failed\n", led_num);
        exit(1);
    }

    int charWritten;

    if(brightness == 0) {
        charWritten = fprintf(pFile, "0");
    }
    else {
        charWritten = fprintf(pFile, "1");
    }
    
    if(charWritten <= 0) {
        printf("Writing to LED %u brightness file failed\n", led_num);
        exit(1);
    }
    fclose(pFile);
    return;
}

void setup_led() {
    FILE *pFile;
    if(1==1) {
        pFile = fopen("/sys/class/leds/beaglebone:green:usr0/trigger", "w");
        if(pFile == NULL) {
            printf("Opening LED 0 trigger file failed");
            exit(1);
        }
        int charWritten = fprintf(pFile, "none");
        if(charWritten <= 0) {
            printf("Witing to LED 0 trigger file failed");
            exit(1);
        }
        fclose(pFile);
    }
    if(1==1) {
        pFile = fopen("/sys/class/leds/beaglebone:green:usr1/trigger", "w");
        if(pFile == NULL) {
            printf("Opening LED 1 trigger file failed");
            exit(1);
        }
        int charWritten = fprintf(pFile, "none");
        if(charWritten <= 0) {
            printf("Witing to LED 1 trigger file failed");
            exit(1);
        }
        fclose(pFile);
    }
    if(1==1) {
        pFile = fopen("/sys/class/leds/beaglebone:green:usr2/trigger", "w");
        if(pFile == NULL) {
            printf("Opening LED 2 trigger file failed");
            exit(1);
        }
        int charWritten = fprintf(pFile, "none");
        if(charWritten <= 0) {
            printf("Witing to LED 2 trigger file failed");
            exit(1);
        }
        fclose(pFile);
    }
    if(1==1) {
        pFile = fopen("/sys/class/leds/beaglebone:green:usr3/trigger", "w");
        if(pFile == NULL) {
            printf("Opening LED 3 trigger file failed");
            exit(1);
        }
        int charWritten = fprintf(pFile, "none");
        if(charWritten <= 0) {
            printf("Witing to LED 3 trigger file failed");
            exit(1);
        }
        fclose(pFile);
    }

}

void setup_joystick_gpios() {
    FILE *pFile;
    if(1==1) {
        runCommnad("config-pin p8.26 gpio");
        DIR *dir = opendir("/sys/class/gpio/gpio26");
        // if gpio26 is not already exported then export it
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
        sleepForMs(300);
    }
    if(1==1) {
        runCommnad("config-pin p8.46 gpio");
        DIR *dir = opendir("/sys/class/gpio/gpio46");
        // if gpio26 is not already exported then export it
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
        sleepForMs(300);
    }
    if(1==1) {
        runCommnad("config-pin p8.47 gpio");
        DIR *dir = opendir("/sys/class/gpio/gpio47");
        // if gpio26 is not already exported then export it
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
        sleepForMs(300);
    }
    if(1==1) {
        runCommnad("config-pin p8.65 gpio");
        DIR *dir = opendir("/sys/class/gpio/gpio65");
        // if gpio26 is not already exported then export it
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
        sleepForMs(300);
    }

}

bool read_gpio(unsigned int gpio) {
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