/*
IOCP 성능이 우수한 이유
- 넌블록킹 방식으로 IO가 진행되기 때문에, IO 작업으로 인한 시간의 지연이 발생하지 않는다.
- IO가 완료된 핸들을 찾기 위해서 반복문을 구성할 필요가 없다. // 운영체제가 완료된 소켓을 알려줌
- IO의 진행대상인 소켓의 핸들을 배열에 저장해 놓고, 관리할 필요가 없다.
- IO의 처리를 위한 쓰레드의 수를 조절할 수 있다. 따라서 실험적 결과를 토대로 적절한 쓰레드의 수를 지정할 수 있다.
*/
#include<stdio.h>
#include<stdlib.h>
#include<process.h>
#include<WinSock2.h>
#include<Windows.h>

#define BUF_SIZE 100
#define READ 3
#define WRITE 5

// 클라이언트와 연결된 소켓 정보를 담기위한 구조체
// 이 구조체가 언제 할당되어 언제 쓰이는지 잘 살필것.
typedef struct
{
	// 클라이언트 소켓
	SOCKET hClntSock;
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

DWORD WINAPI EchoThreadMain(LPVOID CompletionPortIO);

void ErrorHandling(char* message);

int main(int argc, char* argv[])
{
	WSADATA wsaData;
	HANDLE hComPort;
	SYSTEM_INFO sysInfo;
	LPPER_IO_DATA ioInfo;
	LPPER_HANDLE_DATA handleInfo;

	SOCKET hServSock;
	SOCKADDR_IN servAdr;
	int recvBytes, i, flags = 0;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		ErrorHandling("WSAStartup() error!");
	}
	// Completion Port 오브젝트(이하 CP오브젝트) 생성 
	// IO의 완료 상황을 CP오브젝트에 등록됨
	// 마지막 매개변수는 실제 IO의 완료에 대한 응답을 받는 쓰레드의 갯수를 의미한다.(0이면 실행가능한 최대갯수)
	hComPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	// 시스템 정보를 가져옴
	GetSystemInfo(&sysInfo);
	// cpu의 갯수만큼 for문을 반복하여 스레드를 생성
	for (i = 0;i < sysInfo.dwNumberOfProcessors; i++)
	{
		//printf("스레드 갯수 = %d \n", i); 4개
		// 스레드 생성
		// 이때 매개변수로 CP오브젝트를 넘겨줌으로서 스레드가 CP오브젝트에 접근 할 수 있다
		_beginthreadex(NULL, 0, EchoThreadMain, (LPVOID)hComPort, 0, NULL);
	}
	// OVERLAPPED용 소켓 생성
	// 넌 블록킹 소켓은 아니다. 즉 연결요청이 없을때는 accept에서 블록킹 상태가 된다.
	hServSock = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family = AF_INET;
	servAdr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAdr.sin_port = htons(atoi(argv[1]));

	// 문지기소켓과 서버주소 연결
	bind(hServSock, (SOCKADDR*)&servAdr, sizeof(servAdr));
	// 대기
	listen(hServSock, 5);

	while (1)
	{
		SOCKET hClntSock;
		SOCKADDR_IN clntAdr;
		int addrLen = sizeof(clntAdr);
		// 연결 // 만약 연결 요청이 없을경우 블록킹 상태가 된다.
		puts("accept.........");
		hClntSock = accept(hServSock, (SOCKADDR*)&clntAdr, &addrLen);
		// 구조체 메모리 할당(while문 안에 있으므로 새로운 소켓이 만들어질때마다 새롭게 할당됨)
		handleInfo = (LPPER_HANDLE_DATA)malloc(sizeof(PER_HANDLE_DATA));
		// 메모리가 할당된 구조체에 클라이언트 소켓과 주소를 넣음.
		handleInfo->hClntSock = hClntSock;
		memcpy(&(handleInfo->clntAdr), &clntAdr, addrLen);

		//accept로 인해 새로 만들어진 클라이언트 소켓과 CP오브젝트 연결
		// 이렇게 함으로서 "이 소켓을 기반으로 진행되는 IO의 완료 상황은 저 CP 오브젝트에 등록해 주세요"가 된다.
		// 스레드 내에서 handleInfo구조체 값을 사용하기 위해 3번째 인자로 구조체를 넣어준다.
		// 그리고 스레드 내에서 handleInfo구조체 값을 가져오려면 GetQueuedCompletionStatus함수를 호출하여 얻어온다.
		CreateIoCompletionPort((HANDLE)hClntSock, hComPort, (DWORD)handleInfo, 0);

		// WSARecv함수 호출에 필요한 모든게 담긴 구조체 메모리 동적 할당(소켓 연결시마다 늘어남)
		ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
		memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
		ioInfo->wsaBuf.len = BUF_SIZE;
		ioInfo->wsaBuf.buf = ioInfo->buffer;
		// IOCP는 입력의 완료와 출력의 완료를 구분 지어주지 않는다 다만 완료되었다는 사실만 알려줌.
		// 따라서 지금 입력을 할지 출력을 진행할지 별도로 기억해줘야한다. 
		// 지금은 WSARecv을 할것이므로 READ를 저장해두었다.
		ioInfo->rwMode = READ;
		// 일곱번째 인자로 OVERLAPPED 구조체 변수의 주소값을 전달하였는데 이값은 
		// 스레드에서 GetQueuedCompletionStatus함수를 호출하여 다시 얻어오기 위함이다.
		// 구조체 변수의 주소값은 첫번째 주소 값과 동일하므로 
		// LPPER_IO_DATA == &(ioInfo->overlapped)이것이 성립한다.
		// 따라서 스레드내에서 LPPER_IO_DATA ioInfo라고 변수를 선언하고 함수인자로 (LPOVERLAPPED*)&ioInfo라고 해줌
		// WSARecv이것이 호출돼야 스레드도GetQueuedCompletionStatus 함수에서 블록킹 당하지 않고 다음으로 넘어갈 수 있다.
		//puts("메인스레드 WSARecv 호출 한다");
		WSARecv(handleInfo->hClntSock, &(ioInfo->wsaBuf), 1, &recvBytes, &flags, &(ioInfo->overlapped), NULL);
		//printf("recvBytes = %d \n", recvBytes);
		//puts("메인스레드 WSARecv 호출 했다");
	}
	return 0;
}

