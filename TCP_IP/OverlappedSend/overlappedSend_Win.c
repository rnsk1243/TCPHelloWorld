#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<stdio.h>
#include<stdlib.h>
#include<WinSock2.h>
void ErrorHandling(char* msg);

int main(int argc, char* argv[])
{
	WSADATA wsaData;
	SOCKET hSocket;
	SOCKADDR_IN sendAdr;

	WSABUF dataBuf;
	char msg[] = "NetWork is Computer!";
	int sendBytes = 0;

	WSAEVENT evObj;
	WSAOVERLAPPED overlapped;

	if (argc != 3) {
		printf("Usage: %s <IP> <port> \n", argv[0]);
		exit(1);
	}
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		ErrorHandling("WSAStartup() error!");
	}
	// WSA_FLAG_OVERLAPPED를 전달해서 생성되는 소켓에 Overlapped IO가 가능한 속성을 부여하여
	// 소켓을 생성
	hSocket = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	memset(&sendAdr, 0, sizeof(sendAdr));
	sendAdr.sin_family = AF_INET;
	sendAdr.sin_addr.s_addr = inet_addr(argv[1]);
	sendAdr.sin_port = htons(atoi(argv[2]));

	if (connect(hSocket, (SOCKADDR*)&sendAdr, sizeof(sendAdr)) == SOCKET_ERROR)
		ErrorHandling("connect() error!");

	// 메뉴얼모드 non-signaled
	evObj = WSACreateEvent();
	memset(&overlapped, 0, sizeof(overlapped));
	// 이벤트오브젝트와 연결해서 데이터 전송 완료했는지 확인 용도
	overlapped.hEvent = evObj;
	// WSADATA 데이터구조체에 보낸 데이터와 크기 저장 (WSASend함수의 2번째 인자) 
	dataBuf.len = sizeof(msg) + 1;
	dataBuf.buf = msg;
	// WSASend(보낼곳정보담긴소켓, 보낼데이터구조체, 구조체갯수, 전송된바이트수 저장될 변수주소, 전송특성, 전송완료확인용이벤트담긴구조체, Completion Routine함수(데이터전송확인용) 주소)
	if (WSASend(hSocket, &dataBuf, 1, &sendBytes, 0, &overlapped, NULL) == SOCKET_ERROR)
	{
		// 만약 SOCKET_ERROR이 발생하면 원인은 두 가지 이다.
		// 첫째. WSASend함수가 반환을 한 다음에도 계속해서 데이터의 전송이 이뤄지는 상황
		// 둘째. 그외 원인
		////////////////////////////////////////////////////////////////////////////////
		// 첫째원인 일경우
		// WSAGetLastError함수호출을 통해서 오류코드 WSA_IO_PENDING이 등록되었는지 확인한다.
		// 참고로  WSAGetLastError함수는 소켓관련 함수가 호출된 이후에 발생하는 오류 원인정보를 반환함.
		if (WSAGetLastError() == WSA_IO_PENDING)
		{
			// WSAGetLastError() == WSA_IO_PENDING 이경우에는 데이터의 전송이 완료되지는않았지만 계속해서 전송중인상태
			// 따라서 이때 sendBytes에 저장된 값은 쓰래기다.
			puts("Background data send");
			// WSAWaitForMultipleEvents(이벤트오브젝트개수, 이벤트오브젝트, 모두기다림?, 무한?, alertable wait상태 진입?)
			// overlapped.hEvent = evObj; 이렇게 해주었으므로 데이터 전송이 완료되면 evObj가 signaled상태가 된다.
			// 따라서 WSAWaitForMultipleEvents함수를 통해 evObj가 signaled상태가 될때까지 무한 기다리도록 해준다.
			WSAWaitForMultipleEvents(1, &evObj, TRUE, WSA_INFINITE, FALSE);
			// WSAGetOverlappedResult(소켓, 전송완료확인용구조체, 전송된바이트수 저장될 변수주소, 전송완료될때까지 기다릴꺼?, 부수정보 필요?)
			// 전송이 완료되었으므로 이제 sendBytes변수에 전송된 값을 저장한다.
			WSAGetOverlappedResult(hSocket, &overlapped, &sendBytes, FALSE, NULL);
		}
		else
		{
				  // 그외 원인
			ErrorHandling("WSASend() error");
		}
	}
	//SOCKET_ERROR가 발생하지 않으면 WSASend함수가 반환과 동시에 데이터 전송이 완료된것
	printf("Send data size: %d \n", sendBytes);
	WSACloseEvent(evObj);
	closesocket(hSocket);
	WSACleanup();
	return 0;
}

void ErrorHandling(char* msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}