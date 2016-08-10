#include<stdio.h>
#include<stdlib.h>
#include<process.h>
#include<WinSock2.h>
#include<Windows.h>


#define BUF_SIZE 100
#define MAX_CLNT 256
#define READ 3
#define WRITE 5

//unsigned WINAPI HandleClnt(void* arg);
//void SendMsg(char* msg, int len);
void ErrorHandling(char* message);

// 클라이언트와 연결된 소켓 정보를 담기위한 구조체
// 이 구조체가 언제 할당되어 언제 쓰이는지 잘 살필것.
typedef struct
{
	// 클라이언트 소켓
	SOCKET hClntSock[MAX_CLNT];
	// 클라이언트 주소
	SOCKADDR_IN clntAdr;
} PER_HANDLE_DATA, *LPPER_HANDLE_DATA;

// IO에 사용되는 버퍼와 Overlapped IO에 필요한 OVERLAPPED구조체 변수를 정의함
typedef struct
{
	// WSASend, WSARecv에 사용할 오버랩트 구조체
	OVERLAPPED overlapped;
	// 보낼 데이터와 길이가 저장된 구조체
	WSABUF wsaBuf;
	// 보낼 데이터
	char buffer[BUF_SIZE];
	// 데이터를 보냈는지 받았는지 표시하기 위함
	int rwMode;
} PER_IO_DATA, *LPPER_IO_DATA; // LP가 붙으면 포인터 형식이라는 뜻 *DWORD == LPDWORD

// 클라이언트의 갯수
int clntCnt = 0;
HANDLE hMutex;

DWORD WINAPI ChatThreadMain(LPVOID CompletionPortIO);

int main(int argc, char* argv[])
{
	WSADATA wsaData;
	HANDLE hComPort;
	SYSTEM_INFO sysInfo;
	LPPER_IO_DATA ioInfo;
	LPPER_HANDLE_DATA handleInfo;
	for (int i = 0; i < 3; i++)
	{
		handleInfo = (LPPER_HANDLE_DATA)malloc(sizeof(PER_HANDLE_DATA));
	}
	
	SOCKET hServSock;
	SOCKADDR_IN servAdr;

	int recvBytes, i, flags = 0;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		ErrorHandling("WSAStartup() error!");
	}

	hMutex = CreateMutex(NULL, FALSE, NULL);

	hComPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	GetSystemInfo(&sysInfo);
	for (i = 0; i < sysInfo.dwNumberOfProcessors; i++)
	{
		_beginthreadex(NULL, 0, ChatThreadMain, (LPVOID)hComPort, 0, NULL);
	}
	hServSock = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family = AF_INET;
	servAdr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAdr.sin_port = htons(atoi(argv[1]));

	bind(hServSock, (SOCKADDR*)&servAdr, sizeof(servAdr));
	listen(hServSock, 5);

	while (1)
	{
		SOCKET hClntSock;
		SOCKADDR_IN clntAdr;
		int addrLen = sizeof(clntAdr);
		puts("연결요청 없거나 연결중....");
		hClntSock = accept(hServSock, (SOCKADDR*)&clntAdr, &addrLen);

		
		WaitForSingleObject(hMutex, INFINITE);
		handleInfo->hClntSock[clntCnt++] = hClntSock;
		ReleaseMutex(hMutex);
		memcpy(&(handleInfo->clntAdr), &clntAdr, addrLen);

		CreateIoCompletionPort((HANDLE)hClntSock, hComPort, (DWORD)handleInfo, 0);
		puts("CP연결 완");
		ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
		memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
		ioInfo->wsaBuf.len = BUF_SIZE;
		ioInfo->wsaBuf.buf = ioInfo->buffer;

		ioInfo->rwMode = READ;

		//for (int i = 0; i < clntCnt; i++)
		//{
			printf("handleInfo->hClntSock[%d] = %d \n", clntCnt, handleInfo->hClntSock[clntCnt - 1]);
			WSARecv(handleInfo->hClntSock[clntCnt - 1], &(ioInfo->wsaBuf), 1, &recvBytes, &flags, &(ioInfo->overlapped), NULL);
		//}
	}
	return 0;
}

