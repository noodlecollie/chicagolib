#include <string.h>
#include <stdlib.h>
#include "bstfile.h"

static void FreePathList(PathList* paths)
{
	if ( paths->list )
	{
		while ( paths->count > 0 )
		{
			free(paths->list[paths->count - 1]);
			--paths->count;
		}
	}

	free(paths->list);
	paths->count = 0;
	paths->size = 0;
}

static bool ReallocPathsList(PathList* paths, size_t newSize)
{
	void* newMemory = NULL;
	size_t index;

	if ( newSize < paths->count )
	{
		// We're not allowed to trim any entries which have data in them.
		return false;
	}

	newMemory = realloc(paths->list, newSize * sizeof(*paths->list));

	if ( !newMemory )
	{
		return false;
	}

	paths->list = (char**)newMemory;
	paths->size = newSize;

	for ( index = paths->count; index < paths->size; ++index )
	{
		paths->list[index] = NULL;
	}

	return true;
}

static inline bool HasFreeEntries(PathList* paths)
{
	return paths->count < paths->size;
}

bool BootstrapFile_Init(BootstrapFile* file)
{
	if ( !file )
	{
		return false;
	}

	memset(file, 0, sizeof(*file));

	file->compileOptions = (char*)malloc(1);

	if ( !file->compileOptions )
	{
		return false;
	}

	file->compileOptions[0] = '\0';
	file->compileOptionsLength = 1;

	return true;
}

void BootstrapFile_Destroy(BootstrapFile* file)
{
	if ( !file )
	{
		return;
	}

	FreePathList(&file->sourceFiles);

	if ( file->compileOptions )
	{
		free(file->compileOptions);
		file->compileOptions = NULL;
		file->compileOptionsLength = 0;
	}
}

bool BootstrapFile_AddSourceFile(BootstrapFile* file, const char* filePath)
{
	if ( !file || !filePath || !(*filePath) )
	{
		return false;
	}

	if ( !HasFreeEntries(&file->sourceFiles) )
	{
		size_t newSize = file->sourceFiles.size > 0 ? file->sourceFiles.size * 2 : 1;

		if ( !ReallocPathsList(&file->sourceFiles, newSize) )
		{
			// Unable to alloc memory.
			return false;
		}
	}

	file->sourceFiles.list[file->sourceFiles.count++] = strdup(filePath);
	return true;
}

void BootstrapFile_SetTargetName(BootstrapFile* file, const char* targetName)
{
	if ( !file )
	{
		return;
	}

	if ( targetName )
	{
		strcpy_s(file->targetName, sizeof(file->targetName), targetName);
		return;
	}

	if ( !file->fileName[0] )
	{
		file->targetName[0] = '\0';
		return;
	}

	Path_GetFileBaseName(file->fileName, file->targetName, sizeof(file->targetName));
}

bool BootstrapFile_AppendCompileOptions(BootstrapFile* file, const char* options, size_t optionsStrLen)
{
	char* newOptions = NULL;
	size_t newLength = 0;

	if ( !file || !options )
	{
		return false;
	}

	if ( !(*options) )
	{
		return true;
	}

	if ( optionsStrLen < 1 )
	{
		optionsStrLen = strlen(options);
	}
	newLength = file->compileOptionsLength + optionsStrLen;

	if ( file->compileOptionsLength > 1 )
	{
		// There is already an existing string, so we need to add a space.
		++newLength;
	}

	newOptions = (char*)realloc(file->compileOptions, newLength);

	if ( !newOptions )
	{
		// Not enough memory.
		return false;
	}

	if ( file->compileOptionsLength > 1 )
	{
		// There is already an existing string, so we need to add a space.
		newOptions[file->compileOptionsLength - 1] = ' ';
		memcpy(newOptions + file->compileOptionsLength, options, newLength - file->compileOptionsLength);
	}
	else
	{
		memcpy(newOptions, options, newLength);
	}

	file->compileOptions = newOptions;
	file->compileOptionsLength = newLength;
	return true;
}
