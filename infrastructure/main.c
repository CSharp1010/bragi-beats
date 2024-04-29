#include <stdio.h>
#include <raylib.h>
#include <complex.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#endif

#include "globals.h"
#include "../business/audioProcessing.h"
#include "../presentation/uiRendering.h"
#include "../persistence/libraryInitialization.h"

int screenWidth = 1200;
int screenHeight = 900;

Color OFFWHITE = {236, 235, 243, 255};
Color CUSTOMDARKGRAY = {46, 53, 50, 255};

/*
float in_raw[FFT_SIZE];
float in_win[FFT_SIZE];
float complex out_raw[FFT_SIZE];
float  out_log[FFT_SIZE];
float out_smooth[FFT_SIZE];
*/

SongNode* head = NULL;
SongNode* tail = NULL;
SongNode* currentSong = NULL;

SongQueue songQueue = {.front = -1, .rear = -1};

char songTitles[MAX_SONGS][256];
int songCount = 0;

Music currentMusic;
Music previousMusic;
int currentSongIndex = -1;
bool isPlaying = false;
bool showList = false;
bool visualizerListOpen = false;

VisualizerType currentVisualizer = VISUALIZER_BAR_CHART;

bool authorizedUser = false;
bool showLibrary = false;
bool loginActive = false;
char username[256] = {0};
int letterCount = 0;

Library userLibrary = {NULL, 0};

void enqueueSong(Music song, const char* title, const char* fullPath);
void PlayNextSongInQueue();
void processDroppedFiles();
void playSong(SongNode *song);
void PlayPause();
void SkipBackward();
void SkipForward();
bool processAlbumDirectory(const char *albumPath, const char *albumName);
bool IsDirectory(const char *path);
void PlaySong(Song *song);

int main(void) {

    InitWindow(screenWidth, screenHeight, "Bragi Beats");
    SetTargetFPS(60);

    Layout layout = CalculateLayout(screenWidth, screenHeight);

    InitAudioDevice();

    while(!WindowShouldClose()) {
        processDroppedFiles();

        size_t numberFftBins = ProcessFFT(in_raw, out_log, out_smooth);
        DrawUI(layout);
        RenderVisualizer(out_smooth, numberFftBins, layout.center.centerX, layout.center.centerY, layout.visualizerSpace);
    }

    CloseAudioDevice();
    CloseWindow();

    return 0;
}

void enqueueSong(Music song, const char* title, const char* fullPath) {
    SongNode* newNode = (SongNode*)malloc(sizeof(SongNode));
    newNode->song = song;
    strncpy(newNode->title, title, sizeof(newNode->title) - 1);
    newNode->title[sizeof(newNode->title) - 1] = '\0';
    strcpy(newNode->fullPath, fullPath);
    newNode->next = NULL;
    newNode->prev = tail;
    
    if (tail != NULL) {
        tail->next = newNode;
    }
    
    tail = newNode;

    if (head == NULL) {
        head = newNode;
    }

    if (!isPlaying) {
        currentSong = newNode;
        PlayMusicStream(newNode->song);
        SetMusicVolume(newNode->song, 0.5f);
        AttachAudioStreamProcessor(newNode->song.stream, callback);
        isPlaying = true;
    }
    if (songQueue.rear == MAX_SONGS - 1) {
        printf("Queue Full\n");
    } else {
        if (songQueue.front == -1) {
            songQueue.front = 0;
        }
        songQueue.rear++;
        strncpy(songQueue.titles[songQueue.rear], title, sizeof(songQueue.titles[songQueue.rear]) - 1);
   }
}

void processDroppedFiles() {
    if (IsFileDropped()) {
        FilePathList droppedFiles = LoadDroppedFiles();

        for (unsigned int i = 0; i < droppedFiles.count; i++) {
            if (IsDirectory(droppedFiles.paths[i])) {
                const char *albumName = GetFileName(droppedFiles.paths[i]);
                AddAlbumToLibrary(albumName);
                processAlbumDirectory(droppedFiles.paths[i], albumName);
            } else if (IsFileExtension(droppedFiles.paths[i], ".wav")) {
                Music song = LoadMusicStream(droppedFiles.paths[i]);
                if (song.ctxData != NULL) {
                    const char* title = GetFileName(droppedFiles.paths[i]);
                    const char* fullPath = droppedFiles.paths[i];
                    enqueueSong(song, title, fullPath);
                }
            }
        }
        UnloadDroppedFiles(droppedFiles);
    }

    if (isPlaying && currentSong != NULL) {
        UpdateMusicStream(currentSong->song);

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

void PlaySong(Song *song) {
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

bool processAlbumDirectory(const char *baseDir, const char *albumName) {
    bool success = false;
#ifdef _WIN32
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile(strcat(albumPath, "\\*"), &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        perror("Failed to open album directory");
        return;
    }

    do {
        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            continue;
        } else {
            char filePath[1024];
            sprintf(filePath, "%s\\%s", albumPath, findFileData.cFileName);
            if (IsFileExtension(findFileData.cFileName, ".wav")) {
                Music song = LoadMusicStream(filePath);
                if (song.ctxData != NULL) {
                    AddSongToAlbum(albumName, findFileData.cFileName);
                    success = true;
                }
            }
        }
    } while (FindNextFile(hFind, &findFileData) != 0);

    FindClose(hFind);
#else
    DIR *dir;
    struct dirent *entry;
    char pathBuffer[1024];

    if ((dir = opendir(baseDir)) == NULL) {
        perror("Failed to open album directory");
        return false;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;

            sprintf(pathBuffer, "%s/%s", baseDir, entry->d_name);
            if (IsDirectory(pathBuffer)) {
                if (processAlbumDirectory(pathBuffer, entry->d_name)) {
                    success = true;
                }
            }
        } else if (entry->d_type == DT_REG && IsFileExtension(entry->d_name, ".wav")) {
            sprintf(pathBuffer, "%s/%s", baseDir, entry->d_name);
            AddSongToAlbum(albumName ? albumName : "Miscellaneous", entry->d_name, pathBuffer);
            success = true;
        }
    }

    closedir(dir);

#endif

    return success;
}

bool IsDirectory(const char *path) {
    struct stat statbuf;
    #ifdef _WIN32 
        if (_stat(path, &statbuf) != 0) {
        return false;
        }
        return (statbuf.st_mode & _S_IFDIR) != 0;
    #else 
        if (stat(path, &statbuf) != 0) {
            return false;
        }
        return S_ISDIR(statbuf.st_mode);
    #endif
}
