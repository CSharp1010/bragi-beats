#ifndef PLAYBACK_H
#define PLAYBACK_H

#include <raylib.h>
#include "config.h"

typedef struct {
    Music songs[MAX_SONGS];
    char titles[MAX_SONGS][256];
    int front, rear;
} SongQueue;

typedef struct SongNode {
    Music song;
    char title[256];
    char fullPath[1024];
    struct SongNode* next;
    struct SongNode* prev;
} SongNode;

extern SongNode* currentSong;
extern SongNode* head;
extern SongNode* tail;
extern SongQueue songQueue;
extern bool isPlaying;
extern VisualizerType currentVisualizer;

SongNode* createSongNode(Music song, const char* title, const char* fullPath);
void enqueueSong(Music song, const char* title, const char* fullPath);
void playSongNode(SongNode* node);
bool enqueueTitle(const char* title);
void PlayPause();
void SkipForward();
void SkipBackward();
void PlaySong(Song* song);

#endif
