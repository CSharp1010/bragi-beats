#include "../infrastructure/globals.h"
#include "visualizers.h"

#include <raylib.h>
#include <math.h>

void barChartVisual(float out_smooth[], size_t m, Rectangle visualizerSpace) {
    float barWidth = visualizerSpace.width / (float)m;
    float maxHeight = visualizerSpace.height;

    for (size_t i = 0; i < m; ++i) {

        float amplitude = out_smooth[i] * maxHeight;
        float lineThickness = 3.0f;

        Vector2 start = {visualizerSpace.x + barWidth * i, visualizerSpace.y + visualizerSpace.height};
        Vector2 end = {visualizerSpace.x + barWidth * i, visualizerSpace.y + visualizerSpace.height - amplitude};

        Color color = (Color){(unsigned char)(255 * sin(GetTime())), 128, (unsigned char)(255 * cos(GetTime())), 255};

        for (int j = 0; j < 5; j++) {
            float fadeFactor = (5 - j) / 5.0f;
            Color fadedColor = ColorAlpha(color, fadeFactor);
            DrawLineEx(start, end, lineThickness * fadeFactor, fadedColor);
        }
    }
}

void circleVisual(float out_smooth[], size_t m, int centerX, int centerY) {
    float maxRadius = ((float)screenHeight - 40 - 100) / 2;

    for (size_t i = 0; i < m; ++i) {

        float amplitude = out_smooth[i];

        Color color = (Color){(unsigned char)(255 * sin(GetTime())), 128, (unsigned char)(255 * cos(GetTime())), 255};

        for (int j = 0; j < 5; j++) {
            float fadeFactor = (5 - j) / 5.0f;
            Color fadedColor = ColorAlpha(color, fadeFactor);
            DrawCircleLines(centerX, centerY, amplitude*maxRadius, fadedColor);
        }
    }
}

void circleStarVisual(float out_smooth[], size_t m, int centerX, int centerY) {
    Color color = (Color){(unsigned char)(255 * sin(GetTime())), 128, (unsigned char)(255 * cos(GetTime())), 255};
    float maxRadius = ((float)screenHeight - 20 - 100) / 2;
    float angleStep = 360.0f / (float)(m-1);  

    for (size_t i = 0; i < m; ++i) {
        float amplitude = out_smooth[i % m];
        float angle = angleStep * i;
        float lineThickness = 2.0f;
        float radian = angle * (PI / 180.0f);

        Vector2 start = { (float)centerX, (float)centerY };
        Vector2 end = { centerX + cos(radian) * (amplitude * maxRadius),
                        centerY + sin(radian) * (amplitude * maxRadius) };

        Color color = (Color){(unsigned char)(255 * sin(GetTime())), 128, (unsigned char)(255 * cos(GetTime())), 255};

        DrawLineEx(start, end, lineThickness, color);
    }
    Vector2 start = { (float)centerX, (float)centerY };
    Vector2 end = { centerX + cos(0) * (out_smooth[0] * maxRadius),
                    centerY + sin(0) * (out_smooth[0] * maxRadius) };
    DrawLineEx(start, end, 2.0f, color);
}

void wingVisual(float out_smooth[], size_t m, int centerX, int centerY) {
    float maxRadius = ((float)screenHeight - 20 - 100) / 2;
    float angleStep = 180.0f / m;

    for (size_t i = 0; i < m; ++i) {

        float amplitude = out_smooth[i];
        float angle = angleStep * i;
        float lineThickness = 2.0f;
        float radian = angle * (PI / 180.0f);

        Vector2 start = { (float)centerX, (float)centerY };
        Vector2 end = { centerX + sin(radian) * (amplitude * maxRadius),
            centerY + cos(radian) * (amplitude * maxRadius) };
        Vector2 end2 = { centerX + sin(-radian) * (amplitude * maxRadius),
            centerY + cos(-radian) * (amplitude * maxRadius) };

        Color color = (Color){(unsigned char)(255 * sin(GetTime())), 128, (unsigned char)(255 * cos(GetTime())), 255};

        for (int j = 0; j < 5; j++) { // Example: Simulated glow effect
            float fadeFactor = (5 - j) / 5.0f; // Decrease opacity
            Color fadedColor = ColorAlpha(color, fadeFactor);
            DrawLineEx(start, end, lineThickness * fadeFactor, fadedColor);
            DrawLineEx(start, end2, lineThickness * fadeFactor, fadedColor);
        }
    }
}

