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

#define LIST_ITEM(value, platStr, headerDir, cc) headerDir,
static const char* const TARGET_HEADER_DIRS[] =
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

const char* Platform_HeaderDirectory(TargetPlatform platform)
{
	return ((size_t)platform) < ARR_SIZE(TARGET_HEADER_DIRS)
		? TARGET_HEADER_DIRS[platform]
		: NULL;
}

const char* Platform_CCompilerName(TargetPlatform platform)
{
	return ((size_t)platform) < ARR_SIZE(TARGET_C_COMPILERS)
		? TARGET_C_COMPILERS[platform]
		: NULL;
}
