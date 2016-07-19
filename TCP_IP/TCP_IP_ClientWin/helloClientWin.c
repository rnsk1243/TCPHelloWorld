#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<stdio.h>
#include<stdlib.h>
#include<winsock2.h>
#define BUF_SIZE 1024

void ErrorHandling(char* message);

int main(int argc, char* argv[])
{
	WSADATA wsaData;
	// 서버접속을 하기 위한 소켓 디스크립터를 저장할 변수
	SOCKET hSocket;
	// 서버의 정보를 저장할 구조체 선언
	SOCKADDR_IN servAddr;
	// 문자열 길이를 담을 변수
	int strLen;
	// 서버에서 받은 정보를 담을 버퍼
	char message[BUF_SIZE];
	if (argc != 3) {
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}
	// 윈솤 2.2버전 사용
	if (WSAStartup((2, 2), &wsaData) != 0)
	{
		ErrorHandling("WSAStartup() error!");
	}
	// 소켓 생성
	hSocket = socket(PF_INET, SOCK_STREAM, 0);
	if (hSocket == INVALID_SOCKET)
	{
		ErrorHandling("socket() error");
	}
	// 서버의 정보를 담을 구조체를 일단 모두 0으로 초기화시킴
	memset(&servAddr, 0, sizeof(servAddr));
	// TCP사용할것을 저장
	servAddr.sin_family = AF_INET;
	// inet_addr(문자형 ip주소를담은 변수의 주소값) 함수를 호출하여 32비트 네트워크정렬(빅엔디안)해주고 그값을 대입함.
	servAddr.sin_addr.s_addr = inet_addr(argv[1]);
	// 포트(빅엔디안) 초기화
	servAddr.sin_port = htons(atoi(argv[2]));
	// 서버에 요청보내기
	// connect함수가 정상 종료되면 서버에 요청대기 상태가됨(아직 연결은 안된 상태)
	// 이때 커널에 의해 클라이언트 ip주소(호스트)와 임의로 할당된 포트정보가 클라이언트 소켓에 할당되어 만들어진다. 
	if (connect(hSocket, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
	{
		ErrorHandling("connect() error!");
	}
	else
	{
		puts("Connected............");
	}

	while (1)
	{
		fputs("Input message(Q to quit): ", stdout);
		fgets(message, BUF_SIZE, stdin);

		if (!strcmp(message, "q\n") || !strcmp(message, "Q\n"))
			break;

		send(hSocket, message, strlen(message), 0);
		strLen = recv(hSocket, message, BUF_SIZE - 1, 0);
		// 받은 문자열 마지막에 널문자 넣기 이를 위해서 바로위에서 1 빼준것
		message[strLen] = 0;
		printf("Message from server: %s", message);
	}
	closesocket(hSocket);
	WSACleanup();
	return 0;
}

void ErrorHandling(char * message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}