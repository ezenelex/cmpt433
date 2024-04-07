#include "hal/buzzer.h"




void buzzer_init() {
    runCommand("config-pin p9_22 pwm");
    runCommand("echo 0 > /dev/bone/pwm/0/a/enable");
    runCommand("echo 0 > /dev/bone/pwm/0/a/duty_cycle");
    runCommand("echo 1000000 > /dev/bone/pwm/0/a/period");
    return;
}

void buzzer_play_hit() {
    int period = (int)(1.0 / 130.0 * 1000000000);
    FILE* f;
    if(1==1) {
        f = fopen("/dev/bone/pwm/0/a/duty_cycle", "w");
        fprintf(f, "%d", 0);
        fclose(f);

        f = fopen("/dev/bone/pwm/0/a/period", "w");
        fprintf(f, "%d", period);
        fclose(f);

        f = fopen("/dev/bone/pwm/0/a/duty_cycle", "w");
        fprintf(f, "%d", period/2);
        fclose(f);

        f = fopen("/dev/bone/pwm/0/a/enable", "w");
        fprintf(f, "%d", 1);
        fclose(f);

        sleepForMs(100);

        f = fopen("/dev/bone/pwm/0/a/duty_cycle", "w");
        fprintf(f, "%d", 0);
        fclose(f);

        f = fopen("/dev/bone/pwm/0/a/period", "w");
        fprintf(f, "%d", period/2);
        fclose(f);

        f = fopen("/dev/bone/pwm/0/a/duty_cycle", "w");
        fprintf(f, "%d", period/4);
        fclose(f);

        sleepForMs(100);

        f = fopen("/dev/bone/pwm/0/a/duty_cycle", "w");
        fprintf(f, "%d", 0);
        fclose(f);

        f = fopen("/dev/bone/pwm/0/a/period", "w");
        fprintf(f, "%d", period/4);
        fclose(f);

        f = fopen("/dev/bone/pwm/0/a/duty_cycle", "w");
        fprintf(f, "%d", period/8);
        fclose(f);

        sleepForMs(100);

        f = fopen("/dev/bone/pwm/0/a/enable", "w");
        fprintf(f, "%d", 0);
        fclose(f);
    }

    return;
}

void buzzer_play_miss() {
    int period = (int)(1.0 / 130.0 * 1000000000);
    FILE* f;

    if(1==1) {
        f = fopen("/dev/bone/pwm/0/a/duty_cycle", "w");
        fprintf(f, "%d", 0);
        fclose(f);

        f = fopen("/dev/bone/pwm/0/a/period", "w");
        fprintf(f, "%d", period);
        fclose(f);

        f = fopen("/dev/bone/pwm/0/a/duty_cycle", "w");
        fprintf(f, "%d", period/2);
        fclose(f);

        f = fopen("/dev/bone/pwm/0/a/enable", "w");
        fprintf(f, "%d", 1);
        fclose(f);

        sleepForMs(100);

        f = fopen("/dev/bone/pwm/0/a/enable", "w");
        fprintf(f, "%d", 0);
        fclose(f);

        sleepForMs(100);

        f = fopen("/dev/bone/pwm/0/a/enable", "w");
        fprintf(f, "%d", 1);
        fclose(f);

        sleepForMs(100);

        f = fopen("/dev/bone/pwm/0/a/enable", "w");
        fprintf(f, "%d", 0);
        fclose(f);

        sleepForMs(100);

        f = fopen("/dev/bone/pwm/0/a/enable", "w");
        fprintf(f, "%d", 1);
        fclose(f);

        sleepForMs(100);

        f = fopen("/dev/bone/pwm/0/a/enable", "w");
        fprintf(f, "%d", 0);
        fclose(f);

    }

    return;
}