#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../include/playback.h"
#include "../../include/ui.h"
#include "../../include/visualizers.h"
#include "../../include/fft.h"
#include <raylib.h>

extern int screenWidth;
extern int screenHeight;
extern Color DARK_BACKGROUND;
extern Color ACCENT_BLUE;
extern Color LIGHT_TEXT;
extern Color ACCENT_RED;
extern Color DARKER_RED;

static Layout layout;
static bool showVisualizerList = false;
static bool showQueue = true;

void InitUI(void) {
    layout = CalculateLayout(screenWidth, screenHeight);
}

void CleanupUI(void) {
    // Implement cleanup if necessary
}

void RenderUI(size_t numberOfFftBins, AudioData *audioData) {
    BeginDrawing();
    ClearBackground(DARK_BACKGROUND);

    HandleInput();
    DrawUI(layout, numberOfFftBins, audioData);

    EndDrawing();
}

Layout CalculateLayout(int screenWidth, int screenHeight) {
    Layout layout;
    layout.titleBar = (Rectangle){0, 0, screenWidth, 60}; // Fixed height

    layout.playbackControlPanel = (Rectangle){
        0,
        screenHeight - 60,
        screenWidth,
        60
    };

    // Define the progress bar rectangle
    layout.progressBar = (Rectangle){
        .x = 100,
        .y = layout.playbackControlPanel.y - 30, // Position above playback controls
        .width = screenWidth - 200,
        .height = 20
    };

    // Add margin for the progress bar time text (e.g., 30 pixels)
    float marginBelowProgressBar = 30;

    float contentHeight = layout.progressBar.y - layout.titleBar.height - marginBelowProgressBar;

    if (showQueue) {
        layout.queue = (Rectangle){
            0,
            layout.titleBar.height,
            250, // Fixed width for the queue
            contentHeight
        };
        layout.visualizerSpace = (Rectangle){
            layout.queue.width,
            layout.titleBar.height,
            screenWidth - layout.queue.width,
            contentHeight
        };
    } else {
        layout.queue = (Rectangle){0, 0, 0, 0};
        layout.visualizerSpace = (Rectangle){
            0,
            layout.titleBar.height,
            screenWidth,
            contentHeight
        };
    }

    // Center of the visualizer space
    layout.center.centerX = layout.visualizerSpace.x + layout.visualizerSpace.width / 2;
    layout.center.centerY = layout.visualizerSpace.y + layout.visualizerSpace.height / 2;

    return layout;
}

// Enhanced button drawing function
bool DrawButton(Rectangle bounds, const char* text, int fontSize, Color buttonColor, Color textColor) {
    Vector2 mousePoint = GetMousePosition();
    bool isHovering = CheckCollisionPointRec(mousePoint, bounds);
    bool isPressed = false;

    Color currentButtonColor = buttonColor;

    if (isHovering) {
        currentButtonColor = Fade(buttonColor, 0.8f);
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            currentButtonColor = Fade(buttonColor, 0.6f);
        }
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            isPressed = true;
        }
    }

    // Draw rounded rectangle for the button
    DrawRectangleRounded(bounds, 0.25f, 16, currentButtonColor);

    // Center the text
    int textWidth = MeasureText(text, fontSize);
    int textX = bounds.x + (bounds.width - textWidth) / 2;
    int textY = bounds.y + (bounds.height - fontSize) / 2;

    DrawText(text, textX, textY, fontSize, textColor);

    return isPressed;
}

void DrawTextCentered(const char* text, Rectangle bounds, int fontSize, Color color) {
    int textWidth = MeasureText(text, fontSize);
    DrawText(text, bounds.x + (bounds.width - textWidth) / 2, bounds.y + (bounds.height - fontSize) / 2, fontSize, color);
}

// Handle user input separately
void HandleInput(void) {
    // Handle other input events
    if (IsKeyPressed(KEY_SPACE)) {
        PlayPause();
    }
    if (IsKeyPressed(KEY_RIGHT)) {
        SkipForward();
    }
    if (IsKeyPressed(KEY_LEFT)) {
        SkipBackward();
    }

    if (isPlaying && currentSong != NULL) {
        UpdateMusicStream(currentSong->song);
    }
}

