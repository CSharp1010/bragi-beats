#include <raylib.h>
#include <fftw3.h>
#include "config.h"
#include "../business/audioProcessing.h"
#include "../presentation/uiRendering.h"
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
    initializeFFT();

    while (!WindowShouldClose()) {
        processDroppedFiles();

        size_t numberOfFftBins = ProcessFFT(in_raw, out_log, out_smooth, out_phase, out_power);

        DrawUI(layout);
        RenderVisualizer(out_smooth, out_phase, out_power, numberOfFftBins, layout.center.centerX, layout.center.centerY, layout.visualizerSpace);
    }

    cleanupFFT();
    CloseAudioDevice();
    CloseWindow();

    return 0;
}
