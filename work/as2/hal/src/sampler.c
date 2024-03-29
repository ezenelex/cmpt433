

#include "hal/sampler.h"

// int PR_buffer[1000];
// int PR_history[1000];


static double PR_buffer[1000];
static double PR_history[1000];
static unsigned long long num_samples_total;
static unsigned int PR_buffer_len;
static unsigned int PR_history_len;
static double average_reading;
static bool one_second_elapsed;
static unsigned int dips_in_last_second;
pthread_mutex_t history_lock;

static bool done;

void Sampler_init(void) {
    pthread_mutex_init(&history_lock, NULL);
    done = false;
    return;
}

void Sampler_stop(void) {
    done = true;
    pthread_mutex_destroy(&history_lock);
    return;
}


void Sampler_moveCurrentDataToHistory(void) {
    pthread_mutex_lock(&history_lock);
    for(int i = 0; i < (int)PR_buffer_len; i++) {
        PR_history[i] = PR_buffer[i];
    }
    PR_history_len = PR_buffer_len;
    pthread_mutex_unlock(&history_lock);
    return;
}

// starts sampling as many PR values it can in 1 second. after the xyz function sleeps for 1 second, this function is instructed to stop.
// Computes the average measurement after each read
// Stores data into a buffer after each read which is moved to the history after this function completes.
void *Sampler_readPhotoresistor() {
    FILE* f;
    int a2dReading = 0;
    double voltage = 0;
    PR_buffer_len = 0;

    // keep reading light values for 1 second then stop
    pthread_mutex_lock(&history_lock);
    while(!one_second_elapsed) {
        // open the file and read the value, then close it
        f = fopen("/sys/bus/iio/devices/iio:device0/in_voltage1_raw", "r");
        fscanf(f, "%d", &a2dReading);
        fclose(f);
        voltage = ((double)a2dReading / 4095) * 1.8;
        // put the measurement in the buffer
        PR_buffer[PR_buffer_len] = voltage;
        average_reading = 0.001 * voltage + 0.999 * average_reading;
        sleepForMs(1);
        PR_buffer_len++;
        num_samples_total++;

        Period_markEvent(PERIOD_EVENT_SAMPLE_LIGHT);
        //printf("marked event\n");
    }
    pthread_mutex_unlock(&history_lock);
    return NULL;
}

// every second this thread will start other threads
//      Sampler_moveCurrentDataToHistory
//      Sampler_readPhotoresistor
//      Sampler_moveCurrentDataToHistory (not in a thread)
void* Sampler_clock() {
    num_samples_total = 0;
    PR_buffer_len = 0;
    PR_history_len = 0;
    // start the average value by measuring the light once
    FILE* f = fopen("/sys/bus/iio/devices/iio:device0/in_voltage1_raw", "r");
    fscanf(f, "%lf", &average_reading);
    average_reading = (average_reading / 4095) * 1.8;
    fclose(f);

    pthread_t sampler_thread;
    pthread_t count_light_dips_thread;
    while(!done) {
        // set one_second_elapsed to false
        one_second_elapsed = false;

        // not a thread since the sampler_thread needs to have a clear buffer before it can start
        Sampler_moveCurrentDataToHistory();

        // start measurement thread
        pthread_create(&sampler_thread, NULL, Sampler_readPhotoresistor, NULL);
        // start computing number of dips thread
        pthread_create(&count_light_dips_thread, NULL, Sampler_countLightDips, NULL);

        // sleep for 1 second
        sleepForMs(1000);
        // set one_second_elapsed to true to stop the threads that xyz starts
        one_second_elapsed = true;

        // join all threads
        pthread_join(sampler_thread, NULL);
        pthread_join(count_light_dips_thread, NULL);

    }
    return NULL;
}

int Sampler_getHistorySize(void) {
    return PR_history_len;
}

// Get a copy of the samples in the sample history.
// Returns a newly allocated array and sets `size` to be the
// number of elements in the returned array (output-only parameter).
// The calling code must call free() on the returned pointer.
// Note: It provides both data and size to ensure consistency.
double* Sampler_getHistory(int *size) {
    double* sampler_history = (double*)malloc(sizeof(double) * *size);
    pthread_mutex_lock(&history_lock);
    memcpy(sampler_history, PR_history, sizeof(double) * *size);
    pthread_mutex_unlock(&history_lock);
    return sampler_history;
}

// Get the average light level (not tied to the history).
double Sampler_getAverageReading(void){
    return average_reading;
}

// Get the total number of light level samples taken so far.
long long Sampler_getNumSamplesTaken(void) {
    return num_samples_total;
}

// Algorithm to count the light dips
void *Sampler_countLightDips() {
    double avg = average_reading;
    bool in_dip = false;
    dips_in_last_second = 0;
    for (int i = 0; i < (int) PR_history_len; i++) {
        // check if we are in a dip
        if (PR_history[i] <= avg - 0.1) {
            // check if we were already in a dip. if not, increase the dip counter
            if(!in_dip) {
                dips_in_last_second++;
            }
            in_dip = true;
            continue;
        }
        // check if we are still in a dip
        if (in_dip && PR_history[i] <= avg - 0.07) {
            continue;
        }
        // check if we have left a dip
        if (in_dip && PR_history[i] > avg - 0.07) {
            in_dip = false;
            continue;
        }
    }
    return NULL;
}

unsigned int Sampler_getNumDips() {
    return dips_in_last_second;
}