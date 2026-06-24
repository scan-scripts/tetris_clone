#ifndef SOUND_H
#define SOUND_H

#include "raylib.h"
#include "types.h"

extern Music music;
extern bool musicLoaded;
extern const char *musicFiles[];
extern const char *tetrisSfxFiles[];

void InitSound(void);
void TickSound(void);
void CleanupSound(void);

void ApplyMusicVolume(void);
void ApplyMusicStyle(void);

#endif
