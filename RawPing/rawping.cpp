#pragma comment(lib, "ws2_32.lib")

#include <winsock2.h>
#include <stdio.h>
#include <string.h>


void exit_handler();

unsigned short checksum(unsigned short*, int);

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		printf("Usage: %s <host>\n", argv[0]);
		return -1;
	}
	
	atexit(exit_handler);

	char target[128] = "";
	strcpy(target, argv[1]);
}

void exit_handler()
{
	WSACleanup();
}

unsigned short checksum(unsigned short* buff, int len)
{
	unsigned long sum = 0L;
	while (len > 1)
	{
		sum += *buff++;
		len -= sizeof(unsigned short);
	}

	if (len)
	{
		sum += *(unsigned char*)buff;
	}

	sum = (sum >> 16) + (sum & 0xffff);
	sum += sum >> 16;
	return (unsigned short)(~sum);
}