#include "hal/audioMixer.h"
#include "hal/beatPattern.h"
#include <time.h>
#include <stdbool.h>
#include <stdio.h>
#include <pthread.h>
#include "../../app/include/shutdown.h"

void sleepForMs(long long delayInMs)
{
    const long long NS_PER_MS = 1000 * 1000;
    const long long NS_PER_SECOND = 1000000000;

    long long delayNs = delayInMs * NS_PER_MS;
    int seconds = delayNs / NS_PER_SECOND;
    int nanoseconds = delayNs % NS_PER_SECOND;

    struct timespec reqDelay = {seconds, nanoseconds};
    nanosleep(&reqDelay, (struct timespec *) NULL);
}

static wavedata_t base;
static wavedata_t hihat;
static wavedata_t snareSound;
static wavedata_t xSound;
static wavedata_t ySound;
static wavedata_t zSound;
static int bpm = 120;
static int volume = 80;
static int currentBeat = 0;
static long long xSoundLastTime = 0;
static long long ySoundLastTime = 0;
static long long zSoundLastTime = 0;
static long long incTempoLastTime = 0;
static long long decTempoLastTime = 0;
static long long incVolLastTime = 0;
static long long decVolLastTime = 0;
static long long cycleBeatLastTime = 0;

static int i = 0;
static int currentPatternSize = 0;
static int secondPatternSizes[] = {2, 1, 2, 1, 2, 1, 2, 1};
static int thirdPatternSizes[] = {1, 1, 1, 0, 1, 0, 1, 0};
static wavedata_t secondPat[8][2];
static wavedata_t thirdPat[5][1];

static long long getTimeInMs(void);
static void playHalfBeat(void);
static void changeCurrentPatternSize(void);
static pthread_t pid;
static void* getBeat(void* args);

void beatPattern_init(void) {
    AudioMixer_readWaveFileIntoMemory("wave-files/100051__menegass__gui-drum-bd-hard.wav", &base);
    AudioMixer_readWaveFileIntoMemory("wave-files/100053__menegass__gui-drum-cc.wav", &hihat);
    AudioMixer_readWaveFileIntoMemory("wave-files/100059__menegass__gui-drum-snare-soft.wav", &snareSound);
    AudioMixer_readWaveFileIntoMemory("wave-files/100060__menegass__gui-drum-splash-hard.wav", &xSound);
    AudioMixer_readWaveFileIntoMemory("wave-files/100062__menegass__gui-drum-tom-hi-hard.wav", &ySound);
    AudioMixer_readWaveFileIntoMemory("wave-files/100056__menegass__gui-drum-cyn-hard.wav", &zSound);
    AudioMixer_setVolume(volume);

    wavedata_t secondPatTemp[][2] = {{base, hihat}, {hihat}, {hihat, snareSound}, {hihat}, {hihat, base}, {hihat}, {hihat, snareSound}, {hihat}};
    wavedata_t thirdPatTemp[][1] = {{snareSound}, {snareSound}, {snareSound}, {snareSound}, {base}};
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < secondPatternSizes[i]; j++) {
            secondPat[i][j] = secondPatTemp[i][j];
        }
    }
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < thirdPatternSizes[i]; j++) {
            thirdPat[i][j] = thirdPatTemp[i][j];
        }
    }

    pthread_create(&pid, NULL, &getBeat, NULL);
}

void beatPattern_cleanup(void)
{
    AudioMixer_freeWaveFileData(&base);
    AudioMixer_freeWaveFileData(&hihat);
    AudioMixer_freeWaveFileData(&snareSound);
    AudioMixer_freeWaveFileData(&xSound);
    AudioMixer_freeWaveFileData(&ySound);
    AudioMixer_freeWaveFileData(&zSound);
    pthread_join(pid, NULL);
}

