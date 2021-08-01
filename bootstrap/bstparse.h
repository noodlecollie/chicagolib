#ifndef BOOTSTRAP_BSTPARSE_H
#define BOOTSTRAP_BSTPARSE_H

#include <stddef.h>
#include <stdbool.h>
#include "bstfile.h"

void BootstrapParse_SetProjectFilePath(const char* path);
size_t BootstrapParse_ReadLine(FILE* inFile, char* buffer, size_t size);
bool BootstrapParse_ParseLine(BootstrapFile* file, size_t lineNumber, char* line, size_t lineLength);

#endif // BOOTSTRAP_BSTPARSE_H
