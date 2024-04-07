#ifndef _BUZZER_H_
#define _BUZZER_H_

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "hal/utilities.h"

void buzzer_init();

void buzzer_play_hit();

void buzzer_play_miss();

#endif