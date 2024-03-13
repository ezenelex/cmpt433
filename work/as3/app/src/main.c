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
#include <assert.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "hal/pot.h"
#include "hal/led.h"
#include "hal/sampler.h"
#include "hal/utilities.h"
#include "hal/display.h"
#include "hal/periodTimer.h"
#include "hal/audioMixer.h"

int main() {

    AudioMixer_init();
    wavedata_t tom;
    AudioMixer_readWaveFileIntoMemory("wave-files/100066__menegass__gui-drum-tom-mid-hard.wav", &tom);

    wavedata_t hihat;
    AudioMixer_readWaveFileIntoMemory("wave-files/100053__menegass__gui-drum-cc.wav", &hihat);

    for(int i = 0; i < 4; i++) {
        AudioMixer_queueSound(&hihat);
        sleepForMs(250);
        AudioMixer_queueSound(&tom);
        sleepForMs(250);
        AudioMixer_queueSound(&hihat);
        sleepForMs(500);
    }




    AudioMixer_freeWaveFileData(&tom);
    AudioMixer_freeWaveFileData(&hihat);
    AudioMixer_cleanup();

    return 0;

}