void UpdatePlaybackState(void) {
    // Update playback controls
    if (currentSong != NULL) {
        if (GetMusicTimePlayed(currentSong->song) >= GetMusicTimeLength(currentSong->song)) {
            SkipForward();
            if (currentSong == NULL) {
                isPlaying = false;
                currentSong = head;
            } else {
                PlayMusicStream(currentSong->song);
                SetMusicVolume(currentSong->song, 0.5f);
                AttachAudioStreamProcessor(currentSong->song.stream, callback);
            }
        }
    }
}

void DrawUI(Layout layout, size_t numberOfFftBins, AudioData *audioData) {
    DrawTitleBar(layout.titleBar);
    DrawVisualizersButton();

    // Draw the progress bar before playback controls
    if (currentSong != NULL) {
        DrawProgressBar(currentSong->song, layout.progressBar);
        DrawSampleInfo(layout);
    }

    RenderVisualizer(
        audioData->out_smooth,
        numberOfFftBins,
        layout.visualizerSpace
    );

    // Only draw the queue if it's visible
    if (showQueue) {
        DrawSongQueue(layout.queue);
    }

    DrawPlaybackControls(layout.playbackControlPanel);

    // Handle visualizer selection if needed
    if (showVisualizerList) {
        Rectangle visualizerButtonBounds = {
            .x = 20,
            .y = layout.titleBar.y + (layout.titleBar.height - 50) / 2,
            .width = 120,
            .height = 50
        };
        DrawVisualizerSelection(&showVisualizerList, visualizerButtonBounds);
    }

    if (testMode) {
        DrawTestSignalSelectionButton();
    }

    // Display status messages
    if (testMode) {
        DrawStatusMessage("Test Mode Active", layout.titleBar);
    } else if (isPlaying) {
        DrawStatusMessage("Playing Music...", layout.titleBar);
    } else if (!isPlaying && (currentSong == NULL)) {
        DrawStatusMessage("No song is playing", layout.titleBar);
    }
}

void DrawVisualizersButton(void) {
    static bool visualizerButtonPressed = false;
    Rectangle visualizerButtonBounds = {
        .x = 20,
        .y = layout.titleBar.y + (layout.titleBar.height - 50) / 2,
        .width = 120,
        .height = 50
    };

    if (DrawButton(visualizerButtonBounds, "Visualizers", 20, ACCENT_RED, LIGHT_TEXT)) {
        if (!visualizerButtonPressed) {
            visualizerButtonPressed = true;
            showVisualizerList = !showVisualizerList;
        }
    } else {
        visualizerButtonPressed = false;
    }
}


void DrawTitleBar(Rectangle titleBar) {
    DrawRectangleRec(titleBar, ACCENT_BLUE);
    DrawTextCentered("Bragi Beats", titleBar, 30, LIGHT_TEXT);

    const int buttonWidth = 120;
    const int buttonHeight = 40;
    const int buttonPadding  = 10;

    Rectangle queueButtonBounds = {
        .x = titleBar.x + titleBar.width - buttonWidth - 20,
        .y = titleBar.y + (titleBar.height - buttonHeight) / 2,
        .width = buttonWidth,
        .height = buttonHeight
    };

    Rectangle testModeButtonBounds = {
        .x = queueButtonBounds.x - buttonWidth - buttonPadding,
        .y = queueButtonBounds.y,
        .width = buttonWidth,
        .height = buttonHeight
    };

    // show hide queue button
    if (DrawButton(queueButtonBounds, showQueue ? "HideQueue" : "Show Queue", 20, ACCENT_RED, LIGHT_TEXT)) {
        showQueue = !showQueue;

        layout = CalculateLayout(screenWidth, screenHeight);
    }

    // test mode button
    if (DrawButton(testModeButtonBounds, testMode? "Normal" : "Testing", 20, ACCENT_RED, LIGHT_TEXT)) {
        testMode = !testMode;
    }
}

