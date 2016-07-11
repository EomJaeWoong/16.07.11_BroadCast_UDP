#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

void err_display(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("Error[%s] : %s\n", msg, (LPCTSTR)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

int main()
{
	int retval;
	int iPort = 19001;

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return -1;

	SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == INVALID_SOCKET)		err_display("socket()");

	BOOL bEnable = TRUE;
	retval = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char *)&bEnable, sizeof(bEnable));
	if (retval == SOCKET_ERROR)	err_display("setsockopt()");

	SOCKADDR_IN sockaddr;
	memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);

	char cDatagram[10] = { 0xff, 0xee, 0xdd, 0xaa, 0x00, 0x99, 0x77, 0x55, 0x33, 0x11 };
	WCHAR wcBuf[20];
	WCHAR addr[16];

	printf("Name       IP        Port\n");

	while (1)
	{
		if (iPort > 19100)	break;
		sockaddr.sin_port = htons(iPort);

		retval = sendto(sock, cDatagram, sizeof(cDatagram), 0, (SOCKADDR *)&sockaddr,
			sizeof(sockaddr));

		FD_SET ReadSet;
		FD_ZERO(&ReadSet);
		FD_SET(sock, &ReadSet);

		TIMEVAL Timeval;
		Timeval.tv_sec = 0;
		Timeval.tv_usec = 200000; // second¡ª ∫∏±‚

		retval = select(0, &ReadSet, NULL, NULL, &Timeval);
		if (retval < 0) err_display("select()");

		else if (retval > 0)
		{
			if (FD_ISSET(sock, &ReadSet))
			{
				memset(&wcBuf, 0, sizeof(wcBuf));

				int addrlen = sizeof(sockaddr);
				retval = recvfrom(sock, (char*)wcBuf, sizeof(wcBuf), 0, (SOCKADDR *)&sockaddr,
					&addrlen);

				for (int iCnt = 0; iCnt < sizeof(wcBuf); iCnt++)
				{
					if (wcBuf[iCnt] == 0)	wcBuf[iCnt] = '\0';
					break;
				}

				if (retval == SOCKET_ERROR)	err_display("recvfrom()");

				wprintf(L"%s  ", wcBuf);
				InetNtop(AF_INET, &sockaddr.sin_port, addr, sizeof(addr));
				wprintf(L"%s ", addr);
				printf("%d", iPort);
				printf("\n");
			
			}
		}
		iPort++;
	}
}