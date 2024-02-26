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

// This function runs a loop that repeats every 1 second that prints
// all the collected data for the user to see.
// This function runs in a thread
void* printOutput();

// This function opens a socket that listens for a client's UDP messages
// containing commands used to obtain data from the server, which is here.
// It runs in a thread.
void* UDP();

// boolean to signal to UDP() and printOutput() when to finish. Only ever
// set to true by a UDP message containing 'stop' which calls the stopAll() func
bool done;

// Stops all active threads
void stopAll() {
    done = true;
    Sampler_stop();
    Display_stop();
    POT_stop();
    LED_stop();
    return;
}

int main()
{
    // Setup the hardware and period timer
    Display_setupGPIOs();
    int i2cFileDesc = Display_initI2cBus(I2CDRV_LINUX_BUS1, I2C_DEVICE_ADDRESS);
    LED_setup();
    done = false;
    Period_init();
    

    // Start threads

    // start the sampler thread (read sampler.h)
    pthread_t sampler_thread;
    pthread_create( &sampler_thread, NULL, Sampler_clock, NULL);
    // start UDP thread
    pthread_t udp_thread;
    pthread_create( &udp_thread, NULL, UDP, NULL);
    // start LED thread
    pthread_t led_thread;
    pthread_create( &led_thread, NULL, LED_start, NULL);
    // start POT thread
    pthread_t pot_thread;
    pthread_create( &pot_thread, NULL, POT_start, NULL);
    // start seg display thread
    pthread_t display_thread;
    pthread_create( &display_thread, NULL, Display_start, (void*)&i2cFileDesc);
    // start print output thread
    pthread_t print_output_thread;
    pthread_create( &print_output_thread, NULL, printOutput, NULL);
    

    // At this point all threads are in flight
    // Only can continue from this point when 'stop' is received by the UDP thread
    // Which signals all threads to return

    // join all threads
    pthread_join( print_output_thread, NULL);
    pthread_join(led_thread, NULL);
    pthread_join(sampler_thread, NULL);
    pthread_join(display_thread, NULL);
    pthread_join(pot_thread, NULL);
    pthread_join(udp_thread, NULL);
    Period_cleanup();


    return 0;

}

// Runs in a thread
void* printOutput() {
    // used for spacing the printed output evenly
    int width;

    Period_statistics_t *period_stats = (Period_statistics_t*)malloc(sizeof(Period_statistics_t));
    
    // All the data that will be printed
    unsigned int num_samples = 0;                   // num samples recorded in last second
    unsigned int pot = 0;                           // raw POT value
    unsigned int freq = 0;                          // frequency of LED
    double average_light_level = 0;                 // voltage w/ 3 decimal places
    unsigned int dips = 0;                          // num dips in last second
    double min_time_between_samples = 0;            // over the last second
    double max_time_between_samples = 0;            // ''
    double average_time_between_samples = 0;        // ''
    double* history = NULL;                         // array of all samples recorded over the last second. Must be freed!

    long long timestamp = getTimeInMs();            // used for knowing when to print the output

    sleepForMs(2000);

    // Keep printing the output until done
    while(!done) {
        if(getTimeInMs() - timestamp < 1000) {
            continue;
        }
        timestamp = getTimeInMs();   
        Period_getStatisticsAndClear(PERIOD_EVENT_SAMPLE_LIGHT, period_stats);
        num_samples = Sampler_getHistorySize();                   
        pot = POT_getReading();           
        freq = pot/40;                                                         
        average_light_level = Sampler_getAverageReading();                         
        dips = Sampler_getNumDips();
        min_time_between_samples = period_stats->minPeriodInMs;
        max_time_between_samples = period_stats->maxPeriodInMs;
        average_time_between_samples = period_stats->avgPeriodInMs;
        history = Sampler_getHistory((int*)&num_samples);     
        
        // Line 1
        width = 3;
        printf("#Smpl/s = %*u    ", width, num_samples);
        printf("POT @ %*u", width+2, pot);
        printf(" => %*uHz    ", width, freq);
        printf("avg = %.3lfV    ", average_light_level);
        printf("dips = %*u    ", width, dips);
        printf("Smpl ms[ %.3lf, %.3lf] ", min_time_between_samples, max_time_between_samples);
        printf("avg %.3lf/", average_time_between_samples);
        printf("%*u\n", width, num_samples);

        // Line 2
        int spacing = (int)num_samples/10;
        width = 4;
        for(int i = 0; i < 10; i++) {
            assert(i*spacing < (int)num_samples);
            printf("%*d",width, i*spacing);
            printf(":%.3lf    ", history[i*spacing]);
        }
        printf("\n");
        Display_setNumber(dips);

        free(history);                                   
    }
    free(period_stats);


    return NULL;
}

