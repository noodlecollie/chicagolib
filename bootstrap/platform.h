#ifndef BOOTSTRAP_PLATFORM_H
#define BOOTSTRAP_PLATFORM_H

#include <stdbool.h>

#define TARGET_PLATFORM_LIST \
	LIST_ITEM(TP_UNSPECIFIED = 0, "") \
	LIST_ITEM(TP_LINUX, "LINUX") \
	LIST_ITEM(TP_WINDOWS, "NT") \
	LIST_ITEM(TP_DOS, "DOS")

#if defined(__NT__) || defined(__DOS__)
#define PATH_SEP_CHAR '\\'
#define PATH_SEP_STR "\\"
#else
#define PATH_SEP_CHAR '/'
#define PATH_SEP_STR "/"
#endif

#if defined(__NT__)
#define PLATFORM_ID TP_WINDOWS
#elif defined(__DOS__)
#define PLATFORM_ID TP_DOS
#elif defined(__LINUX__)
#define PLATFORM_ID TP_LINUX
#else
#error Unrecognised platform!
#endif

#define LIST_ITEM(value, str) value,
typedef enum _TargetPlatform
{
	TARGET_PLATFORM_LIST
} TargetPlatform;
#undef LIST_ITEM

const char* Platform_IDToString(TargetPlatform platform);
bool Platform_StringToID(const char* str, TargetPlatform* outPlatform);

#endif // BOOTSTRAP_PLATFORM_H
