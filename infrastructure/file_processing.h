#ifndef FILE_PROCESSING_H
#define FILE_PROCESSING_H

#include <stdbool.h>

bool processAlbumDirectory(const char* baseDir, const char* albumName);
bool IsDirectory(const char* path);
void processDroppedFiles();
void AddSongToAlbum(const char* albumName, const char* songName, const char* path);
void AddAlbumToLibrary(const char* albumName);

#endif
