#ifndef BOOTSTRAP_PLATFORM_H
#define BOOTSTRAP_PLATFORM_H

#include <stdbool.h>

#define TARGET_PLATFORM_LIST \
	/*        ID                  Plat str   Header dir  C compiler */ \
	LIST_ITEM(TP_UNSPECIFIED = 0, "",        "",         ""          ) \
	LIST_ITEM(TP_LINUX,           "LINUX",   "lh",       "wcc386"    ) \
	LIST_ITEM(TP_WINDOWS,         "NT",      "h",        "wcc386"    ) \
	LIST_ITEM(TP_DOS,             "DOS",     "h",        "wcc"       )

#if defined(__NT__) || defined(__DOS__)
#define PATH_SEP_CHAR '\\'
#define PATH_SEP_STR "\\"
#else
#define PATH_SEP_CHAR '/'
#define PATH_SEP_STR "/"
#endif

#if defined(__NT__)
#define HOST_PLATFORM_ID TP_WINDOWS
#elif defined(__DOS__)
#define HOST_PLATFORM_ID TP_DOS
#elif defined(__LINUX__)
#define HOST_PLATFORM_ID TP_LINUX
#else
#define HOST_PLATFORM_ID TP_UNSPECIFIED
#endif

// The host platform ID is not allowed to be unspecified,
// because this is a special value used only to indicate
// that the target's platform is the same as the host.
#if HOST_PLATFORM_ID == TP_UNSPECIFIED
#error No host platform ID was set up!
#endif

#define LIST_ITEM(value, platStr, headerDir, cc) value,
typedef enum _TargetPlatform
{
	TARGET_PLATFORM_LIST
} TargetPlatform;
#undef LIST_ITEM

const char* Platform_IDToString(TargetPlatform platform);
bool Platform_StringToID(const char* str, TargetPlatform* outPlatform);

const char* Platform_HeaderDirectory(TargetPlatform platform);
const char* Platform_CCompilerName(TargetPlatform platform);

#endif // BOOTSTRAP_PLATFORM_H