// Time For Half Beat [sec] = 60 [sec/min] / BPM / 2 [half-beats per beat]
// volume default is 80, bpm is 120
void increaseTempo(){
    long long currentTime = getTimeInMs();
    if (currentTime - incTempoLastTime < 200) {
        return;
    }
    if(bpm < 300){
        bpm += 5;
    }
    incTempoLastTime = currentTime;
}
void decreaseTempo(){
    long long currentTime = getTimeInMs();
    if (currentTime - decTempoLastTime < 200) {
        return;
    }
    if(bpm > 40){
        bpm -=5;
    }
    decTempoLastTime = currentTime;
}

int getBPM(){
    return bpm;
}

void increaseVolume(){
    long long currentTime = getTimeInMs();
    if (currentTime - incVolLastTime < 200) {
        return;
    }
    if(volume < 100){
        volume += 5;
        AudioMixer_setVolume(volume);
    }
    incVolLastTime = currentTime;
}
void decreaseVolume(){
    long long currentTime = getTimeInMs();
    if (currentTime - decVolLastTime < 200) {
        return;
    }
    if(volume > 0){
        volume -= 5;
        AudioMixer_setVolume(volume);
    }
    decVolLastTime = currentTime;
}
int getVolume(){
    return volume;
}
static int getTempo(){
    return 1000*60/bpm/2;
}

void cycleBeat(){
    long long currentTime = getTimeInMs();
    if (currentTime - cycleBeatLastTime < 200) {
        return;
    }
    i = 0;
    currentBeat++;
    if(currentBeat == 3){
        currentBeat = 0;
    }
    changeCurrentPatternSize();
    cycleBeatLastTime = currentTime;
}

void changeBeatMode(int num) 
{
    long long currentTime = getTimeInMs();
    if (currentTime - cycleBeatLastTime < 200) {
        return;
    }
    i = 0;
    currentBeat = num;
    changeCurrentPatternSize();
    cycleBeatLastTime = currentTime;
}

static void changeCurrentPatternSize(void)
{
    if (currentBeat == 0) {
        currentPatternSize = 0;
    } else if (currentBeat == 1) {
        currentPatternSize = 8;
    } else if (currentBeat == 2){
        currentPatternSize = 5;
    }
}
static void* getBeat(void* args){
    (void)args;
    
    while (!Shutdown_isShutdown()) {
        if (i < currentPatternSize) {
            playHalfBeat();
            sleepForMs(getTempo());
            i++;
        } else {
            i = 0;
        }
    }
    return NULL;
    
}

void playDrumSound(int num)
{
    if (num == 1) {
        AudioMixer_queueSound(&hihat);
    } else if (num == 2) {
        AudioMixer_queueSound(&base);
    } else if (num == 3) {
        AudioMixer_queueSound(&snareSound);
    }
}

static void playHalfBeat(void)
{
    if (currentBeat == 1) {
        for (int j = 0; j < secondPatternSizes[i]; j++) {
            AudioMixer_queueSound(&secondPat[i][j]);
        }
    } else if (currentBeat == 2){
        for (int j = 0; j < thirdPatternSizes[i]; j++) {
            AudioMixer_queueSound(&thirdPat[i][j]);
        }
    }
}

void playXSound() {
    long long currentTime = getTimeInMs();
    if (currentTime - xSoundLastTime < 200) {
        return;
    }
    AudioMixer_queueSound(&xSound);
    xSoundLastTime = currentTime;
}

void playYSound(){
    long long currentTime = getTimeInMs();
    if (currentTime - ySoundLastTime < 200) {
        return;
    }
    AudioMixer_queueSound(&ySound);
    ySoundLastTime = currentTime;
}

void playZSound(){
    long long currentTime = getTimeInMs();
    if (currentTime - zSoundLastTime < 200) {
        return;
    }
    AudioMixer_queueSound(&zSound);
    zSoundLastTime = currentTime;
}

int getBeatNum(void)
{
    return currentBeat;
}

static long long getTimeInMs(void)
{
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    long long seconds = spec.tv_sec;
    long long nanoseconds = spec.tv_nsec;
    long long milliSeconds = seconds * 1000
                + nanoseconds / 1000000;
    return milliSeconds;
}