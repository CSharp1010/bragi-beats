#include "playback.h"
#include "../audio_processing/audio_processing.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

SongNode* head = NULL;
SongNode* tail = NULL;
SongNode* currentSong = NULL;
SongQueue songQueue = {.front = -1, .rear = -1};
bool isPlaying = false;
bool authorizedUser = false;
bool showLibrary = false;
bool showList = false;
bool loginActive = false;
bool visualizerListOpen = false;
VisualizerType currentVisualizer = VISUALIZER_BAR_CHART;
Library userLibrary = {NULL, 0};

SongNode* createSongNode(Music song, const char* title, const char* fullPath) {
    SongNode* newNode = (SongNode*)malloc(sizeof(SongNode));
    if (newNode == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }

    newNode->song = song;
    strncpy(newNode->title, title, sizeof(newNode->title) - 1);
    newNode->title[sizeof(newNode->title) - 1] = '\0';
    strcpy(newNode->fullPath, fullPath);
    newNode->next = NULL;
    newNode->prev = tail;
    return newNode;
}

void enqueueSong(Music song, const char* title, const char* fullPath) {
    SongNode* newNode = createSongNode(song, title, fullPath);
    if (tail != NULL) {
        tail->next = newNode;
    } else {
        head = newNode;
    }
    tail = newNode;

    if (!isPlaying) {
        playSongNode(newNode);
    }

    if (!enqueueTitle(title)) {
        printf("Queue Full\n");
    }
}

void playSongNode(SongNode* node) {
    currentSong = node;
    PlayMusicStream(node->song);
    SetMusicVolume(node->song, 0.5f);
    AttachAudioStreamProcessor(node->song.stream, callback);
    isPlaying = true;
}

bool enqueueTitle(const char* title) {
    if (songQueue.rear == MAX_SONGS - 1) {
        return false;
    }

    if (songQueue.front == -1) {
        songQueue.front = 0;
    }
    songQueue.rear++;
    strncpy(songQueue.titles[songQueue.rear], title, sizeof(songQueue.titles[songQueue.rear]) - 1);
    return true;
}

void PlayPause() {
    printf("Play/Pause()\n");
    if (currentSong != NULL) {
        isPlaying = !isPlaying;
        if (isPlaying) {
            PlayMusicStream(currentSong->song);
        } else {
            PauseMusicStream(currentSong->song);
        }
    }
}

void SkipForward() {
    printf("Attempting to skip forward from song: %s\n", currentSong ? currentSong->title : "None");
    if (currentSong && currentSong->next) {
        UnloadMusicStream(currentSong->song);
        currentSong = currentSong->next;
        currentSong->song = LoadMusicStream(currentSong->fullPath);

        if (currentSong->song.stream.buffer != NULL) {
            PlayMusicStream(currentSong->song);
            SetMusicVolume(currentSong->song, 0.5f);
            AttachAudioStreamProcessor(currentSong->song.stream, callback);
            printf("Skipped forward to: %s\n", currentSong->title);
        } else {
            printf("Error: Failed to load song stream for: %s\n", currentSong->title);
        }
    } else {
        printf("Cannot skip forward: No next song or stream load failed.\n");
    }
}

void SkipBackward() {
    if (currentSong && currentSong->prev) {
        printf("SkipBackward() - Current song: %s\n", currentSong->title);
        UnloadMusicStream(currentSong->song);

        currentSong = currentSong->prev;
        printf("Loading previous song: %s\n", currentSong->title);
        currentSong->song = LoadMusicStream(currentSong->fullPath);
        if (currentSong->song.stream.buffer != NULL) {
            PlayMusicStream(currentSong->song);
            SetMusicVolume(currentSong->song, 0.5f);
            AttachAudioStreamProcessor(currentSong->song.stream, callback);
        } else {
            printf("Error: Failed to load the previous song stream.\n");
        }
    } else {
        printf("No previous song to play or failed to load song.\n");
        isPlaying = false;
    }
}

void PlaySong(Song* song) {
    if (song == NULL) {
        fprintf(stderr, "Invalid song path.\n");
        return;
    }

    if (isPlaying && currentSong != NULL) {
        StopMusicStream(currentSong->song);
        UnloadMusicStream(currentSong->song);
    }

    Music newSong = LoadMusicStream(song->filePath);
    if (newSong.ctxData != NULL) {
        if (currentSong != NULL) {
            free(currentSong);
        }

        currentSong = (SongNode*)malloc(sizeof(SongNode));
        if (currentSong == NULL) {
            fprintf(stderr, "Failed to allocate memory for the new song node.\n");
            return;
        }

        strncpy(currentSong->title, song->name, sizeof(currentSong->title) - 1);
        currentSong->title[sizeof(currentSong->title) - 1] = '\0';
        strncpy(currentSong->fullPath, song->filePath, sizeof(currentSong->fullPath) - 1);
        currentSong->fullPath[sizeof(currentSong->fullPath) - 1] = '\0';
        currentSong->song = newSong;

        PlayMusicStream(newSong);
        SetMusicVolume(newSong, 0.5f);
        isPlaying = true;
        AttachAudioStreamProcessor(newSong.stream, callback);
    } else {
        fprintf(stderr, "Failed to load song: %s\n", song->filePath);
    }
}
