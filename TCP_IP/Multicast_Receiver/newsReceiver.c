#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<WinSock2.h>
#include<WS2tcpip.h>

#define BUF_SIZE 30

void ErrorHandling(char* message);

int main(int argc, char* argv[])
{
	WSADATA wsaData;
	SOCKET hRecvSock;
	SOCKADDR_IN adr;
	// 멀티캐스트 그룹에 가입하기 위해 소켓옵션에 들어갈 구조체 선언
	struct ip_mreq joinAdr;
	char buf[BUF_SIZE];
	int strLen;

	if (argc != 3)
	{
		exit(1);
	}
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		ErrorHandling("WSAStartup() error!");
	}
	// UDP소켓 사용
	hRecvSock = socket(PF_INET, SOCK_DGRAM, 0);
	// 소켓과 ip,port 결합을 위해 구조체 초기화
	memset(&adr, 0, sizeof(adr));
	adr.sin_family = AF_INET;
	// 듣는 소켓이므로 ip는 자기 자신으로한다.
	adr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	// 멀티캐스트 포트(sender의 포트번호)로 맞춰야하므로 argv[2]을 넣는다.
	adr.sin_port = htons(atoi(argv[2]));
	if (bind(hRecvSock, (SOCKADDR*)&adr, sizeof(adr)) == SOCKET_ERROR)
	{
		ErrorHandling("bind() error");
	}
	// 멀티캐스트 그룹에 가입 정보 초기화
	// 멀티캐스트의 ip주소 초기화 (Sender가 지정한 ip주소 입력)
	joinAdr.imr_multiaddr.S_un.S_addr = inet_addr(argv[1]);
	// 멀티캐스트에 가입하고자 하는 자의 ip주소 (나 자신)
	joinAdr.imr_interface.S_un.S_addr = htonl(INADDR_ANY);
	// 소켓옵션을 이용하여 가입.
	if (setsockopt(hRecvSock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void*)&joinAdr, sizeof(joinAdr)) == SOCKET_ERROR)
	{
		ErrorHandling("setsock() error");
	}
	while (1)
	{
		// 응답 받기 
		// 5번째 인자가 NULL인 이유는 굳이 목적지 정보를 저장할 필요가 없기때문(내가 목적지에 데이터를 보낼일도 없으니까)
		// 그리고 소켓옵션으로 그룹에 가입했으므로 별다른 조건없이 데이터를 받을 수 있음
		strLen = recvfrom(hRecvSock, buf, BUF_SIZE - 1, 0, NULL, 0);
		if (strLen < 0)
		{
			break;
		}
		buf[strLen] = 0;
		fputs(buf, stdout);
	}
	closesocket(hRecvSock);
	WSACleanup();

	return 0;
}