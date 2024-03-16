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
#include <sys/sysinfo.h>

#include "hal/utilities.h"
#include "hal/audioMixer.h"
#include "hal/joystick.h"
#include "hal/accelerometer.h"
#include "hal/periodTimer.h"

#define BEAT_1_NONE 0
#define BEAT_2_ROCK 1
#define BEAT_3_DNB  2

char* beatModeNames[] = {"None", "Rock", "Custom"};

// thread to queue sounds 
void* beatThread();

// thread to poll joystick
void* joystickThread();

// thread to print output
void* printThread();

// thread to poll accelerometer
void* accelerometerThread();

// thread to listen for UDP messages
void* UDPThread();

// changed by the UDP thread
int currentBPM;

// set to true by the stopAll() function
bool stopping;

int currentBeat = BEAT_1_NONE;


// sleep for music related lengths. useful for creating beats and keeping a consistent BPM
void sleepFor8thNote() {
    sleepForMs((long long)((30.0 / (float)currentBPM) * 1000));
}
void sleepFor16thNote() {
    sleepForMs((long long)((15.0 / (float)currentBPM) * 1000));
}

// Stop all threads and shutdown gracefully. Only able to be called by the UDP thread
void stopAll() {
    stopping = true;
    return;
}

wavedata_t kick;
wavedata_t hihat;
wavedata_t snare;
wavedata_t open_hihat;
wavedata_t conga;
wavedata_t tube;
wavedata_t crash;

int socket_desc;
struct sockaddr_in server_addr;
struct sockaddr_in client_addr;
char server_message[500], client_message[30];
socklen_t client_struct_length = sizeof(client_addr);

void sendUDPMessage(char* message) {
    printf(message);
    if (sendto(socket_desc, message, strlen(message), 0, (struct sockaddr*)&client_addr, client_struct_length) < 0){
        printf("Can't send\n");
        
    }    
    return;
}

int main() {
    currentBPM = 120;
    stopping = false;

    // Setup some HAL stuff
    Period_init();
    AudioMixer_init();
    Joystick_setupGPIOs();
    Accelerometer_init();

    // Load drum samples into memory
    AudioMixer_readWaveFileIntoMemory("wave-files/kick.wav", &kick);
    AudioMixer_readWaveFileIntoMemory("wave-files/hihat.wav", &hihat);
    AudioMixer_readWaveFileIntoMemory("wave-files/snare.wav", &snare);
    AudioMixer_readWaveFileIntoMemory("wave-files/conga.wav", &conga);
    AudioMixer_readWaveFileIntoMemory("wave-files/tube.wav", &tube);
    AudioMixer_readWaveFileIntoMemory("wave-files/open-hihat.wav", &open_hihat);
    AudioMixer_readWaveFileIntoMemory("wave-files/crash.wav", &crash);


    // Create threads
    pthread_t beat_thread;
    pthread_create(&beat_thread, NULL, beatThread, NULL);
    pthread_t joystick_thread;
    pthread_create(&joystick_thread, NULL, joystickThread, NULL);
    pthread_t print_thread;
    pthread_create(&print_thread, NULL, printThread, NULL);
    pthread_t accelerometer_thread;
    pthread_create(&accelerometer_thread, NULL, accelerometerThread, NULL);
    pthread_t udp_thread;
    pthread_create(&udp_thread, NULL, UDPThread, NULL);

    // Cleanup

    // Join all threads
    pthread_join(beat_thread, NULL);
    pthread_join(joystick_thread, NULL);
    pthread_join(print_thread, NULL);
    pthread_join(accelerometer_thread, NULL);
    pthread_join(udp_thread, NULL);
    // Free the audio data from memory
    AudioMixer_freeWaveFileData(&kick);
    AudioMixer_freeWaveFileData(&snare);
    AudioMixer_freeWaveFileData(&hihat);
    AudioMixer_freeWaveFileData(&conga);
    AudioMixer_freeWaveFileData(&tube);
    AudioMixer_freeWaveFileData(&open_hihat);
    AudioMixer_freeWaveFileData(&crash);
    // Cleanup some HAL stuff
    AudioMixer_cleanup();
    Accelerometer_stop();
    Period_cleanup();

    return 0;

}

