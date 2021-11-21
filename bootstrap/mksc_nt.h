#ifndef BOOTSTRAP_MKSC_NT_H
#define BOOTSTRAP_MKSC_NT_H

#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include "bstfile.h"
#include "options.h"
#include "path.h"
#include "platform.h"
#include "mksc_cmn.h"

#define SCRIPT_FILE_EXT "bat"

// These don't have their % signs escaped since they're used as
// fprintf arguments, rather than a format string itself.
#define LK_STATEMENT_EXECUTABLE "FIL %~2.obj"
#define LK_STATEMENT_STATIC_LIB "+'%~2.obj'"

#define FPRINTF(...) do { if ( fprintf(__VA_ARGS__) < 0 ) return false; } while ( false )

static char OutFilePath[_MAX_PATH];
static char PathBuffer[_MAX_PATH];

static bool ComputeOutputPath(BootstrapFile* inFile)
{
	if ( strcpy_s(OutFilePath, sizeof(OutFilePath), BootstrapFile_GetFileName(inFile)) != 0 )
	{
		return false;
	}

	return Path_SetExt(OutFilePath, sizeof(OutFilePath), SCRIPT_FILE_EXT);
}

static inline void ToPathBufferUppercase(const char* input)
{
	if ( strcpy_s(PathBuffer, sizeof(PathBuffer), input) != 0 )
	{
		PathBuffer[0] = '\0';
	}

	Mksc_ToUppercase(PathBuffer);
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
	TargetPlatform adjustedPlatform;
	size_t targetHeaderCount;
	const char* targetName;

	targetPlatform = BootstrapFile_GetTargetPlatform(inFile);
	adjustedPlatform = targetPlatform != TP_UNSPECIFIED ? targetPlatform : HOST_PLATFORM_ID;

	targetHeaderCount = Platform_HeaderDirectoryCount(adjustedPlatform);
	targetName = BootstrapFile_GetTargetName(inFile);

	FPRINTF(outFile,
		"@ECHO OFF\n"
		"REM Compilation script for %s created by Bootstrap utility\n\n",
		BootstrapFile_GetFileName(inFile));

	FPRINTF(outFile,
		"IF NOT DEFINED WATCOM (\n"
		"\tECHO No OpenWatcom toolchain specified - set path in WATCOM environment variable.\n"
		"\tEXIT /B 1\n"
		")\n\n");

	FPRINTF(outFile,
		"PATH %%WATCOM%%\\binnt64;%%WATCOM%%\\binnt;%%PATH%%\n"
		"SET EDPATH=%%WATCOM%%\\eddat\n");

	if ( targetHeaderCount > 0 )
	{
		size_t index;

		FPRINTF(outFile,
			"SET INCLUDE=");

		for ( index = 0; index < targetHeaderCount; ++index )
		{
			FPRINTF(outFile,
				"%s%%WATCOM%%\\%s",
				index > 0 ? ";" : "",
				Platform_HeaderDirectory(adjustedPlatform, index));
		}

		FPRINTF(outFile,
			"\n");
	}

	FPRINTF(outFile,
			"\n");

	FPRINTF(outFile,
		"SET SCRIPTDIR=%~dp0\n\n");

	FPRINTF(outFile,
		"REM Clear out the linker file\n");

	FPRINTF(outFile,
		"ECHO.>%s.lk1\n\n",
		targetName);

	FPRINTF(outFile,
		"SET FAILURES=0\n\n");

	return true;
}

