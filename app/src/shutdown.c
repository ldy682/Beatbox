#include "shutdown.h"
#include <pthread.h>
#include <stdio.h>
#include <stdbool.h>
#include "../../hal/include/hal/beatPattern.h"

static pthread_mutex_t shutdownMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t shutdownCond = PTHREAD_COND_INITIALIZER;
static bool isShutdown;

void Shutdown_init(void)
{
    isShutdown = false;
}

void Shutdown_cleanup(void)
{
    pthread_mutex_destroy(&shutdownMutex);
    pthread_cond_destroy(&shutdownCond);
}

bool Shutdown_isShutdown(void)
{
    return isShutdown;
}

void Shutdown_signalShutdown(void)
{
    pthread_mutex_lock(&shutdownMutex);
    {
        pthread_cond_signal(&shutdownCond);
        isShutdown = true;
    }
    pthread_mutex_unlock(&shutdownMutex);
}

void Shutdown_waitForShutdown(void)
{
    pthread_mutex_lock(&shutdownMutex);
    {
        pthread_cond_wait(&shutdownCond, &shutdownMutex);
    }
    pthread_mutex_unlock(&shutdownMutex);
}