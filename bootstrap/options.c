#include <string.h>
#include <stdio.h>
#include "options.h"

typedef struct _FlagOption
{
	const char* longName;
	const char* shortName;
	const char* desc;
	bool* var;
} FlagOption;

bool Option_Verbose = false;
const char* Option_BSTFilePath = "";

static const FlagOption FLAG_OPTIONS[] =
{
	{ "--verbose", "-v", "Print operational messages to stdout.", &Option_Verbose },
	{ NULL, NULL, NULL }
};

static bool HandleOption(const char* option)
{
	const FlagOption* optStruct;

	for ( optStruct = FLAG_OPTIONS; optStruct->var; ++optStruct )
	{
		if ( (optStruct->shortName && strcmp(optStruct->shortName, option) == 0) ||
		     (optStruct->longName && strcmp(optStruct->longName, option) == 0) )
		{
			*optStruct->var = true;
			return true;
		}
	}

	printf("Unrecognised option: %s\n", option);
	return false;
}

static inline void PrintHelp(void)
{
	const FlagOption* optStruct = NULL;

	fprintf(stderr, "Usage: bootstrap [options] <file.bst>\n");
	fprintf(stderr, "  Options:\n");

	for ( optStruct = FLAG_OPTIONS; optStruct->var; ++optStruct )
	{
		fprintf(stderr, "    %s%s%s : %s\n",
			optStruct->shortName ? optStruct->shortName : "",
			(optStruct->shortName && optStruct->longName) ? "/" : "",
			optStruct->longName ? optStruct->longName : "",
			optStruct->desc);
	}
}

bool Options_Parse(int argc, char** argv)
{
	int argIndex;
	bool failedOption = false;

	if ( argc < 1 || !argv )
	{
		return false;
	}

	if ( argc == 1 )
	{
		PrintHelp();
		return false;
	}

	for ( argIndex = 1; argIndex < argc; ++argIndex )
	{
		char* arg = argv[argIndex];

		if ( *arg == '-' )
		{
			if ( !HandleOption(arg) )
			{
				failedOption = true;
			}
		}
		else
		{
			if ( argIndex != argc - 1 )
			{
				fprintf(stderr, "Error: Project file \"%s\" must be the final argument specified.\n", arg);
				return false;
			}

			Option_BSTFilePath = arg;
		}
	}

	return !failedOption;
}
