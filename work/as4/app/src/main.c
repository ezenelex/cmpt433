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
#include <sys/sysinfo.h>
#include <sys/mman.h>

#include "hal/utilities.h"
#include "hal/joystick.h"
#include "hal/accelerometer.h"
//#include "hal/periodTimer.h"
#include "hal/display.h"
#include "led-shared-struct.h"
#include "hal/buzzer.h"

// General PRU Memomry Sharing Routine
// ----------------------------------------------------------------
#define PRU_ADDR      0x4A300000   // Start of PRU memory Page 184 am335x TRM
#define PRU_LEN       0x80000      // Length of PRU memory
#define PRU0_DRAM     0x00000      // Offset to DRAM
#define PRU1_DRAM     0x02000
#define PRU_SHAREDMEM 0x10000      // Offset to shared memory
#define PRU_MEM_RESERVED 0x200     // Amount used by stack and heap

// Convert base address to each memory section
#define PRU0_MEM_FROM_BASE(base) ( (base) + PRU0_DRAM + PRU_MEM_RESERVED)
#define PRU1_MEM_FROM_BASE(base) ( (base) + PRU1_DRAM + PRU_MEM_RESERVED)
#define PRUSHARED_MEM_FROM_BASE(base) ( (base) + PRU_SHAREDMEM)

volatile void *pPruBase;
volatile sharedMemStruct_t *pSharedPru0;

// thread to poll joystick
void* joystickThread();
// thread to print output
void* printThread();
// thread to poll accelerometer
void* accelerometerThread();
// buzzer
void* buzzerThread();


// set to true by the stopAll() function
bool stopping;
int score;

bool playingHit;
bool playingMiss;

// Return the address of the PRU's base memory
volatile void* getPruMmapAddr(void)
{
    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd == -1) {
        perror("ERROR: could not open /dev/mem");
        exit(EXIT_FAILURE);
    }

    // Points to start of PRU memory.
    volatile void* pPruBase = mmap(0, PRU_LEN, PROT_READ | PROT_WRITE, MAP_SHARED, fd, PRU_ADDR);
    if (pPruBase == MAP_FAILED) {
        perror("ERROR: could not map memory");
        exit(EXIT_FAILURE);
    }
    close(fd);

    return pPruBase;
}

void freePruMmapAddr(volatile void* pPruBase)
{
    if (munmap((void*) pPruBase, PRU_LEN)) {
        perror("PRU munmap failed");
        exit(EXIT_FAILURE);
    }
}

// Stop all threads and shutdown gracefully. Only able to be called by the UDP thread
void stopAll() {
    stopping = true;
    Display_stop();
    return;
}


int main() {
    pPruBase = getPruMmapAddr();
    pSharedPru0 = PRU0_MEM_FROM_BASE(pPruBase);
    stopping = false;
    score = 0;
    playingHit = false;
    playingMiss = false;

    // Setup some HAL stuff
    runCommand("config-pin p8.11 pruout");
    buzzer_init();
    Display_setupGPIOs();
    int i2cFileDesc = Display_initI2cBus(I2CDRV_LINUX_BUS1, I2C_DEVICE_ADDRESS);
    Joystick_setupGPIOs();
    Accelerometer_init();
    // Get access to PRU shared memory

    // Create threads
    pthread_t joystick_thread; 
    pthread_create(&joystick_thread, NULL, joystickThread, NULL);
    pthread_t print_thread;
    pthread_create(&print_thread, NULL, printThread, NULL);
    pthread_t accelerometer_thread;
    pthread_create(&accelerometer_thread, NULL, accelerometerThread, NULL);
    pthread_t display_thread;
    pthread_create( &display_thread, NULL, Display_start, (void*)&i2cFileDesc);
    pthread_t buzzer_thread;
    pthread_create( &buzzer_thread, NULL, buzzerThread, NULL);
    


    // Cleanup

    // Join all threads
    pthread_join(joystick_thread, NULL);
    pthread_join(print_thread, NULL);
    pthread_join(accelerometer_thread, NULL);
    pthread_join(display_thread, NULL);
    pthread_join(buzzer_thread, NULL);

    // Cleanup some HAL stuff
    Accelerometer_stop();
    freePruMmapAddr(pPruBase);
    return 0;
}

void* buzzerThread() {
    while(!stopping) {
        if(playingHit) {
            buzzer_play_hit();
            playingHit = false;
        }
        if(playingMiss) {
            buzzer_play_miss();
            playingMiss = false;
        }
    }
    return NULL;
}

void* joystickThread() {
    printf("Joystick thread started\n");
    long long timeSinceLastFire = 0;
    pSharedPru0->target_x = rand() % 100 - 50;
    pSharedPru0->target_y = rand() % 100 - 50;
    while(!stopping) {
    
        if(pSharedPru0->js_down_pressed) {
            if(getTimeInMs() - timeSinceLastFire < 250) {
                continue;
            }
            timeSinceLastFire = getTimeInMs();
            // hit
            if(pSharedPru0->on_target) {
                score++;
                Display_increment();
                playingHit = true;
                pSharedPru0->target_x = rand() % 100 - 50;
                pSharedPru0->target_y = rand() % 100 - 50;
            // miss
            } else {
                playingMiss = true;
            }
        }
        if(pSharedPru0->js_right_pressed) {
            printf("stopping program\n");
            stopAll();
            pSharedPru0->done = true;
            return NULL;
        }
    }
    return NULL;
}

void* accelerometerThread() {
    printf("Accelerometer thread started\n");
    while(!stopping) {
        Accelerometer_read(OUT_X_MSB);
        pSharedPru0->accel_x = Accelerometer_getX() / 256;
        pSharedPru0->accel_y = Accelerometer_getY() / 256;
        sleepForMs(50);
    }
    return NULL;
}


void* printThread() {
    printf("Print thread started\n");
    while(!stopping) {

    }
    return NULL;
}
