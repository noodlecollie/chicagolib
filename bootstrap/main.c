#include <stdio.h>
#include <libgen.h>
#include "bstfile.h"
#include "bstparse.h"
#include "options.h"
#include "mkscript.h"

static BootstrapFile LocalFile;

static void ReadBSTFile(FILE* inFile, BootstrapFile* outFile)
{
	static char lineBuffer[BST_MAX_LINE_LENGTH];
	size_t lineNumber = 1;

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
			BootstrapParse_ParseLine(&LocalFile, lineNumber, lineBuffer, lineLength);
		}

		++lineNumber;
	}
}

int main(int argc, char** argv)
{
	FILE* inFile = NULL;
	bool success = false;

	if ( !Options_Parse(argc, argv) )
	{
		return 1;
	}

	inFile = fopen(Option_BSTFilePath, "r");

	if ( !inFile )
	{
		fprintf(stderr, "Could not open: %s\n", Option_BSTFilePath);
		return 1;
	}

	BootstrapFile_Init(&LocalFile);
	BootstrapFile_SetFilePath(&LocalFile, Option_BSTFilePath);
	BootstrapFile_SetTargetName(&LocalFile, NULL);

	VLOG("Reading: %s\n", BootstrapFile_GetFilePath(&LocalFile));

	BootstrapParse_SetProjectFilePath(BootstrapFile_GetFilePath(&LocalFile));
	ReadBSTFile(inFile, &LocalFile);
	fclose(inFile);

	// REMOVE ME
	printf("Compile options: %s\n", BootstrapFile_GetCompileOptions(&LocalFile));

	if ( BootstrapFile_SourceFileCount(&LocalFile) > 0 )
	{
		VLOG("%s: target has %u source files.\n",
			BootstrapFile_GetFilePath(&LocalFile),
			BootstrapFile_SourceFileCount(&LocalFile));

		success = MakeScript_WriteScriptFile(&LocalFile);

		if ( !success )
		{
			fprintf(stderr, "Failed to write build script.\n");
		}
	}
	else
	{
		fprintf(stderr, "%s did not provide any source files to build.\n",
			BootstrapFile_GetFilePath(&LocalFile));
	}

	BootstrapFile_Destroy(&LocalFile);

	return success ? 0 : 1;
}
