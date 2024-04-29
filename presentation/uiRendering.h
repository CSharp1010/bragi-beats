#ifndef UI_RENDERING_H
#define UI_RENDERING_H

#include <raylib.h>
#include <stdlib.h>

extern Color OFFWHITE;
extern Color CUSTOMDARKGRAY;

typedef struct {
    int centerX;
    int centerY;
} VisualizerCenterPoint;

typedef struct {
    Rectangle titleBar;
    Rectangle queue;
    Rectangle visualizerSpace;
    Rectangle playbackControlPanel;
    Rectangle library;
    VisualizerCenterPoint center;
} Layout;

void RenderVisualizer(float out_smooth[], size_t numberFftBins, int centerX, int centerY, Rectangle visualizerSpace);
bool DrawButton(Rectangle bounds, const char* text, int fontSize);
void DrawTitleBar();
void DrawSongQueue(Rectangle queue);
void DrawBottomBar(int screenWidth, int screnHeight);
void DrawProgressBar(Music music, int screenHeight, int screenWidth);
void DrawVisualizerSelection(bool *showList, Rectangle buttonBounds);
void DrawLibrary(Rectangle libraryBounds);
void DrawLibraryOrQueue(Layout layout);
void DrawPlaybackControls(Rectangle playbackControlPanel);
Layout CalculateLayout(int screenWidth, int screenHeight);
void DrawUI(Layout layout);
void DrawTextBox(Rectangle textBoxBounds, char* text, int maxLength, bool* isActive);
void DrawTotalTime(Music music, int x, int y);
void DrawSampleInfo(Layout layout);

void PlayPause();

#endif
