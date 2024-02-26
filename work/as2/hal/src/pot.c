
#include "hal/pot.h"


static unsigned int POT;
static bool done;

// Thread that runs a loop continuously sampling the potentiometer every 100ms.
void* POT_start() {
    done = false;
    FILE* f;
    unsigned int a2dReading = 0;
    int itemsRead = 0;
    while(!done) {
        f = fopen("/sys/bus/iio/devices/iio:device0/in_voltage0_raw", "r");
        if(!f) {
            printf("ERROR: Unable to open POT voltage file. Cape loaded?\n");
            exit(-1);
        }
        // Get reading
        itemsRead = fscanf(f, "%u", &a2dReading);
        if(itemsRead <= 0) {
            printf("ERROR: Unable to read values from voltage input file.\n");
            exit(-1);
        }
        fclose(f);
        POT = a2dReading;
        sleepForMs(100);
    }
    return NULL;
}

void POT_stop() {
    done = true;
    return;
}

unsigned int POT_getReading() {
    return POT;
}

