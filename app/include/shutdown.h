// Holds a condition variable that can 
// trigger cleanup of all modules 

#ifndef SHUTDOWN_H
#define SHUTDOWN_H

#include <stdbool.h>

void Shutdown_init(void);
void Shutdown_cleanup(void);
bool Shutdown_isShutdown(void);
void Shutdown_signalShutdown(void);
void Shutdown_waitForShutdown(void); 

#endif