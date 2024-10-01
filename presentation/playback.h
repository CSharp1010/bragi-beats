// playback.h

#ifndef PLAYBACK_H
#define PLAYBACK_H

#include <raylib.h>
#include <stdbool.h>

#define MAX_SONGS 100

typedef enum {
    VISUALIZER_BAR_CHART,
    VISUALIZER_IRIDESCENT
} VisualizerType;

typedef struct SongNode {
    Music song;
    char title[256];
    char fullPath[1024];
    struct SongNode* next;
    struct SongNode* prev;
} SongNode;

typedef struct {
    Music songs[MAX_SONGS];
    char titles[MAX_SONGS][256];
    int front, rear;
} SongQueue;

// External variables
extern SongNode* currentSong;
extern SongNode* head;
extern SongNode* tail;
extern SongQueue songQueue;
extern bool isPlaying;
extern VisualizerType currentVisualizer;

// Function declarations
void StopCurrentSong(void);
void PlayCurrentSong(void);
void PlayPause(void);
void SkipForward(void);
void SkipBackward(void);
void playSongNode(SongNode* node);
bool enqueueTitle(const char* title);
void enqueueSong(Music song, const char* title, const char* fullPath);

#endif // PLAYBACK_H
