#include <stdio.h>

static char LineBuffer[32];

int main(int argc, char** argv)
{
	const char* path = NULL;
	FILE* inFile = NULL;

	if ( argc != 2 )
	{
		fprintf(stderr, "sdcat - Displays contents of files in 'secret' DOS drives\n");
		fprintf(stderr, "Usage: sdcat <path>\n");
		return 1;
	}

	path = argv[1];

	inFile = fopen(path, "r");

	if ( !inFile )
	{
		fprintf(stderr, "Could not open %s\n", path);
		return 1;
	}

	while ( !feof(inFile) )
	{
		size_t bytesRead = 0;

		bytesRead = fread(LineBuffer, 1, sizeof(LineBuffer) - 1, inFile);
		LineBuffer[bytesRead] = '\0';

		printf("%s", LineBuffer);
	}

	printf("\n");

	fclose(inFile);
	return 0;
}
