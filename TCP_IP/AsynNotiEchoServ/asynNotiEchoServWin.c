#include<stdio.h>
#include<string.h>
#include<WinSock2.h>

#define BUF_SIZE 100

void CompressSockets(SOCKET hSockArr[], int idx, int total);
void CompressEvents(WSAEVENT hEventArr[], int idx, int total);
void ErrorHandling(char* msg);

int main(int argc, char* argv[])
{
	WSADATA wsaData;
	SOCKET hServSock, hClntSock;
	SOCKADDR_IN servAdr, clntAdr;

	SOCKET hSockArr[WSA_MAXIMUM_WAIT_EVENTS];
	WSAEVENT hEventArr[WSA_MAXIMUM_WAIT_EVENTS];
	WSAEVENT newEvent;
	WSANETWORKEVENTS netEvents;

	int numOfClntSock = 0;
	int strLen, i;
	int posInfo, startIdx;
	int clntAdrLen;
	char msg[BUF_SIZE];

	if (argc != 2)
	{
		printf("Usage: %s <port>\n", argv[0]);
		exit(0);
	}
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		ErrorHandling("WSAStartup() error!");
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////

	hServSock = socket(PF_INET, SOCK_STREAM, 0);
	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family = AF_INET;
	servAdr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAdr.sin_port = htons(atoi(argv[1]));

	if (bind(hServSock, (SOCKADDR*)&servAdr, sizeof(servAdr)) == SOCKET_ERROR)
	{
		ErrorHandling("bind() Error");
	}
	if (listen(hServSock, 5) == SOCKET_ERROR)
	{
		ErrorHandling("listen() error");
	}
	// 메뉴얼, non-시그널상태 이벤트 커널오브젝트 생성
	newEvent = WSACreateEvent();
	// 문지기 서버소켓을 대상으로 ACCEPT이벤트가 발생했는지 관찰명령을 내림
	// 만약 ACCEPT이벤트가 발생하면 newEvent의 시그널 상태를 non-signaled에서 signaled로 바꿈
	// 따라서 WSAEventSelect함수를 Event커널오브젝트과 소켓을 연결하는 함수라고도 부름
	if (WSAEventSelect(hServSock, newEvent, FD_ACCEPT) == SOCKET_ERROR)
	{
		ErrorHandling("WSAEventSelect() Error");
	}
	// hSockArr[n]에 저장된 소켓과 연결된 Event 오브젝트는 hEventArr[n]에 저장되어있다고 알기위해 반대로
	// hEventArr[n]에 저장된 Event오브젝트와 연결된 소켓은 hSockArr[n]에 저장되어있다고 알 수 있다.
	hSockArr[numOfClntSock] = hServSock;
	hEventArr[numOfClntSock] = newEvent;
	numOfClntSock++;

	while (1)
	{
		// (이벤트갯수, 이벤트오브젝트, 전부기다릴꺼냐?, 얼마나기다릴꺼냐?, 모름)
		// 정수를 반환함
		posInfo = WSAWaitForMultipleEvents(numOfClntSock, hEventArr, FALSE, WSA_INFINITE, FALSE);
		// signaled 상태가 된 Event오브젝트 핸들 시작 인덱스 = posInfo(반환된정수값) - WSA_WAIT_EVENT_0;
		startIdx = posInfo - WSA_WAIT_EVENT_0;
		
		// signaled상태가된 Event오브젝트의 시작 인덱스부터 전체 이벤트 갯수까지 i값을 하나씩 증가시키면서
		// 모든 이벤트 배열을 돌며 signaled상태가된 Event오브젝트를 찾음 
		for (i = startIdx; i < numOfClntSock;i++)
		{
			// 하나의 이벤트에 대해서 이 이벤트가 signaled상태인지 확인을 위해 일단 sigEventIdx에 반환값을 넣음
			// hEventArr에 들어있는 이벤트오브젝트가 메뉴얼이기 때문에 WSAWaitForMultipleEvents함수를 호출하여도
			// 자동으로 signaled상태 -> non-signaled상태로 바뀌지 않기때문에 이렇게 호출 할 수 있다.
			// (바뀌어 버리면 써보지도 못하고 신호가 꺼져버리는 결과가 됨.)
			// 그리고 기다리는 시간이 0이므로 호출과동시에 반환이 이루어서 비동기처리가 가능하다.
			int sigEventIdx = WSAWaitForMultipleEvents(1, &hEventArr[i], TRUE, 0, FALSE);
			// 이 반환값을 이용하여 이 이벤트가 signaled상태인지 FAILED인지 TIMEOUT인지 알 수 있다.
			if ((sigEventIdx == WSA_WAIT_FAILED || sigEventIdx == WSA_WAIT_TIMEOUT))
			{
				// 이 이벤트는 signaled상태가 아니므로 다시 for문의 다음 인덱스로 넘어가서 다음 이벤트를 조사함. 
				continue;
			}else
			{
				// 이 이벤트는 signaled상태 입니다. if문을 이용하여 이벤트 정보에따라 처리해 줍니다.

				// signaled상태의 이벤트가 담긴 배열 인덱스를 넣음 
				// 즉 이벤트가 발생한 이벤트에 접근이 가능!
				sigEventIdx = i;
				// 비어있는 netEvents 구조체 주소를 넣어주면 이벤트 정보와 오류정보를 채워주는 함수
				// 위에서 이벤트오브젝트의 인덱스와 이 이벤트와 연결된 소켓의 인덱스를 맞추었으므로 
				// 똑같이 sigEventIdx인덱스를 이용할 수 있다.
				// WSAEnumNetworkEvents(이벤트와연결된 소켓, 소켓과 연결된 이벤트, 결과값을 담을 비어있는 구조체);
				WSAEnumNetworkEvents(hSockArr[sigEventIdx], hEventArr[sigEventIdx], &netEvents);
				// 이벤트&오류 정보가 담긴 구조체의 이벤트정보가 연결요청시 이면
				// 연결요청 이벤트가 발생한 소켓에 대해서 처리
				if (netEvents.lNetworkEvents & FD_ACCEPT)
				{
					// 오류 정보가 연결요청 에러이면
					if (netEvents.iErrorCode[FD_ACCEPT_BIT] != 0)
					{
						puts("Accept Error");
						break;
					}
					// 클라이언트 주소 길이 
					clntAdrLen = sizeof(clntAdr);
					// 이벤트가 발생한 소켓에 대해서 연결한다
					hClntSock = accept(hSockArr[sigEventIdx], (SOCKADDR*)&clntAdr, &clntAdrLen);
					// 새로운 이벤트 커널오브젝트를 만듬 (accept로 인해 새로만들어진 클라이언트 소켓과 연결할 이벤트임)
					newEvent = WSACreateEvent();
					// 클라이언트 소켓과 이벤트를 연결 (관찰할 이벤트 정보는 입력버퍼에 읽을 정보가 있는가? 종료 정보가 있는가?)
					WSAEventSelect(hClntSock, newEvent, FD_READ | FD_CLOSE);

					// 클라이언트 소켓과 이벤트 오브젝트를 같은 인덱스로 배열에 넣음
					hEventArr[numOfClntSock] = newEvent;
					hSockArr[numOfClntSock] = hClntSock;
					// 다음에 추가될 소켓과 이벤트를 위해 인덱스를 증가시킴(인덱스 중복을 피함)
					numOfClntSock++;
					puts("connected new client...");
				}

				// 이벤트&오류 정보가 담긴 구조체의 이벤트정보가 입력수신버퍼에 데이터가 있으면(데이터 수신 시)
				// 데이터를 수신한 소켓에 대해서 처리
				if (netEvents.lNetworkEvents & FD_READ)
				{
					// 오류 정보가 수신 에러이면
					if (netEvents.iErrorCode[FD_READ_BIT] != 0)
					{
						puts("Read Error");
						break;
					}
					// 에코서버이므로 전송대상, 수신대상이 같다 따라서 소켓 배열에서 이벤트가 발생한 소켓 인덱스로
					// 이벤트소켓을 불러와 recv, send함수를 호출한다.
					strLen = recv(hSockArr[sigEventIdx], msg, sizeof(msg), 0);
					send(hSockArr[sigEventIdx], msg, strLen, 0);
				}

				// 이벤트&오류 정보가 담긴 구조체의 이벤트정보가 종료 요청시
				// 종료요청한 클라이언트 소켓에 대해서 처리
				if (netEvents.lNetworkEvents & FD_CLOSE)
				{
					// 오류 정보가 종료 에러이면
					if (netEvents.iErrorCode[FD_CLOSE_BIT] != 0)
					{
						puts("Close Error");
						break;
					}
					// 해당하는 이벤트와 소켓에 대해 종료 처리
					WSACloseEvent(hEventArr[sigEventIdx]);
					closesocket(hSockArr[sigEventIdx]);
					// 없어진 소켓과 이벤트가 있으므로 인덱스를 하나 감소
					numOfClntSock--;
					//
					CompressSockets(hSockArr, sigEventIdx, numOfClntSock);
					CompressEvents(hEventArr, sigEventIdx, numOfClntSock);
				}
			}
		}

	}
	WSACleanup();
	return 0;
}
// 삭제된 소켓의 빈 인덱스를 채움
void CompressSockets(SOCKET hSockArr[], int idx, int total)
{
	int i;
	// 지워진 소켓을 매꾸기위해 바로 옆에 있던 소켓으로 채움 (빈자리를 뒤에서 땡겨 앉는다)
	for (i = idx; i < total; i++)
		hSockArr[i] = hSockArr[i + 1];
}
// 삭제된 이벤트의 빈 인덱스를 채움
void CompressEvents(WSAEVENT hEventArr[], int idx, int total)
{
	int i;
	for (i = idx; i < total; i++)
		hEventArr[i] = hEventArr[i + 1];
}
void ErrorHandling(char* msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}