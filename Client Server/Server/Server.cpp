
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
	printf("Server.exe \nBoev A.Y. \n%s", ctime(&timenow));

	WSADATA     wsadata;
	WORD Version;
	Version = MAKEWORD(2, 2);
	SOCKADDR_IN address{},udp_address{};
	int sizeofaddr = sizeof(address);
	address.sin_addr.s_addr = inet_addr("127.0.0.1");
	address.sin_family = AF_INET;
	address.sin_port = htons(5555);

	if (WSAStartup(Version, &wsadata)) { Error_code(1); }
	else printf("WinSock started\n");

	SOCKET ListenTCPsocket;
	ListenTCPsocket = socket(AF_INET, SOCK_STREAM, NULL);
	if (ListenTCPsocket == INVALID_SOCKET) { Error_code(2); }
	else printf("TCPSocket created\n");

	if (bind(ListenTCPsocket, (SOCKADDR*)&address, sizeof(address)) == SOCKET_ERROR) { Error_code(3); }
	else printf("Binded succefully\n");
	bool port_filename_confirmation = true;
	char port_and_filename[30];
	char name[25];
	while (true)
	{
		if (listen(ListenTCPsocket, 2) == SOCKET_ERROR) { Error_code(4); }
		else printf("Listening...\n");

		SOCKET Connection;
		if ((Connection = accept(ListenTCPsocket, (SOCKADDR*)&address, &sizeofaddr)) == INVALID_SOCKET) { Error_code(5); }
		else printf("Client connected\n");

		recv(Connection, port_and_filename, 30, 0);
		u_short port;
		std::stringstream str(port_and_filename);
		str >> port;
		str >> name;
		printf("Received udpport: %hu\n", port);
		printf("Received name: %s\n", name);
		FILE* output_file = fopen(name, "w");
		
		SOCKET UDPsocket = socket(AF_INET, SOCK_DGRAM, 0);
		int sizeofaddr = sizeof(udp_address);
		udp_address.sin_addr.s_addr = inet_addr("127.0.0.1");
		udp_address.sin_family = AF_INET;
		udp_address.sin_port = htons(port);
		bind(UDPsocket, (SOCKADDR*)&udp_address, sizeof(udp_address));


		char data[2048];
		char* bigdata = new char[10485760];
		int msg_lenght;
		int output_lenght=0;
		bool strnflag = true;
		char id[1];
		char response[40];
		if (port_filename_confirmation) send(Connection, "Server:received port and filename\n", 40, 0);
		port_filename_confirmation = false;
		while(true)
		{
			msg_lenght = recvfrom(UDPsocket, data, 2048, 0, (SOCKADDR*)&udp_address, &sizeofaddr);
		//	while ((msg_lenght = recvfrom(UDPsocket, data, 2048, 0, (SOCKADDR*)&udp_address, &sizeofaddr)) == INVALID_SOCKET)
		//	{
		//		send(Connection, "Server:badpacket", 40, 0);
		//	}
			if (strcmp(data, "done\n") == 0)
			{
				printf("Client sent all of the packages\n");
				port_filename_confirmation = true;
				break;
			}
			output_lenght = output_lenght + msg_lenght - 12;
			id[0] = data[0];
			memcpy(data, data + 2 , msg_lenght);
			if(strnflag)
			{ 
				strncpy(bigdata, data, msg_lenght);
				strnflag = false;
			}
			else strncat(bigdata, data, msg_lenght);
			//printf("%s", data);
			printf("Received package Number: %.1s\n", id);
			strcpy(response, "Server:confirmed ");
			strncat(response, id, 1);
			send(Connection, response, 40, 0);
			memset(data, NULL, sizeof(data));
			
		}
		fwrite(bigdata, 1, output_lenght, output_file);
		fclose(output_file); 
		if (send(Connection, "check", 40, 0))
		{
			printf("Client disconected\n");
		}
		else printf("Сlient still hasn't disconnected, problem\n");

		delete[] bigdata;
	}

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
		printf("Socket is not created\n");
		break;
	case 3:
		printf("could not bind\n");
		break;
	case 4:
		printf("can not listen\n");
		break;
	case 5:
		printf("Failed to accept client connection\n");
		break;
	
	}
	printf("Error code: %d\n", WSAGetLastError());
	return 0;
}


