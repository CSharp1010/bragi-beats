#include "../infrastructure/config.h"
#include "visualizers.h"

#include <raylib.h>
#include <math.h>

Color getDynamicColor() {
    float time = GetTime();
    return (Color){
        (unsigned char)(128 + 127 * sin(time)),
        (unsigned char)(128 + 127 * sin(time + PI / 3)),
        (unsigned char)(128 + 127 * cos(time + PI / 3)),
        255
    };
}

Color getWarmPastelColor(int index) {
    Color colors[] = {
        {255, 182, 193, 255}, // Light Pink
        {255, 160, 122, 255}, // Light Salmon
        {240, 230, 140, 255}, // Khaki
        {173, 216, 230, 255}, // Light Blue
        {221, 160, 221, 255}  // Plum
    };
    return colors[index % 5];
}

void DrawShadow(Vector2 start, Vector2 end, float thickness, Color shadowColor, float offsetX, float offsetY) {
    Vector2 shadowStart = { start.x + offsetX, start.y + offsetY };
    Vector2 shadowEnd = { end.x + offsetX, end.y + offsetY };
    DrawLineEx(shadowStart, shadowEnd, thickness, shadowColor);
}


void drawFadeLines(Vector2 start, Vector2 end, float lineThickness, Color color) {
    for (int j = 0; j < 5; j++) {
        float fadeFactor = (5 - j) / 5.0f;
        Color fadedColor = ColorAlpha(color, fadeFactor);
        DrawLineEx(start, end, lineThickness * fadeFactor, fadedColor);
    }
}

void barChartVisual(float out_smooth[], size_t m, Rectangle visualizerSpace) {
    float barWidth = visualizerSpace.width / (float)m;
    float maxHeight = visualizerSpace.height;

    for (size_t i = 0; i < m; ++i) {
        float amplitude = out_smooth[i] * maxHeight;
        float lineThickness = 3.0f;

        Vector2 start = {visualizerSpace.x + barWidth * i, visualizerSpace.y + visualizerSpace.height};
        Vector2 end = {visualizerSpace.x + barWidth * i, visualizerSpace.y + visualizerSpace.height - amplitude};

        Color color = getDynamicColor();
        drawFadeLines(start, end, lineThickness, color);
    }
}

void circleVisual(float out_smooth[], size_t m, int centerX, int centerY) {
    float maxRadius = ((float)screenHeight - 40 - 100) / 2;

    for (size_t i = 0; i < m; ++i) {
        float amplitude = out_smooth[i];
        Color color = getDynamicColor();

        for (int j = 0; j < 5; j++) {
            float fadeFactor = (5 - j) / 5.0f;
            Color fadedColor = ColorAlpha(color, fadeFactor);
            DrawCircleLines(centerX, centerY, amplitude * maxRadius, fadedColor);
        }
    }
}

void circleStarVisual(float out_smooth[], size_t m, int centerX, int centerY) {
    float maxRadius = ((float)screenHeight - 20 - 100) / 2;
    float angleStep = 360.0f / (float)(m - 1);

    for (size_t i = 0; i < m; ++i) {
        float amplitude = out_smooth[i % m];
        float angle = angleStep * i;
        float lineThickness = 2.0f;
        float radian = angle * (PI / 180.0f);

        Vector2 start = {(float)centerX, (float)centerY};
        Vector2 end = {centerX + cos(radian) * (amplitude * maxRadius),
                       centerY + sin(radian) * (amplitude * maxRadius)};

        Color color = getDynamicColor();
        DrawLineEx(start, end, lineThickness, color);
    }
}

void wingVisual(float out_smooth[], size_t m, int centerX, int centerY) {
    float maxRadius = ((float)screenHeight - 20 - 100) / 2;
    float angleStep = 180.0f / m;

    for (size_t i = 0; i < m; ++i) {
        float amplitude = out_smooth[i];
        float angle = angleStep * i;
        float lineThickness = 2.0f;
        float radian = angle * (PI / 180.0f);

        Vector2 start = {(float)centerX, (float)centerY};
        Vector2 end = {centerX + sin(radian) * (amplitude * maxRadius),
                       centerY + cos(radian) * (amplitude * maxRadius)};
        Vector2 end2 = {centerX + sin(-radian) * (amplitude * maxRadius),
                        centerY + cos(-radian) * (amplitude * maxRadius)};

        Color color = getDynamicColor();
        drawFadeLines(start, end, lineThickness, color);
        drawFadeLines(start, end2, lineThickness, color);
    }
}

