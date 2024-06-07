#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "uiRendering.h"
#include "visualizers.h"
#include "../infrastructure/config.h"
#include "../infrastructure/playback.h"
#include "../infrastructure/file_processing.h"

Layout CalculateLayout(int screenWidth, int screenHeight) {
    Layout layout;
    layout.titleBar = (Rectangle){0, 0, screenWidth, screenHeight * 0.0875};
    layout.queue = (Rectangle){0, layout.titleBar.height, screenWidth * 0.20, screenHeight - layout.titleBar.height - (screenHeight * 0.15)};
    layout.library = layout.queue;
    layout.visualizerSpace = (Rectangle){layout.queue.width, layout.titleBar.height, screenWidth - layout.queue.width, screenHeight - layout.titleBar.height - (screenHeight * 0.15)};
    layout.playbackControlPanel = (Rectangle){0, screenHeight * 0.85, screenWidth, screenHeight * 0.15};
    layout.center.centerX = layout.visualizerSpace.x + layout.visualizerSpace.width / 2;
    layout.center.centerY = layout.visualizerSpace.y + layout.visualizerSpace.height / 2;

    return layout;
}

bool DrawButton(Rectangle bounds, const char* text, int fontSize) {
    Vector2 mousePoint = GetMousePosition();
    bool isHovering = CheckCollisionPointRec(mousePoint, bounds);
    bool isPressed = false;

    if (isHovering) {
        DrawRectangleRec(bounds, IsMouseButtonDown(MOUSE_BUTTON_LEFT) ? CUSTOMDARKGRAY : LIGHTGRAY);
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            isPressed = true;
        }
    } else {
        DrawRectangleRec(bounds, GRAY);
    }

    int textWidth = MeasureText(text, fontSize);
    DrawText(text, bounds.x + (bounds.width / 2) - ((float)textWidth / 2), bounds.y + (bounds.height / 2) - ((float)fontSize / 2), fontSize, BLACK);

    return isPressed;
}

void DrawTextCentered(const char* text, Rectangle bounds, int fontSize, Color color) {
    int textWidth = MeasureText(text, fontSize);
    DrawText(text, bounds.x + (bounds.width - textWidth) / 2, bounds.y + (bounds.height - fontSize) / 2, fontSize, color);
}

void DrawLibraryOrQueue(Layout layout) {
    if (authorizedUser && showLibrary) {
        DrawLibrary(layout.library);
    } else {
        DrawSongQueue(layout.queue);
    }
}

void DrawLibrary(Rectangle libraryBounds) {
    if (visualizerListOpen) {
        return;
    }

    int offsetY = 0;
    int itemHeight = 30;
    int indentSize = 30;
    int maxViewableItems = libraryBounds.height / itemHeight;
    int currentItem = 0;

    Album *currentAlbum = userLibrary.albums;

    while (currentAlbum != NULL && currentItem < maxViewableItems) {

        Rectangle albumBounds = {libraryBounds.x, libraryBounds.y + offsetY, libraryBounds.width, itemHeight};
        bool albumClicked = DrawButton(albumBounds, currentAlbum->name, 12);
        offsetY += itemHeight;
        currentItem++;

        if (currentAlbum->expanded) {
            Song *currentSong = currentAlbum->songs;
            while (currentSong != NULL && currentItem < maxViewableItems) {
                Rectangle songBounds = {libraryBounds.x + indentSize, libraryBounds.y + offsetY, libraryBounds.width - indentSize, itemHeight};

                if (CheckCollisionPointRec(GetMousePosition(), songBounds) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    PlaySong(currentSong);
                } else {
                    DrawText(currentSong->name, songBounds.x + 5, songBounds.y + 5, 12, BLACK);
                }

                offsetY += itemHeight;
                currentItem++;
                currentSong = currentSong->next;
            }
        }
        if (albumClicked) {
            currentAlbum->expanded = !currentAlbum->expanded;
        }

        currentAlbum = currentAlbum->next;

        if (offsetY > libraryBounds.height) {
            // Scroll feature
        }
    }
}

