#ifndef VISUALIZERS_H
#define VISUALIZERS_H

#include <raylib.h>
#include <stdlib.h>

typedef struct {
    Vector2 position;
    Vector2 velocity;
    Color color;
    float life; // Remaining life of the particle
    float size;
} Particle;

void DrawBarChart(float out_smooth[], size_t numBins, Rectangle visualizerSpace);
void DrawIridescentVisualizer(float out_smooth[], size_t numBins, Rectangle visualizerSpace);
void Draw3DTimeTunnelVisualizer(float out_smooth[], size_t numBins, Rectangle visualizerSpace);

#endif // VISUALIZERS_H
