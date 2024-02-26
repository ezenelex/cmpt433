
#include "hal/led.h"

static bool done;
static unsigned int freq;
static unsigned int period;
static unsigned int pot;

// PWM control for LED
void *LED_start() {
    FILE* f;
    while(!done) {
        pot = POT_getReading();
        if(freq != pot/40) {
            freq = pot/40;
            period = (int)((double)(1.0/(double)freq) * 1000000000);
            if(period == 0) period++;
            f = fopen("/dev/bone/pwm/0/b/period", "w");
            fprintf(f, "%d", period);
            fclose(f);
            f = fopen("/dev/bone/pwm/0/b/duty_cycle", "w");
            fprintf(f, "%d", period/2);
            fclose(f);
        }
    }
    runCommand("echo 0 > /dev/bone/pwm/0/b/enable");
    return NULL;
}

void LED_setup() {
    done = false;
    runCommand("config-pin P9_21 pwm");
    runCommand("echo 100000000 > /dev/bone/pwm/0/b/period");
    runCommand("echo 1 > /dev/bone/pwm/0/b/enable");
    runCommand("echo 0 > /dev/bone/pwm/0/b/duty_cycle");
    return;
}

void LED_stop() {
    done = true;
    return;
}

unsigned int LED_getFreq() {
    return freq;
}

unsigned int LED_getPeriod() {
    return period;
}