void DrawSongQueue(Rectangle queue) {
    int sidebarWidth = queue.width;
    int sidebarHeight = queue.height;
    int sidebarX = queue.x;
    int sidebarY = queue.y;

    DrawRectangle(sidebarX, sidebarY, sidebarWidth, sidebarHeight, OFFWHITE);

    int fontSize = 12;
    int startY = sidebarY + 10;
    int padding = 5;
    int textHeight = 20;
    SongNode* node = head;
    
    while (node != NULL && startY < sidebarY + sidebarHeight) {
        Color textColor = BLACK;
        Rectangle textBackground = { sidebarX + 5, startY, sidebarWidth - 10, textHeight };

        if (node == currentSong) {
            textColor = GOLD;
            DrawRectangleRec(textBackground, BLACK);
        }

        int textX = sidebarX + 10;
        int textY = startY + (textHeight / 2) - (fontSize / 2);
        DrawText(node->title, textX, textY, fontSize, textColor);

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

    bool hasSongInList = currentSong != NULL;
    if (currentSong != NULL) {

        if (DrawButton(playPauseBounds, isPlaying ? "Pause" : "Play", 20) && hasSongInList) {
            printf("Play/Pause");
            PlayPause();
        }

        if (DrawButton(skipBackBounds, "<<", 20) && hasSongInList) {
            printf("skip backward");
            SkipBackward();
        }

        if (DrawButton(skipForwardBounds, ">>", 20) && hasSongInList) {
            printf("skip forward");
            SkipForward();
        }
    }
}

void DrawProgressBar(Music music, int screenHeight, int screenWidth) {
    if (music.stream.buffer == NULL) return;

    const int progressBarX = 100;
    const int progressBarY = screenHeight - 50;
    const int progressBarWidth = screenWidth - 200;
    const int progressBarHeight = 20;
    const float minProgressBarWidth = 5.0f;

    float songLength = GetMusicTimeLength(music);
    float currentTime = GetMusicTimePlayed(music);
    float progress = currentTime / songLength;

    float progressBarActualWidth = progressBarWidth * progress;
    progressBarActualWidth = (progressBarActualWidth < minProgressBarWidth) ? minProgressBarWidth : progressBarActualWidth;

    Rectangle progressBarRectangle = { progressBarX, progressBarY, progressBarActualWidth, progressBarHeight };
    Rectangle fullProgressBarRectangle = { progressBarX, progressBarY, progressBarWidth, progressBarHeight };

    DrawTotalTime(music, progressBarX + 5, progressBarY);
    DrawRectangleRec(fullProgressBarRectangle, DARKGRAY);
    DrawRectangleRec(progressBarRectangle, LIGHTGRAY);
    DrawRectangleLines(progressBarX, progressBarY, progressBarWidth, progressBarHeight, BLACK);

    if (CheckCollisionPointRec(GetMousePosition(), fullProgressBarRectangle)) {
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            float newProgress = (GetMouseX() - progressBarX) / (float)progressBarWidth;
            SeekMusicStream(music, newProgress * songLength);
            currentTime = GetMusicTimePlayed(music);
            progress = currentTime / songLength;
            progressBarActualWidth = progressBarWidth * progress;
        }
    }
}

void DrawTotalTime(Music music, int x, int y) {
    float songLength = GetMusicTimeLength(music);
    float timePlayed = GetMusicTimePlayed(music);

    int minutesTotal = (int)songLength / 60;
    int secondsTotal = (int)songLength % 60;
    int minutesCurrent = (int)timePlayed / 60;
    int secondsCurrent = (int)timePlayed % 60;

    char timeInfo[256];
    snprintf(timeInfo, sizeof(timeInfo), "%02d:%02d / %02d:%02d", minutesCurrent, secondsCurrent, minutesTotal, secondsTotal);

    DrawText(timeInfo, x, y - 30, 18, BLACK);
}

