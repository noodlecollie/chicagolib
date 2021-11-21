#ifndef BOOTSTRAP_PLATFORM_H
#define BOOTSTRAP_PLATFORM_H

#include <stdbool.h>
#include <stddef.h>

// Maximum length of the "Header dirs" string below:
#define PLATFORM_MAX_HEADER_DIR_STR_LENGTH 256

#define TARGET_PLATFORM_LIST \
	/*        ID                  Plat str   Header dirs                             C compiler */ \
	LIST_ITEM(TP_UNSPECIFIED = 0, "",        "",                                     ""          ) \
	LIST_ITEM(TP_LINUX,           "LINUX",   "lh",                                   "wcc386"    ) \
	LIST_ITEM(TP_WINDOWS,         "NT",      "h\0h\\nt\0h\\nt\\directx\0h\\nt\\ddk", "wcc386"    ) \
	LIST_ITEM(TP_DOS,             "DOS",     "h",                                    "wcc"       )

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
#endif

#ifndef HOST_PLATFORM_ID
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

size_t Platform_HeaderDirectoryCount(TargetPlatform platform);
const char* Platform_HeaderDirectory(TargetPlatform platform, size_t index);

const char* Platform_CCompilerName(TargetPlatform platform);

#endif // BOOTSTRAP_PLATFORM_H