void kaleidoscopeVisual(float out_smooth[], size_t m, int centerX, int centerY) {
    float maxRadius = ((float)screenHeight - 40 - 100) / 2;
    float angleStep = 360.0f / m;

    for (size_t i = 0; i < m; ++i) {
        float amplitude = out_smooth[i];
        float angle = angleStep * i;
        float radian = angle * (PI / 180.0f);

        Vector2 start = {(float)centerX, (float)centerY};
        Vector2 end = {centerX + sin(radian) * amplitude * maxRadius,
                       centerY + cos(radian) * amplitude * maxRadius};

        Color color = getDynamicColor();
        drawFadeLines(start, end, 2.0f, color);

        // Mirror the line for the kaleidoscope effect
        Vector2 mirroredEnd = {centerX - sin(radian) * amplitude * maxRadius,
                               centerY - cos(radian) * amplitude * maxRadius};
        drawFadeLines(start, mirroredEnd, 2.0f, color);
    }

    for (size_t i = 0; i < m; ++i) {
        for (size_t j = 0; j < 5; j++) {
            float fadeFactor = (5 - j) / 5.0f;
            Color fadedColor = ColorAlpha(getDynamicColor(), fadeFactor);

            float angle = angleStep * i;
            float radian = angle * (PI / 180.0f);
            Vector2 offset = {sin(radian + GetTime()) * maxRadius / 4,
                              cos(radian + GetTime()) * maxRadius / 4};

            Vector2 start = {(float)centerX, (float)centerY};
            Vector2 end = {centerX + sin(radian) * out_smooth[i] * maxRadius + offset.x,
                           centerY + cos(radian) * out_smooth[i] * maxRadius + offset.y};

            DrawLineEx(start, end, 2.0f * fadeFactor, fadedColor);
        }
    }
}

void spiralVisual(float out_smooth[], size_t m, int centerX, int centerY) {
    float maxRadius = ((float)screenHeight - 40 - 100) / 2;
    float angleStep = 2.0f * PI / m; // Spiral step size

    Vector2 previousPoint = { centerX, centerY };

    for (size_t i = 0; i < m * 10; ++i) {
        float amplitude = out_smooth[i % m];
        float angle = angleStep * i;
        float radius = amplitude * maxRadius * (i / (float)(m * 10));

        Vector2 currentPoint = { centerX + cos(angle) * radius, centerY + sin(angle) * radius };
        Color color = getDynamicColor();

        drawFadeLines(previousPoint, currentPoint, 2.0f, color);

        previousPoint = currentPoint;
    }
}

void centerLineVisualizer(float out_smooth[], size_t m, Rectangle visualizerSpace) {
    float centerY = visualizerSpace.y + visualizerSpace.height / 2;
    float lineThickness = 2.0f;
    float maxHeight = visualizerSpace.height / 2;
    float spacing = visualizerSpace.width / (float)(m - 1);

    for (size_t i = 0; i < m; ++i) {
        float amplitude = out_smooth[i] * maxHeight;

        Vector2 startTop = {visualizerSpace.x + i * spacing, centerY};
        Vector2 endTop = {visualizerSpace.x + i * spacing, centerY - amplitude};

        Vector2 startBottom = {visualizerSpace.x + i * spacing, centerY};
        Vector2 endBottom = {visualizerSpace.x + i * spacing, centerY + amplitude};

        Color color = getDynamicColor();
        drawFadeLines(startTop, endTop, lineThickness, color);
        drawFadeLines(startBottom, endBottom, lineThickness, color);
    }
}

void sineWaveVisualizer(float out_smooth[], size_t m, Rectangle visualizerSpace) {
    float centerY = visualizerSpace.y + visualizerSpace.height / 2;
    float maxHeight = visualizerSpace.height / 2.0f;
    float spacing = visualizerSpace.width / (float)(m - 1); // Adjust spacing to fill the width
    float frequency = 1.0f; // You can adjust this to change the frequency of the sine wave

    for (size_t i = 0; i < m; ++i) {
        float amplitude = out_smooth[i] * maxHeight;
        float x = visualizerSpace.x + i * spacing;
        float y = centerY + amplitude * sinf(frequency * i * 2 * PI / m);

        Color color = getDynamicColor();
        DrawCircleV((Vector2){x, y}, 2.0f, color);
    }
}