void DrawSampleInfo(Layout layout) {
    if (currentSong != NULL && currentSong->song.stream.buffer != NULL) {
        int sampleSize = currentSong->song.stream.sampleSize;
        int sampleRate = currentSong->song.stream.sampleRate;

        char infoText[256];
        sprintf(infoText, "%d Hz\n%d bit", sampleRate, sampleSize);

        int fontSize = 20;
        int textWidth = MeasureText(infoText, fontSize);
        int textX = layout.titleBar.x + layout.titleBar.width - textWidth - 10;
        int textY = layout.titleBar.y + (layout.titleBar.height - fontSize) / 2;

        DrawText(infoText, textX, textY, fontSize, BLACK);
    }
}

void DrawUI(Layout layout) {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    layout = CalculateLayout(screenWidth, screenHeight);

    Rectangle visualizerButtonBounds = {
        .x = 20,
        .y = layout.titleBar.y + (layout.titleBar.height - 50) /2,
        .width = 120,
        .height = 50
    };

    Rectangle loginButtonBounds = {
        .x = visualizerButtonBounds.x + visualizerButtonBounds.width + 20,
        .y = visualizerButtonBounds.y,
        .width = 120,
        .height = 50
    };

    Rectangle textBoxBounds = {
        .x = loginButtonBounds.x + loginButtonBounds.width + 20,
        .y = loginButtonBounds.y,
        .width = 120,
        .height = 50
    };

    Color textBoxColor = loginActive ? SKYBLUE : LIGHTGRAY;

    BeginDrawing();
    ClearBackground(OFFWHITE);

    static char username[256] = {0};
    static int letterCount = 0;
    static bool loginButtonPreviouslyClicked = false;
    static bool inputActive = false;

    static bool visualizerButtonPreviouslyPressed = false;

    DrawPlaybackControls(layout.playbackControlPanel);
    if (currentSong != NULL) {
        DrawProgressBar(currentSong->song, screenHeight, screenWidth);
    }

    DrawLibraryOrQueue(layout);

    if (DrawButton(visualizerButtonBounds, "Visualizers", 20) && !visualizerButtonPreviouslyPressed) {
        visualizerButtonPreviouslyPressed = true;
        showList = !showList;
    } else if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        visualizerButtonPreviouslyPressed = false;
    }

    if (DrawButton(loginButtonBounds, "Login", 20) && !loginButtonPreviouslyClicked) {
        loginButtonPreviouslyClicked = true;
        inputActive = true;
    }

    if (inputActive) {
        DrawRectangleRec(textBoxBounds, textBoxColor);
        DrawText(username, textBoxBounds.x + 5, textBoxBounds.y + (textBoxBounds.height / 2) - 10, 20, BLACK);
        
        int key = GetCharPressed();
        while (key > 0) {
            if ((key > 32) && (key <= 125) && (letterCount < 255)) {
                username[letterCount] = (char)key;
                username[letterCount + 1] = '\0';
                letterCount++;
            }
            key = GetCharPressed();
        }

        if (IsKeyPressed(KEY_BACKSPACE) && letterCount > 0) {
            letterCount--;
            username[letterCount] = '\0';
        }
        if (IsKeyPressed(KEY_ENTER)) {
            char path[512] = "./database/music/users/aaron";
            if (processAlbumDirectory(path,"aaron")) {
                authorizedUser = true;
                showLibrary = true;
                printf("Directory processed successfully. Library is drawing.\n");
            } else {
                printf("Failed to process directory");
            }
            inputActive = false;
        }

    }

    DrawVisualizerSelection(&showList, visualizerButtonBounds);
    DrawSampleInfo(layout);

    if (isPlaying) {
        const char* text = "Playing Music...";
        int fontSize = 30;
        int textWidth = MeasureText(text, fontSize);
        int textX = layout.titleBar.x + (layout.titleBar.width - textWidth) / 2;
        int textY = layout.titleBar.y + (layout.titleBar.height - fontSize) / 2;
        DrawText(text, textX, textY, fontSize, CUSTOMDARKGRAY);
    } else if (!isPlaying && (currentSong == NULL)) {
        const char* text = "Drag and Drop a Song to Start Playing";
        int fontSize = 34;
        int textWidth = MeasureText(text, fontSize);
        int textX = 150 + (screenWidth - textWidth) / 2;
        int textY = 20 + (screenHeight / 2) - (fontSize / 2);
        DrawText(text, textX, textY, fontSize, CUSTOMDARKGRAY);
    }

    EndDrawing();
}

