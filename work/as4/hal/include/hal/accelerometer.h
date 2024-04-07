#ifndef _ACCELEROMETER_H_
#define _ACCELEROMETER_H_

#define OUT_X_MSB 0x01
#define OUT_X_LSB 0x02
#define OUT_Y_MSB 0x03
#define OUT_Y_LSB 0x04
#define OUT_Z_MSB 0x05
#define OUT_Z_LSB 0x06


void Accelerometer_init();

void Accelerometer_read(unsigned char regAddr);

void Accelerometer_stop();

int16_t Accelerometer_getX();
int16_t Accelerometer_getY();
int16_t Accelerometer_getZ();

#endif