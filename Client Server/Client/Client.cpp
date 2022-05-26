#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <stdio.h>
#include <ctime>
#include <sstream>
#pragma comment(lib, "Ws2_32.lib") //комментарий линкеру

int Error_code(int x);
int main()
{
	time_t timenow = time(0);
	printf("Client.exe \nBoev A.Y. \n%s", ctime(&timenow));

	WSADATA     wsadata;
	WORD Version;
	Version = MAKEWORD(2, 2);
	SOCKADDR_IN TCPaddress{};
	TCPaddress.sin_addr.s_addr = inet_addr("127.0.0.1");
	TCPaddress.sin_family = AF_INET;
	TCPaddress.sin_port = htons(5555);

	if (WSAStartup(Version, &wsadata)) { Error_code(1); }
	else printf("WinSock started\n");

	SOCKET TCPsocket;
	TCPsocket = socket(AF_INET, SOCK_STREAM, NULL);
	if (TCPsocket == INVALID_SOCKET) { Error_code(2); }
	else printf("TCPSocket created\n");

	printf("Trying to connect...\n");
	if (connect(TCPsocket, (SOCKADDR*)&TCPaddress, sizeof(TCPaddress))) { Error_code(4); }
	else printf("Connected to a server\n");

	char port[5];
	char filename[25];
	char message[30];

	printf("Write desired port:");
	if (scanf("%s", &port) == 0) { Error_code(10); }
	printf("Write file name:");
	if (scanf("%s", &filename) == 0) { Error_code(10); }
	strcpy(message, port);
	strcat(message, " ");
	strcat(message, filename);
	strcat(message, ".txt");
	if (send(TCPsocket, message, 40, 0) == SOCKET_ERROR) { Error_code(5); }
	char confirmation[40];
	if (recv(TCPsocket, confirmation, 40, 0) == SOCKET_ERROR) { Error_code(6); }
	printf("%s", confirmation);
	u_short uport;
	std::stringstream str(message);
	str >> uport;
	SOCKET UDPsocket;
	UDPsocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (UDPsocket == INVALID_SOCKET) { Error_code(7); }
	else printf("UDPSocket created\n");
	SOCKADDR_IN UDPaddress{};

	
	int sizeofaddr = sizeof(UDPaddress);
	UDPaddress.sin_addr.s_addr = inet_addr("127.0.0.1");
	UDPaddress.sin_family = AF_INET;
	UDPaddress.sin_port = htons(0);
	if (bind(UDPsocket, (SOCKADDR*)&UDPaddress, sizeof(UDPaddress)) == SOCKET_ERROR) { Error_code(8); }
	else printf("UDPSocket binded\n");
	UDPaddress.sin_port = htons(uport);

	int tm = 500;
	unsigned int  sz = sizeof(tm);
	if (setsockopt(TCPsocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&tm, sz)) { Error_code(3); }
	else printf("recv timeout set\n");

	FILE* input_file = fopen("logfile.txt", "r");
	memset(confirmation, 0, sizeof(confirmation));

	if (input_file != 0)
	{
		char buffer[1032];
		char idbuffer[1036];
		int truesize;
		int i = 1;
		while ((truesize = fread(buffer, 1, 1024, input_file)) != 0)
		{
			sprintf(idbuffer, "%d ", i);
			strncat(idbuffer, buffer, truesize);
			idbuffer[1026] = '\0';

			sendto(UDPsocket, (char*)idbuffer, truesize + 12, 0, (SOCKADDR*)&UDPaddress, sizeof(UDPaddress));
			printf("Packet %d sent\n",i);
			while ((recv(TCPsocket, confirmation, 40, 0))<=0)
			{
				printf("No confirmation\n");
				if (sendto(UDPsocket, (char*)idbuffer, truesize + 12, 0, (SOCKADDR*)&UDPaddress, sizeof(UDPaddress)) == SOCKET_ERROR) { Error_code(9); }
				else printf("Packet %d resent\n",i);
			}
			

			while (strcmp(confirmation, "Server:badpacket") == 0)
			{
				if (sendto(UDPsocket, (char*)idbuffer, truesize + 12, 0, (SOCKADDR*)&UDPaddress, sizeof(UDPaddress)) == SOCKET_ERROR) { Error_code(9); }
				printf("%s\n", confirmation);
			}

			printf("%s\n", confirmation);
			memset(confirmation, 0, sizeof(confirmation));
			memset(buffer, 0, sizeof(buffer));
			i++;


		}
		if (sendto(UDPsocket, "done\n", 6, 0, (SOCKADDR*)&UDPaddress, sizeof(UDPaddress)) == SOCKET_ERROR) { Error_code(9); }
		else printf("\n\nServer notified, process terminated\n");

	}



	closesocket(TCPsocket);
	closesocket(UDPsocket);
	WSACleanup();

	return 0;
}

int Error_code(int x)
{
	switch (x)
	{
	case 1:
		printf("WSAstartup failed\n");
		break;
	case 2:
		printf("TCPSocket is not created\n");
		break;
	case 3:
		printf("Timeout set failed\n");
		break;
	case 4:
		printf("Could not connect\n");
		break;
	case 5:
		printf("Could not send\n");
		break;
	case 6:
		printf("recv failed\n");
		break;
	case 7:
		printf("UDPSocket is not created\n");
		break;
	case 8:
		printf("UDPSocket could not bind\n");
		break;
	case 9:
		printf("sendto failed\n");
		break;
	case 10:
		printf("invalid data input\n");
		break;
	}
	printf("Error code: %d\n", WSAGetLastError());
	WSACleanup();
	return 0;
}
