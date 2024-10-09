// main.c

#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#include "../include/playback.h"
#include "../include/fft.h"
#include "../include/ui.h"

#define MAX_SONGS 100
#define ARRAY_LEN(xs) (sizeof(xs) / sizeof((xs)[0]))

typedef struct Song {
    char name[256];
    char filePath[1024];
    struct Song* next;
} Song;

typedef struct Album {
    char name[256];
    Song* songs;
    struct Album* next;
    int songCount;
    bool expanded;
} Album;

typedef struct {
    Album* albums;
    int albumCount;
} Library;

AudioData audioData;

int screenWidth = 1200;
int screenHeight = 900;

// Define the new colors for dark mode
Color DARK_BACKGROUND = {242, 242, 247, 255};
Color ACCENT_BLUE = {0, 122, 255, 255};
Color LIGHT_TEXT = {0, 0, 0, 255};
Color ACCENT_RED = {142, 142, 147, 255};
Color DARKER_RED = {88, 86, 214, 255};


Library userLibrary = {NULL, 0};
SongNode* currentSong = NULL;
SongNode* head = NULL;
SongNode* tail = NULL;
SongQueue songQueue = {.front = -1, .rear = -1};
bool isPlaying = false;
VisualizerType currentVisualizer = VISUALIZER_BAR_CHART;

// Function declarations
void LoadMediaLibrary();
void processAlbumDirectory(const char* baseDir, const char* albumName);
bool IsDirectory(const char* path);
bool IsFileExtension(const char* filename, const char* ext);
void AddAlbumToLibrary(const char* albumName);
void AddSongToAlbum(const char* albumName, const char* songName, const char* filePath);
SongNode* createSongNode(Music song, const char* title, const char* fullPath);
void enqueueSong(Music song, const char* title, const char* fullPath);
void playSongNode(SongNode* node);
bool enqueueTitle(const char* title);
void PlayPause();
void SkipForward();
void SkipBackward();
void PlaySong(Song* song);

int main(void) {
    InitWindow(screenWidth, screenHeight, "Bragi Beats");
    SetTargetFPS(60);

    InitAudioDevice();

    init_audio_data(&audioData); // Initialize AudioData
    set_audio_data(&audioData);  // Set AudioData for the callback

    InitUI();

    // Load media library
    LoadMediaLibrary();

    while (!WindowShouldClose()) {
        // Check for dropped files
        if (IsFileDropped()) {
            FilePathList droppedFiles = LoadDroppedFiles();  // Get the list of dropped files
            for (unsigned int i = 0; i < droppedFiles.count; i++) {
                // Check if the file is a valid audio file
                if (IsFileExtension(droppedFiles.paths[i], ".wav") || IsFileExtension(droppedFiles.paths[i], ".mp3")) {
                    // Load the dropped file as a new song and add to the queue
                    Music song = LoadMusicStream(droppedFiles.paths[i]);
                    if (song.stream.buffer != NULL) {
                        enqueueSong(song, GetFileName(droppedFiles.paths[i]), droppedFiles.paths[i]);
                    } else {
                        printf("Failed to load dropped file: %s\n", droppedFiles.paths[i]);
                    }
                } else {
                    printf("Dropped file is not a supported audio file: %s\n", droppedFiles.paths[i]);
                }
            }
            UnloadDroppedFiles(droppedFiles); // Free the dropped files buffer
        }

        // Handle input and update playback state
        HandleInput();
        UpdatePlaybackState();

        // Process audio data
        size_t numberOfFftBins = ProcessFFT(&audioData);

        // Render UI
        RenderUI(numberOfFftBins, &audioData);
    }

    // Clean up
    CloseAudioDevice();
    CloseWindow();

    return 0;
}

void LoadMediaLibrary() {
    const char* mediaPath = "./media";
    processAlbumDirectory(mediaPath, NULL);
}

void processAlbumDirectory(const char* baseDir, const char* albumName) {
    DIR* dir;
    struct dirent* entry;
    char pathBuffer[1024];

    if ((dir = opendir(baseDir)) == NULL) {
        perror("Failed to open media directory");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        // Ignore "." and ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        sprintf(pathBuffer, "%s/%s", baseDir, entry->d_name);

        if (IsDirectory(pathBuffer)) {
            // Recurse into subdirectories
            processAlbumDirectory(pathBuffer, entry->d_name);
        } else if (IsFileExtension(entry->d_name, ".wav") || IsFileExtension(entry->d_name, ".mp3")) {
            // Add song to album
            const char* album = albumName ? albumName : "Miscellaneous";
            AddSongToAlbum(album, entry->d_name, pathBuffer);
        }
    }

    closedir(dir);
}

bool IsDirectory(const char* path) {
    struct stat statbuf;
    if (stat(path, &statbuf) != 0) {
        return false;
    }
    return S_ISDIR(statbuf.st_mode);
}

bool IsFileExtension(const char* filename, const char* ext) {
    const char* dot = strrchr(filename, '.');
    if (!dot || dot == filename) return false;
    return strcmp(dot, ext) == 0;
}

