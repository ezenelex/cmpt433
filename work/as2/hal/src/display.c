



#include "hal/display.h"




static bool done;
static unsigned int number;
static const int digit_a[] = {0xa1, 0x04, 0x31, 0xb0, 0x90, 0xb0, 0xb1, 0x80, 0xb1, 0xb0};
static const int digit_b[] = {0x86, 0x20, 0x0e, 0x0e, 0x8a, 0x8c, 0x8c, 0x06, 0x8e, 0x8e};


void Display_writeI2cReg(int i2cFileDesc, unsigned char regAddr, unsigned char value) {
    unsigned char buff[2];
    buff[0] = regAddr;
    buff[1] = value;
    int res = write(i2cFileDesc, buff, 2);
    if (res != 2) {
        perror("I2C: Unable to write i2c register.");
        exit(1);
    }
}

// runs in a thread
void *Display_start(void *i2cFileDesc) {
    int first_digit;
    int second_digit;

    // files to enable/disable digits
    FILE* f1;
    FILE* f2;

    while(!done) {
        first_digit = floor(number / 10);
        second_digit = number % 10;
        printf("First digit: %d Second digit: %d\n", first_digit, second_digit);
        // turn off both digits
        f1 = fopen("/sys/class/gpio/gpio61/value", "w");
        f2 = fopen("/sys/class/gpio/gpio44/value", "w"); 
        fprintf(f1, "%d", 0);
        fprintf(f2, "%d", 0);
        fclose(f1);
        fclose(f2);
        // Drive I2C GPIO extender to display pattern for left digit
        Display_writeI2cReg(*(int*)i2cFileDesc, REG_OUTA, digit_a[first_digit]);
        Display_writeI2cReg(*(int*)i2cFileDesc, REG_OUTB, digit_b[first_digit]);
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
        Display_writeI2cReg(*(int*)i2cFileDesc, REG_OUTA, digit_a[second_digit]);
        Display_writeI2cReg(*(int*)i2cFileDesc, REG_OUTB, digit_b[second_digit]);
        // Turn on right digit via GPIO 44 set to a 1. Wait for a little time (5ms)
        f2 = fopen("/sys/class/gpio/gpio44/value", "w"); 
        fprintf(f2, "%d", 1);
        fclose(f2);
        sleepForMs(5);
    }
    f1 = fopen("/sys/class/gpio/gpio61/value", "w");
    f2 = fopen("/sys/class/gpio/gpio44/value", "w"); 
    fprintf(f1, "%d", 0);
    fprintf(f2, "%d", 0);
    fclose(f1);
    fclose(f2);

    return NULL;
}

void Display_setupGPIOs() {
    FILE *f;

    // check if GPIO 61 is exported
    DIR *dir = opendir("/sys/class/gpio/gpio61");
    // if GPIO 61 isnt already exported, then export it
    if(!dir) {
        f = fopen("/sys/class/gpio/export", "w");
        if (f == NULL) {
            printf("Couldn't open gpio export file\n");
            exit(1);
        }
        fprintf(f, "%d", 61);
        fclose(f);
        sleepForMs(300);
    }
    // change GPIO 61 to an output
    f = fopen("/sys/class/gpio/gpio61/direction", "w");
    if (f == NULL) {
        printf("Couldn't open gpio61 direction file\n");
        exit(1);
    }
    fprintf(f, "%s", "out");
    fclose(f);
    sleepForMs(300);

    // check if GPIO 44 is exported
    dir = opendir("/sys/class/gpio/gpio44");
    // if GPIO 44 isnt already exported, then export it
    if(!dir) {
        f = fopen("/sys/class/gpio/export", "w");
        if (f == NULL) {
            printf("Couldn't open gpio export file\n");
            exit(1);
        }
        fprintf(f, "%d", 44);
        fclose(f);
        sleepForMs(300);
    }
    // change GPIO 44 to an output
    f = fopen("/sys/class/gpio/gpio44/direction", "w");
    if (f == NULL) {
        printf("Couldn't open gpio44 direction file\n");
        exit(1);
    }
    fprintf(f, "%s", "out");
    fclose(f);
    sleepForMs(300);

    return;
}

int Display_initI2cBus(char* bus, int address) {
    runCommand("config-pin P9_18 i2c");
    runCommand("config-pin P9_17 i2c");
    int i2cFileDesc = open(bus, O_RDWR);
    int result = ioctl(i2cFileDesc, I2C_SLAVE, address);
    if (result < 0) {
        perror("I2C: Unable to set I2C device to slave address.");
        exit(1);
    }
    runCommand("i2cset -y 1 0x20 0x00 0x00");
    runCommand("i2cset -y 1 0x20 0x01 0x00");
    return i2cFileDesc;
}

void Display_setNumber(unsigned int num) {
    number = num;
    return;
}

void Display_stop() {
    done = true;
    return;
}
