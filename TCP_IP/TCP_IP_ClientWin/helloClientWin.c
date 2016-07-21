#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<stdlib.h>
#include<winsock2.h>
#define BUF_SIZE 1024
#define RLT_SIZE 4
#define OPSZ 4

void ErrorHandling(char* message);

int main(int argc, char* argv[])
{
	WSADATA wsaData;
	// 서버접속을 하기 위한 소켓 디스크립터를 저장할 변수
	SOCKET hSocket;
	// 서버의 정보를 저장할 구조체 선언
	SOCKADDR_IN servAddr;
	char opmsg[BUF_SIZE];
	int result, opnd_cnt, i;

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

	// 콘솔 아웃(마치 cout과 같은 역할)
	fputs("Operand count: ", stdout);
	//c언어 입력받기 (입력받아 넣을 곳의 변수주소)
	// 입력 받을 피연산자의 개수
	scanf("%d", (char*)&opnd_cnt);
	// char형으로 바꾸어 버퍼에 저장
	opmsg[0] = (char)opnd_cnt;
	// 피연산자 입력 받기
	for (i = 0; i < opnd_cnt; i++)
	{
		printf("Operand %d: ", i + 1);
		//OPSZ 만큼씩 뛰어서 저장 char 하나당 숫자 하나씩 저장하므로 OPSZ 자리수 만큼 저장하려면 
		// OPSZ만큼 건너뚜ㅣ어 이어서 저장해야한다.
		// 예 123 저장시 opmsg[0] = '1'; ... opmsg[2] = '3'; 이렇게 한다.
		// 그뒤에 456 저장 opmsg[3] 부터 4를 저장한다.  
		// 1을 더한건 위에서 피연산자의 개수 때문
		scanf("%d", (int*)&(opmsg[i*OPSZ + 1]));
	}
	// 밑에서 scanf호출하면서 문자를 입력받는데 이미 버퍼에 남아있는 \n문자의 삭제를 위함
	fgetc(stdin);// 문자열 읽는 함수
	fputs("Operator: ", stdout);
	// 연산자 입력받기 
	scanf("%c", &opmsg[opnd_cnt*OPSZ + 1]);
	// 입력받은 것을 서버에 보내기 + 2는 피연산자 갯수 숫자 1 그리고 연산자 1 해서 1+1 = 2이다. 
	send(hSocket, opmsg, opnd_cnt*OPSZ + 2, 0);
	// 서버에서 연산결과 받기
	recv(hSocket, &result, RLT_SIZE, 0);

	printf("Operation result: %d \n", result);

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