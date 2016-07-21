#include<stdio.h>
#include<stdlib.h>
#include<WinSock2.h>
#define OPSZ 4
#define BUF_SIZE 1024

void ErrorHandling(char* message);
int calculate(int opnum, int opnds[], char oprator);

int main(int argc, char* argv[])
{
	WSADATA wsaData;
	// 소켓 디스크립터를 저장한 변수(인트값이 들어갈 것)
	SOCKET hServSock, hClntSock;
	// 서버와 클라이언트의 주소,포트정보를 담을 구조체 선언
	SOCKADDR_IN servAddr, clntAddr;
	// 클라이언트에게 받은 식
	char opinfo[BUF_SIZE];
	int result, opnd_cnt, i;
	int recvCnt, recvLen;
	int szClntAddr;
	
	int strLen;
	
	if (argc != 2)
	{
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}
	// 윈솤 2.2버전을 사용하겠다.
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		ErrorHandling("WSAStartup() error!");
	}
	// 서버소켓(문지기) 생성 IPv4사용하겠다, TCP사용하겠음!
	hServSock = socket(PF_INET, SOCK_STREAM, 0);
	if (hServSock == INVALID_SOCKET)
	{
		ErrorHandling("socket() error");
	}
	// 서버 주소와 포트번호를 담을 구조체를 우선 0으로 초기화
	memset(&servAddr, 0, sizeof(servAddr));
	// TCP사용할 것
	servAddr.sin_family = AF_INET;
	// 서버 주소 32비트 네트워크 정렬(빅엔디안)로 전환후 할당
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	// 포트번호 네트워크 정렬후 할당
	servAddr.sin_port = htons(atoi(argv[1]));
	
	// 서버주소와 포트번호가 채워진 구조체를 서버소켓에 적용시킴
	// 서버소켓디스크립터, 구조체의 주소(이때 SOCKADDR의 주소형태로 변환후 넣어준다), 구조체의 크기
	if (bind(hServSock, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR )
	{
		ErrorHandling("bind() error");
	}
	// 서버소켓이 문지기 역활을 할 수 있도록 연결요청 대기상태에 돌입하도록 함.
	// 클라이언트 연결요청큐의 갯수는 연결요청대기 크기를 나타냄
	// 문지기역할을 할 소켓(위에서 만든 서버소켓), 연결요청큐의 크기
	if (listen(hServSock, 5) == SOCKET_ERROR)
	{
		ErrorHandling("listen() error");
	}
	// 클라이언트 주소 크기 accept함수에서 제대로된 값이 초기화 된다.
	szClntAddr = sizeof(clntAddr);

	for (i = 0; i < 5; i++)
	{
		// 피연산자 갯수
		opnd_cnt = 0;
		hClntSock = accept(hServSock, (SOCKADDR*)&clntAddr, &szClntAddr);
		// 클라에서 처번째로 넣었던 연산자 갯수 받기
		recv(hClntSock, &opnd_cnt, 1, 0);
		// 현재 클라에서 온 바이트 갯수 넣을 변수
		recvLen = 0;
		// 전체 바이트 갯수 > 현재 받은 바이트 이면 아직 더 받을게 남았으므로 true로 또 recv하기위해 while문 돈다.
		// + 1 한 이유는 마지막에 넣은 연산자까지 포함 하려구
		while ((opnd_cnt*OPSZ + 1) > recvLen)
		{
			// 배열 opinfo 0번 인덱스부터 클라에서 온 숫자들과 연산자 저장
			// 피연산자 갯수만 따로 recv한 이유는 while문 조건에 쓰려구
			// recvCnt는 현재 받은 바이트를 저장 하고 recvLen에 넣어 누적시킴 
			// 이 누적시킨 값이 인덱스가 되어 다음 배열부터 opinfo에 누적 저장됨.
			recvCnt = recv(hClntSock, &opinfo[recvLen], BUF_SIZE - 1, 0);
			// 누적
			recvLen += recvCnt;
		}
		// 결과 저장
		result = calculate(opnd_cnt, (int*)opinfo, opinfo[recvLen - 1]);
		// 결과 클라에게 보냄
		send(hClntSock, (char*)&result, sizeof(result), 0);
		closesocket(hClntSock);
	}
	
	closesocket(hServSock);
	WSACleanup();
	return 0;
}

void ErrorHandling(char * message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

int calculate(int opnum, int opnds[], char oprator)
{
	int result = opnds[0], i;
	switch (oprator)
	{
	case '+':
		for (i = 1; i < opnum; i++)
			result += opnds[i];
		break;
	case '-':
		for (i = 1; i < opnum; i++)
			result -= opnds[i];
		break;
	case '*':
		for (i = 1; i < opnum; i++)
			result *= opnds[i];
		break;
	default:
		break;
	}
	return result;
}
