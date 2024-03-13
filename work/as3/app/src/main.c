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

#include "hal/utilities.h"
#include "hal/audioMixer.h"
#include "hal/joystick.h"

#define BEAT_1_NONE 0
#define BEAT_2_ROCK 1
#define BEAT_3_DNB  2

// thread to queue sounds 
void* beatThread();

// thread to poll joystick
void* joystickThread();

// thread to print output
//void* printThread();

int currentBPM;

bool stopping;

int currentBeat = BEAT_2_ROCK;

void sleepFor8thNote() {
    sleepForMs((long long)((30.0 / (float)currentBPM) * 1000));
}

void sleepFor16thNote() {
    sleepForMs((long long)((15.0 / (float)currentBPM) * 1000));
}

wavedata_t kick;
wavedata_t hihat;
wavedata_t snare;
wavedata_t open_hihat;
wavedata_t conga;
wavedata_t tube;
wavedata_t crash;

int main() {
    currentBPM = 120;
    stopping = false;

    AudioMixer_init();
    Joystick_setupGPIOs();

    // load drum samples
    //AudioMixer_readWaveFileIntoMemory("wave-files/100051__menegass__gui-drum-bd-hard.wav", &kick);
    //AudioMixer_readWaveFileIntoMemory("wave-files/100053__menegass__gui-drum-cc.wav", &hihat);
    //AudioMixer_readWaveFileIntoMemory("wave-files/100058__menegass__gui-drum-snare-hard.wav", &snare);
    AudioMixer_readWaveFileIntoMemory("wave-files/kick.wav", &kick);
    AudioMixer_readWaveFileIntoMemory("wave-files/hihat.wav", &hihat);
    AudioMixer_readWaveFileIntoMemory("wave-files/snare.wav", &snare);
    AudioMixer_readWaveFileIntoMemory("wave-files/conga.wav", &conga);
    AudioMixer_readWaveFileIntoMemory("wave-files/tube.wav", &tube);
    AudioMixer_readWaveFileIntoMemory("wave-files/open-hihat.wav", &open_hihat);
    AudioMixer_readWaveFileIntoMemory("wave-files/crash.wav", &crash);


    // create threads
    pthread_t beat_thread;
    pthread_create(&beat_thread, NULL, beatThread, NULL);
    pthread_t joystick_thread;
    pthread_create(&joystick_thread, NULL, joystickThread, NULL);
    //pthread_t print_thread;
    //pthread_create(&print_thread, NULL, printThread, NULL);

    sleepForMs(60000);

    // cleanup
    stopping = true;
    pthread_join(beat_thread, NULL);
    pthread_join(joystick_thread, NULL);
    //pthread_join(print_thread, NULL);
    AudioMixer_freeWaveFileData(&kick);
    AudioMixer_freeWaveFileData(&snare);
    AudioMixer_freeWaveFileData(&hihat);
    AudioMixer_freeWaveFileData(&conga);
    AudioMixer_freeWaveFileData(&tube);
    AudioMixer_freeWaveFileData(&open_hihat);
    AudioMixer_freeWaveFileData(&crash);
    AudioMixer_cleanup();

    return 0;

}

void* beatThread() {

    while(!stopping) {

        // drum beat 1 (none)
        while(currentBeat == BEAT_1_NONE && !stopping) {
            sleepFor8thNote();
        }

        // drum beat 2 (rock beat)
        while(currentBeat == BEAT_2_ROCK && !stopping) {
            AudioMixer_queueSound(&kick);
            AudioMixer_queueSound(&hihat);
            sleepFor8thNote();  
            AudioMixer_queueSound(&hihat);
            sleepFor8thNote();
            AudioMixer_queueSound(&snare);
            AudioMixer_queueSound(&hihat);
            sleepFor8thNote();
            AudioMixer_queueSound(&hihat);
            sleepFor8thNote();
        }
        // drum beat 3 (drum and bass)
        while(currentBeat == BEAT_3_DNB && !stopping) {  
            AudioMixer_queueSound(&kick);
            AudioMixer_queueSound(&hihat);
            AudioMixer_queueSound(&tube);
            AudioMixer_queueSound(&crash);
            sleepFor8thNote();
            AudioMixer_queueSound(&kick);
            AudioMixer_queueSound(&open_hihat);
            sleepFor8thNote();
            AudioMixer_queueSound(&snare);
            AudioMixer_queueSound(&hihat);
            sleepFor8thNote();
            AudioMixer_queueSound(&hihat);
            sleepFor16thNote();
            AudioMixer_queueSound(&tube);
            sleepFor16thNote();
            AudioMixer_queueSound(&kick);
            AudioMixer_queueSound(&hihat);
            sleepFor8thNote();
            AudioMixer_queueSound(&hihat);
            AudioMixer_queueSound(&conga);
            sleepFor8thNote();
            AudioMixer_queueSound(&snare);
            AudioMixer_queueSound(&hihat);
            sleepFor8thNote();
            AudioMixer_queueSound(&hihat);
            sleepFor8thNote();
            AudioMixer_queueSound(&hihat);
            AudioMixer_queueSound(&kick);
            sleepFor8thNote();
            AudioMixer_queueSound(&hihat);
            sleepFor8thNote();
            AudioMixer_queueSound(&hihat);
            AudioMixer_queueSound(&tube);
            AudioMixer_queueSound(&snare);
            sleepFor8thNote();
            AudioMixer_queueSound(&hihat);
            sleepFor8thNote();
            AudioMixer_queueSound(&kick);
            AudioMixer_queueSound(&hihat);
            AudioMixer_queueSound(&tube);
            sleepFor8thNote();
            AudioMixer_queueSound(&kick);
            AudioMixer_queueSound(&hihat);
            AudioMixer_queueSound(&conga);
            sleepFor8thNote();
            AudioMixer_queueSound(&open_hihat);
            AudioMixer_queueSound(&snare);
            sleepFor8thNote();
            AudioMixer_queueSound(&open_hihat);
            AudioMixer_queueSound(&hihat);
            sleepFor8thNote();
        }
    }

    return NULL;
}

void* joystickThread() {
    while(!stopping) {
        sleepForMs(100);
        // increase volume by 5
        if(!Joystick_readGPIO(JOYSTICK_UP_GPIO)) {
            AudioMixer_setVolume(AudioMixer_getVolume() + 5);
        }
        // decrease volume by 5
        if(!Joystick_readGPIO(JOYSTICK_DOWN_GPIO)) {
            AudioMixer_setVolume(AudioMixer_getVolume() - 5);
        }
        // decrease bpm by 5
        if(!Joystick_readGPIO(JOYSTICK_LEFT_GPIO)) {
            currentBPM -= 5;
            if(currentBPM <= 40) {
                currentBPM = 40;
            }
        }
        // increase bpm by 5
        if(!Joystick_readGPIO(JOYSTICK_RIGHT_GPIO)) {
            currentBPM += 5;
            if(currentBPM >= 300) {
                currentBPM = 300;
            }
        }
        // cycle beat mode
        if(!Joystick_readGPIO(JOYSTICK_PUSHED_GPIO)) {
            currentBeat = (currentBeat + 1) % 3;
        }
    }

    return NULL;
}

