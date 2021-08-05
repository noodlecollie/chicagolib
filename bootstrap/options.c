#include <string.h>
#include <stdio.h>
#include "options.h"

bool Option_Verbose = false;
const char* Option_BSTFilePath = "";
TargetPlatform Option_TargetPlatform = TP_UNSPECIFIED;

typedef struct _OptionSwitch
{
	const char* longName;
	const char* shortName;
	const char* desc;
	bool takesArg;
	bool (*handler)(const char*, const char*);
} OptionSwitch;

static bool HandleVerbose(const char* key, const char* value)
{
	(void)key;
	(void)value;

	Option_Verbose = true;
	return true;
}

static bool HandlePlatform(const char* key, const char* value)
{
	(void)key;

	if ( !Platform_StringToID(value, &Option_TargetPlatform) )
	{
		fprintf(stderr, "Unrecognised target platform \"%s\".\n", value ? value : "");
		return false;
	}

	return true;
}

static const OptionSwitch OPTION_HANDLERS[] =
{
	{ "--verbose", "-v", "Print operational messages to stdout.", false, &HandleVerbose },
	{ "--platform", "-p", "Platform to build target for.", true, &HandlePlatform },
	{ 0 }
};

static bool HandleOption(size_t argc, char** argv, size_t* index)
{
	const OptionSwitch* optStruct;
	const char* option = NULL;

	if ( *index >= argc )
	{
		return false;
	}

	option = argv[*index];

	for ( optStruct = OPTION_HANDLERS; optStruct->handler; ++optStruct )
	{
		if ( (optStruct->shortName && strcmp(optStruct->shortName, option) == 0) ||
		     (optStruct->longName && strcmp(optStruct->longName, option) == 0) )
		{
			bool success = false;

			if ( optStruct->takesArg && *index == argc - 1 )
			{
				fprintf(stderr, "Required argument for option \"%s\" was missing.\n", option);
				return false;
			}

			success = (*optStruct->handler)(option, optStruct->takesArg ? argv[(*index) + 1] : NULL);

			*index += optStruct->takesArg ? 2 : 1;
			return success;
		}
	}

	printf("Unrecognised option: %s\n", option);
	return false;
}

static inline void PrintHelp(void)
{
	const OptionSwitch* optStruct = NULL;

	fprintf(stderr, "Usage: bootstrap [options] <file.bst>\n");
	fprintf(stderr, "  Options:\n");

	for ( optStruct = OPTION_HANDLERS; optStruct->handler; ++optStruct )
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
	size_t argIndex;
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

	for ( argIndex = 1; argIndex < (size_t)argc; /* manual increment */ )
	{
		char* arg = argv[argIndex];

		if ( *arg == '-' )
		{
			// HandleOption increments index if successful.
			if ( HandleOption((size_t)argc, argv, &argIndex) )
			{
				continue;
			}

			failedOption = true;
		}
		else
		{
			if ( argIndex != (size_t)(argc - 1) )
			{
				fprintf(stderr, "Error: Project file \"%s\" must be the final argument specified.\n", arg);
				return false;
			}

			Option_BSTFilePath = arg;
		}

		++argIndex;
	}

	return !failedOption;
}
