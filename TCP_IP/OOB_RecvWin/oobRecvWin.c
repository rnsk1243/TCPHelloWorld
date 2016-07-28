#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<stdio.h>
#include<stdlib.h>
#include<WinSock2.h>

#define BUF_SIZE 30
void ErrorHandling(char* message);

int main(int argc, char* argv[])
{
	WSADATA wsaData;
	SOCKET hAcptSock, hRecvSock;

	SOCKADDR_IN recvAdr, sendAdr;
	int sendAdrSize, strLen;
	char buf[BUF_SIZE];
	int result;

	fd_set read, except, readCopy, exceptCopy;
	struct timeval timeout;

	if (argc != 2)
	{
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		ErrorHandling("WSAStartup() error!");
	}

	hAcptSock = socket(PF_INET, SOCK_STREAM, 0);
	memset(&recvAdr, 0, sizeof(recvAdr));
	recvAdr.sin_family = AF_INET;
	recvAdr.sin_addr.s_addr = htonl(INADDR_ANY);
	recvAdr.sin_port = htons(atoi(argv[1]));

	// 서버니까 클라이언트가 하는 말을 받는 recvAdr과 hAcptSock을 연결함
	// 클라이언트가 하는 말은 전부 hAcptSock에 오도록 만듬
	if (bind(hAcptSock, (SOCKADDR*)&recvAdr, sizeof(recvAdr)) == SOCKET_ERROR)
	{
		ErrorHandling("bind() error");
	}
	if (listen(hAcptSock, 5) == SOCKET_ERROR)
	{
		ErrorHandling("listen() error");
	}

	sendAdrSize = sizeof(sendAdr);
	// 클라이언트 connect부분에서 sendAdr로 연결 요청을 하고
	// 서버는 accept부분에서 클라이언트에서 온 sendAdr과 hAcptSock을 연결시킴
	hRecvSock = accept(hAcptSock, (SOCKADDR*)&sendAdr, &sendAdrSize);
	FD_ZERO(&read);
	FD_ZERO(&except);
	// 관찰 대상으로 Recv소켓을 세팅
	// 여기는 서버정의부분이니까 듣는거에 관심이 있어
	FD_SET(hRecvSock, &read);
	FD_SET(hRecvSock, &except);
	
	while (1)
	{
		// 원본 손상 방지용
		readCopy = read;
		exceptCopy = except;
		// 블록킹 방지용 지정한 시간이 지나면 0을 리턴하고 빠져나옴(select함수에서) 
		timeout.tv_sec = 5;
		timeout.tv_usec = 0;

		result = select(0, &readCopy, 0, &exceptCopy, &timeout);
		// 반응이 있는 소켓이 있다면
		if (result > 0)
		{
			// Recv소켓에 예외셋이 발생하면, 
			// MSG_OOB을 이용해서 클라이언트가 send로 보내면 예외셋 hRecvSock 발생
			// 예외셋으로 보내진 데이터는 recv옵션으로 MSG_OOB를 줌
			if (FD_ISSET(hRecvSock, &exceptCopy))
			{
				// 
				strLen = recv(hRecvSock, buf, BUF_SIZE - 1, MSG_OOB);
				buf[strLen] = 0;
				printf("Urgent message: %s \n", buf);
			}
			// Recv소켓에 읽기셋이 발생하면
			if (FD_ISSET(hRecvSock, &readCopy))
			{
				strLen = recv(hRecvSock, buf, BUF_SIZE - 1, 0);
				// 0이면 클라이언트가 종료신호를 보낸것이므로
				if (strLen == 0)
				{
					// 종료해
					break;
					closesocket(hRecvSock);
				}else
				{
					buf[strLen] = 0;
					puts(buf);
				}
			}
		}
	}
	closesocket(hAcptSock);
	WSACleanup();
	return 0;
}

void ErrorHandling(char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}