void mathVisualizer(float out_smooth[], float out_phase[], float out_power[], size_t numberOfFftBins, Rectangle visualizerSpace) {
    float centerX = visualizerSpace.x + visualizerSpace.width / 2;
    float centerY = visualizerSpace.y + visualizerSpace.height / 2;
    float maxRadius = fmin(visualizerSpace.width, visualizerSpace.height) / 2.0f;
    float angleStep = (2 * PI) / numberOfFftBins;
    float shadowOffsetX = 2.5f;
    float shadowOffsetY = 2.5f;
    Color shadowColor = (Color){ 100, 100, 100, 25 }; // Deep shadow color

    // Draw the circular bars with shadows
    for (size_t i = 0; i < numberOfFftBins; ++i) {
        float amplitude = out_smooth[i] * maxRadius;
        float angle = i * angleStep;
        float x1 = centerX + cosf(angle) * maxRadius;
        float y1 = centerY + sinf(angle) * maxRadius;
        float x2 = centerX + cosf(angle) * (maxRadius + amplitude) + 30;
        float y2 = centerY + sinf(angle) * (maxRadius + amplitude) + 30;

        Color color = getWarmPastelColor(i);
        DrawShadow((Vector2){centerX, centerY}, (Vector2){x2, y2}, 7.0f, shadowColor, shadowOffsetX, shadowOffsetY);
        DrawLineEx((Vector2){centerX, centerY}, (Vector2){x2, y2}, 5.0f, color);
    }

    // Floating dots with smoother movement and shadows
    static Vector2 dot_positions[FFT_SIZE];
    static Vector2 dot_velocities[FFT_SIZE];
    static bool initialized = false;
    if (!initialized) {
        for (size_t i = 0; i < numberOfFftBins; ++i) {
            dot_positions[i] = (Vector2){ visualizerSpace.x + GetRandomValue(0, visualizerSpace.width), visualizerSpace.y + GetRandomValue(0, visualizerSpace.height) };
            dot_velocities[i] = (Vector2){ GetRandomValue(-50, 50) / 100.0f, GetRandomValue(-50, 50) / 100.0f };
        }
        initialized = true;
    }

    for (size_t i = 0; i < numberOfFftBins; ++i) {
        float power = out_power[i] * maxRadius;

        // Update position smoothly
        dot_positions[i].x += dot_velocities[i].x * power / 1000.0f;
        dot_positions[i].y += dot_velocities[i].y * power / 1000.0f;

        // Ensure dots stay within the visualizer space
        if (dot_positions[i].x < visualizerSpace.x || dot_positions[i].x > visualizerSpace.x + visualizerSpace.width) {
            dot_velocities[i].x *= -1;
        }
        if (dot_positions[i].y < visualizerSpace.y || dot_positions[i].y > visualizerSpace.y + visualizerSpace.height) {
            dot_velocities[i].y *= -1;
        }

        Color color = getWarmPastelColor(i + 5);
        DrawShadow(dot_positions[i], (Vector2){dot_positions[i].x, dot_positions[i].y}, power / 2.0f, shadowColor, shadowOffsetX, shadowOffsetY);
        DrawCircleV(dot_positions[i], power / 50.0f, color); // Adjust the divisor to control dot size
    }
}

Color getAmplitudeColor(float amplitude, float maxAmplitude) {
    float ratio = amplitude / maxAmplitude;
    return (Color){
        (unsigned char)(255 * ratio),
        (unsigned char)(0),
        (unsigned char)(255 * (1 - ratio)),
        255
    };
}

void radialVisualizer(float out_smooth[], size_t m, Rectangle visualizerSpace) {
    float centerX = visualizerSpace.x + visualizerSpace.width / 2;
    float centerY = visualizerSpace.y + visualizerSpace.height / 2;
    float maxRadius = fmin(visualizerSpace.width, visualizerSpace.height) / 1.0f;
    float angleStep = 2 * PI / m;

    float maxAmplitude = 0;
    for (size_t i = 0; i < m; ++i) {
        if (out_smooth[i] > maxAmplitude) {
            maxAmplitude = out_smooth[i];
        }
    }

    for (size_t i = 0; i < m; ++i) {
        float amplitude = out_smooth[i];
        float radius = (amplitude / maxAmplitude) * maxRadius;
        float angle = i * angleStep;

        float x = centerX + radius * cosf(angle);
        float y = centerY + radius * sinf(angle);

        Color color = getAmplitudeColor(amplitude, maxAmplitude);
        DrawLine(centerX, centerY, x, y, color);
        DrawCircleV((Vector2){x, y}, 3.0f, color);
    }
}
