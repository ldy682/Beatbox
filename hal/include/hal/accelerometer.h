// Module has a thread that continuously reads the 
// Zen Cape's accelerometer, and plays a sound if 
// significant acceleration is detected.

#ifndef ACCELEROMETER_H
#define ACCELEROMETER_H
#define OUT_X_LSB 0
#define OUT_X_MSB 1
#define OUT_Y_LSB 2
#define OUT_Y_MSB 3
#define OUT_Z_LSB 4
#define OUT_Z_MSB 5

void Accelerometer_init(void);
void Accelerometer_cleanup(void);

#endif