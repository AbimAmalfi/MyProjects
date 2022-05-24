
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <stdio.h>
#include <Mstcpip.h>
#include <ctime>

#pragma comment(lib, "Ws2_32.lib")


void Error_code(int x);
void StartReceiving(SOCKET s);
void TypeOfPacket(char* Buffer, int size);
void ParseIP(char* Buffer);
void ParceTcp(char* Buffer, int size);
void ParceUdp(char* Buffer, int size);

struct IP_hdr
{
	unsigned char ip_header_len : 4;
	unsigned char ip_version : 4;
	unsigned char ip_tos;
	unsigned short ip_total_length;
	unsigned short ip_id;

	unsigned char ip_frag_offset : 5;

	unsigned char ip_more_fragment : 1;
	unsigned char ip_dont_fragment : 1;
	unsigned char ip_reserved_zero : 1;

	unsigned char ip_frag_offset1;

	unsigned char ip_ttl; //время жизни
	unsigned char ip_protocol;
	unsigned short ip_checksum;
	unsigned int ip_srcaddr;
	unsigned int ip_destaddr;
} IP_HDR;

struct UDP_hdr
{
	unsigned short source_port;
	unsigned short dest_port;
	unsigned short udp_length;
	unsigned short udp_checksum;
} UDP_HDR;

struct TCP_hdr
{
	unsigned short source_port;
	unsigned short dest_port;
	unsigned int sequence;
	unsigned int acknowledge;

	unsigned char ns : 1;
	unsigned char reserved_part1 : 3;
	unsigned char data_offset : 4;

	unsigned char fin : 1; 
	unsigned char syn : 1;
	unsigned char rst : 1;
	unsigned char psh : 1;
	unsigned char ack : 1;
	unsigned char urg : 1;

	unsigned char ecn : 1;
	unsigned char cwr : 1;


	unsigned short window;
	unsigned short checksum;
	unsigned short urgent_pointer;
} TCP_HDR;

FILE* logfile;
SOCKADDR_IN source, destination;
IP_hdr* iphdr;
UDP_hdr* udphdr;
TCP_hdr* tcphdr;

int main()
{
	u_long prommode_flag = 1;
	WORD Version;
	WSADATA     wsadata;
	SOCKET s;
	int i = 0;
	Version = MAKEWORD(2, 2);
	char        hostname[32];
	struct hostent* local{};
	SOCKADDR_IN selected_address{};
	struct in_addr addr;
	int input = 0;
	int scanreturn = 0;
	
	time_t timenow = time(0);
	printf("sniffer.exe \nBoev A.Y. \n%s", ctime(&timenow));
	logfile = fopen("logfile.txt", "w");
	if (logfile == NULL) Error_code(11);
	else printf("logfile created\n");	
	fprintf(logfile, "Date of creation: %s", ctime(&timenow));

	if (WSAStartup(Version, &wsadata)) { Error_code(1); }
	else printf("WinSock started\n");

	s = socket(AF_INET, SOCK_RAW, IPPROTO_IP);
	if (s == INVALID_SOCKET) { Error_code(2); }
	else printf("RawSocket created\n");

	if (gethostname(hostname, sizeof(hostname))) { Error_code(3); }
	printf("Hostname: %s\n", hostname);
	local = gethostbyname(hostname);
	if (local == NULL) { Error_code(4); }
	for (i = 0; local->h_addr_list[i] != 0; ++i)
	{
		memcpy(&addr, local->h_addr_list[i], sizeof(in_addr));
		printf("Num: |%d| Address: %s\n", i + 1, inet_ntoa(addr));
	}
	printf("Stop by pressing ESC\n");
	printf("Enter the number you would like to start sniffing: ");
	while ((scanf(" %d", &input) == 0) || (0 >= input) || (input >= i + 1))
	{
		while ((getchar()) != '\n');
		Error_code(9);
		printf("Enter the number you would like to start sniffing: ");
	}
	


	selected_address.sin_family = AF_INET;
	selected_address.sin_port = htons(0);
	selected_address.sin_addr.s_addr = inet_addr(local->h_addr_list[input - 1]);


	if (bind(s, (struct sockaddr*)&selected_address, sizeof(selected_address)) == SOCKET_ERROR) { Error_code(5); }
	else printf("Binded succefully\n");

	if (ioctlsocket(s, SIO_RCVALL, &prommode_flag)) { Error_code(6); }
	else printf("Promisious mode on, receiving all\n");
	StartReceiving(s);
	closesocket(s);
	WSACleanup(); 
	fclose(logfile);
	printf("\nProcess stopped by user\n\n");
	system("pause");
	return 0;
}