void DrawTextBox(Rectangle textBoxBounds, char* text, int maxLength, bool* isActive) {
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (CheckCollisionPointRec(GetMousePosition(), textBoxBounds)) {
            *isActive = true;
        } else {
            *isActive = false;
        }
    }

    if (*isActive) {
        int key = GetCharPressed();
        while (key > 0) {
            if ((key >= 32) && (key <= 125) && ((int)strlen(text) < maxLength)) {
                int length = strlen(text);
                text[length] = (char)key;
                text[length + 1] = '\0';
            }

            key = GetCharPressed();
        }
        if (IsKeyPressed(KEY_BACKSPACE)) {
            int length = strlen(text);
            if (length > 0) {
                text[length - 1] = '\0';
            }
        }
    }

    DrawRectangleRec(textBoxBounds, *isActive ? LIGHTGRAY : DARKGRAY);
    DrawText(text, textBoxBounds.x + 5, textBoxBounds.y + 8, 20, BLACK);
}

void DrawVisualizerSelection(bool *showList, Rectangle buttonBounds) {
    const char* visualizerNames[] = {"Bar Chart", "Circle Star", "Wing", "Kaleidoscope", "Spiral", "Center", "Sine Wave", "Math", "Radial"};
    int visualizerCount = ARRAY_LEN(visualizerNames);
    int paddingBetweenButtonAndList = 10;
    int buttonHeight = 30;

    if (*showList) {
        visualizerListOpen = true;
        int listStartY = buttonBounds.y + buttonBounds.height + paddingBetweenButtonAndList;

        for (int i = 0; i < visualizerCount; i++) {
            Rectangle itemBounds = {buttonBounds.x, listStartY + i * (buttonHeight + paddingBetweenButtonAndList), buttonBounds.width, buttonHeight};

            if (DrawButton(itemBounds, visualizerNames[i], 20)) {
                currentVisualizer = (VisualizerType)i;
                *showList = false;
            }
        }
    } else {
        visualizerListOpen = false;
    }
}

void RenderVisualizer(float out_smooth[], float out_phase[], float out_power[], size_t numberOfFftBins, int centerX, int centerY, Rectangle visualizerSpace) {
    switch (currentVisualizer) {
        case VISUALIZER_BAR_CHART:
            barChartVisual(out_smooth, numberOfFftBins, visualizerSpace);
            break;
        case VISUALIZER_CIRCLE_STAR:
            circleStarVisual(out_smooth, numberOfFftBins, centerX, centerY);
            break;
        case VISUALIZER_WING:
            wingVisual(out_smooth, numberOfFftBins, centerX, centerY);
            break;
        case VISUALIZER_KALEIDOSCOPE:
            kaleidoscopeVisual(out_smooth, numberOfFftBins, centerX, centerY);
            break;
        case VISUALIZER_SPIRAL:
            spiralVisual(out_smooth, numberOfFftBins, centerX, centerY);
            break;
        case VISUALIZER_CENTER_LINE:
            centerLineVisualizer(out_smooth, numberOfFftBins, visualizerSpace);
            break;
        case VISUALIZER_SINE_WAVE:
            sineWaveVisualizer(out_smooth, numberOfFftBins, visualizerSpace);
            break;
        case VISUALIZER_MATH:
             mathVisualizer(out_smooth, out_phase, out_power, numberOfFftBins, visualizerSpace);
            break;
        case VISUALIZER_RADIAL:
            radialVisualizer(out_smooth, numberOfFftBins, visualizerSpace);
            break;
        default:
            break;
    }
}
