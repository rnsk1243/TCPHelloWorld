#include<stdio.h>
#include<stdlib.h>
#include<WinSock2.h>

#define BUF_SIZE 1024
void CALLBACK CompRoutine(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
WSABUF dataBuf;
void ErrorHandling(char* msg);
int recvBytes = 0;
	char buf[BUF_SIZE];

int main(int argc, char* argv[])
{
	WSADATA wsaData;
	SOCKET hLisnSock, hRecvSock;
	SOCKADDR_IN lisnAdr, recvAdr;
	int recvAdrSz, idx;


	WSAEVENT evObj;
	WSAOVERLAPPED overlapped;

	int flags = 0;
	if (argc != 2) {
		printf("Usage: %s <IP> <port> \n", argv[0]);
		exit(1);
	}
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		ErrorHandling("WSAStartup() error!");
	}

	hLisnSock = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	memset(&lisnAdr, 0, sizeof(lisnAdr));
	lisnAdr.sin_family = AF_INET;
	lisnAdr.sin_addr.s_addr = htonl(INADDR_ANY);
	lisnAdr.sin_port = htons(atoi(argv[1]));

	if (bind(hLisnSock, (SOCKADDR*)&lisnAdr, sizeof(lisnAdr)) == SOCKET_ERROR)
	{
		ErrorHandling("bind() error");
	}
	if (listen(hLisnSock, 5) == SOCKET_ERROR)
		ErrorHandling("Listen() error");

	recvAdrSz = sizeof(recvAdr);
	hRecvSock = accept(hLisnSock, (SOCKADDR*)&recvAdr, &recvAdrSz);

	memset(&overlapped, 0, sizeof(overlapped));
	dataBuf.len = BUF_SIZE;
	dataBuf.buf = buf;
	evObj = WSACreateEvent();

	if (WSARecv(hRecvSock, &dataBuf, 1, &recvBytes, &flags, &overlapped, CompRoutine) == SOCKET_ERROR)
	{
		if (WSAGetLastError() == WSA_IO_PENDING)
		{
			puts("Background data receive");
			//WSAWaitForMultipleEvents(1, &evObj, TRUE, WSA_INFINITE, FALSE);
			//WSAGetOverlappedResult(hRecvSock, &overlapped, &recvBytes, FALSE, NULL);
		}
	}

	idx = WSAWaitForMultipleEvents(1, &evObj, FALSE, WSA_INFINITE, TRUE);

	if (idx == WAIT_IO_COMPLETION)
		puts("Overlapped I/O Comleted");
	else
		ErrorHandling("WSARecv() error");

	printf("Received message: %s \n", buf);
	WSACloseEvent(evObj);
	closesocket(hRecvSock);
	closesocket(hLisnSock);
	WSACleanup();
	return 0;
}

void CALLBACK CompRoutine(DWORD dwError, DWORD szRecvBytes, LPWSAOVERLAPPED lpOverlapped, DWORD flags)
{
	if (dwError != 0)
	{
		ErrorHandling("CompRoutine error");
	}
	else
	{
		recvBytes = szRecvBytes;
		printf("Received message: %s \n", buf);
	}
}

void ErrorHandling(char* msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}