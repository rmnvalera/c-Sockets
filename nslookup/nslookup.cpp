// nslookup.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

void handle_he_ip(const char*, int);
void exit_handler();

int main(int argc, char* argv[])
{
	//Atach exit handler
	//WARNING!!! Use only exit function to finish program
	atexit(exit_handler);

	char domain_name[256];
	memset(domain_name, 0, sizeof domain_name);

	//Check command line argument
	//For example, using this program as 'nslookup www.domain_name.com'
	if (argc >= 2) {
		strcpy(domain_name, argv[1]);
	}
	else {
		printf("Enter domain name: ");
		scanf("%s", domain_name);
	}

	//Init WinSock2
	WSAData ws;
	if (WSAStartup(MAKEWORD(2, 2), &ws)) {
		printf("%s\n", "Error init of WinSock2");
		return -1;
	}
	//Retrieve DNS records
	hostent* he = gethostbyname(domain_name);
	if (he == NULL) {
		printf("%s\n", "Error DNS record retrieve");
		return -1;
	}
	//Loop ip list
	for (char** pa = he->h_addr_list; *pa != NULL; ++pa) {
		handle_he_ip(*pa, he->h_length);
	}

	return 0;
}

void handle_he_ip(const char* pa, int len)
{
	in_addr addr;
	memcpy(&addr, pa, len);
	printf("%s\n", inet_ntoa(addr));
}

void exit_handler()
{
	WSACleanup();
	printf("%s\n", "Program finished and release all resources");
}