void DrawSongQueue(Rectangle queueBounds) {
    int fontSize = 12;
    int startY = queueBounds.y + 10;
    int padding = 5;
    int textHeight = 20;
    SongNode* node = head;

    DrawRectangleRec(queueBounds, DARK_BACKGROUND);

    while (node != NULL && startY < queueBounds.y + queueBounds.height) {
        Color textColor = LIGHT_TEXT;
        Rectangle textBackground = {queueBounds.x + 5, startY, queueBounds.width - 10, textHeight};

        if (node == currentSong) {
            textColor = ACCENT_RED;
            DrawRectangleRec(textBackground, ACCENT_BLUE);
        }

        if (CheckCollisionPointRec(GetMousePosition(), textBackground) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            playSongNode(node);
        }

        DrawText(node->title, queueBounds.x + 10, startY + (textHeight - fontSize) / 2, fontSize, textColor);

        startY += textHeight + padding;
        node = node->next;
    }
}

void DrawPlaybackControls(Rectangle playbackControlPanel) {
    const int buttonHeight = 30;
    const int buttonWidth = 80;
    const int spacing = 30;

    float middleX = playbackControlPanel.x + (playbackControlPanel.width / 2);
    float playPauseX = middleX - ((float)buttonWidth / 2);
    float playPauseY = playbackControlPanel.y + (playbackControlPanel.height / 2) - ((float)buttonHeight / 2);

    Rectangle playPauseBounds = {playPauseX, playPauseY, buttonWidth, buttonHeight};
    Rectangle skipBackBounds = {playPauseX - buttonWidth - spacing, playPauseY, buttonWidth, buttonHeight};
    Rectangle skipForwardBounds = {playPauseX + buttonWidth + spacing, playPauseY, buttonWidth, buttonHeight};

    if (DrawButton(skipBackBounds, "<<", 20, ACCENT_RED, LIGHT_TEXT)) {
        SkipBackward();
    }

    if (DrawButton(playPauseBounds, isPlaying ? "Pause" : "Play", 20, ACCENT_RED, LIGHT_TEXT)) {
        PlayPause();
    }

    if (DrawButton(skipForwardBounds, ">>", 20, ACCENT_RED, LIGHT_TEXT)) {
        SkipForward();
    }
}

void DrawProgressBar(Music music, Rectangle progressBarBounds) {
    if (music.stream.buffer == NULL) return;

    const float minProgressBarWidth = 5.0f;

    float songLength = GetMusicTimeLength(music);
    float currentTime = GetMusicTimePlayed(music);
    float progress = currentTime / songLength;

    float progressBarActualWidth = progressBarBounds.width * progress;
    progressBarActualWidth = (progressBarActualWidth < minProgressBarWidth) ? minProgressBarWidth : progressBarActualWidth;

    Rectangle progressBarRectangle = {
        progressBarBounds.x,
        progressBarBounds.y,
        progressBarActualWidth,
        progressBarBounds.height
    };

    DrawRectangleRec(progressBarBounds, ACCENT_BLUE);
    DrawRectangleRec(progressBarRectangle, ACCENT_RED);
    DrawRectangleLinesEx(progressBarBounds, 1, LIGHT_TEXT);

    // Center the time text above the progress bar
    DrawTotalTime(music, progressBarBounds);

    if (CheckCollisionPointRec(GetMousePosition(), progressBarBounds)) {
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            float newProgress = (GetMouseX() - progressBarBounds.x) / (float)progressBarBounds.width;
            SeekMusicStream(music, newProgress * songLength);
        }
    }
}

void DrawTotalTime(Music music,Rectangle progressBarBounds) {
    float songLength = GetMusicTimeLength(music);
    float timePlayed = GetMusicTimePlayed(music);

    int minutesTotal = (int)songLength / 60;
    int secondsTotal = (int)songLength % 60;
    int minutesCurrent = (int)timePlayed / 60;
    int secondsCurrent = (int)timePlayed % 60;

    char timeInfo[256];
    snprintf(timeInfo, sizeof(timeInfo), "%02d:%02d / %02d:%02d", minutesCurrent, secondsCurrent, minutesTotal, secondsTotal);

    int fontSize = 18;
    int textWidth = MeasureText(timeInfo, fontSize);
    int textX = progressBarBounds.x + (progressBarBounds.width - textWidth) / 2;
    int textY = progressBarBounds.y - fontSize - 5;

    DrawText(timeInfo, textX, textY, fontSize, LIGHT_TEXT);
}

