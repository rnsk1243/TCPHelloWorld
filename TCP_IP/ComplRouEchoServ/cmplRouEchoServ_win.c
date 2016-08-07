#include<stdio.h>
#include<stdlib.h>
#include<WinSock2.h>

#define BUF_SIZE 1024

void CALLBACK ReadCompRoutine(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
void CALLBACK WriteCompRoutine(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
void ErrorHandling(char* message);

typedef struct
{
	SOCKET hClntSock;
	char buf[BUF_SIZE];
	WSABUF wsaBuf;
} PER_IO_DATA, *LPPER_IO_DATA;

int main(int argc, char* argv[])
{
	WSADATA wsaData;
	SOCKET hLisnSock, hRecvSock;
	SOCKADDR_IN lisnAdr, recvAdr;
	// WSAOVERLAPPED의 포인터형태가 LPWSAOVERLAPPED 이거임
	LPWSAOVERLAPPED lpOvLp;
	DWORD recvBytes;
	LPPER_IO_DATA hbInfo;
	int mode = 1, recvAdrSz, flagInfo = 0;

	if (argc != 2) {
		printf("Usage: %s <IP> <port> \n", argv[0]);
		exit(1);
	}
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		ErrorHandling("WSAStartup() error!");
	}

	hLisnSock = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	// non-blocking 소켓 만들기
	// ioctlsocket(대상소켓, 모드종류, 모드값);
	ioctlsocket(hLisnSock, FIONBIO, &mode);

	memset(&lisnAdr, 0, sizeof(lisnAdr));
	lisnAdr.sin_family = AF_INET;
	lisnAdr.sin_addr.s_addr = htonl(INADDR_ANY);
	lisnAdr.sin_port = htons(atoi(argv[1]));

	if (bind(hLisnSock, (SOCKADDR*)&lisnAdr, sizeof(lisnAdr)) == SOCKET_ERROR)
		ErrorHandling("bind() error");
	if (listen(hLisnSock, 5) == SOCKET_ERROR)
		ErrorHandling("listen() Error");

	recvAdrSz = sizeof(recvAdr);
	while (1)
	{
		// Completion Routine을 호출하기 위해 alertable wait상태로 만듬
		// alertable wait상태는 운영체제가 전달하는 메시지의 수신을 대기하는 쓰레드의 상태임
		// 이것은 매우 중요한 작업진행중에 Completion Routine을 호출해버리면 중요한 작업을 완료할 수 없으니까
		// alertable wait상태일때만 Completion Routine을 호출하도록 만들어짐
		puts("하핫!");
		SleepEx(100, TRUE);
		// hLisnSock이 넌블록킹 소켓이므로 새로 생성되는 소켓 역시 넌-블로킹 소켓이된다.
		hRecvSock = accept(hLisnSock, (SOCKADDR*)&recvAdr, &recvAdrSz);
		// 만약 클라이언트의 연결요청이 존재하지 않는 상태에서 accept함수가 호출되면
		// hRecvSock는 넌블럭소켓이므로 곧바로 INVALID_SOCKET가 반환 된다.
		if (hRecvSock == INVALID_SOCKET)
		{
			// 이어서 에러확인 함수를 호출하면 WSAEWOULDBLOCK이 찍히는데 이런것은
			// 걍 while문 처음부터 다시 시작하라고 한다.
			// while문이 무한 반복이고 넌블록 소켓이므로 연결요청없이도 accept함수를 바로 빠져나오므로 이러한
			// 방법을 쓰나 봅니다.
			if (WSAGetLastError() == WSAEWOULDBLOCK)
			{
				puts("연결요청없었어...");
				// 클라이언트의 연결요청이 없었던거로군... 다시 while문 처음으로
				continue;
			}else
			{
				ErrorHandling("accept() error");
			}
		}
		// 클라이언트의 연결요청이 있었던거
		puts("Client connected......");
		// Overlapped IO에 필요한(WSASend, WSARecv함수에 필요) WSAOVERLAPPED구조체 메모리 할당
		// 반복문 안에서하는 이유는 클라이언트 하나당 새로운 WSAOVERLAPPED구조체가 필요하기 때문
		lpOvLp = (LPWSAOVERLAPPED)malloc(sizeof(WSAOVERLAPPED));
		// 구조체 초기화
		memset(lpOvLp, 0, sizeof(WSAOVERLAPPED));

		// 보낼정보가 담긴 구조체 메모리 공간얻기 (새로 연결요청되면 늘려줘야함)
		// 이러므로서 구조체 안에있는 버퍼와 버퍼정보도 새롭게 만들어지고 연결되는 소켓이 사용하게 된다.
		// 한번당 PER_IO_DATA 만큼씩
		hbInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
		// accept로인해 만들어진 소켓으로 초기화(연결되는 소켓)
		hbInfo->hClntSock = (DWORD)hRecvSock;
		// 버퍼 초기화
		(hbInfo->wsaBuf).buf = hbInfo->buf;
		// 사이즈 초기화
		(hbInfo->wsaBuf).len = BUF_SIZE;

		// Completion Routine기반의 Overlapped IO에서는 Event오브젝트가 불필요하기 때문에 hEvent에 필요한 다른 정보를 채움
		// Completion Routine의 3번째 인자로부터 입출력이 완료된 소켓(보낼주소,보낼내용,크기 등)을 얻기위해
		// 이렇게 쓸모가 없는 hEvent변수에 저장해준다.(Event오브젝트와 Completion Routine은 같이 사용하지 못할듯.)
		// 따라서 ReadCompRoutine의 3번째 인자로 소켓정보를 얻어 초기화하는 모슬 을 볼 수 있을 것이다.
		lpOvLp->hEvent = (HANDLE)hbInfo;
		// WSARecv을 호출함으로서 ReadCompRoutine함수(마지막 매개변수)를 Completion Routine으로 지정함
		// 여기서 6번째 인자로 전달한 WSAOVERLAPPED구조체 변수의 주소값은 Completion Routine의 3번째 매개변수에 전달됨.
		// 즉, ReadCompRoutine함수의 세번째 인자에 전달됨.
		// 따라서  Completion Routine함수는 입출력이 완료된 소켓의 핸들과 버퍼에 접근할 수 있다.
		// SleepEx(100, TRUE);을 반복호출 통해서 Completion Routine을 실행할 수 있다
		WSARecv(hRecvSock, &(hbInfo->wsaBuf), 1, &recvBytes, &flagInfo, lpOvLp, ReadCompRoutine);
	}
	closesocket(hRecvSock);
	closesocket(hLisnSock);
	WSACleanup();
	return 0;
}

// WSARecv의 마지막 인자로 등록된 이 함수가 호출되었다는 것은 데이터 입력이 완료되었다는 뜻 
// 즉, WSARecv함수가 모든 데이터를 백그라운드에서든지 해서 전부 가져왔다는 것.
void CALLBACK ReadCompRoutine(DWORD dwError, DWORD szRecvBytes, LPWSAOVERLAPPED lpOverlapped, DWORD flags)
{
	puts("ReadCompRoutine 호출");
	// 3번째 매개변수로 소켓과 보낼데이터 구조체 가져오기
	//(메인 함수에서 미리 저장해두었기때문에 가져올 수 있었다.)
	LPPER_IO_DATA hbInfo = (LPPER_IO_DATA)(lpOverlapped->hEvent);
	// 구조체에서 사용편하게끔 소켓 꺼내기
	SOCKET hSock = hbInfo->hClntSock;
	// 버퍼구조체 꺼내기
	LPWSABUF bufInfo = &(hbInfo->wsaBuf);
	// 보낸 바이트수
	DWORD sentBytes;

	// 만약 받은 데이터 수가 0이면(EOF수신으로 종료를 받음)
	if (szRecvBytes == 0)
	{
		// 소켓 종료
		closesocket(hSock);
		// 메모리 해제
		free(lpOverlapped->hEvent);
		free(lpOverlapped);
		puts("Client disconnected......");
	}
	else// 받은 데이터 수가 0보다 크면 에코
	{
		// 받은 데이터수 초기화
		bufInfo->len = szRecvBytes;
		// 에코! 클라이언트로 메시지 전송하는데 전송이 완료되면 WriteCompRoutine함수를 호출해라
		WSASend(hSock, bufInfo, 1, &sentBytes, 0, lpOverlapped, WriteCompRoutine);
	}

}
// WSASend함수가 모든 데이터를 전송완료하면 다음 함수가 호출된다. 
void CALLBACK WriteCompRoutine(DWORD dwError, DWORD szSendBytes, LPWSAOVERLAPPED lpOverlapped, DWORD flags)
{
	puts("WriteCompRoutine 호출");
	LPPER_IO_DATA hbInfo = (LPPER_IO_DATA)(lpOverlapped->hEvent);
	SOCKET hSock = hbInfo->hClntSock;
	LPWSABUF bufInfo = &hbInfo->wsaBuf;
	DWORD recvBytes;
	int flagInfo = 0;
	// 클라이언트로부터 온 메시지 수신
	WSARecv(hbInfo, bufInfo, 1, &recvBytes, &flagInfo, lpOverlapped, ReadCompRoutine);
	puts("끼에에에에에엑");

}

void ErrorHandling(char* msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}