// Runs in a thread and queues the sounds required to construct each beat.
// The beat mode can changed by pressing the joystick in, or by sending the command "beat [mode]" in a UDP message
// The modes are none, rock, and custom
// Keeps running until the stopAll() function is called
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


// Runs in a thread and constantly checks the state of the joystick to see what is pressed
// Performs the appropriate action when detecting a press
// Keeps running until the stopAll() function is called
void* joystickThread() {
    char message[30];
    while(!stopping) {
        sleepForMs(100);
        // increase volume by 5
        if(!Joystick_readGPIO(JOYSTICK_UP_GPIO)) {
            AudioMixer_setVolume(AudioMixer_getVolume() + 5);
            sprintf(message, "Volume: %d\n", AudioMixer_getVolume());
            sendUDPMessage(message);
        }
        // decrease volume by 5
        if(!Joystick_readGPIO(JOYSTICK_DOWN_GPIO)) {
            AudioMixer_setVolume(AudioMixer_getVolume() - 5);
            sprintf(message, "Volume: %d\n", AudioMixer_getVolume());
            sendUDPMessage(message);
        }
        // decrease bpm by 5
        if(!Joystick_readGPIO(JOYSTICK_LEFT_GPIO)) {
            currentBPM -= 5;
            if(currentBPM <= 40) {
                currentBPM = 40;
            }
            sprintf(message, "BPM: %d\n", currentBPM);
            sendUDPMessage(message);
        }
        // increase bpm by 5
        if(!Joystick_readGPIO(JOYSTICK_RIGHT_GPIO)) {
            currentBPM += 5;
            if(currentBPM >= 300) {
                currentBPM = 300;
            }
            sprintf(message, "BPM: %d\n", currentBPM);
            sendUDPMessage(message);
        }
        // cycle beat mode
        if(!Joystick_readGPIO(JOYSTICK_PUSHED_GPIO)) {
            currentBeat = (currentBeat + 1) % 3;
            sprintf(message, "Current beat mode: %s\n", beatModeNames[currentBeat]);
            sendUDPMessage(message);
        }
        
    }

    return NULL;
}

// Runs in a thread and keeps reading the accelerometer values in XYZ
// When it detects a large acceleration in a certain direction, it plays the associated sound
// Keeps running until the stopAll() function is called
void* accelerometerThread() {
    int16_t x_prev = 0;
    int16_t y_prev = 0;
    int16_t z_prev = 0;
    int16_t x = 0;
    int16_t y = 0;
    int16_t z = 0;
    long long x_trig = 0;
    long long y_trig = 0;
    long long z_trig = 0;

    while(!stopping) {
        // reads all accelerometer values and puts them in variables in accelerometer.c
        Accelerometer_read(OUT_X_MSB);
        x = Accelerometer_getX();
        y = Accelerometer_getY();
        z = Accelerometer_getZ();

        //printf("%05d %05d %05d \n", x, y, z);

        if(abs(x) > 14000) {
            if(x_prev == 0 && getTimeInMs() - x_trig > 150) {
                x_trig = getTimeInMs();
                AudioMixer_queueSound(&snare);
                x_prev = 1;
            }
        }
        if(x_prev == 1 && abs(x) < 8000) {
            x_prev = 0;
        }

        if(abs(y) > 14000) {
            if(y_prev == 0 && getTimeInMs() - y_trig > 150) {
                y_trig = getTimeInMs();
                AudioMixer_queueSound(&kick);
                y_prev = 1;
            }
        }
        if(y_prev == 1 && abs(y) < 8000) {
            y_prev = 0;
        }

        if(abs(z-16400) > 14000) {
            if(z_prev == 0 && getTimeInMs() - z_trig > 150) {
                z_trig = getTimeInMs();
                AudioMixer_queueSound(&hihat);
                z_prev = 1;
            }
        }
        if(z_prev == 1 && abs(z-16400) < 8000) {
            z_prev = 0;
        }

        sleepForMs(50);
    }
    return NULL;
}

