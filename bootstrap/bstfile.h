#ifndef BOOTSTRAP_BSTFILE_H
#define BOOTSTRAP_BSTFILE_H

#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "path.h"
#include "platform.h"

#define BST_MAX_TARGET_NAME 32
#define BST_MAX_LINE_LENGTH 512

typedef enum _TargetType
{
	TT_EXECUTABLE = 0,
	TT_STATIC_LIB
} TargetType;

typedef struct _PathList
{
	char** list;
	size_t count;	// Number of valid items in the list
	size_t size;	// Number of allocated elements in the list.
} PathList;

typedef struct _BootstrapFile
{
	char fullPath[_MAX_PATH];
	const char* fileName;

	char targetName[BST_MAX_TARGET_NAME];
	TargetType targetType;
	TargetPlatform targetPlatform;

	PathList sourceFiles;
	char* compileOptions;
	size_t compileOptionsLength;
} BootstrapFile;

static inline size_t BootstrapFile_SourceFileCount(BootstrapFile* file)
{
	return file ? file->sourceFiles.count : 0;
}

static inline const char* BootstrapFile_SourceFilePath(BootstrapFile* file, size_t index)
{
	return (file && index < file->sourceFiles.count)
		? file->sourceFiles.list[index]
		: NULL;
}

static inline const char* BootstrapFile_GetFilePath(BootstrapFile* file)
{
	return file ? file->fullPath: "";
}

static inline void BootstrapFile_SetFilePath(BootstrapFile* file, const char* path)
{
	size_t length = 0;

	if ( file )
	{
		_fullpath(file->fullPath, path, sizeof(file->fullPath));
	}
	else
	{
		file->fullPath[0] = '\0';
	}

	file->fileName = file->fullPath;
	length = strlen(file->fullPath);

	if ( length < 1 )
	{
		file->fileName = file->fullPath;
		return;
	}

	for ( length = strlen(file->fullPath) - 1; length > 0; --length )
	{
		if ( file->fullPath[length] == PATH_SEP_CHAR )
		{
			file->fileName = &file->fullPath[length] + 1;
			break;
		}
	}
}

static inline const char* BootstrapFile_GetFileName(BootstrapFile* file)
{
	return file ? file->fileName : "";
}

static inline const char* BootstrapFile_GetTargetName(BootstrapFile* file)
{
	return file ? file->targetName : "";
}

static inline const char* BootstrapFile_GetCompileOptions(BootstrapFile* file)
{
	return file ? file->compileOptions : "";
}

static inline TargetType BootstrapFile_GetTargetType(BootstrapFile* file)
{
	return file ? file->targetType : TT_EXECUTABLE;
}

static inline void BootstrapFile_SetTargetType(BootstrapFile* file, TargetType tType)
{
	if ( file )
	{
		file->targetType = tType;
	}
}

static inline TargetPlatform BootstrapFile_GetTargetPlatform(BootstrapFile* file)
{
	return file ? file->targetPlatform : HOST_PLATFORM_ID;
}

static inline void BootstrapFile_SetTargetPlatform(BootstrapFile* file, TargetPlatform platform)
{
	if ( file )
	{
		file->targetPlatform = platform;
	}
}

// Assumes input file is uninitialised.
bool BootstrapFile_Init(BootstrapFile* file);

// Assumes input file is initialised.
void BootstrapFile_Destroy(BootstrapFile* file);

bool BootstrapFile_AddSourceFile(BootstrapFile* file, const char* filePath);
void BootstrapFile_SetTargetName(BootstrapFile* file, const char* targetName);
bool BootstrapFile_AppendCompileOptions(BootstrapFile* file, const char* options, size_t optionsStrLen);

#endif // BOOTSTRAP_BSTFILE_H
