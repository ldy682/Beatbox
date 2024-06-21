// Functions that detect if the Joystick is pressed, 
// and in which direction the Joystick is pressed.

#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <stdbool.h>

typedef enum e_JoystickDirection {
    JOYSTICK_UP, 
    JOYSTICK_DOWN, 
    JOYSTICK_LEFT, 
    JOYSTICK_RIGHT,
    JOYSTICK_IN, 
    JOYSTICK_NO_DIRECTION
} JoystickDirection;

void Joystick_init(void);
void Joystick_cleanup(void);
bool Joystick_isPressedIn(void);
bool Joystick_isPressedUp(void);
bool Joystick_isPressedDown(void);
bool Joystick_isPressedLeft(void);
bool Joystick_isPressedRight(void);
JoystickDirection Joystick_getDirectionPressed(void);

#endif