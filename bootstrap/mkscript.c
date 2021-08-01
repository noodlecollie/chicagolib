
#include <stdio.h>
#include "mkscript.h"
#include "options.h"

#ifdef PLATFORM_LINUX
#include "mksc_l.h"
#else
#error No MakeScript implementation available for the chosen platform!
#endif

bool MakeScript_WriteScriptFile(BootstrapFile* file)
{
	VLOG("Creating script file for target \"%s\"\n", BootstrapFile_GetTargetName(file));

	return MakeScript_Impl(file);
}
