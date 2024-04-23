#include "libraryInitialization.h"
#include "../infrastructure/globals.h"

void initializeLibrary(const char* userPath);
void AddSongToAlbum(const char* albumName, const char* songName, const char* filePath);
void LoadMusicLibrary(const char *userPath);

void initializeLibrary(const char* userPath) {
    DIR *dir;
    struct dirent *entry;
    if ((dir = opendir(userPath)) == NULL) {
        perror("Failed to open directory");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry ->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            AddAlbumToLibrary(entry->d_name);
        } else if (entry->d_type == DT_REG) {
        }
    }
    closedir(dir);
}

void AddSongToAlbum(const char* albumName, const char* songName, const char* filePath) {
    if (!albumName || !songName) {
        fprintf(stderr, "Invalid album or song name provided.\n");
        return;
    }

    Album *currentAlbum = userLibrary.albums;

    // Search for an existing album
    while (currentAlbum != NULL && strcmp(currentAlbum->name, albumName) != 0) {
        currentAlbum = currentAlbum->next;
    }

    // If no existing album found, create a new one
    if (currentAlbum == NULL) {
        Album *newAlbum = (Album*)malloc(sizeof(Album));
        if (!newAlbum) {
            fprintf(stderr, "Failed to allocate memory for new album.\n");
            return;
        }
        strcpy(newAlbum->name, albumName);
        newAlbum->songs = NULL;
        newAlbum->next = userLibrary.albums;
        newAlbum->songCount = 0;
        newAlbum->expanded = false;
        userLibrary.albums = newAlbum;
        userLibrary.albumCount++;
        currentAlbum = newAlbum;
    }

    // Create a new song
    Song *newSong = (Song*)malloc(sizeof(Song));
    if (!newSong) {
        fprintf(stderr, "Failed to allocate memory for new song.\n");
        return;
    }
    strcpy(newSong->name, songName);
    newSong->next = currentAlbum->songs;
    currentAlbum->songs = newSong;
    currentAlbum->songCount++;  // Increment the song count for the album
}

void LoadMusicLibrary(const char *userPath) {
    char fullPath[1024];
    sprintf(fullPath, "./business/music/users/%s", userPath);
    processAlbumDirectory(fullPath, NULL);
}