static bool WriteCompileSourceFiles(BootstrapFile* inFile, FILE* outFile)
{
	size_t fileCount;
	size_t index;

	fileCount = BootstrapFile_SourceFileCount(inFile);

	for ( index = 0; index < fileCount; ++index )
	{
		FPRINTF(outFile,
			"%sECHO [%u/%u] %s\n",
			index > 0 ? "\n" : "",
			index + 1,
			fileCount,
			BootstrapFile_SourceFilePath(inFile, index));

		Mksc_ExtractFileDirPath(PathBuffer, sizeof(PathBuffer), BootstrapFile_SourceFilePath(inFile, index));

		FPRINTF(outFile,
			"CALL :COMPILEFILE \"%s\" , ",
			PathBuffer);

		Path_GetFileBaseName(BootstrapFile_SourceFilePath(inFile, index), PathBuffer, sizeof(PathBuffer));

		FPRINTF(outFile, "\"%s\"\n", PathBuffer);
	}

	FPRINTF(outFile,
		"\n");

	return true;
}

static bool WriteLinkExecutable(BootstrapFile* inFile, FILE* outFile)
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

static bool WriteLinkStaticLib(BootstrapFile* inFile, FILE* outFile)
{
	const char* targetName;

	targetName = BootstrapFile_GetTargetName(inFile);

	// TODO: Options based on what was specified in the .bst file
	FPRINTF(outFile,
		"wlib -b -c -n -q -p=512 %s.lib @%s.lk1\n\n",
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
		"IF %%ERRORLEVEL%% NEQ 0 (\n"
		"\tECHO Linking was unsuccessful\n"
		"\tEXIT /B 1\n"
		")\n\n"
		"ECHO Done\n"
		"GOTO :EOF\n\n");

	FPRINTF(outFile, "ECHO Built target: %s\n", BootstrapFile_GetTargetName(inFile));

	return true;
}

static bool WriteCompileFileFunction(BootstrapFile* inFile, FILE* outFile)
{
	TargetPlatform targetPlatform;
	TargetPlatform adjustedPlatform;
	const char* linkerListFileStatement;
	const char* targetCCompiler;
	const char* targetPlatformName;
	const char* targetName;

	targetPlatform = BootstrapFile_GetTargetPlatform(inFile);
	adjustedPlatform = targetPlatform != TP_UNSPECIFIED ? targetPlatform : HOST_PLATFORM_ID;

	linkerListFileStatement = GetLinkerListFileStatement(inFile);
	targetCCompiler = Platform_CCompilerName(adjustedPlatform);
	targetPlatformName = Platform_IDToString(targetPlatform);
	targetName = BootstrapFile_GetTargetName(inFile);

	if ( !linkerListFileStatement )
	{
		fprintf(stderr, "Unrecognised target type when writing %s\n", OutFilePath);
		return false;
	}

	FPRINTF(outFile,
		":COMPILEFILE\n");

	FPRINTF(outFile,
		"ECHO %s>>%s.lk1\n\n",
		linkerListFileStatement,
		targetName);

	FPRINTF(outFile,
		"REM These compile options were generated by Open Watcom's ide.exe when building in release mode.\n");

	FPRINTF(outFile,
		"%s \"%%~1%%~2.c\" -i=\"%%INCLUDE%%\"",
		targetCCompiler
	);

	if ( targetPlatformName && *targetPlatformName )
	{
		FPRINTF(outFile, " -bt=%s", targetPlatformName);
	}

	FPRINTF(outFile, " %s\n\n",
		BootstrapFile_GetCompileOptions(inFile));

	FPRINTF(outFile,
		"REM For some reason ERRORLEVEL doesn't get set properly on Windows, so we check for a .err file instead.\n"
		"IF EXIST %%~2.ERR (\n"
		"\tECHO Compilation was not successful for %%~2.c\n"
		"\tSET /a FAILURES=FAILURES+1\n"
		")\n"
		"\n"
		"EXIT /B 0\n");

	return true;
}

static inline bool WriteScript(BootstrapFile* inFile, FILE* outFile)
{
	return
		WritePrelude(inFile, outFile) &&
		WriteCompileSourceFiles(inFile, outFile) &&
		WriteLinkTarget(inFile, outFile) &&
		WriteCompileFileFunction(inFile, outFile);
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

	VLOG("Writing script file was %s\n", success ? "successful" : "not successful");

	return success;
}

#endif // BOOTSTRAP_MKSC_NT_H
