
#include <stdio.h>
#include "mkscript.h"
#include "options.h"
#include "platform.h"

#if defined(__LINUX__)
#include "mksc_l.h"
#elif defined(__NT__)
#include "mksc_nt.h"
#else
#error No MakeScript implementation available for the chosen platform!
#endif

bool MakeScript_WriteScriptFile(BootstrapFile* file)
{
	size_t numSourceFiles;

	numSourceFiles = BootstrapFile_SourceFileCount(file);

	if ( numSourceFiles < 1 )
	{
		fprintf(stderr, "No source files were provided to compile!\n");
		return false;
	}

	VLOG("Creating script file for target \"%s\"\n", BootstrapFile_GetTargetName(file));

	return MakeScript_Impl(file);
}
