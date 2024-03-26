/*
    NeoPixel RGBW demo program for 8 LED unit such as:
        https://www.adafruit.com/product/2868
    NOTE: This is RGBW, not RGB!

    Install process
    - Put the NeoPixel into a breadboard
    - Connect the NeoPixel with 3 wires:
        - Connect NeoPixel "GND" and "DIN" (data in) to the 3-pin "LEDS" header on Zen
            Zen Cape's LEDS header:
                Pin 1: DIN (Data): left most pin; beside USB-micro connection, connects to P8.11
                Pin 2: GND (Ground): middle pin
                Pin 3: Unused (it's "5V external power", which is not powered normally on the BBG)
        - Connect NeoPixel "5VDC" to P9.7 or P9.8
            Suggest using the header-extender to make it easier to make a good connection.
        - OK to wire directly to BBG: no level shifter required.
    - Software Setup
        - On Host
            make      # on parent folder to copy to NFS
        - On Target: 
            config-pin P8.11 pruout
            make
            make install_PRU0
    - All lights should light up on the LED strip

    Based on code from the PRU Cookbook by Mark A. Yoder:
        https://beagleboard.org/static/prucookbook/#_setting_neopixels_to_different_colors
*/

#include <stdint.h>
#include <pru_cfg.h>
#include "resource_table_empty.h"
#include "../led-matrix-linux/led-shared-struct.h"

#define STR_LEN         64      // # LEDs in our string
#define oneCyclesOn     700/5   // Stay on 700ns
#define oneCyclesOff    800/5
#define zeroCyclesOn    350/5
#define zeroCyclesOff   600/5
#define resetCycles     60000/5 // Must be at least 50u, use 60u

#define OUTPUT_PIN 15         

volatile register uint32_t __R30;   // output gpio register
volatile register uint32_t __R31;   // input gpio register

// Shared Memory Configuration
// -----------------------------------------------------------
#define THIS_PRU_DRAM       0x00000         // Address of DRAM
#define OFFSET              0x200           // Skip 0x100 for Stack, 0x100 for Heap (from makefile)
#define THIS_PRU_DRAM_USABLE (THIS_PRU_DRAM + OFFSET)

volatile sharedMemStruct_t *pSharedMemStruct = (volatile void *)THIS_PRU_DRAM_USABLE;

// COLOURS
// - 1st element in array is 1st (next to wires) on LED strip; last element is last on strip (furthest from wires)
// - Bits: {Unused/8 bits} {Green/8 bits} {Red/8 bits} {Blue/8 bits}
// - Example: 0x000f0000 is green, 0x00000f00 is red, etc
uint32_t matrix[STR_LEN];

void led_show(void) {
    for(int j = 0; j < STR_LEN; j++) {
        for(int i = 23; i >= 0; i--) {
            if(matrix[j] & ((uint32_t)0x1 << i)) {
                __R30 |= 0x1<<OUTPUT_PIN;      // Set the GPIO pin to 1
                __delay_cycles(oneCyclesOn-1);
                __R30 &= ~(0x1<<OUTPUT_PIN);   // Clear the GPIO pin
                __delay_cycles(oneCyclesOff-2);
            } else {
                __R30 |= 0x1<<OUTPUT_PIN;      // Set the GPIO pin to 1
                __delay_cycles(zeroCyclesOn-1);
                __R30 &= ~(0x1<<OUTPUT_PIN);   // Clear the GPIO pin
                __delay_cycles(zeroCyclesOff-2);
            }
        }
    }
}

void led_init(void) {
    // Clear SYSCFG[STANDBY_INIT] to enable OCP master port
    CT_CFG.SYSCFG_bit.STANDBY_INIT = 0;

    // set the color[] array to all off initially
    for(int i = 0; i < STR_LEN; i++) {
        matrix[i] = 0;
    }
    pSharedMemStruct->temperature = 0;
    pSharedMemStruct->humidity = 0;
    pSharedMemStruct->water = 0;
    pSharedMemStruct->light = 0;

    __delay_cycles(resetCycles);
}

void led_setColor(int row, int col, uint8_t g, uint8_t r, uint8_t b) {
    matrix[8 * row + col] = (g << 16) + (r << 8) + b;
}

void led_updateMatrix(void) {
    // temperature
    for(int i = 0; i < 8; i++) {
        led_setColor(2, i, 0, 0, 0);
        for(int j = 0; j < pSharedMemStruct->temperature / 32; j++) {
            led_setColor(2, j, 10, 0, 0);
        }
    }
    // humidity
    for(int i = 0; i < 8; i++) {
        led_setColor(3, i, 0, 0, 0);
    }
    for(int j = 0; j < pSharedMemStruct->humidity / 32; j++) {
        led_setColor(3, j, 10, 0, 0);
    }
    // water
    for(int i = 0; i < 8; i++) {
        led_setColor(4, i, 0, 0, 0);
    }
    for(int j = 0; j < pSharedMemStruct->water / 32; j++) {
        led_setColor(4, j, 10, 0, 0);
    }
    // light
    for(int i = 0; i < 8; i++) {
        led_setColor(5, i, 0, 0, 0);
    }
    for(int j = 0; j < pSharedMemStruct->light / 32; j++) {
        led_setColor(5, j, 10, 0, 0);
    }
}



void main(void)
{
    led_init();

    while(true) {
        led_updateMatrix();
        led_show();
        __delay_cycles(resetCycles);
    }

    // Send Reset
    __R30 &= ~(0x1<<OUTPUT_PIN);   // Clear the GPIO pin
    __delay_cycles(resetCycles);

    __halt();
}
