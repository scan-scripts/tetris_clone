#include "sound.h"
#include "types.h"

extern GameState gameState;

Music music;
bool musicLoaded = false;

const char *musicFiles[] = {
    [CLASSIC] = "resources/classical_tetris_music.mp3",
    [SILLY] = "resources/electronic_tetris_music.mp3",
};

const char *tetrisSfxFiles[] = {
    [MOVE_SOUND] = "resources/tetris-gb-18-move-piece.mp3",
    [MENU_SOUND] = "resources/tetris-gb-17-menu-sound.mp3",
    [LINE_CLEARED_SOUND] = "resources/tetris-gb-21-line-clear.mp3",
    [SINGLE] = "resources/sfx_single.wav",
    [DOUBLE_SOUND] = "resources/sfx_single.wav",
    [TRIPLE_SOUND] = "resources/sfx_triple.wav",
    [TETRIS_SOUND] = "resources/sfx_tetris.wav",
    [B2B_TETRIS_SOUND] = "resources/sfx_back_to_back_tetris.wav",
};

void ApplyMusicVolume(void) {
    if (musicLoaded) SetMusicVolume(music, gameState.settings.musicVolume / 100.0f);
}

void ApplyMusicStyle(void) {
    if (musicLoaded) {
        StopMusicStream(music);
        UnloadMusicStream(music);
        musicLoaded = false;
    }
    if (gameState.settings.musicStyle != NONE) {
        music = LoadMusicStream(musicFiles[gameState.settings.musicStyle]);
        PlayMusicStream(music);
        musicLoaded = true;
    }
}

void InitSound(void) {
    InitAudioDevice();
    if (gameState.settings.musicStyle != NONE) {
        music = LoadMusicStream(musicFiles[gameState.settings.musicStyle]);
        PlayMusicStream(music);
        musicLoaded = true;
    }
}

void TickSound(void) {
    if (musicLoaded) UpdateMusicStream(music);
}

void CleanupSound(void) {
    if (musicLoaded) UnloadMusicStream(music);
    CloseAudioDevice();
}
