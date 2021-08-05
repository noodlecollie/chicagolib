#ifndef BOOTSTRAP_MKSC_L_H
#define BOOTSTRAP_MKSC_L_H

#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include "bstfile.h"
#include "options.h"
#include "path.h"
#include "platform.h"

#define SCRIPT_FILE_EXT "sh"

#define LK_STATEMENT_EXECUTABLE "FIL $2.obj"
#define LK_STATEMENT_STATIC_LIB "+'$2.obj'"

#define FPUTS(...) do { if ( fputs(__VA_ARGS__) == EOF ) return false; } while ( false )
#define FPRINTF(...) do { if ( fprintf(__VA_ARGS__) < 0 ) return false; } while ( false )

static char OutFilePath[_MAX_PATH];

static bool ComputeOutputPath(BootstrapFile* inFile)
{
	if ( strcpy_s(OutFilePath, sizeof(OutFilePath), BootstrapFile_GetFileName(inFile)) != 0 )
	{
		return false;
	}

	return Path_SetExt(OutFilePath, sizeof(OutFilePath), SCRIPT_FILE_EXT);
}

static inline const char* GetLinkerListFileStatement(BootstrapFile* inFile)
{
	switch ( BootstrapFile_GetTargetType(inFile) )
	{
		case TT_EXECUTABLE:
		{
			return LK_STATEMENT_EXECUTABLE;
		}

		case TT_STATIC_LIB:
		{
			return LK_STATEMENT_STATIC_LIB;
		}

		default:
		{
			return NULL;
		}
	}
}

static bool WritePrelude(BootstrapFile* inFile, FILE* outFile)
{
	TargetPlatform targetPlatform;
	const char* targetName;
	const char* targetPlatformName;
	const char* targetHeaderDir;
	const char* targetCCompiler;
	const char* linkerListFileStatement;

	targetPlatform = BootstrapFile_GetTargetPlatform(inFile);
	targetName = BootstrapFile_GetTargetName(inFile);
	targetPlatformName = Platform_IDToString(targetPlatform);
	targetHeaderDir = Platform_HeaderDirectory(targetPlatform != TP_UNSPECIFIED ? targetPlatform : HOST_PLATFORM_ID);
	targetCCompiler = Platform_CCompilerName(targetPlatform != TP_UNSPECIFIED ? targetPlatform : HOST_PLATFORM_ID);
	linkerListFileStatement = GetLinkerListFileStatement(inFile);

	if ( !linkerListFileStatement )
	{
		fprintf(stderr, "Unrecognised target type when writing %s\n", OutFilePath);
		return false;
	}

	FPRINTF(outFile,
		"# Compilation script for %s created by Bootstrap utility\n\n",
		BootstrapFile_GetFileName(inFile));

	FPRINTF(outFile,
		"if [ -z \"$WATCOM\" ]; then\n"
		"\techo \"No WATCOM environment variable set!\"\n"
		"\texit 1\n"
		"fi\n\n");

	// Path remains pointing to Linux dirs, because it's the path to
	// the host compiler executables. The include path does have to
	// change based on the target platform, however.
	FPRINTF(outFile,
		"export PATH=$WATCOM/binl64:$WATCOM/binl:$PATH\n"
		"export EDPATH=$WATCOM/eddat\n"
		"export INCLUDE=$WATCOM/%s\n\n",
		targetHeaderDir);

	FPRINTF(outFile, "failures=0\n\n");

	FPRINTF(outFile,
		"function processFile () {\n"
		"\techo \"%s\" >> %s.lk1\n\n",
		linkerListFileStatement,
		targetName);

	FPRINTF(outFile,
		"\t%s \"$1$2.c\" -i=\"$INCLUDE\"",
		targetCCompiler);

	if ( targetPlatformName && *targetPlatformName )
	{
		FPRINTF(outFile, " -bt=%s", targetPlatformName);
	}

	FPRINTF(outFile, " %s\n\n",
		BootstrapFile_GetCompileOptions(inFile));

	FPRINTF(outFile,
		"\tif [ $? -ne 0 ]; then\n"
		"\t\techo \"Compilation was not successful for $1$2.c\"\n"
		"\t\tfailures=$((failures+1))\n"
		"\tfi\n"
		"}\n\n");

	FPRINTF(outFile,
		"# Create empty linker file\n"
		"> %s.lk1\n\n",
		targetName);

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
		int lastSep = -1;

		FPRINTF(outFile,
			"%secho \"[%u/%u] %s\"\n",
			index > 0 ? "\n" : "",
			index + 1,
			fileCount,
			BootstrapFile_SourceFilePath(inFile, index));

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

		FPRINTF(outFile,
			"processFile \"%s\" ",
			filePath);

		Path_GetFileBaseName(BootstrapFile_SourceFilePath(inFile, index), filePath, sizeof(filePath));
		FPRINTF(outFile, "\"%s\"\n", filePath);
	}

	FPRINTF(outFile,
		"\nif [ $failures -ne 0 ]; then\n"
		"\techo \"$failures file(s) failed to compile.\"\n"
		"\texit 1\n"
		"fi\n\n");

	return true;
}

static inline bool WriteLinkExecutable(BootstrapFile* inFile, FILE* outFile)
{
	const char* targetName;
	const char* targetPlatformName;

	targetName = BootstrapFile_GetTargetName(inFile);
	targetPlatformName = Platform_IDToString(BootstrapFile_GetTargetPlatform(inFile));

	FPRINTF(outFile,
		"wlink name %s",
		targetName);

	if ( targetPlatformName && *targetPlatformName )
	{
		FPRINTF(outFile,
			" sys %s",
			targetPlatformName);
	}

	// TODO: Options based on what was specified in the .bst file
	FPRINTF(outFile,
		" d all op m op maxe=25 op q op symf @%s.lk1\n\n",
		targetName);

	return true;
}

static inline bool WriteLinkStaticLib(BootstrapFile* inFile, FILE* outFile)
{
	const char* targetName;

	targetName = BootstrapFile_GetTargetName(inFile);

	// TODO: Options based on what was specified in the .bst file
	FPRINTF(outFile,
		"wlib -b -c -n -q -p=512  %s.lib @%s.lk1\n\n",
		targetName,
		targetName);

	return true;
}

static bool WriteLinkTarget(BootstrapFile* inFile, FILE* outFile)
{
	bool success = false;

	switch ( BootstrapFile_GetTargetType(inFile) )
	{
		case TT_EXECUTABLE:
		{
			success = WriteLinkExecutable(inFile, outFile);
			break;
		}

		case TT_STATIC_LIB:
		{
			success = WriteLinkStaticLib(inFile, outFile);
			break;
		}

		default:
		{
			fprintf(stderr, "Unrecognised target type when writing %s\n", OutFilePath);
			return false;
		}
	}

	FPRINTF(outFile,
		"if [ $? -ne 0 ]; then\n"
		"\techo \"Linking was not successful.\"\n"
		"\texit 1\n"
		"fi\n\n");

	FPRINTF(outFile, "echo \"Built target: %s\"\n", BootstrapFile_GetTargetName(inFile));

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
		remove(OutFilePath);
	}

	return success;
}

#endif // BOOTSTRAP_MKSC_L_H
