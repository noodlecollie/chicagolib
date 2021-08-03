#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "bstparse.h"
#include "path.h"
#include "options.h"

#define CH_BEGIN_CATEGORY '['
#define CH_END_CATEGORY ']'
#define CH_COMMENT '#'

#define CATEGORY_SOURCES "sources"
#define CATEGORY_COMPILE_OPTIONS "compile_options"
#define CATEGORY_TARGET_NAME "target_name"
#define CATEGORY_TARGET_TYPE "target_type"

#define TARGET_TYPE_EXECUTABLE "executable"
#define TARGET_TYPE_STATIC_LIB "static_lib"

typedef enum _ParseState
{
	PS_FAILURE = -1,
	PS_DEFAULT = 0,
	PS_SOURCE_FILES,
	PS_COMPILE_OPTIONS,
	PS_TARGET_NAME,
	PS_TARGET_TYPE
} ParseState;

typedef struct _CategoryToState
{
	const char* categoryName;
	ParseState state;
} CategoryToState;

static ParseState PState = PS_DEFAULT;
static char RootDir[_MAX_PATH];
static size_t RootDirLength = 0;

static inline char* SkipWhitespace(char* str)
{
	while ( *str && isspace(*str) )
	{
		++str;
	}

	return str;
}

static inline size_t TrimTrailingWhitespace(char* str, size_t length)
{
	if ( length < 1 )
	{
		length = strlen(str);

		if ( length < 1 )
		{
			return 0;
		}
	}

	for ( ; length > 0; --length )
	{
		if ( !isspace(str[length - 1]) )
		{
			break;
		}

		str[length - 1] = '\0';
	}

	return length;
}

static inline char* FindLineComment(char* str)
{
	bool inQuotes = false;
	bool firstChar = true;

	for ( ; *str; ++str, firstChar = false )
	{
		if ( *str == '"' && (firstChar || *(str - 1) != '\\') )
		{
			inQuotes = !inQuotes;
		}
		else if ( *str == '#' && !inQuotes )
		{
			return str;
		}
	}

	return NULL;
}

static ParseState StateForCategory(const char* categoryName)
{
	static const CategoryToState CTS_MAP[] =
	{
		{ CATEGORY_SOURCES, PS_SOURCE_FILES },
		{ CATEGORY_COMPILE_OPTIONS, PS_COMPILE_OPTIONS },
		{ CATEGORY_TARGET_NAME, PS_TARGET_NAME },
		{ CATEGORY_TARGET_TYPE, PS_TARGET_TYPE },
		{ NULL, PS_FAILURE }
	};

	const CategoryToState* item;

	for ( item = CTS_MAP; item->categoryName; ++item )
	{
		if ( strcmp(item->categoryName, categoryName) == 0 )
		{
			return item->state;
		}
	}

	return PS_FAILURE;
}

static inline bool LineDenotesCategory(const char* line)
{
	return *line == CH_BEGIN_CATEGORY;
}

static inline bool LineDenotesComment(const char* line)
{
	return *line == CH_COMMENT;
}

static ParseState DetermineCategory(BootstrapFile* file, size_t lineNumber, const char* line)
{
	ParseState nextState = PS_FAILURE;
	char* end;

	end = strchr(line, CH_END_CATEGORY);

	if ( !end )
	{
		fprintf(stderr, "%s(%u): Error: Missing '%c' to terminate category declaration.\n",
			BootstrapFile_GetFilePath(file),
			lineNumber,
			CH_END_CATEGORY);

		return PS_FAILURE;
	}

	*end = '\0';
	nextState = StateForCategory(line);

	if ( nextState == PS_FAILURE )
	{
		fprintf(stderr, "%s(%u): Error: Unrecognised category \"%s\".\n",
			BootstrapFile_GetFilePath(file),
			lineNumber,
			line);

		return PS_FAILURE;
	}

	return nextState;
}

static inline ParseState ParseLine_Default(BootstrapFile* file, size_t lineNumber)
{
	// We haven't come across a category yet, so refuse anything that isn't one.
	// If this line were a category, it'd have been picked up by now already.
	fprintf(stderr, "%s(%u): Error: Expected category declaration.\n",
		BootstrapFile_GetFilePath(file),
		lineNumber);

	return PS_FAILURE;
}

static ParseState ParseLine_SourceFile(BootstrapFile* file, size_t lineNumber, char* line)
{
	static char absPath[_MAX_PATH];

	if ( sprintf_s(absPath, sizeof(absPath), "%s/%s", RootDir, line) < 0 )
	{
		fprintf(stderr, "%s(%u): Error: Could not construct absolute path for %s.\n",
			BootstrapFile_GetFilePath(file),
			lineNumber,
			line);

		return PS_FAILURE;
	}

	VLOG("Adding source file: %s\n", absPath);

	if ( !BootstrapFile_AddSourceFile(file, absPath) )
	{
		fprintf(stderr, "%s(%u): Error: Could not allocate internal memory to track source file entry %s.\n",
			BootstrapFile_GetFilePath(file),
			lineNumber,
			line);

		return PS_FAILURE;
	}

	return PS_SOURCE_FILES;
}