void kaleidoscopeVisual(float out_smooth[], size_t m, int centerX, int centerY) {
    float maxRadius = ((float)screenHeight - 40 - 100) / 2;
    float angleStep = 180.0f / m;

/*
    size_t q = m/2, w = m/3;

   for (size_t i = 0; i < m; ++i) {

        float amplitude = out_smooth[i];
        float angle = angleStep * i;
        float radian = angle *(PI / 360.0f);
        Vector2 centerVis = { centerX + sin(radian*i - (0.25 * GetTime())) * 0.75*maxRadius ,
                            centerY + cos(radian*i - (0.25 * GetTime())) * 0.75*maxRadius };

        Color color = (Color){(unsigned char)(255 * sin(GetTime())), 128, (unsigned char)(255 * cos(GetTime())), 255};

        for (int j = 0; j < 5; j++) { // Example: Simulated glow effect
            float fadeFactor = (5 - j) / 5.0f; // Decrease opacity
            Color fadedColor = ColorAlpha(color, fadeFactor);

            DrawPolyLines(centerVis, 3, amplitude * 0.25 * maxRadius, 64*GetTime(), fadedColor);
        }
    }

    for (size_t i = 0; i < q; ++i) {
*/
    for (size_t i = 0; i < m; ++i) {


        float amplitude = out_smooth[i];
        float angle = angleStep * i;
        float radian = angle * (PI / 180.0f);

/*
        int endX1 = { centerX + sin(radian*i - (0.25 * GetTime())) * maxRadius };
        int endX2 = { centerX + sin(-radian*i - (0.25 * GetTime())) * maxRadius };

        int endY1 = { centerY + cos(radian*i - (0.25 * GetTime())) * maxRadius };
        int endY2 = { centerY + cos(-radian*i - (0.25 * GetTime())) * maxRadius };
*/
        int endX1 = { centerX + sin(radian*i) * maxRadius };
        int endX2 = { centerX + sin(-radian*i) * maxRadius };
        int endX1A ={ centerX + 0.5*(sin(radian*i) * maxRadius) };
        int endX2A = { centerX + 0.5*(sin(-radian*i) * maxRadius) };

        int endY1 = { centerY + cos(radian*i) * maxRadius };
        int endY2 = { centerY + cos(-radian*i) * maxRadius };
        int endY1A = { centerY + 0.5*(-cos(radian*i) * maxRadius) };
        int endY2A = { centerY + 0.5*(-cos(-radian*i) * maxRadius) };

        Color color = (Color){(unsigned char)(255 * sin(GetTime())), 128, (unsigned char)(255 * cos(GetTime())), 255};

        for (int j = 0; j < 5; j++) { // Example: Simulated glow effect
            float fadeFactor = (5 - j) / 5.0f; // Decrease opacity
            Color fadedColor = ColorAlpha(color, fadeFactor);
            DrawCircleLines(endX1, endY1, amplitude * 0.25*maxRadius, fadedColor);
            DrawCircleLines(endX2, endY2, amplitude * 0.25*maxRadius, fadedColor);
        }
    }

        for (size_t i = 0; i < w; ++i) {

        float amplitude = out_smooth[i];
        float angle = angleStep * i;
        float radian = angle * (PI / 180.0f);

        int endX1A ={ centerX + 0.5*(sin(radian*i - (0.25 * GetTime())) * maxRadius) };
        int endX2A = { centerX + 0.5*(sin(-radian*i - (0.25 * GetTime())) * maxRadius) };

        int endY1A = { centerY + 0.5*(-cos(radian*i - (0.25 * GetTime())) * maxRadius) };
        int endY2A = { centerY + 0.5*(-cos(-radian*i - (0.25 * GetTime())) * maxRadius) };

        Color color = (Color){(unsigned char)(255 * sin(GetTime())), 128, (unsigned char)(255 * cos(GetTime())), 255};

        for (int j = 0; j < 5; j++) { // Example: Simulated glow effect
            float fadeFactor = (5 - j) / 5.0f; // Decrease opacity
            Color fadedColor = ColorAlpha(color, fadeFactor);

            DrawCircleLines(endX1A, endY1A, amplitude * 0.2*maxRadius, fadedColor);
            DrawCircleLines(endX2A, endY2A, amplitude * 0.2*maxRadius, fadedColor);
        }
    }
}