void Error_code(int x)
{
	switch (x)
	{
	case 1:
		printf("WSAstartup failed\n");
		break;
	case 2:
		printf("RawSocket is not created\n");
		break;
	case 3:
		printf("could not get hostname\n");
		break;
	case 4:
		printf("could not get ip adresses\n");
		break;
	case 5:
		printf("Could not bind\n");
		break;
	case 6:
		printf("promiscious mode did not turn on\n");
		break;
	case 7:
		printf("memory allocation failed\n");
		break;
	case 8:
		printf("recv failed\n");
		break;
	case 9:
		printf("invalid number\n");
		break;
	case 10:
		printf("\033[31mNo packets or no connection (possibly buffer is overflowed)\033[0m\n");
		break;
	case 11:
		printf("Unable to create file\n");
		break;
	}
	printf("Error code: %d\n", WSAGetLastError());
}

void StartReceiving(SOCKET s)
{
	char Buffer[8192];
	int count;
	if (Buffer == NULL) Error_code(7);


	do
	{
		count = recv(s, Buffer, sizeof(Buffer), 0);
		if (GetAsyncKeyState(VK_ESCAPE) != 0) break;
		if (count > sizeof(IP_hdr))
		{
			TypeOfPacket(Buffer, count);
		}
		else Error_code(8);
	} while (true);

}

void TypeOfPacket(char* Buffer, int size)
{

	if (size == -1) { Error_code(10); time_t timenow = time(0); fprintf(logfile,"\nNo packets or buffer is overflowed | %.24s", ctime(&timenow)); return;}
	iphdr = (IP_hdr*)Buffer;

	ParseIP(Buffer);

	switch (iphdr->ip_protocol)
	{
	case 6: //tcp
		ParceTcp(Buffer, size);
		break;
	case 17: //udp
		ParceUdp(Buffer, size);
		break;
	}

}

void ParseIP(char* Buffer)
{
	unsigned short iphdrlen;

	iphdr = (IP_hdr*)Buffer;
	iphdrlen = iphdr->ip_header_len * 4;

	memset(&source, 0, sizeof(source));
	source.sin_addr.s_addr = iphdr->ip_srcaddr;

	memset(&destination, 0, sizeof(destination));
	destination.sin_addr.s_addr = iphdr->ip_destaddr;

/*
	printf("\n");
	printf("IP Header\n");
	printf(" IP Version : %d\n", (unsigned int)iphdr->ip_version);
	printf(" IP Header Length : %d Bytes\n", ((unsigned int)(iphdr->ip_header_len)) * 4);
	printf(" Type Of Service : %d\n", (unsigned int)iphdr->ip_tos);
	printf(" IP Total Length : %d Bytes(Size of Packet)\n", ntohs(iphdr->ip_total_length));
	printf(" Identification : %d\n", ntohs(iphdr->ip_id));
	printf(" Reserved ZERO Field : %d\n", (unsigned int)iphdr->ip_reserved_zero);
	printf(" Dont Fragment Field : %d\n", (unsigned int)iphdr->ip_dont_fragment);
	printf(" More Fragment Field : %d\n", (unsigned int)iphdr->ip_more_fragment);
	printf(" TTL : %d\n", (unsigned int)iphdr->ip_ttl);
	printf(" Protocol : %d\n", (unsigned int)iphdr->ip_protocol);
	printf(" Checksum : %d\n", ntohs(iphdr->ip_checksum));
	printf(" Source IP : %s\n", inet_ntoa(source.sin_addr));
	printf(" Destination IP : %s\n", inet_ntoa(destination.sin_addr));
*/

	time_t timenow = time(0);
	printf("\x1B[36mIP  Header received %.19s\033[0m\n", ctime(&timenow));
	fprintf(logfile, "\n");
	fprintf(logfile, "IP  Header Time: %.24s ", ctime(&timenow));
	fprintf(logfile, "| Source IP: %s ", inet_ntoa(source.sin_addr));
	fprintf(logfile, "| Destination IP: %s ", inet_ntoa(destination.sin_addr));
	fprintf(logfile, "| IP Version: %d ", (unsigned int)iphdr->ip_version);
	fprintf(logfile, "| IP Header Length: %d Bytes ", ((unsigned int)(iphdr->ip_header_len)) * 4);
	fprintf(logfile, "| Type Of Service: %d ", (unsigned int)iphdr->ip_tos);
	fprintf(logfile, "| IP Total Length: %d Bytes(Size of Packet) ", ntohs(iphdr->ip_total_length));
	fprintf(logfile, "| Identification: %d ", ntohs(iphdr->ip_id));
	fprintf(logfile, "| Reserved ZERO Field: %d ", (unsigned int)iphdr->ip_reserved_zero);
	fprintf(logfile, "| Dont Fragment Field: %d ", (unsigned int)iphdr->ip_dont_fragment);
	fprintf(logfile, "| More Fragment Field: %d ", (unsigned int)iphdr->ip_more_fragment);
	fprintf(logfile, "| TTL: %d ", (unsigned int)iphdr->ip_ttl);
	fprintf(logfile, "| Protocol: %d ", (unsigned int)iphdr->ip_protocol);
	fprintf(logfile, "| Checksum: %d ", ntohs(iphdr->ip_checksum));
}

