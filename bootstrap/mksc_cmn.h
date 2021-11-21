#ifndef BOOTSTRAP_MKSC_CMN_H
#define BOOTSTRAP_MKSC_CMN_H

#include <stddef.h>
#include <string.h>
#include "path.h"

static inline void Mksc_ExtractFileDirPath(char* buffer, size_t bufferSize, const char* filePath)
{
	int lastSep;

	strcpy_s(buffer, bufferSize, filePath);
	lastSep = Path_IndexOfLastSeparator(filePath);

	if ( lastSep >= 0 )
	{
		buffer[lastSep + 1] = '\0';
	}
	else
	{
		buffer[0] = '\0';
	}
}

static inline void Mksc_ToUppercase(char* buffer)
{
	if ( !buffer )
	{
		return;
	}

	for ( ; *buffer; ++buffer )
	{
		if ( *buffer >= 'a' && *buffer <= 'z' )
		{
			*buffer -= 32;
		}
	}
}

#endif // BOOTSTRAP_MKSC_CMN_H
