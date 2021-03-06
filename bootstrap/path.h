#ifndef BOOTSTRAP_PATH_H
#define BOOTSTRAP_PATH_H

#include <stdbool.h>
#include "platform.h"

// For some reason the compiler's dirname() function doesn't work properly,
// so we roll our own.
const char* Path_DirName(const char* origPath);

int Path_IndexOfLastSeparator(const char* path);
int Path_IndexOfExtensionSeparator(const char* path);
bool Path_SetExt(char* origPath, size_t bufferLength, const char* newExt);
bool Path_GetFileBaseName(const char* path, char* buffer, size_t length);

#endif // BOOTSTRAP_PATH_H