DWORD WINAPI ChatThreadMain(LPVOID CompletionPortIO)
{
	HANDLE hComPort = (HANDLE)CompletionPortIO;
	SOCKET sock[MAX_CLNT];
	DWORD bytesTrans;
	LPPER_HANDLE_DATA handleInfo;
	LPPER_IO_DATA ioInfo;
	DWORD flags = 0;

	while (1)
	{
		puts("GetQueuedCompletionStatus.........");
		GetQueuedCompletionStatus(hComPort, &bytesTrans, (LPDWORD)&handleInfo, (LPOVERLAPPED*)&ioInfo, INFINITE);
		WaitForSingleObject(hMutex, INFINITE);
		for (int i = 0; i < clntCnt; i++)
		{
			sock[i] = handleInfo->hClntSock[i];
		}
		ReleaseMutex(hMutex);

		if (ioInfo->rwMode == READ)
		{
			// 클라이언트로부터 종료(EOF)를 전달 받았다면
			if (bytesTrans == 0)
			{
				// 클라이언트 소켓 종료, 메모리 해제
				closesocket(sock);
				free(handleInfo);
				free(ioInfo);
				// 다시 GetQueuedCompletionStatus부터 초기화하면서 다시
				continue;
			}

			memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
			// 받은 데이터 크기를 다시 초기화해줌
			ioInfo->wsaBuf.len = bytesTrans;
			// 이제 데이터를 클라이언트로 보낼거니까 보낸다는 표시해줌.
			ioInfo->rwMode = WRITE;
			// 데이터 보내기
			// LPPER_IO_DATA구조체를 보면 WSABUF과 OVERLAPPED 모두 포인터 형태로 정의되어 있지 않으므로
			// 주소를 넘기기 위해 &을 붙혀주었다.
			for (int i = 0; i < clntCnt; i++)
			{
				printf("sock[%d] = %d \n", i, sock[i]);
				printf("&(ioInfo->overlapped) = %d \n", &(ioInfo->overlapped));
				puts("보낼할꺼야");
				WSASend(sock[i], &(ioInfo->wsaBuf), 1, NULL, 0, &(ioInfo->overlapped), NULL);
				puts("보냄");
			}
			
			
			// 메시지 재전송 이후에 클라이언트가 전송하는 메시지를 수신해야한다.
			// 수신할 데이터를 저장할 메모리를 할당한다.
			ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
			memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
			ioInfo->wsaBuf.len = BUF_SIZE;
			ioInfo->wsaBuf.buf = ioInfo->buffer;
			// 데이터를 수신할 것임을 표시
			ioInfo->rwMode = READ;
			//puts("수신표시완료");
			// 데이터 수신
			for (int i = 0; i < clntCnt; i++)
			{
				WSARecv(sock[i], &(ioInfo->wsaBuf), 1, NULL, &flags, &(ioInfo->overlapped), NULL);
			}
			puts("WSARecv호출함 데이터 수인 완료를 언제 할지는 모름");
			// 클라이언트가 데이터를 보냈던 안보냈던간에 WSARecv을 호출했다. 이것은 WSARecv함수가 논블록킹 이기때문에 가능하다
			// 무한 반복하는 while문 안이기 때문에 GetQueuedCompletionStatus함수를 다시 만나는데 WSARecv함수가 데이터 수신을 완료하면
			// GetQueuedCompletionStatus함수를 빠져나올 수 있다.
			// 즉 클라이언트가 메세지를 보내야 GetQueuedCompletionStatus을 빠져나오므로 
			// 굳이 반복문을 통해 IO가 완료된 소켓을 찾을 필요가 없다.

		}
		else
		{
			// WSASend함수 호출 한 뒤라면 (수신한 데이터가 없다면)
			puts("message send");
			// IO버퍼 메모리 해제
			//free(ioInfo);
		}

	}

	return 0;
}

void ErrorHandling(char* msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}