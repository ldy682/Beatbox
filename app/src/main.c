#include "../../hal/include/hal/audioMixer.h"
#include "../../hal/include/hal/beatPattern.h"
#include "../../hal/include/hal/joystick.h"
#include "../../hal/include/hal/accelerometer.h"
#include "periodTimer.h"
#include "shutdown.h"
#include "network.h"
#include <stdio.h>

int main(void)
{
    AudioMixer_init();
    beatPattern_init();
    Joystick_init();
    Accelerometer_init();
    Shutdown_init();
    Network_init();

    Shutdown_waitForShutdown();
    
    Network_cleanup();
    Shutdown_cleanup();
    Accelerometer_cleanup();
    beatPattern_cleanup();
    AudioMixer_cleanup();
    Joystick_cleanup();

    return 0;
}