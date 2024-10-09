// ui_renderer.h

#ifndef UI_RENDERER_H
#define UI_RENDERER_H

#include <raylib.h>
#include <stddef.h>
#include "fft.h"

// Structure to represent the center point of the visualizer space
typedef struct {
    int centerX;
    int centerY;
} VisualizerCenterPoint;

// Structure to hold all layout rectangles for UI components
typedef struct {
    Rectangle titleBar;
    Rectangle progressBar;
    Rectangle queue;
    Rectangle visualizerSpace;
    Rectangle playbackControlPanel;
    VisualizerCenterPoint center;
} Layout;

// Function declarations
void InitUI(void);
void HandleInput(void);
void UpdatePlaybackState(void);
void RenderUI(size_t numberOfFftBins, AudioData *audioData);
void CleanupUI(void);
Layout CalculateLayout(int screenWidth, int screenHeight);
bool DrawButton(Rectangle bounds, const char* text, int fontSize, Color buttonColor, Color textColor);
void DrawVisualizersButton(void);
void DrawTextCentered(const char* text, Rectangle bounds, int fontSize, Color color);
void DrawUI(Layout layout, size_t numberOfFftBins, AudioData *audioData);
void DrawTitleBar(Rectangle titleBar);
void DrawSongQueue(Rectangle queueBounds);
void DrawPlaybackControls(Rectangle playbackControlPanel);
void DrawProgressBar(Music music,Rectangle progressBarBounds);
void DrawTotalTime(Music music, Rectangle progressBarBounds);
void DrawSampleInfo(Layout layout);
void DrawVisualizerSelection(bool* showList, Rectangle buttonBounds);
void RenderVisualizer(float out_smooth[], size_t numBins, Rectangle visualizerSpace);
void DrawStatusMessage(const char* text, Rectangle titleBar);
void DrawTestSignalSelection(bool* showList, Rectangle buttonBounds);
void DrawTestSignalSelectionButton(void);

#endif // UI_RENDERER_H
