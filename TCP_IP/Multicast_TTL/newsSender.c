#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<WinSock2.h>
#include<WS2tcpip.h>

// 얼마나 많이 라우터를 통과할 건가?(클수록 트래픽 증가)
#define TTL 64
#define BUF_SIZE 30

void ErrorHandling(char* message);

int main(int argc, char* argv[])
{
	WSADATA wsaData;
	SOCKET hSendSock;
	SOCKADDR_IN mulAdr;
	int timeLive = TTL;
	FILE* fp;
	char buf[BUF_SIZE];

	if (argc != 3)
	{
		exit(1);
	}
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		ErrorHandling("WSAStartup() error!");
	}
	hSendSock = socket(PF_INET, SOCK_DGRAM, 0);
	memset(&mulAdr, 0, sizeof(mulAdr));
	mulAdr.sin_family = AF_INET;
	// 멀티캐스트 전용 ip를 224.0.0.0~239 사용
	// Receiver소켓에서 이 ip로 그룹에 가입하게 된다.
	mulAdr.sin_addr.S_un.S_addr = inet_addr(argv[1]);
	mulAdr.sin_port = htons(atoi(argv[2]));

	// TTL 옵션 지정
	setsockopt(hSendSock, IPPROTO_IP, IP_MULTICAST_TTL, (void*)&timeLive, sizeof(timeLive));
	if ((fp = fopen("news.txt", "r")) == NULL)
		ErrorHandling("fopen() error");
	while (!feof(fp))
	{
		fgets(buf, BUF_SIZE, fp);
		// 멀티캐스트 ip와 포트번호를 지정해준 구조체(목적지정보=mulAdr)로 데이터를 보낸다.
		// 이 데이터를 받으려면 위에서 지정해준 멀티캐스트 ip와 포트번호로 bind된 소켓이 recv해야한다.
		// 즉, 그룹에 가입하거나 목적지정보로 bind된 소켓이여야 데이터를 받을 수 있다.
		sendto(hSendSock, buf, strlen(buf), 0, (SOCKADDR*)&mulAdr, sizeof(mulAdr));
		Sleep(2000);
	}
	closesocket(hSendSock);
	WSACleanup();
	return 0;
}

void ErrorHandling(char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}