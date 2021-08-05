#include <stdio.h>
#include <libgen.h>
#include "bstfile.h"
#include "bstparse.h"
#include "options.h"
#include "mkscript.h"

static BootstrapFile LocalFile;

static bool ReadBSTFile(FILE* inFile, BootstrapFile* outFile)
{
	static char lineBuffer[BST_MAX_LINE_LENGTH];
	size_t lineNumber = 1;

	BootstrapParse_SetProjectFilePath(BootstrapFile_GetFilePath(&LocalFile));

	while ( !feof(inFile) )
	{
		size_t lineLength;

		lineLength = BootstrapParse_ReadLine(inFile, lineBuffer, sizeof(lineBuffer));

		if ( lineLength >= sizeof(lineBuffer) )
		{
			// Line was too long.
			fprintf(stderr, "%s(%u): Warning: Line length of %u exceeded maximum allowed length of %u characters.\n",
				BootstrapFile_GetFilePath(outFile),
				lineNumber,
				lineLength,
				sizeof(lineBuffer) - 1);
		}

		if ( lineLength > 0 )
		{
			if ( !BootstrapParse_ParseLine(&LocalFile, lineNumber, lineBuffer, lineLength) )
			{
				return false;
			}
		}

		++lineNumber;
	}

	return true;
}

static inline bool ReadFile()
{
	FILE* inFile = NULL;
	bool success = false;

	do
	{
		inFile = fopen(Option_BSTFilePath, "r");

		if ( !inFile )
		{
			fprintf(stderr, "Could not open: %s\n", Option_BSTFilePath);
			break;
		}

		BootstrapFile_SetFilePath(&LocalFile, Option_BSTFilePath);
		BootstrapFile_SetTargetName(&LocalFile, NULL);
		BootstrapFile_SetTargetPlatform(&LocalFile, Option_TargetPlatform);

		VLOG("Reading: %s\n", BootstrapFile_GetFilePath(&LocalFile));
		success = ReadBSTFile(inFile, &LocalFile);
	}
	while ( false );

	if ( inFile )
	{
		fclose(inFile);
	}

	return success;
}

int main(int argc, char** argv)
{
	bool success = false;

	if ( !Options_Parse(argc, argv) )
	{
		return 1;
	}

	if ( !BootstrapFile_Init(&LocalFile) )
	{
		fprintf(stderr, "Could not allocate memory to begin parsing.\n");
		return 1;
	}

	do
	{
		if ( !ReadFile() )
		{
			break;
		}

		if ( BootstrapFile_SourceFileCount(&LocalFile) < 1 )
		{
			fprintf(stderr, "%s did not provide any source files to build.\n",
				BootstrapFile_GetFilePath(&LocalFile));

			break;
		}

		VLOG("%s: target has %u source files.\n",
			BootstrapFile_GetFilePath(&LocalFile),
			BootstrapFile_SourceFileCount(&LocalFile));

		if ( !MakeScript_WriteScriptFile(&LocalFile) )
		{
			fprintf(stderr, "Failed to write build script.\n");
			break;
		}

		success = true;
	}
	while ( false );

	BootstrapFile_Destroy(&LocalFile);

	return success ? 0 : 1;
}
