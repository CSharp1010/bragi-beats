#ifndef UI_RENDERING_H
#define UI_RENDERING_H

#include "../infrastructure/config.h"
#include "../infrastructure/playback.h"
#include "../infrastructure/file_processing.h"
#include <raylib.h>

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

void RenderVisualizer(float out_smooth[], float out_phase[], float out_power[], size_t numberOfFftBins, int centerX, int centerY, Rectangle visualizerSpace);
bool DrawButton(Rectangle bounds, const char* text, int fontSize);
void DrawTitleBar();
void DrawSongQueue(Rectangle queue);
void DrawBottomBar(int screenWidth, int screenHeight);
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

#endif
