#ifndef BOOTSTRAP_MKSC_L_H
#define BOOTSTRAP_MKSC_L_H

#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include "bstfile.h"
#include "options.h"
#include "path.h"

#define SCRIPT_FILE_EXT "sh"

#define FPUTS(...) do { if ( fputs(__VA_ARGS__) == EOF ) return false; } while ( false )

static char OutFilePath[_MAX_PATH];

// Example of invoking compiler on Linux:
// wcc386 myfile.c -i="$WATCOM/lh" -d__STDC_WANT_LIB_EXT1__=1 -dPLATFORM_LINUX=1 -w4 -we -e25 -zq -ze -od -d2 -6r -bt=linux -fo=.obj -mf

// Example of adding to linker list file on Linux:
// echo "FIL myfile.obj" >> myprog.lk1

// Example of invoking linker on Linux:
// wlink name myprog d all sys linux op m op maxe=25 op q op symf @myprog.lk1

static bool ComputeOutputPath(BootstrapFile* inFile)
{
	if ( strcpy_s(OutFilePath, sizeof(OutFilePath), BootstrapFile_GetFileName(inFile)) != 0 )
	{
		return false;
	}

	return Path_SetExt(OutFilePath, sizeof(OutFilePath), SCRIPT_FILE_EXT);
}

static bool WritePrelude(BootstrapFile* inFile, FILE* outFile)
{
	const char* targetName;

	targetName = BootstrapFile_GetTargetName(inFile);

	FPUTS("# Compilation script for ", outFile);
	FPUTS(BootstrapFile_GetFileName(inFile), outFile);
	FPUTS(" created by Bootstrap utility\n\n", outFile);

	FPUTS(
		"if [ -z \"$WATCOM\" ]; then\n"
		"\techo \"No WATCOM environment variable set!\"\n"
		"\texit 1\n"
		"fi\n\n",
		outFile);

	FPUTS("export PATH=$WATCOM/binl64:$WATCOM/binl:$PATH\n", outFile);
	FPUTS("export EDPATH=$WATCOM/eddat\n", outFile);
	FPUTS("export INCLUDE=$WATCOM/lh\n\n", outFile);

	FPUTS("failures=0\n\n", outFile);

	FPUTS(
		"function processFile () {\n"
		"\techo \"FIL $2.obj\" >> ",
		outFile
	);

	FPUTS(targetName, outFile);

	// TODO: Options based on what was specified in the .bst file
	FPUTS(".lk1\n"
		"\n"
		"\twcc386 \"$1$2.c\" -i=\"$INCLUDE\" ", outFile);
	FPUTS(BootstrapFile_GetCompileOptions(inFile), outFile);
	FPUTS(
		"\n\n"
		"\tif [ $? -ne 0 ]; then\n"
		"\t\techo \"Compilation was not successful for $1$2.c\"\n"
		"\t\tfailures=$((failures+1))\n"
		"\tfi\n"
		"}\n\n",
		outFile);

	FPUTS("# Create empty linker file\n", outFile);
	FPUTS("> ", outFile);
	FPUTS(targetName, outFile);
	FPUTS(".lk1\n\n", outFile);

	return true;
}

static bool WriteCompileSourceFiles(BootstrapFile* inFile, FILE* outFile)
{
	static char filePath[_MAX_PATH];

	size_t index = 0;
	size_t fileCount = 0;

	fileCount = BootstrapFile_SourceFileCount(inFile);

	for ( index = 0; index < fileCount; ++index )
	{
		char counter[32];
		int lastSep = -1;

		if ( index > 0 )
		{
			FPUTS("\n", outFile);
		}

		// Example line: echo "[1/5] /path/to/file.c"
		sprintf_s(counter, sizeof(counter), "[%u/%u]", index + 1, fileCount);

		FPUTS("echo \"", outFile);
		FPUTS(counter, outFile);
		FPUTS(" ", outFile);
		FPUTS(BootstrapFile_SourceFilePath(inFile, index), outFile);
		FPUTS("\"\n", outFile);

		// Example line: processFile "/path/to/" "file"
		FPUTS("processFile \"", outFile);

		strcpy_s(filePath, sizeof(filePath), BootstrapFile_SourceFilePath(inFile, index));
		lastSep = Path_IndexOfLastSeparator(filePath);

		if ( lastSep >= 0 )
		{
			filePath[lastSep + 1] = '\0';
		}
		else
		{
			filePath[0] = '\0';
		}

		FPUTS(filePath, outFile);
		FPUTS("\" \"", outFile);

		Path_GetFileBaseName(BootstrapFile_SourceFilePath(inFile, index), filePath, sizeof(filePath));
		FPUTS(filePath, outFile);
		FPUTS("\"\n", outFile);
	}

	FPUTS("\nif [ $failures -ne 0 ]; then\n", outFile);
	FPUTS("\techo \"$failures file(s) failed to compile.\"\n", outFile);
	FPUTS("\texit 1\n", outFile);
	FPUTS("fi\n\n", outFile);

	return true;
}

static bool WriteLinkTarget(BootstrapFile* inFile, FILE* outFile)
{
	const char* targetName;

	targetName = BootstrapFile_GetTargetName(inFile);

	// TODO: Options based on what was specified in the .bst file
	FPUTS("wlink name ", outFile);
	FPUTS(targetName, outFile);
	FPUTS(" d all sys linux op m op maxe=25 op q op symf @", outFile);
	FPUTS(targetName, outFile);
	FPUTS(".lk1\n\n", outFile);

	FPUTS("if [ $? -ne 0 ]; then\n", outFile);
	FPUTS("\techo \"Linking was not successful.\"\n", outFile);
	FPUTS("\texit 1\n", outFile);
	FPUTS("fi\n\n", outFile);

	FPUTS("echo \"Built target: ", outFile);
	FPUTS(BootstrapFile_GetTargetName(inFile), outFile);
	FPUTS("\"\n", outFile);

	return true;
}

static inline bool WriteScript(BootstrapFile* inFile, FILE* outFile)
{
	return
		WritePrelude(inFile, outFile) &&
		WriteCompileSourceFiles(inFile, outFile) &&
		WriteLinkTarget(inFile, outFile);
}

static bool MakeScript_Impl(BootstrapFile* inFile)
{
	FILE* outFile = NULL;
	bool success = false;

	if ( !ComputeOutputPath(inFile) )
	{
		fprintf(stderr, "Could not compute output script path for %s\n", BootstrapFile_GetFilePath(inFile));
		return false;
	}

	VLOG("Writing script file: %s\n", OutFilePath);

	outFile = fopen(OutFilePath, "w");

	if ( !outFile )
	{
		fprintf(stderr, "Could not open %s for writing.\n", OutFilePath);
		return false;
	}

	success = WriteScript(inFile, outFile);
	fclose(outFile);

	if ( !success )
	{
		fprintf(stderr, "Failed to write %s.\n", OutFilePath);
	}

	return success;
}

#endif // BOOTSTRAP_MKSC_L_H
