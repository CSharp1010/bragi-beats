#ifndef CONFIG_H
#define CONFIG_H

#include <raylib.h>

#define FFT_SIZE (1 << 14)
#define MAX_SONGS 100
#define ARRAY_LEN(xs) sizeof(xs) / sizeof(xs[0])

typedef enum {
    VISUALIZER_BAR_CHART,
    VISUALIZER_CIRCLE_STAR,
    VISUALIZER_WING,
    VISUALIZER_KALEIDOSCOPE,
    VISUALIZER_SPIRAL,
    VISUALIZER_CENTER_LINE,
    VISUALIZER_SINE_WAVE,
    VISUALIZER_MATH,
    VISUALIZER_RADIAL
} VisualizerType;

typedef struct Song {
    char name[256];
    char filePath[1024];
    struct Song* next;
} Song;

typedef struct Album {
    char name[256];
    Song* songs;
    struct Album* next;
    int songCount;
    bool expanded;
} Album;

typedef struct {
    Album* albums;
    int albumCount;
} Library;

extern int screenWidth;
extern int screenHeight;
extern Color OFFWHITE;
extern Color CUSTOMDARKGRAY;

extern bool authorizedUser;
extern bool showLibrary;
extern bool showList;
extern bool loginActive;
extern bool visualizerListOpen;
extern Library userLibrary;

#endif
