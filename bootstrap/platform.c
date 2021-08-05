#include <stddef.h>
#include <string.h>
#include "platform.h"

#define LIST_ITEM(value, str) str,
static const char* const TARGET_PLATFORM_STRINGS[] =
{
	TARGET_PLATFORM_LIST
};
#undef LIST_ITEM

#define NUM_TARGET_PLATFORM_STRINGS (sizeof(TARGET_PLATFORM_STRINGS) / sizeof(TARGET_PLATFORM_STRINGS[0]))

const char* Platform_IDToString(TargetPlatform platform)
{
	return ((size_t)platform) < NUM_TARGET_PLATFORM_STRINGS
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

	for ( index = 0; index < NUM_TARGET_PLATFORM_STRINGS; ++index )
	{
		if ( strcmp(TARGET_PLATFORM_STRINGS[index], str) == 0 )
		{
			*outPlatform = (TargetPlatform)index;
			return true;
		}
	}

	return false;
}
