#include <raylib.h>
#include "config.h"
#include "../audio_processing/audio_processing.h"
#include "../presentation/ui_renderer.h"
#include "file_processing.h"

int screenWidth = 1200;
int screenHeight = 900;
Color OFFWHITE = {229, 235, 234, 255};
Color CUSTOMDARKGRAY = {46, 53, 50, 255};

int main(void) {
    InitWindow(screenWidth, screenHeight, "Bragi Beats");
    SetTargetFPS(60);

    Layout layout = CalculateLayout(screenWidth, screenHeight);

    InitAudioDevice();

    while (!WindowShouldClose()) {
        processDroppedFiles();

        size_t numberOfFftBins = ProcessFFT(in_raw, out_log, out_smooth);

        DrawUI(layout);
        RenderVisualizer(out_smooth, out_phase, out_power, numberOfFftBins, layout.center.centerX, layout.center.centerY, layout.visualizerSpace);
    }

    CloseAudioDevice();
    CloseWindow();

    return 0;
}