// Runs in a thread and constantly prints statistics to stdout
// Keeps running until the stopAll() function is called
void* printThread() {
    Period_statistics_t* period_stats_buffer = (Period_statistics_t*)malloc(sizeof(Period_statistics_t));
    Period_statistics_t* period_stats_accelo = (Period_statistics_t*)malloc(sizeof(Period_statistics_t));
    // beat mode
    // bpm
    // volume
    // audio[min, max] avg (num/num-samples)
    // accel[min, max] avg (num/num-samples)

    while(!stopping){
        Period_getStatisticsAndClear(PERIOD_EVENT_ACCELEROMETER_SAMPLED, period_stats_accelo);
        Period_getStatisticsAndClear(PERIOD_EVENT_BUFFER_FILLED, period_stats_buffer);
        printf("M%d ", currentBeat);
        printf("%*dbpm ", 3, currentBPM);
        printf("vol:%*d    ", 3, AudioMixer_getVolume());
        printf("Audio[%*.3lf, ", 5, period_stats_buffer->minPeriodInMs);
        printf("%*.3lf] ", 5, period_stats_buffer->maxPeriodInMs);
        printf("avg %*.3lf/", 5, period_stats_buffer->avgPeriodInMs);
        printf("%*d    ", 2, period_stats_buffer->numSamples);
        printf("Accel[%*.3lf, ", 5, period_stats_accelo->minPeriodInMs);
        printf("%*.3lf] ", 5, period_stats_accelo->maxPeriodInMs);
        printf("avg %*.3lf/", 5, period_stats_accelo->avgPeriodInMs);
        printf("%*d ", 2, period_stats_accelo->numSamples);
        printf("\n");
        sleepForMs(1000);
    }

    free(period_stats_accelo);
    free(period_stats_buffer);

    
    return NULL;
}



