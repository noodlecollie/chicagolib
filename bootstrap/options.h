#ifndef BOOTSTRAP_OPTIONS_H
#define BOOTSTRAP_OPTIONS_H

#include <stdbool.h>
#include <stdio.h>

extern bool Option_Verbose;
extern const char* Option_BSTFilePath;

bool Options_Parse(int argc, char** argv);

#define VLOG(...) do { if ( Option_Verbose ) { printf(__VA_ARGS__); } } while ( false )

#endif // BOOTSTRAP_OPTIONS_H