void ParceTcp(char* Buffer, int size)
{
	unsigned short iphdrlen;
	iphdr = (IP_hdr*)Buffer;
	iphdrlen = iphdr->ip_header_len * 4;


	tcphdr = (TCP_hdr*)(Buffer + iphdrlen);

/*
	printf("\n");
	printf("TCP Header\n");
	printf(" Source Port : %u\n", ntohs(tcphdr->source_port));
	printf(" Destination Port : %u\n", ntohs(tcphdr->dest_port));
	printf(" Sequence Number : %u\n", ntohl(tcphdr->sequence));
	printf(" Acknowledge Number : %u\n", ntohl(tcphdr->acknowledge));
	printf(" Header Length : %d BYTES\n", (unsigned int)tcphdr->data_offset * 4);
	printf(" CWR Flag : %d\n", (unsigned int)tcphdr->cwr);
	printf(" ECN Flag : %d\n", (unsigned int)tcphdr->ecn);
	printf(" Urgent Flag : %d\n", (unsigned int)tcphdr->urg);
	printf(" Acknowledgement Flag : %d\n", (unsigned int)tcphdr->ack);
	printf(" Push Flag : %d\n", (unsigned int)tcphdr->psh);
	printf(" Reset Flag : %d\n", (unsigned int)tcphdr->rst);
	printf(" Synchronise Flag : %d\n", (unsigned int)tcphdr->syn);
	printf(" Finish Flag : %d\n", (unsigned int)tcphdr->fin);
	printf(" Window : %d\n", ntohs(tcphdr->window));
	printf(" Checksum : %d\n", ntohs(tcphdr->checksum));
	printf(" Urgent Pointer : %d\n", tcphdr->urgent_pointer);
*/
	time_t timenow = time(0);
	printf("\x1B[93mTCP Header received %.19s\033[0m\n", ctime(&timenow));
	fprintf(logfile, "\n");
	fprintf(logfile, "TCP Header Time: %.24s ", ctime(&timenow));
	fprintf(logfile, "| Source Port: %u ", ntohs(tcphdr->source_port));
	fprintf(logfile, "| Dest Port: %u ", ntohs(tcphdr->dest_port));
	fprintf(logfile, "| TCP Header Length: %d Bytes ", (unsigned int)tcphdr->data_offset * 4);
	fprintf(logfile, "| Sequence Number: %u ", ntohl(tcphdr->sequence));
	fprintf(logfile, "| Acknowledge Number: %u ", ntohl(tcphdr->acknowledge));
	fprintf(logfile, "| CWRflg: %d ", (unsigned int)tcphdr->cwr);
	fprintf(logfile, "| ECNflg: %d ", (unsigned int)tcphdr->ecn);
	fprintf(logfile, "| Urgflg: %d ", (unsigned int)tcphdr->urg);
	fprintf(logfile, "| Acknflg: %d ", (unsigned int)tcphdr->ack);
	fprintf(logfile, "| Pushflg: %d ", (unsigned int)tcphdr->psh);
	fprintf(logfile, "| Rstflg: %d ", (unsigned int)tcphdr->rst);
	fprintf(logfile, "| Syncflg: %d ", (unsigned int)tcphdr->syn);
	fprintf(logfile, "| Finflg: %d ", (unsigned int)tcphdr->fin);
	fprintf(logfile, "| Window: %d ", ntohs(tcphdr->window));
	fprintf(logfile, "| Urgent Pointer: %d ", tcphdr->urgent_pointer);
	fprintf(logfile, "| Checksum: %d", ntohs(tcphdr->checksum));

}

void ParceUdp(char* Buffer, int size)
{
	unsigned short iphdrlen;
	iphdr = (IP_hdr*)Buffer;
	iphdrlen = iphdr->ip_header_len * 4;
	udphdr = (UDP_hdr*)(Buffer + iphdrlen);

/*
	printf("\nUDP Header\n");
	printf(" Source Port : %d\n", ntohs(udphdr->source_port));
	printf(" Destination Port : %d\n", ntohs(udphdr->dest_port));
	printf(" UDP Length : %d\n", ntohs(udphdr->udp_length));
	printf(" UDP Checksum : %d\n", ntohs(udphdr->udp_checksum));
*/
	time_t timenow = time(0);
	printf("\x1B[32mUDP Header received %.19s\033[0m\n", ctime(&timenow));
	fprintf(logfile, "\n");
	fprintf(logfile, "UDP Header Time: %.24s ", ctime(&timenow));
	fprintf(logfile, "| Source Port: %d ", ntohs(udphdr->source_port));
	fprintf(logfile, "| Dest Port: %d ", ntohs(udphdr->dest_port));
	fprintf(logfile, "| UDP Header Length: %d ", ntohs(udphdr->udp_length));
	fprintf(logfile, "| UDP Checksum : %d", ntohs(udphdr->udp_checksum));
	
}



