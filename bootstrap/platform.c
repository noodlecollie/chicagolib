#include <stddef.h>
#include <string.h>
#include "platform.h"

#define ARR_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#define LIST_ITEM(value, platStr, headerDir, cc) platStr,
static const char* const TARGET_PLATFORM_STRINGS[] =
{
	TARGET_PLATFORM_LIST
};
#undef LIST_ITEM

#define LIST_ITEM(value, platStr, headerDir, cc) cc,
static const char* const TARGET_C_COMPILERS[] =
{
	TARGET_PLATFORM_LIST
};
#undef LIST_ITEM

// Condensed on demand:
#define LIST_ITEM(value, platStr, headerDir, cc) headerDir,
static char TargetHeaderDirs[ARR_SIZE(TARGET_PLATFORM_STRINGS)][PLATFORM_MAX_HEADER_DIR_STR_LENGTH] =
{
	TARGET_PLATFORM_LIST
};
#undef LIST_ITEM

// Computed on demand and cached:
static size_t TargetHeaderDirCounts[ARR_SIZE(TARGET_PLATFORM_STRINGS)];

static void CondenseHeaderDirsString(char* string)
{
	char* in = NULL;
	char* out = NULL;

	string[PLATFORM_MAX_HEADER_DIR_STR_LENGTH - 1] = '\0';

	for ( in = string, out = string; ; ++in )
	{
		bool isSeparator = false;
		bool isRedundantSeparator = false;

		if ( !(*in) )
		{
			*out = '\0';
			break;
		}

		isSeparator = *in == ';';
		isRedundantSeparator =
			isSeparator &&
			(out == string || *(out - 1) == '\0');

		if ( !isRedundantSeparator )
		{
			*(out++) = isSeparator ? '\0' : *in;
		}
	}
}

static size_t CountSubstringsInHeaderDirString(const char* string)
{
	const char* cursor = NULL;
	size_t count = 0;

	if ( !string || !(*string) )
	{
		return 0;
	}

	cursor = string;

	while ( cursor - string < PLATFORM_MAX_HEADER_DIR_STR_LENGTH )
	{
		size_t substrLength = strlen(cursor);

		if ( substrLength < 1 )
		{
			break;
		}

		++count;
		cursor += substrLength + 1;
	}

	return count;
}

static void EnsureHeaderListIsPrepared(size_t platformIndex)
{
	if ( platformIndex >= ARR_SIZE(TargetHeaderDirs) ||
	     TargetHeaderDirs[platformIndex][0] == '\0' ||
	     TargetHeaderDirCounts[platformIndex] > 0 )
	{
		return;
	}

	CondenseHeaderDirsString(TargetHeaderDirs[platformIndex]);
	TargetHeaderDirCounts[platformIndex] = CountSubstringsInHeaderDirString(TargetHeaderDirs[platformIndex]);
}

const char* Platform_IDToString(TargetPlatform platform)
{
	return ((size_t)platform) < ARR_SIZE(TARGET_PLATFORM_STRINGS)
		? TARGET_PLATFORM_STRINGS[platform]
		: NULL;
}

bool Platform_StringToID(const char* str, TargetPlatform* outPlatform)
{
	size_t index;

	if ( !str || !(*str) || !outPlatform )
	{
		return false;
	}

	for ( index = 0; index < ARR_SIZE(TARGET_PLATFORM_STRINGS); ++index )
	{
		if ( strcmp(TARGET_PLATFORM_STRINGS[index], str) == 0 )
		{
			*outPlatform = (TargetPlatform)index;
			return true;
		}
	}

	return false;
}

size_t Platform_HeaderDirectoryCount(TargetPlatform platform)
{
	size_t platformIndex = (size_t)platform;

	if ( platformIndex >= ARR_SIZE(TargetHeaderDirCounts) )
	{
		return 0;
	}

	EnsureHeaderListIsPrepared(platformIndex);
	return TargetHeaderDirCounts[platformIndex];
}

const char* Platform_HeaderDirectory(TargetPlatform platform, size_t index)
{
	const char* begin = NULL;
	const char* end = NULL;
	const char* cursor = NULL;
	size_t platformIndex = (size_t)platform;

	if ( platformIndex >= ARR_SIZE(TargetHeaderDirs) )
	{
		return NULL;
	}

	EnsureHeaderListIsPrepared(platformIndex);

	if ( index >= TargetHeaderDirCounts[platformIndex] )
	{
		return NULL;
	}

	begin = TargetHeaderDirs[platformIndex];
	end = begin + PLATFORM_MAX_HEADER_DIR_STR_LENGTH;
	cursor = begin;

	while ( cursor < end && index > 0 )
	{
		if ( !(*(cursor++)) )
		{
			--index;
		}
	}

	return (cursor < end && *cursor && index == 0) ? cursor : NULL;
}

const char* Platform_CCompilerName(TargetPlatform platform)
{
	return ((size_t)platform) < ARR_SIZE(TARGET_C_COMPILERS)
		? TARGET_C_COMPILERS[platform]
		: NULL;
}
