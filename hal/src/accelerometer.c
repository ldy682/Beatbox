#include "hal/accelerometer.h"
#include "hal/beatPattern.h"
#include "../../app/include/periodTimer.h"
#include "../../app/include/shutdown.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <pthread.h>
#include <stdint.h>

#define I2CDRV_LINUX_BUS1 "/dev/i2c-1"
#define I2C_DEVICE_ADDRESS 0x18

static pthread_t accelerometerThreadId;
static pthread_t terminalThreadId;
static int i2cFileDesc;
static int16_t lastXVal = 0;
static int16_t lastYVal = 0;
static int16_t lastZVal = 17000;
// static bool stopping = false;
static int initI2cBus(char* bus, int address);
static void writeI2cReg(int i2cFileDesc, unsigned char regAddr,
                        unsigned char value);
static void* accelThreadFn(void* args);
static void* terminalThreadFn(void* args);
static void terminalOut(Period_statistics_t* accelStats, Period_statistics_t* bufferStats);
static unsigned char readI2cReg(int i2cFileDesc, unsigned char regAddr);

void Accelerometer_init(void)
{
    i2cFileDesc = initI2cBus(I2CDRV_LINUX_BUS1, I2C_DEVICE_ADDRESS);
    writeI2cReg(i2cFileDesc, 0x20, 0xc7); // turn on CTRL_REG1
    pthread_create(&accelerometerThreadId, NULL, &accelThreadFn, NULL);
    pthread_create(&terminalThreadId, NULL, &terminalThreadFn, NULL);
}

void Accelerometer_cleanup(void)
{
    pthread_join(accelerometerThreadId, NULL);
    pthread_join(terminalThreadId, NULL);
}

static void* accelThreadFn(void* args)
{
    (void)args;
    while (!Shutdown_isShutdown()) {
        //read accel
        readI2cReg(i2cFileDesc, 0xa8);
        Period_markEvent(PERIOD_EVENT_READ_ACCELEROMETER);
        sleepForMs(10);
    }
    return NULL;
}

static void* terminalThreadFn(void* args)
{
    (void)args;
    while (!Shutdown_isShutdown()) {
        Period_statistics_t* accelStats = malloc(sizeof(Period_statistics_t));
        Period_getStatisticsAndClear(PERIOD_EVENT_READ_ACCELEROMETER, accelStats);
        Period_statistics_t* bufferStats = malloc(sizeof(Period_statistics_t));
        Period_getStatisticsAndClear(PERIOD_EVENT_FILL_BUFFER, bufferStats);
        terminalOut(accelStats, bufferStats);
        sleepForMs(1000);
    }
    return NULL;
}

static void terminalOut(Period_statistics_t* accelStats, Period_statistics_t* bufferStats)
{    
    printf("M%d %dbpm vol:%d   Audio[%.3f, %.3f] avg %.3f/%d"
            "   Accel[%.3f, %.3f] avg %.3f/%d\n", getBeatNum(),
            getBPM(), getVolume(), bufferStats->minPeriodInMs, bufferStats->maxPeriodInMs, 
            bufferStats->avgPeriodInMs, bufferStats->numSamples
            , accelStats->minPeriodInMs, accelStats->maxPeriodInMs
            , accelStats->avgPeriodInMs, accelStats->numSamples);
    
    free(bufferStats);
    free(accelStats);
}

static int initI2cBus(char* bus, int address)
{
    i2cFileDesc = open(bus, O_RDWR);
    int result = ioctl(i2cFileDesc, I2C_SLAVE, address);
    if (result < 0) {
        perror("I2C: Unable to set I2C device to slave address.");
        exit(1);
    }
    return i2cFileDesc;
}

static void writeI2cReg(int i2cFileDesc, unsigned char regAddr,
                        unsigned char value)
{
        unsigned char buff[2];
        buff[0] = regAddr;
        buff[1] = value;
        int res = write(i2cFileDesc, buff, 2);
        if (res != 2) {
        perror("I2C: Unable to write i2c register.");
        exit(1);
    }
}

static unsigned char readI2cReg(int i2cFileDesc, unsigned char regAddr)
{
    // To read a register, must first write the address
    int res = write(i2cFileDesc, &regAddr, sizeof(regAddr));
    if (res != sizeof(regAddr)) {
        perror("I2C: Unable to write to i2c register.");
        exit(1);
    }
    // Now read the value and return it
    unsigned char buff[6];
    res = read(i2cFileDesc, &buff, 6*sizeof(unsigned char));
    if (res != sizeof(buff)) {
        perror("I2C: Unable to read from i2c register");
        exit(1);
    }

    int16_t x = (buff[OUT_X_MSB] << 8) | (buff[OUT_X_LSB]);
    int16_t y = (buff[OUT_Y_MSB] << 8) | (buff[OUT_Y_LSB]);
    int16_t z = (buff[OUT_Z_MSB] << 8) | (buff[OUT_Z_LSB]);

    if (abs(x - lastXVal) > 8000) {
        playXSound();
    }
    if (abs(y - lastYVal) > 8000) {
        playYSound();
    }
    if (abs(z - lastZVal) > 14000) {
        playZSound();
    }
    lastXVal = x;
    lastYVal = y;
    lastZVal = z;
    unsigned char value = 0;
    return value;
}
