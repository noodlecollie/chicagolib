#include <stdio.h>
#include <stdlib.h>
#include <bios.h>

static void PrintStatus(unsigned short status)
{
	if ( (status & 0xFF00) == 0 )
	{
		printf(" <none>\n");
		return;
	}

	if ( status & (1 << 15) )
	{
		printf(" timed out\n");
	}

	if ( status & (1 << 14) )
	{
		printf(" transmit shift register empty\n");
	}

	if ( status & (1 << 13) )
	{
		printf(" transmit holding register empty\n");
	}

	if ( status & (1 << 12) )
	{
		printf(" break detected\n");
	}

	if ( status & (1 << 11) )
	{
		printf(" framing error\n");
	}

	if ( status & (1 << 10) )
	{
		printf(" parity error\n");
	}

	if ( status & (1 << 9) )
	{
		printf(" overrun error\n");
	}

	if ( status & (1 << 8) )
	{
		printf(" data ready\n");
	}
}

static void PrintInitStatus(unsigned short status)
{
	if ( (status & 0x00FF) == 0 )
	{
		printf(" <none>\n");
		return;
	}

	if ( status & (1 << 7) )
	{
		printf(" receive-line signal detected\n");
	}

	if ( status & (1 << 6) )
	{
		printf(" Ring indicator\n");
	}

	if ( status & (1 << 5) )
	{
		printf(" Data-set ready\n");
	}

	if ( status & (1 << 4) )
	{
		printf(" Clear to send\n");
	}

	if ( status & (1 << 3) )
	{
		printf(" Receive line signal detector changed\n");
	}

	if ( status & (1 << 2) )
	{
		printf(" Trailing-edge ring detector\n");
	}

	if ( status & (1 << 1) )
	{
		printf(" Data set ready changed\n");
	}

	if ( status & (1 << 0) )
	{
		printf(" Clear to send changed\n");
	}
}

int main(int argc, char** argv)
{
	int portNumber;
	unsigned short ret;
	int iteration;

	if ( argc != 2 )
	{
		fprintf(stderr, "Usage: srtx <port number>\n");
		return 1;
	}

	portNumber = atoi(argv[1]);

	if ( portNumber < 1 )
	{
		fprintf(stderr, "Serial port number must be at least 1.\n");
		return 1;
	}

	printf("Opening serial comms on port COM%d\n", portNumber);
	ret = _bios_serialcom(_COM_INIT, portNumber - 1, _COM_9600 | _COM_NOPARITY | _COM_STOP1 | _COM_CHR8);
	printf("Result: 0x%04x\n", ret);

	printf("General status:\n");
	PrintStatus(ret);

	printf("Init status:\n");
	PrintInitStatus(ret);

	printf("Writing...\n");

	for ( iteration = 0; iteration < 26; ++iteration )
	{
		int ctsAttempts = 0;

		ret = _bios_serialcom(_COM_SEND, portNumber - 1, 'A' + (char)iteration);

		if ( ret & (1 << 15) )
		{
			fprintf(stderr, "Failed to write '%c' to serial port.\n", ret & 0xFF);
		}

		do
		{
			printf("Checking CTS...\n");
			ret = _bios_serialcom(_COM_STATUS, portNumber - 1, 0);
			++ctsAttempts;
		}
		while ( !(ret & (1 << 4)) && ctsAttempts < 20 );

		if ( ctsAttempts >= 20 )
		{
			fprintf(stderr, "Timed out while waiting for CTS.\n");
			break;
		}

		printf("Continuing.\n");
	}

	printf("Finished.\n");
	return 0;
}
