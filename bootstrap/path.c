#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "path.h"

int Path_IndexOfLastSeparator(const char* path)
{
	size_t pathLength = 0;
	int index;

	if ( !path )
	{
		return -1;
	}

	pathLength = strlen(path);

	if ( pathLength < 1 )
	{
		return -1;
	}

	for ( index = pathLength - 1; index >= 0; --index )
	{
		if ( path[index] == PATH_SEP_CHAR )
		{
			break;
		}
	}

	return index;
}

int Path_IndexOfExtensionSeparator(const char* path)
{
	size_t pathLength = 0;
	int index;

	if ( !path )
	{
		return -1;
	}

	pathLength = strlen(path);

	if ( pathLength < 1 )
	{
		return -1;
	}

	for ( index = pathLength; index > 0; --index )
	{
		if ( path[index] == '.' )
		{
			if ( index == 0 || (index > 0 && path[index - 1] == PATH_SEP_CHAR) )
			{
				// The file name begins with a dot, so it has no extension.
				break;
			}

			return index;
		}
	}

	return -1;
}

const char* Path_DirName(const char* origPath)
{
	static char internalPath[_MAX_PATH];

	int index;
	int lastSep = -1;

	internalPath[0] = '.';
	internalPath[1] = '\0';

	if ( !origPath || !(*origPath) )
	{
		return internalPath;
	}

	for ( index = 0; index < _MAX_PATH && origPath[index]; ++index )
	{
		if ( origPath[index] == PATH_SEP_CHAR )
		{
			lastSep = index;
		}
	}

	if ( lastSep < 1 )
	{
		return internalPath;
	}

	memcpy(internalPath, origPath, lastSep);
	internalPath[lastSep] = '\0';

	return internalPath;
}

bool Path_SetExt(char* origPath, size_t bufferLength, const char* newExt)
{
	size_t extLength = 0;
	size_t origPathLength = 0;
	int indexOfSep = 0;

	if ( !newExt )
	{
		newExt = "";
	}

	if ( !origPath || bufferLength < 1 )
	{
		return false;
	}

	origPathLength = strlen(origPath);

	if ( origPathLength < 1 || origPathLength >= bufferLength )
	{
		// The path is empty, or some overflow has happened.
		return false;
	}

	for ( indexOfSep = origPathLength - 1; indexOfSep >= 0; --indexOfSep )
	{
		if ( origPath[indexOfSep] == '.' )
		{
			break;
		}
	}

	if ( indexOfSep == 0 )
	{
		// The separator was the first character, so treat
		// as if there was no separator.
		indexOfSep = -1;
	}

	if ( indexOfSep < 0 )
	{
		// There was no dot, so use the entire file name.

		if ( *newExt )
		{
			return
				strcat_s(origPath, bufferLength, ".") == 0 &&
				strcat_s(origPath, bufferLength, newExt) == 0;
		}
		else
		{
			return true;
		}
	}

	extLength = strlen(newExt);

	// Make sure we have enough space to print the entire extension:
	// file name length + 1 (for the dot) + the extension.
	if ( indexOfSep + 1 + extLength > bufferLength )
	{
		return false;
	}

	if ( *newExt )
	{
		return sprintf_s(origPath + indexOfSep, bufferLength - indexOfSep, ".%s", newExt) > 0;
	}
	else
	{
		origPath[indexOfSep] = '\0';
		return true;
	}
}

bool Path_GetFileBaseName(const char* path, char* buffer, size_t length)
{
	int lastSep = -1;
	int dotIndex = -1;

	if ( !path || !buffer || length < 1 )
	{
		return false;
	}

	lastSep = Path_IndexOfLastSeparator(path);
	dotIndex = Path_IndexOfExtensionSeparator(path);

	// lastSep = 1
	// dotIndex = 5
	// first valid = 2
	// last valid = 4
	// length of valid = 3

	if ( dotIndex - lastSep - 1 >= length )
	{
		// Too big.
		return false;
	}

	memcpy(buffer, path + lastSep + 1, dotIndex - lastSep - 1);
	buffer[dotIndex - lastSep - 1] = '\0';
	return true;
}
