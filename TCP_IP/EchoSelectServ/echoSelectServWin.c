#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<WinSock2.h>

#define BUF_SIZE 1024
void ErrorHandling(char* message);

int main(int argc, char* argv[])
{

	WSADATA wsaData;
	SOCKET hServSock, hClntSock;
	SOCKADDR_IN servAdr, clntAdr;
	// select함수 호출후에 영원히 블록킹 당하지 않게 하기 위해 타임아웃시간을 주어
	// 0을 리턴하고 강제로 빠져나오게 하기위함.
	TIMEVAL timeout;
	// 읽기셋 선언
	fd_set reads, cpyReads;

	int adrSz;
	// fdNum은 select함수 호출후에 변화가 발생한 소켓의 수를 담기 위함
	// 이값이 -1인지 0인지 양의정수인지에따라 각가 처리한다.
	int strLen, fdNum, i;
	char buf[BUF_SIZE];

	if (argc != 2)
	{
		printf("Usage : %s <port> \n", argv[0]);
		exit(1);
	}
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error!");

	hServSock = socket(PF_INET, SOCK_STREAM, 0);
	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family = AF_INET;
	servAdr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAdr.sin_port = htons(atoi(argv[1]));

	if (bind(hServSock, (SOCKADDR*)&servAdr, sizeof(servAdr)) == SOCKET_ERROR)
		ErrorHandling("bind() Error!");
	if (listen(hServSock, 5) == SOCKET_ERROR)
		ErrorHandling("listen() error");
	// FD_ZERO(fd_set* fdset)
	// 쓰기셋 초기화
	FD_ZERO(&reads);
	// 서버소켓 쓰기셋 등록
	FD_SET(hServSock, &reads);

	while (1)
	{
		// 원본 훼손을 막기 위함
		// while문을 돌때마다 cpyReads을 원본으로 초기화 해준다.(select함수를 호출완료하면 cpyReads값이 바뀌므로)
		cpyReads = reads;
		// select 함수 호출 직전에 초기화 해주어야한다 select 함수 호출 이후에 이 값은 다시 5초로 초기화 되는 것이 아니라
		// 쓰고 남아있는 시간으로 유지되기 때문 (매번 일정하게 5초간 기다리게 하기 위함)
		timeout.tv_sec = 5;
		timeout.tv_usec = 5000;
		// select함수 호출(0, 읽기셋, 쓰기셋, 예외셋, 타임아웃구조체)
		// 반환형 타임아웃시 0, 에러시 -1, 정상시 변화가 발생한 소켓의 수
		if ((fdNum = select(0, &cpyReads, 0, 0, &timeout)) == SOCKET_ERROR)
		{
			break;
		}
		else if (fdNum == 0)
		{
			// 타임아웃이 발생하면 다시 while문 처음으로 돌아가도록 함.
			continue;
		}
		else
		{
			// reads(fd_set 구조체)에 있는 fd_array배열에 소켓을 담는데
			// 이 배열의 끝을 알기위해 구조체 안에 fd_count라는 변수가 있다.
			// 즉, for문은 fd_array배열에 담긴 모든 소켓에 대하여 처리하기 위해서
			// 사용한다.
			for (i = 0; i < (int)reads.fd_count; i++)
			{
				// 변화가 생긴 소켓이 존재하면
				// 원본 배열과 카피 배열을 비교함.
				if (FD_ISSET(reads.fd_array[i], &cpyReads))
				{
					// 변화가 발생한 소켓이 서버 소켓이면
					if (reads.fd_array[i] == hServSock)
					{
						adrSz = sizeof(clntAdr);
						hClntSock = accept(hServSock, (SOCKADDR*)&clntAdr, &adrSz);
						// 클라이언트소켓을 원본 읽기셋에 추가시킴
						// 복사본(cpyReads)에 추가하면 while문 시작시 사라지므로 안됨.
						// 원본(reads)을 사용하는 경우는 FD_함수류, send, recv 함수
						// 카피을 사용하는 경우 closesocket(이유: 
						FD_SET(hClntSock, &reads);
						printf("connected client: %d \n", hClntSock);
					}
					else // 변화가 생긴 소켓이 클라이언트 소켓이면 
					{	
						strLen = recv(reads.fd_array[i], buf, BUF_SIZE - 1, 0);
						// EOD가 넘어오면 (종료신호가 오면)
						if (strLen == 0)
						{
							// 원본 배열에 소켓삭제
							FD_CLR(reads.fd_array[i], &reads);
							// 카피 배열에 소켓 닫음.
							closesocket(cpyReads.fd_array[i]);
							printf("closed client: %d \n", cpyReads.fd_array[i]);
						}
						else
						{
							send(reads.fd_array[i], buf, strLen, 0);
						}
					}

				}
			}
		}

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