// GetQueuedCompletionStatus함수를 호출하는 스레드 이므로 CP 오브젝트에 할당된 스레드이다.
DWORD WINAPI EchoThreadMain(LPVOID CompletionPortIO)
{
	HANDLE hComPort = (HANDLE)CompletionPortIO;
	SOCKET sock;
	DWORD bytesTrans;
	LPPER_HANDLE_DATA handleInfo;
	LPPER_IO_DATA ioInfo;
	DWORD flags = 0;

	while (1)
	{
		// WSASend, WSARecv를 스레드에서 호출 하기위해 바로 위에서 선언한 변수들을 초기화 한다.
		// 이때 필요한 함수가 GetQueuedCompletionStatus함수 이며 이 함수를 통해 값을 가져올 수 있는 이유는
		// 메인에서 두번째 CreateIoCompletionPort함수 호출때 3번째 매개변수로 LPPER_HANDLE_DATA을 넣어주었고
		// 메인 마지막에 WSARecv함수 7번째 매개변수로 overlapped변수를 넣어주었기때문에(== LPPER_IO_DATA구조체의 주소)
		// 122,123줄의 두개의 구조체 LPPER_HANDLE_DATA, LPPER_IO_DATA 모두를 초기화 할 수 있다.
		// 자 이제 함수를 호출하여 초기화 해보자. // 이것이 초기화 되야 뭐든 되니 무한기다린다.
		// GetQueuedCompletionStatus함수는 IO가 왼료되고, 이에 대한 정보가 등록되었을때 반환한다.
		puts("GetQueuedCompletionStatus.........");
		GetQueuedCompletionStatus(hComPort, &bytesTrans, (LPDWORD)&handleInfo, (LPOVERLAPPED*)&ioInfo, INFINITE);
		// WSARecv함수가 완료되어 GetQueuedCompletionStatus함수를 빠져나오면 수신을 완료한 소켓이 handleInfo 구조체 안에 있다는 것
		// 따라서 이제 이 소켓으로 데이터를 보내면 됨. 
		// 스레드는 4개이므로 이 EchoThreadMain함수는 4개가 가동되지만 여러개의 클라이언트를 상대할 수 있는 이유는
		// GetQueuedCompletionStatus함수에서 블록킹당하다가 수신을 완료한 소켓에 대해서만 운영체제가 알려주므로 이 소켓만 다음으로 넘어가도록 해준다.
		// 그리고 소켓이 만들어질때마다 CreateIoCompletionPort함수를 통해 소켓과 CP가 연결되므로(등록) 운영체제가 이 소켓이 IO가 완료되었는지 알려줄 수 있는거다.
		// 이제 초기화된 구조체를 이용하여 소켓을 초기화 한다.
		sock = handleInfo->hClntSock;
		// 만약에 메인함수에서 넘어온 LPPER_IO_DATA 구조체 중에 rwMode값이 READ이면
		// 즉, WSARecv함수를 호출한 뒤라면 클라이언트로부터 데이터를 받았으니 그대로 다시 보내준다(에코서버니까)
		if (ioInfo->rwMode == READ)
		{
			puts("message received!");
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
			// 데이터를 받고 종료를 전달 받은것이 아니라면 받은 데이터를 다시 클라이언트에게 보내야한다.
			// WSASend함수에 사용할 OVERLAPPED 구조체를 다시 새로 초기화 해준다.(이전에 WSARecv때 사용하던 데이터는 버린다.)
			// OVERLAPPED구조체를 사용해야 비동기 IO가 가능하므로! 꼭꼭 WSASend함수를 사용합시다.
			memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
			// 받은 데이터 크기를 다시 초기화해줌
			ioInfo->wsaBuf.len = bytesTrans;
			// 이제 데이터를 클라이언트로 보낼거니까 보낸다는 표시해줌.
			ioInfo->rwMode = WRITE;
			// 데이터 보내기
			// LPPER_IO_DATA구조체를 보면 WSABUF과 OVERLAPPED 모두 포인터 형태로 정의되어 있지 않으므로
			// 주소를 넘기기 위해 &을 붙혀주었다.
			WSASend(sock, &(ioInfo->wsaBuf), 1, NULL, 0, &(ioInfo->overlapped), NULL);

			puts("수신할꺼야");
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
			WSARecv(sock, &(ioInfo->wsaBuf), 1, NULL, &flags, &(ioInfo->overlapped), NULL);
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
			free(ioInfo);
		}
	} // while문 끝
	return 0;
}

void ErrorHandling(char* msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}