void DrawSampleInfo(Layout layout) {
    if (currentSong != NULL && currentSong->song.stream.buffer != NULL) {
        int sampleSize = currentSong->song.stream.sampleSize;
        int sampleRate = currentSong->song.stream.sampleRate;

        char infoText[256];
        snprintf(infoText, sizeof(infoText), "%d Hz\n%d bit", sampleRate, sampleSize);

        int fontSize = 20;
        int textWidth = MeasureText(infoText, fontSize);
        int textX = layout.titleBar.x + layout.titleBar.width - textWidth - 10;
        int textY = layout.titleBar.y + layout.titleBar.height + 5;

        DrawText(infoText, textX, textY, fontSize, LIGHT_TEXT);
    }
}

void DrawVisualizerSelection(bool* showList, Rectangle buttonBounds) {
    const char* visualizerNames[] = {"Bar Chart", "Iridescent", "3D Time Tunnel"};
    int visualizerCount = sizeof(visualizerNames) / sizeof(visualizerNames[0]);
    int paddingBetweenButtonAndList = 10;
    int buttonHeight = 30;

    if (*showList) {
        int listStartY = buttonBounds.y + buttonBounds.height + paddingBetweenButtonAndList;

        for (int i = 0; i < visualizerCount; i++) {
            Rectangle itemBounds = {buttonBounds.x, listStartY + i * (buttonHeight + paddingBetweenButtonAndList), buttonBounds.width, buttonHeight};

            if (DrawButton(itemBounds, visualizerNames[i], 20, ACCENT_RED, LIGHT_TEXT)) {
                currentVisualizer = (VisualizerType)i;
                *showList = false;
            }
        }
    }
}

void DrawTestSignalSelection(bool* showList, Rectangle buttonBounds) {
    const char* testSignalNames[] = {"Sine Wave", "Multi-Sine", "Chirp", "White Noise"};
    int testSignalCount = sizeof(testSignalNames) / sizeof(testSignalNames[0]);
    int paddingBetweenButtonAndList = 10;
    int buttonHeight = 30;

    if (*showList) {
        int listStartY = buttonBounds.y + buttonBounds.height + paddingBetweenButtonAndList;

        for (int i = 0; i < testSignalCount; i++) {
            Rectangle itemBounds = {buttonBounds.x, listStartY + i * (buttonHeight + paddingBetweenButtonAndList), buttonBounds.width, buttonHeight};

            if (DrawButton(itemBounds, testSignalNames[i], 20, ACCENT_RED, LIGHT_TEXT)) {
                currentTestSignal = (TestSignalType)i;
                *showList = false;
            }
        }
    }
}

void DrawTestSignalSelectionButton(void) {
    static bool testSignalButtonPressed = false;
    static bool showTestSignalList = false;

    Rectangle testSignalButtonBounds = {
        .x = 150, // Position it appropriately
        .y = layout.titleBar.y + (layout.titleBar.height - 50) / 2,
        .width = 120,
        .height = 50
    };

    if (DrawButton(testSignalButtonBounds, "Test Signals", 20, ACCENT_RED, LIGHT_TEXT)) {
        if (!testSignalButtonPressed) {
            testSignalButtonPressed = true;
            showTestSignalList = !showTestSignalList;
        }
    } else {
        testSignalButtonPressed = false;
    }

    if (showTestSignalList) {
        DrawTestSignalSelection(&showTestSignalList, testSignalButtonBounds);
    }
}

void DrawStatusMessage(const char* text, Rectangle titleBar) {
    int fontSize = 20;
    int textWidth = MeasureText(text, fontSize);
    int textX = titleBar.x + (titleBar.width - textWidth) / 2;
    int textY = titleBar.y + titleBar.height + 5;
    DrawText(text, textX, textY, fontSize, LIGHT_TEXT);
}

void RenderVisualizer(float out_smooth[], size_t numBins, Rectangle visualizerSpace) {
    DrawRectangleRec(visualizerSpace, DARK_BACKGROUND);

    switch (currentVisualizer) {
        case VISUALIZER_BAR_CHART:
            DrawBarChart(out_smooth, numBins, visualizerSpace);
            break;
        case VISUALIZER_IRIDESCENT:
            DrawIridescentVisualizer(out_smooth, numBins, visualizerSpace);
            break;
        case VISUALIZER_3D_TIME_TUNNEL:
            Draw3DTimeTunnelVisualizer(out_smooth, numBins, visualizerSpace);
            break;
        default:
            DrawBarChart(out_smooth, numBins, visualizerSpace);
            break;
    }
}