// Runs in a thread
void *UDP() {

    bool repeat_last_command = false;
    char client_message_prev[20];
    double* history = NULL;
    int history_size = 0;
    int bytes_written = 0;
    int values_since_last_newline = 0;

    int socket_desc;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    char server_message[5000], client_message[20];
    socklen_t client_struct_length = sizeof(client_addr);

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

    while(!done) {
        if(repeat_last_command == false) {
            strcpy(client_message_prev, client_message);
            if (recvfrom(socket_desc, client_message, sizeof(client_message), 0, (struct sockaddr*)&client_addr, &client_struct_length) < 0){
                printf("Couldn't receive\n");
                return NULL;
            }
        } else {
            strcpy(client_message, client_message_prev);
            printf("%s\n", client_message);
        }
        repeat_last_command = false;

        //printf("Received message from IP: %s and port: %i\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        //printf("Msg from client: %s\n", client_message);

        if(strncmp(client_message, "help", 4)  == 0 || strncmp(client_message, "?", 1) == 0) {
            strcpy(server_message, "Accepted command examples:\ncount    -- get the total number of samples taken.\ndips     -- get the number of dips in the previously completed second.\nhistory  -- get all the samples in the previously completed second.\nstop     -- cause the server program to end.\n<enter>  -- repeat last command.\n");
        } else if (strncmp(client_message, "count", 5) == 0) {
            sprintf(server_message, "Samples taken total: %lld\n", Sampler_getNumSamplesTaken());
        } else if (strncmp(client_message, "length", 6) == 0) {
            sprintf(server_message, "Samples taken last second: %d\n", Sampler_getHistorySize());
        } else if (strncmp(client_message, "dips", 4) == 0) {
            sprintf(server_message, "Dips: %u\n", Sampler_getNumDips());
        } else if (strncmp(client_message, "history", 7) == 0) {
            bytes_written = 1;
            history_size = Sampler_getHistorySize();
            history = Sampler_getHistory(&history_size);
            values_since_last_newline = 0;
            for(int i = 0; i < history_size; i++) {
                // check if we are exceeding the max data size
                if(bytes_written + 7 > 1500) {
                    // send what has been added to the server message so far
                    if (sendto(socket_desc, server_message, strlen(server_message), 0, (struct sockaddr*)&client_addr, client_struct_length) < 0){
                        printf("Can't send\n");
                        return NULL;
                    }
                    // clear the server message
                    memset(server_message, '\0', sizeof(server_message));

                    // send an indication that the message has been trimmed
                    strcpy(server_message, "\n                  -----------trimmed------------                      \n");
                    if (sendto(socket_desc, server_message, strlen(server_message), 0, (struct sockaddr*)&client_addr, client_struct_length) < 0){
                        printf("Can't send\n");
                        return NULL;
                    }


                    values_since_last_newline = 0;

                    //reset the bytes_written counter
                    bytes_written = 1;
                }
                sprintf(server_message + bytes_written - 1, "%.3lf, ", history[i]);
                bytes_written += 7;
                values_since_last_newline++;
                if(values_since_last_newline == 10) {
                    sprintf(server_message + bytes_written - 1, "\n");
                    bytes_written++;
                    values_since_last_newline = 0;
                }
            }
            sprintf(server_message + bytes_written - 1,"\n");
        } else if (strncmp(client_message, "stop", 4) == 0) {
            stopAll();
        } else if (strncmp(client_message, "\n", 1) == 0) {
            repeat_last_command = true;
            continue;
        } else {
            strcpy(server_message, "Unknown command\n");
        }

        if(strncmp(client_message, "stop", 4) != 0) {
            if (sendto(socket_desc, server_message, strlen(server_message), 0, (struct sockaddr*)&client_addr, client_struct_length) < 0){
                printf("Can't send\n");
                return NULL;
            }
        }
        
    }
    close(socket_desc);
    free(history);
    return NULL;
    
}