#ifndef _SHARED_DATA_STRUCT_H_
#define _SHARED_DATA_STRUCT_H_

#include <stdbool.h>
#include <stdint.h>


// WARNING:
// Fields in the struct must be aligned to match ARM's alignment
//    bool/char, uint8_t:   byte aligned
//    int/long,  uint32_t:  word (4 byte) aligned
//    double,    uint64_t:  dword (8 byte) aligned
// Add padding fields (char _p1) to pad out to alignment.

typedef struct {
    int8_t accel_x;
    int8_t accel_y;
    int8_t target_x;
    int8_t target_y;
    bool on_target;
    bool js_down_pressed;
    bool js_right_pressed;
    bool done;

} sharedMemStruct_t;

#endif
