#ifndef GLOBALS_H
#define GLOBALS_H

#include <complex.h>
#include <raylib.h>

#define FFT_SIZE (1<<15)
#define MAX_SONGS 100
#define ARRAY_LEN(xs) sizeof(xs)/sizeof(xs[0])


typedef struct {
    Music songs[MAX_SONGS];
    char titles[MAX_SONGS][256];
    int front, rear;
} SongQueue;

typedef struct SongNode {
    Music song;
    char title[256];
    char fullPath[1014];
    struct SongNode* next;
    struct SongNode* prev;
} SongNode;

extern SongNode* currentSong;
extern SongNode* prevSong;
extern SongNode* nextSong;
extern SongNode* head;

extern SongQueue songQueue;

typedef enum {
    VISUALIZER_BAR_CHART,
    VISUALIZER_CIRCLE_STAR,
    VISUALIZER_WING,
    VISUALIZER_KALEIDOSCOPE
} VisualizerType;

extern VisualizerType currentVisualizer;

extern int screenWidth;
extern int screenHeight;
extern float in_raw[FFT_SIZE];
extern float in_win[FFT_SIZE];
extern float complex out_raw[FFT_SIZE];
extern float out_log[FFT_SIZE];
extern float out_smooth[FFT_SIZE];

extern Music currentMusic;
extern Music previousMusic;
extern bool isPlaying;
extern bool showList;

extern Color OFFWHITE;
extern Color CUSTOMDARKGRAY;

extern bool authorizedUser;
extern bool showLibrary;
extern bool loginActive;
extern char username[256];
extern int letterCount;
extern bool visualizerListOpen;

typedef struct Song {
    char name[256];
    char filePath[1024];
    struct Song *next;
} Song;

typedef struct Album {
    char name[256];
    Song* songs;
    struct Album *next;
    int songCount;
    bool expanded;
} Album;

typedef struct {
    Album *albums;
    int albumCount;
} Library;

Library userLibrary;

void SkipForward();
void SkipBackward();
bool processAlbumDirectory(const char *albumPath, const char *albumName);
void LoginUser();
void PlaySong(Song *song);
#endif
