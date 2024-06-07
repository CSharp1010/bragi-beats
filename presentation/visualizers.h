#ifndef VISUALIZERS_H
#define VISUALIZERS_H

#include <raylib.h>
#include <stdlib.h>

void barChartVisual(float out_smooth[], size_t m, Rectangle visualizerSpace);
void circleStarVisual(float out_smooth[], size_t m, int centerX, int centerY);
void circleVisual(float out_smooth[], size_t m, int centerX, int centerY);
void wingVisual(float out_smooth[], size_t m, int centerX, int centerY);
void kaleidoscopeVisual(float out_smooth[], size_t m, int centerX, int centerY);
void spiralVisual(float out_smooth[], size_t m, int centerX, int centerY);
void centerLineVisualizer(float out_smooth[], size_t m, Rectangle visualizerSpace);
void sineWaveVisualizer(float out_smooth[], size_t m, Rectangle visualizerSpace);
void mathVisualizer(float out_smooth[], float out_phase[], float out_power[], size_t numberOfFftBins, Rectangle visualizerSpace);
void radialVisualizer(float out_smooth[], size_t m, Rectangle visualizerSpace);
#endif
