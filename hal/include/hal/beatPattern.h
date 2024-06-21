// Functions to play a sound or a beat pattern, and
// control the tempo and volume of the beatbox.

#ifndef _PATTERN_H_
#define _PATTERN_H_

void beatPattern_init(void);
void beatPattern_cleanup(void);
void increaseTempo(void);
void decreaseTempo(void);
void increaseVolume(void);
void decreaseVolume(void);
void sleepForMs(long long);
void cycleBeat(void);
void playXSound(void);
void playYSound(void);
void playZSound(void);
int getBeatNum(void);
int getBPM(void);
int getVolume(void);
void changeBeatMode(int num);
void playDrumSound(int num);
#endif