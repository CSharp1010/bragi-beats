#ifndef LIBRARY_INITIALIZATION
#define LIBRARY_INITIALIZATION

#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>

void AddSongToAlbum(const char* albumName, const char* songName, const char* filePath);
void AddAlbumToLibrary(const char* albumName);

#endif