// Runs in a thread and opens a UDP socket that listens for client commands (change volume, beat mode, bpm, etc)
// Also used for talking to the Node.js webserver so the user can perform commands from the browser instead of command line 
// The user can connect to the socket from the command line by the command
//      netcat -u 192.168.7.2 12345
// Keeps running until the stopAll() function is called
void* UDPThread() {
    // used for getting the BBG's uptime
    struct sysinfo info;
    int hours;
    int minutes;
    int seconds;

    bool repeat_last_command = false;
    char client_message_prev[30];
    

    memset(server_message, '\0', sizeof(server_message));
    memset(client_message, '\0', sizeof(client_message));

    socket_desc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if(socket_desc < 0) {
        printf("Error while creating socket\n");
        return NULL;
    }
    printf("Socket created successfully\n");

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(12345);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        printf("Couldn't bind to the port\n");
        return NULL;
    }    

    while(!stopping) {
        
        if(repeat_last_command == false) {
            strcpy(client_message_prev, client_message);
            if (recvfrom(socket_desc, client_message, sizeof(client_message), 0, (struct sockaddr*)&client_addr, &client_struct_length) < 0){
                printf("Couldn't receive\n");
                return NULL;
            }
        } else {
            strcpy(client_message, client_message_prev);
            //printf("%s\n", client_message);
        }
        repeat_last_command = false;
        
        //printf("Received message from IP: %s and port: %i\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        //printf("Msg from client: %s\n", client_message);

        if(strncmp(client_message, "help", 4)  == 0 || strncmp(client_message, "?", 1) == 0) {
            strcpy(server_message, "Accepted commands:\nmode [beat]   -- switch to a beat mode.\n    Available modes:\n        none\n        rock\n        custom\nvolume up     -- increase volume by 5 percent.\nvolume down   -- decrease volume by 5 percent.\nbpm up        -- increase the BPM by 5.\nbpm down      -- decrease the BPM by 5.\nplay [sound]  --play a sound\n    Available sounds:\n        kick\n        snare\n        hihat\n        conga\n        tube\n        open hihat\n        crash\n");
        
        } else if (strncmp(client_message, "mode ", 5) == 0) {
            if(strncmp(client_message + 5, "none", 4) == 0) {
                currentBeat = BEAT_1_NONE;
            } else if(strncmp(client_message + 5, "rock", 4) == 0) {
                currentBeat = BEAT_2_ROCK;
            } else if(strncmp(client_message + 5, "custom", 6) == 0) {
                currentBeat = BEAT_3_DNB;
            }   
            sprintf(server_message, "Current beat mode: %s\n", beatModeNames[currentBeat]);
        
        } else if (strncmp(client_message, "volume up", 9) == 0) {
            AudioMixer_setVolume(AudioMixer_getVolume() + 5);
            sprintf(server_message, "Volume: %d\n", AudioMixer_getVolume());
        
        } else if (strncmp(client_message, "volume down", 11) == 0) {
            AudioMixer_setVolume(AudioMixer_getVolume() - 5);
            sprintf(server_message, "Volume: %d\n", AudioMixer_getVolume());
        
        } else if (strncmp(client_message, "bpm up", 6) == 0) {
            currentBPM += 5;
            if(currentBPM >= 300) {
                currentBPM = 300;
            }
            sprintf(server_message, "BPM: %d\n", currentBPM);
        
        } else if (strncmp(client_message, "bpm down", 8) == 0) {
            currentBPM -= 5;
            if(currentBPM <= 40) {
                currentBPM = 40;
            }
            sprintf(server_message, "BPM: %d\n", currentBPM);
        
        } else if (strncmp(client_message, "play ", 5) == 0) {
            if(strncmp(client_message + 5, "kick", 4) == 0) {
                AudioMixer_queueSound(&kick);
                strcpy(server_message, "Playing kick\n");
            } else if(strncmp(client_message + 5, "hihat", 5) == 0) {
                AudioMixer_queueSound(&hihat);
                strcpy(server_message, "Playing hihat\n");
            } else if(strncmp(client_message + 5, "snare", 5) == 0) {
                AudioMixer_queueSound(&snare);
                strcpy(server_message, "Playing snare\n");
            } else if(strncmp(client_message + 5, "conga", 5) == 0) {
                AudioMixer_queueSound(&conga);
                strcpy(server_message, "Playing conga\n");
            } else if(strncmp(client_message + 5, "tube", 4) == 0) {
                AudioMixer_queueSound(&tube);
                strcpy(server_message, "Playing tube\n");
            } else if(strncmp(client_message + 5, "open hihat", 10) == 0) {
                AudioMixer_queueSound(&open_hihat);
                strcpy(server_message, "Playing open hihat\n");
            } else if(strncmp(client_message + 5, "crash", 5) == 0) {
                AudioMixer_queueSound(&crash);
                strcpy(server_message, "Playing crash\n");
            } else {
                strcpy(server_message, "Unknown sound\n");
            }

        } else if (strncmp(client_message, "stop", 4) == 0) {
            strcpy(server_message, "Stopping Beat Box\n");
            break;

        } else if (strncmp(client_message, "uptime", 6) == 0) {
            sysinfo(&info);
            hours = info.uptime / 3600;
            minutes = (info.uptime - (3600*hours))/60;
            seconds = (info.uptime - (3600*hours) - (minutes*60));
            sprintf(server_message, "Uptime: %02d:%02d:%02d\n", hours, minutes, seconds);

        } else if (strncmp(client_message, "\n", 1) == 0) {
            repeat_last_command = true;
            continue;

        } else {
            strcpy(server_message, "Unknown command\n");
        }

        if(strncmp(client_message, "stop", 4) != 0) {
            sendUDPMessage(server_message);
        }

        
    }
    stopAll();
    close(socket_desc);
    return NULL;
}



