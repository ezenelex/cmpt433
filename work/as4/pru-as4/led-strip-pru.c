/*
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
#include "led-shared-struct.h"

#define STR_LEN         8      // # LEDs in our string
#define oneCyclesOn     700/5   // Stay on 700ns
#define oneCyclesOff    800/5
#define zeroCyclesOn    350/5
#define zeroCyclesOff   600/5
#define resetCycles     60000/5 // Must be at least 50u, use 60u

// i dont think these are pins
#define OUTPUT_PIN 15 
#define JOYSTICK_RIGHT_PIN 15
#define JOYSTICK_DOWN_PIN 14   

#define JOYSTICK_DOWN_MASK (1 << JOYSTICK_DOWN_PIN)
#define JOYSTICK_RIGHT_MASK (1 << JOYSTICK_RIGHT_PIN)

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
uint32_t strip[STR_LEN];



// actually shows the contents of the strip[] matrix onto the real life strip
void led_show(void) {
    for(int j = 0; j < STR_LEN; j++) {
        for(int i = 23; i >= 0; i--) {
            if(strip[j] & ((uint32_t)0x1 << i)) {
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
        strip[i] = 0;
    }
    pSharedMemStruct->target_x = 0;
    pSharedMemStruct->target_y = 0;
    pSharedMemStruct->on_target = false;
    pSharedMemStruct->js_down_pressed = false;
    pSharedMemStruct->js_right_pressed = false;
    pSharedMemStruct->done = false;


    __delay_cycles(resetCycles);
}

// sets a single pixel to a color
// need to run led_show() to see it irl
void led_setColor(int led, uint8_t g, uint8_t r, uint8_t b) {
    if(led > STR_LEN) return;
    if(led < 0) return;
    strip[led] = (g << 16) + (r << 8) + b;
}

// clears the strip[] array
// need to run led_show() to see it irl
void led_clearStrip(void) {
    for(int i = 0; i < STR_LEN; i++) {
        led_setColor(i, 0 ,0 ,0);
    }
}

// deltaX positive means aiming too far to left
// deltaX negative means aiming too far to right
// deltaY positive means aiming too far up
// deltaY negative means aiming too far down

// update the strip[] matrix to store the target at the correct location
void led_drawTarget(int led, uint8_t g, uint8_t r, uint8_t b) {
    led_setColor(led + 1, g/5, r/5, b/5);
    led_setColor(led, g, r, b);
    led_setColor(led - 1, g/5, r/5, b/5);
}

// set the whole strip 1 color
// need to run led_show() to see it irl
void led_setStrip(uint8_t g, uint8_t r, uint8_t b) {
    for(int i = 0; i < STR_LEN; i++) {
        led_setColor(i, g, r, b);
    }
}

// find out where the target is in relation to the current pointing direction
// then draw the point with led_drawTarget()
void led_updateTarget(int deltaX, int deltaY) {
    // clear the previous target that was drawn
    led_clearStrip();
    if(deltaY > 5) deltaY = 5;
    if(deltaY < -5) deltaY = -5;

    // aiming at correct X value (blue)
    if(deltaX == 0 ) {
        // on target
        if(deltaY == 0) {
            pSharedMemStruct->on_target = true;
            led_setStrip(0,0,255);
        // aiming too high
        } else if(deltaY > 0) {
            pSharedMemStruct->on_target = false;
            led_drawTarget(4 - deltaY, 0, 0, 255);
        } else {
            pSharedMemStruct->on_target = false;
            led_drawTarget(3 - deltaY, 0, 0, 255);
        }
    // aiming too far left (green)
    } else if(deltaX > 0 ) {
        pSharedMemStruct->on_target = false;
        // on target vertically
        if(deltaY == 0) {
            led_setStrip(255,0,0);
        // aiming too high
        } else if(deltaY > 0) {
            led_drawTarget(4 - deltaY, 255, 0, 0);
        } else {
            led_drawTarget(3 - deltaY, 255, 0, 0);
        }
    // aiming too far right (red)
    } else if(deltaX < 0) {
        pSharedMemStruct->on_target = false;
        // aiming too low
        if(deltaY == 0) {
            led_setStrip(0,255,0);
        } else if(deltaY > 0) {
            led_drawTarget(4 - deltaY, 0,255,0);
        } else {
            led_drawTarget(3 - deltaY, 0,255,0);
        }        
    }
}

// start the computation process of figuring out where the target is in relation
// to where the device is pointing. then figure out where to draw that point on
// the strip
void led_updateStrip(void) {
    int16_t deltaX = pSharedMemStruct->target_x - pSharedMemStruct->accel_x;
    int16_t deltaY = pSharedMemStruct->target_y - pSharedMemStruct->accel_y;
    led_updateTarget(deltaX/10, deltaY/10);
}


void joystick_read(void) {
    // joystick down pressed
    if(!(__R31 & JOYSTICK_DOWN_MASK)) {
        pSharedMemStruct->js_down_pressed = true;
    } else {
        pSharedMemStruct->js_down_pressed = false;
    }
    // joystick right pressed
    if(!(__R31 & JOYSTICK_RIGHT_MASK)) {
        pSharedMemStruct->js_right_pressed = true;
    } else {
        pSharedMemStruct->js_right_pressed = false;
    }
}

void main(void)
{
    led_init();

    while(true) {
        // read joystick
        joystick_read();
        // do the math
        led_updateStrip();
        // show the results
        led_show();

        if(pSharedMemStruct->done) {
            led_clearStrip();
            __delay_cycles(resetCycles * 10);
            led_show();
            __delay_cycles(resetCycles * 10);
            break;
        }

        // delay so it doesnt flicker
        __delay_cycles(resetCycles * 10);
    }

    // Send Reset
    __R30 &= ~(0x1<<OUTPUT_PIN);   // Clear the GPIO pin
    __delay_cycles(resetCycles);

    __halt();
}