static ParseState ParseLine_CompileOptions(BootstrapFile* file, size_t lineNumber, char* line, size_t lineLength)
{
	if ( !BootstrapFile_AppendCompileOptions(file, line, lineLength) )
	{
		fprintf(stderr, "%s(%u): Error: Could not allocate internal memory to track compile options \"%s\".\n",
			BootstrapFile_GetFilePath(file),
			lineNumber,
			line);

		return PS_FAILURE;
	}

	return PS_COMPILE_OPTIONS;
}

static ParseState ParseLine_TargetName(BootstrapFile* file, char* line)
{
	BootstrapFile_SetTargetName(file, line);
	return PS_DEFAULT;
}

static ParseState ParseLine_TargetType(BootstrapFile* file, size_t lineNumber, char* line)
{
	if ( strcmp(line, TARGET_TYPE_EXECUTABLE) == 0 )
	{
		BootstrapFile_SetTargetType(file, TT_EXECUTABLE);
	}
	else if ( strcmp(line, TARGET_TYPE_STATIC_LIB) == 0 )
	{
		BootstrapFile_SetTargetType(file, TT_STATIC_LIB);
	}
	else
	{
		fprintf(stderr, "%s(%u): Error: Unrecognised target type \"%s\".\n",
			BootstrapFile_GetFilePath(file),
			lineNumber,
			line);

		return PS_FAILURE;
	}

	return PS_DEFAULT;
}

void BootstrapParse_SetProjectFilePath(const char* path)
{
	if ( !path || !_fullpath(RootDir, Path_DirName(path), sizeof(RootDir)) )
	{
		strcpy_s(RootDir, sizeof(RootDir), ".");
	}

	RootDirLength = strlen(RootDir);
}

size_t BootstrapParse_ReadLine(FILE* inFile, char* buffer, size_t size)
{
	size_t lineLength = 0;

	if ( !inFile || !buffer || size < 1 )
	{
		return 0;
	}

	// Ensure the buffer is terminated, in case we don't read anything at all.
	*buffer = '\0';

	while ( true )
	{
		int ch;

		ch = fgetc(inFile);

		if ( ch == '\r' )
		{
			// No use for this, so skip.
			continue;
		}

		if ( ch == '\n' || ch == EOF )
		{
			// Treat this as a string terminator.
			ch = '\0';
		}

		// Only write to the buffer if we're in range.
		if ( lineLength < size )
		{
			*(buffer++) = (char)ch;
		}

		if ( ch )
		{
			// This was a non-terminating character, so increment the line length.
			// We keep track of how many characters we would have written if the
			// buffer had been large enough, regardless of how many we actually wrote.
			++lineLength;
		}
		else
		{
			break;
		}
	}

	return lineLength;
}

bool BootstrapParse_ParseLine(BootstrapFile* file, size_t lineNumber, char* line, size_t lineLength)
{
	char* firstChar = NULL;
	char* trailingComment = NULL;

	if ( !file || !line || lineLength < 1 )
	{
		return false;
	}

	firstChar = SkipWhitespace(line);
	lineLength -= firstChar - line;

	if ( !(*firstChar) )
	{
		// Line was empty, do nothing.
		return true;
	}

	if ( LineDenotesComment(firstChar) )
	{
		// Line begins with a comment - do nothing.
		return true;
	}

	// Ensure any trailing line comment is trimmed.
	trailingComment = FindLineComment(firstChar);

	if ( trailingComment )
	{
		*trailingComment = '\0';
		lineLength = trailingComment - firstChar;
	}

	lineLength = TrimTrailingWhitespace(firstChar, lineLength);

	if ( LineDenotesCategory(firstChar) )
	{
		PState = DetermineCategory(file, lineNumber, line + 1);
	}
	else
	{
		switch ( PState )
		{
			case PS_SOURCE_FILES:
			{
				PState = ParseLine_SourceFile(file, lineNumber, firstChar);
				break;
			}

			case PS_COMPILE_OPTIONS:
			{
				PState = ParseLine_CompileOptions(file, lineNumber, firstChar, lineLength);
				break;
			}

			case PS_TARGET_NAME:
			{
				PState = ParseLine_TargetName(file, firstChar);
				break;
			}

			case PS_TARGET_TYPE:
			{
				PState = ParseLine_TargetType(file, lineNumber, firstChar);
				break;
			}

			default:
			{
				PState = ParseLine_Default(file, lineNumber);
				break;
			}
		}
	}

	return PState != PS_FAILURE;
}
