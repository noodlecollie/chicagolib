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

void BootstrapFile_Init(BootstrapFile* file)
{
	if ( !file )
	{
		return;
	}

	memset(file, 0, sizeof(*file));
}

void BootstrapFile_Destroy(BootstrapFile* file)
{
	if ( !file )
	{
		return;
	}

	FreePathList(&file->sourceFiles);
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
