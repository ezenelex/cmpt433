#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>

#include "hal/led.h"

static long long getTimeInMs(void)
{
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    long long seconds = spec.tv_sec;
    long long nanoSeconds = spec.tv_nsec;
    long long milliSeconds = seconds * 1000 + nanoSeconds / 1000000;
    return milliSeconds;
}

void flash_leds(unsigned int between_flashes_ms, unsigned int duration_flashes_ms, unsigned int num_flashes) {
    for(int i = 0; i < (int)num_flashes; i++) {
        control_led(0,1);
        control_led(1,1);
        control_led(2,1);
        control_led(3,1);
        sleepForMs(duration_flashes_ms);
        control_led(0,0);
        control_led(1,0);
        control_led(2,0);
        control_led(3,0);
        sleepForMs(between_flashes_ms);
    }
    return;
}

int main()
{
    printf("Hello embedded world, from Osman!\n");
    setup_led();
    bool joystick_up_value;
    bool joystick_down_value;
    bool joystick_left_value;
    bool joystick_right_value;

    long long round_duration_ms;
    long long reaction_time_ms;

    bool round_up_or_down;

    long long start_time_ms;
    long long end_time_ms;
    long long best_reaction_time_ms = 5000;

    printf("When the LEDs light up, press the joystick in that direction!\n");
    printf("(Press left or right to exit)\n");

    control_led(0,0);
    control_led(1,0);
    control_led(2,0);
    control_led(3,0);

    while(1) {
        printf("Get ready...\n");
        control_led(0,0);
        control_led(1,1);
        control_led(2,1);
        control_led(3,0);

        round_duration_ms = (rand() % (2501)) + 500;
        round_up_or_down = rand() % 2;

        sleepForMs(round_duration_ms);

        if(round_up_or_down == 1) {
            printf("Press UP now!\n");
            control_led(0,1);
            control_led(1,0);
            control_led(2,0);
            control_led(3,0);
        } else {
            printf("Press DOWN now!\n");
            control_led(0,0);
            control_led(1,0);
            control_led(2,0);
            control_led(3,1);
        }

        start_time_ms = getTimeInMs();

        while(1) {

            joystick_up_value = read_gpio(JOYSTICK_UP_GPIO);
            joystick_down_value = read_gpio(JOYSTICK_DOWN_GPIO);
            joystick_left_value = read_gpio(JOYSTICK_LEFT_GPIO);
            joystick_right_value = read_gpio(JOYSTICK_RIGHT_GPIO);

            if(getTimeInMs() - start_time_ms > 5000) {
                printf("No input within 5000ms; quitting!\n");
                return 0;
            }

            // up joystick pressed
            if(joystick_up_value == 0) {
                if(round_up_or_down == 1) {
                    end_time_ms = getTimeInMs();
                    reaction_time_ms = end_time_ms - start_time_ms;
                    if(reaction_time_ms < best_reaction_time_ms) {
                        best_reaction_time_ms = reaction_time_ms;
                    }
                    printf("Correct!\n");
                    printf("Your reaction time was %llums; best so for in game is %llums\n", reaction_time_ms, best_reaction_time_ms);
                    flash_leds(500, 500, 2);
                    break;
                } else { // wrong joystick pressed
                    printf("Incorrect.\n");
                    flash_leds(200, 200, 5);
                    break;
                }
            // down joystick pressed
            } else if(joystick_down_value == 0){
                if(round_up_or_down == 0) { // correct joystick pressed
                    end_time_ms = getTimeInMs();
                    reaction_time_ms = end_time_ms - start_time_ms;
                    if(reaction_time_ms < best_reaction_time_ms) {
                        best_reaction_time_ms = reaction_time_ms;
                    }
                    printf("Correct!\n");
                    printf("Your reaction time was %llums; best so for in game is %llums\n", reaction_time_ms, best_reaction_time_ms);
                    flash_leds(500, 500, 2);
                    break;
                } else { // wrong joystick pressed
                    printf("Incorrect.\n");
                    flash_leds(200, 200, 5);
                    break;
                }
            // left or right joystick pressed
            } else if (!joystick_left_value || !joystick_right_value) {
                printf("Ending game\n");
                printf("Your highscore was %llums!\n", best_reaction_time_ms);
                return 0;
            }
            
        }

    }
}