#include "hal/joystick.h"
#include "hal/beatPattern.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "../../app/include/shutdown.h"

static void runCommand(char* command);
static void writeToFile(char* filePath, char* str);
static int readIntFromFile(char* filePath);
static void* joystickThreadFn(void* args);

static pthread_t joystickThreadId;
// static bool stopping = false;

void Joystick_init(void) 
{
    // config joystick pins for gpio
    runCommand("config-pin p8.14 gpio");
    runCommand("config-pin p8.15 gpio");
    runCommand("config-pin p8.16 gpio");
    runCommand("config-pin p8.18 gpio");
    runCommand("config-pin p8.17 gpio");
    runCommand("config-pin p9_18 i2c");
    runCommand("config-pin p9_17 i2c");

    // config joystick pins for input
    writeToFile("/sys/class/gpio/gpio26/direction", "in");
    writeToFile("/sys/class/gpio/gpio47/direction", "in");
    writeToFile("/sys/class/gpio/gpio46/direction", "in");
    writeToFile("/sys/class/gpio/gpio65/direction", "in");
    writeToFile("/sys/class/gpio/gpio27/direction", "in");
    
    pthread_create(&joystickThreadId, NULL, &joystickThreadFn, NULL);
}

void Joystick_cleanup(void) 
{
    // stopping = true;
    pthread_join(joystickThreadId, NULL);
}

JoystickDirection Joystick_getDirectionPressed(void)
{
    if (Joystick_isPressedUp()) {
        return JOYSTICK_UP;
    } else if (Joystick_isPressedDown()) {
        return JOYSTICK_DOWN;
    } else if (Joystick_isPressedLeft()) {
        return JOYSTICK_LEFT;
    } else if (Joystick_isPressedRight()) {
        return JOYSTICK_RIGHT;
    } else if (Joystick_isPressedIn()) {
        return JOYSTICK_IN;
    }

    // return NO_DIRECTION if Joystick is not pressed
    return JOYSTICK_NO_DIRECTION;
}

static void* joystickThreadFn(void* args)
{
    (void)args;

    while (!Shutdown_isShutdown()) {
        JoystickDirection joystickDirection = Joystick_getDirectionPressed();
        if (joystickDirection == JOYSTICK_IN) {
            cycleBeat();
        } else if (joystickDirection == JOYSTICK_DOWN) {
            decreaseVolume();
        } else if (joystickDirection == JOYSTICK_LEFT) {
            decreaseTempo();
        } else if (joystickDirection == JOYSTICK_RIGHT) {
            increaseTempo();
        } else if (joystickDirection == JOYSTICK_UP) {
            increaseVolume();
        }
        sleepForMs(10);
    }
    return NULL;
}

// call functions that read a file to determine if the Joystick is pressed
bool Joystick_isPressedIn(void)
{
    return readIntFromFile("/sys/class/gpio/gpio27/value") == 0;
}

bool Joystick_isPressedUp(void)
{
    return readIntFromFile("/sys/class/gpio/gpio26/value") == 0;
}

bool Joystick_isPressedDown(void)
{
    return readIntFromFile("/sys/class/gpio/gpio46/value") == 0;
}

bool Joystick_isPressedLeft(void)
{
    return readIntFromFile("/sys/class/gpio/gpio65/value") == 0;
}

bool Joystick_isPressedRight(void)
{
    return readIntFromFile("/sys/class/gpio/gpio47/value") == 0;
}

static void runCommand(char* command)
{
    // Execute the shell command (output into pipe)
    FILE *pipe = popen(command, "r");

    // Ignore output of the command; but consume it
    // so we don't get an error when closing the pipe.
    char buffer[1024];
    while (!feof(pipe) && !ferror(pipe)) {
        if (fgets(buffer, sizeof(buffer), pipe) == NULL)
            break;
        // printf("--> %s", buffer); // Uncomment for debugging
    }

    // Get the exit code from the pipe; non-zero is an error:
    int exitCode = WEXITSTATUS(pclose(pipe));
    if (exitCode != 0) {
        perror("Unable to execute command:");
        printf(" command: %s\n", command);
        printf(" exit code: %d\n", exitCode);
    }
}

static void writeToFile(char* filePath, char* str)
{
    FILE *pFile = fopen(filePath, "w");

    if (pFile == NULL) {
        printf("ERROR: Unable to open file %s\n", filePath);
        exit(1);
    }

    int charWritten = fprintf(pFile, str);
    if (charWritten <= 0) {
        printf("Error writing data to file %s\n", filePath);
        exit(1);
    }

    fclose(pFile);
}

static int readIntFromFile(char* filePath)
{
	// Open file
	FILE *f = fopen(filePath, "r");
	if (!f) {
		printf("ERROR: Unable to open voltage input file. Cape loaded?\n");
		printf("       Check /boot/uEnv.txt for correct options.\n");
		exit(-1);
	}

	// Get reading
	int a2dReading = 0;
	int itemsRead = fscanf(f, "%d", &a2dReading);
	if (itemsRead <= 0) {
		printf("ERROR: Unable to read values from voltage input file.\n");
		exit(-1);
	}

	// Close file
	fclose(f);

	return a2dReading;
}