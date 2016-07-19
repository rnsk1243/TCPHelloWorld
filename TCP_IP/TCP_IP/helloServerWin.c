#include<stdio.h>
#include<stdlib.h>
#include<WinSock2.h>
#define BUF_SIZE 1024

void ErrorHandling(char* message);

int main(int argc, char* argv[])
{
	WSADATA wsaData;
	// 소켓 디스크립터를 저장한 변수(인트값이 들어갈 것)
	SOCKET hServSock, hClntSock;
	// 서버와 클라이언트의 주소,포트정보를 담을 구조체 선언
	SOCKADDR_IN servAddr, clntAddr;

	// 클라이언트 주소 길이를 담을 변수
	int szClntAddr;
	// 클라이언트에게 보낼 메시지를 담을 변수
	char message[BUF_SIZE];
	// 보낼 문자열의 길이
	int strLen, i;
	
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
		//accept가 호출이 완료되면 클라이언트 주소길이 정보가 szClntAddr에 바이트 단위로 채워진다.
		// 연결요청이 들어와 수락할 순서가 되면 클라이언트 주소정보를 저장할 구조체에 클라이언트 주소,포트번호를 저장, 크기도 저장
		// 이때 accept함수 안에서 자동으로 새로운 소켓이 생성되고 크라이언트 소켓과 연결된다.(서버소켓은 문지기 일뿐!)
		hClntSock = accept(hServSock, (SOCKADDR*)&clntAddr, &szClntAddr);
		if (hClntSock == INVALID_SOCKET)
		{
			ErrorHandling("accept() error");
		}
		else
		{
			printf("Connected client %d \n", i + 1);
		}
		// 클라이언트로부터 받아서 message에 값 저장 한번 호출 될때마다 BUF_SIZE만큼씩 받음
		// 받은 문자열 크기를 strLen에 저장
		/*
		그런데 다음과 같이 recv와 send가 호출될 때마다 실제 입출력이 이루어 지는데 이는 문제가 있다.
		TCP는 데이터의 경계가 존재하지 않기 때문에 만약 둘 이상의 send함수에 의해 전달된 문자열 정보가 묶여서 한번에
		서버로 전송될 수 있다. 이렇게 되면 서버는 클라이언트에게 둘 이상의 문자열을 한번에 받아서 문제가 발생할 수 있다.
		*/
		while ((strLen = recv(hClntSock, message, BUF_SIZE, 0)) != 0)
		{
			// 클라이언트에게 보내기 strLen만큼
			send(hClntSock, message, strLen, 0);
		}
		// accept호출로 인해 만들어진 소켓 닫기(서비스 종료)
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