void AddAlbumToLibrary(const char* albumName) {
    if (!albumName) {
        fprintf(stderr, "Invalid album name provided.\n");
        return;
    }

    Album* newAlbum = (Album*)malloc(sizeof(Album));
    if (!newAlbum) {
        fprintf(stderr, "Failed to allocate memory for new album.\n");
        return;
    }
    strncpy(newAlbum->name, albumName, sizeof(newAlbum->name) - 1);
    newAlbum->name[sizeof(newAlbum->name) - 1] = '\0';
    newAlbum->songs = NULL;
    newAlbum->next = userLibrary.albums;
    newAlbum->songCount = 0;
    newAlbum->expanded = false;
    userLibrary.albums = newAlbum;
    userLibrary.albumCount++;
}

void AddSongToAlbum(const char* albumName, const char* songName, const char* filePath) {
    if (!albumName || !songName || !filePath) {
        fprintf(stderr, "Invalid album or song name provided.\n");
        return;
    }

    Album* currentAlbum = userLibrary.albums;

    // Search for existing album
    while (currentAlbum != NULL && strcmp(currentAlbum->name, albumName) != 0) {
        currentAlbum = currentAlbum->next;
    }

    // If no existing album found, create a new one
    if (currentAlbum == NULL) {
        AddAlbumToLibrary(albumName);
        currentAlbum = userLibrary.albums; // since we add to the front
    }

    // Create new song
    Song* newSong = (Song*)malloc(sizeof(Song));
    if (!newSong) {
        fprintf(stderr, "Failed to allocate memory for new song.\n");
        return;
    }
    strncpy(newSong->name, songName, sizeof(newSong->name) -1);
    newSong->name[sizeof(newSong->name) - 1] = '\0';
    strncpy(newSong->filePath, filePath, sizeof(newSong->filePath) - 1);
    newSong->filePath[sizeof(newSong->filePath) - 1] = '\0';
    newSong->next = currentAlbum->songs;
    currentAlbum->songs = newSong;
    currentAlbum->songCount++;
}

SongNode* createSongNode(Music song, const char* title, const char* fullPath) {
    SongNode* newNode = (SongNode*)malloc(sizeof(SongNode));
    if (newNode == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }

    newNode->song = song;
    strncpy(newNode->title, title, sizeof(newNode->title) -1);
    newNode->title[sizeof(newNode->title) -1] = '\0';
    strncpy(newNode->fullPath, fullPath, sizeof(newNode->fullPath) -1);
    newNode->fullPath[sizeof(newNode->fullPath) -1] = '\0';
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
    strncpy(songQueue.titles[songQueue.rear], title, sizeof(songQueue.titles[songQueue.rear]) -1);
    songQueue.titles[songQueue.rear][sizeof(songQueue.titles[songQueue.rear]) -1] = '\0';
    return true;
}

void PlayPause() {
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
    if (currentSong && currentSong->next) {
        UnloadMusicStream(currentSong->song);
        currentSong = currentSong->next;
        currentSong->song = LoadMusicStream(currentSong->fullPath);

        if (currentSong->song.stream.buffer != NULL) {
            PlayMusicStream(currentSong->song);
            SetMusicVolume(currentSong->song, 0.5f);
            AttachAudioStreamProcessor(currentSong->song.stream, callback);
        } else {
            printf("Error: Failed to load song stream for: %s\n", currentSong->title);
        }
    } else {
        printf("Cannot skip forward: No next song.\n");
    }
}

void SkipBackward() {
    if (currentSong && currentSong->prev) {
        UnloadMusicStream(currentSong->song);

        currentSong = currentSong->prev;
        currentSong->song = LoadMusicStream(currentSong->fullPath);
        if (currentSong->song.stream.buffer != NULL) {
            PlayMusicStream(currentSong->song);
            SetMusicVolume(currentSong->song, 0.5f);
            AttachAudioStreamProcessor(currentSong->song.stream, callback);
        } else {
            printf("Error: Failed to load the previous song stream.\n");
        }
    } else {
        printf("No previous song to play.\n");
        isPlaying = false;
    }
}

void PlaySong(Song* song) {
    if (song == NULL) {
        fprintf(stderr, "Invalid song.\n");
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

        strncpy(currentSong->title, song->name, sizeof(currentSong->title) -1);
        currentSong->title[sizeof(currentSong->title) -1] = '\0';
        strncpy(currentSong->fullPath, song->filePath, sizeof(currentSong->fullPath) -1);
        currentSong->fullPath[sizeof(currentSong->fullPath) -1] = '\0';
        currentSong->song = newSong;

        PlayMusicStream(newSong);
        SetMusicVolume(newSong, 0.5f);
        isPlaying = true;
        AttachAudioStreamProcessor(newSong.stream, callback);
    } else {
        fprintf(stderr, "Failed to load song: %s\n", song->filePath);
    }
}

void StopCurrentSong(void) {
    if (currentSong != NULL) {
        StopMusicStream(currentSong->song);
    }
    isPlaying = false;
}

void PlayCurrentSong(void) {
    if (currentSong != NULL) {
        PlayMusicStream(currentSong->song);
        isPlaying = true;
    }
}
