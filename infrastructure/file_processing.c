#include "file_processing.h"
#include "playback.h"
#include "../audio_processing/audio_processing.h"
#include "../persistence/library_init.h"
#include <raylib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

bool processAlbumDirectory(const char* baseDir, const char* albumName) {
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
    DIR* dir;
    struct dirent* entry;
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

bool IsDirectory(const char* path) {
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

void processDroppedFiles() {
    if (IsFileDropped()) {
        FilePathList droppedFiles = LoadDroppedFiles();

        for (unsigned int i = 0; i < droppedFiles.count; i++) {
            if (IsDirectory(droppedFiles.paths[i])) {
                const char* albumName = GetFileName(droppedFiles.paths[i]);
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

/*
void AddSongToAlbum(const char* albumName, const char* songName, const char* path) {
    // Find or create the album
    Album* album = userLibrary.albums;
    while (album != NULL) {
        if (strcmp(album->name, albumName) == 0) {
            break;
        }
        album = album->next;
    }

    if (album == NULL) {
        album = (Album*)malloc(sizeof(Album));
        if (album == NULL) {
            fprintf(stderr, "Failed to allocate memory for album\n");
            return;
        }
        strncpy(album->name, albumName, sizeof(album->name) - 1);
        album->name[sizeof(album->name) - 1] = '\0';
        album->songs = NULL;
        album->next = userLibrary.albums;
        userLibrary.albums = album;
        userLibrary.albumCount++;
    }

    // Add the song to the album
    Song* newSong = (Song*)malloc(sizeof(Song));
    if (newSong == NULL) {
        fprintf(stderr, "Failed to allocate memory for song\n");
        return;
    }
    strncpy(newSong->name, songName, sizeof(newSong->name) - 1);
    newSong->name[sizeof(newSong->name) - 1] = '\0';
    strncpy(newSong->filePath, path, sizeof(newSong->filePath) - 1);
    newSong->filePath[sizeof(newSong->filePath) - 1] = '\0';
    newSong->next = album->songs;
    album->songs = newSong;
    album->songCount++;
}

void AddAlbumToLibrary(const char* albumName) {
    Album* album = (Album*)malloc(sizeof(Album));
    if (album == NULL) {
        fprintf(stderr, "Failed to allocate memory for album\n");
        return;
    }
    strncpy(album->name, albumName, sizeof(album->name) - 1);
    album->name[sizeof(album->name) - 1] = '\0';
    album->songs = NULL;
    album->next = userLibrary.albums;
    userLibrary.albums = album;
    userLibrary.albumCount++;
}
*/
