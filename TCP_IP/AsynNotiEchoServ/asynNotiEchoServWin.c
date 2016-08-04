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
	// �޴���, non-�ñ׳λ��� �̺�Ʈ Ŀ�ο�����Ʈ ����
	newEvent = WSACreateEvent();
	// ������ ���������� ������� ACCEPT�̺�Ʈ�� �߻��ߴ��� ��������� ����
	// ���� ACCEPT�̺�Ʈ�� �߻��ϸ� newEvent�� �ñ׳� ���¸� non-signaled���� signaled�� �ٲ�
	// ���� WSAEventSelect�Լ��� EventĿ�ο�����Ʈ�� ������ �����ϴ� �Լ���� �θ�
	if (WSAEventSelect(hServSock, newEvent, FD_ACCEPT) == SOCKET_ERROR)
	{
		ErrorHandling("WSAEventSelect() Error");
	}
	// hSockArr[n]�� ����� ���ϰ� ����� Event ������Ʈ�� hEventArr[n]�� ����Ǿ��ִٰ� �˱����� �ݴ��
	// hEventArr[n]�� ����� Event������Ʈ�� ����� ������ hSockArr[n]�� ����Ǿ��ִٰ� �� �� �ִ�.
	hSockArr[numOfClntSock] = hServSock;
	hEventArr[numOfClntSock] = newEvent;
	numOfClntSock++;

	while (1)
	{
		// (�̺�Ʈ����, �̺�Ʈ������Ʈ, ���α�ٸ�����?, �󸶳���ٸ�����?, ��)
		// ������ ��ȯ��
		posInfo = WSAWaitForMultipleEvents(numOfClntSock, hEventArr, FALSE, WSA_INFINITE, FALSE);
		// signaled ���°� �� Event������Ʈ �ڵ� ���� �ε��� = posInfo(��ȯ��������) - WSA_WAIT_EVENT_0;
		startIdx = posInfo - WSA_WAIT_EVENT_0;
		
		// signaled���°��� Event������Ʈ�� ���� �ε������� ��ü �̺�Ʈ �������� i���� �ϳ��� ������Ű�鼭
		// ��� �̺�Ʈ �迭�� ���� signaled���°��� Event������Ʈ�� ã�� 
		for (i = startIdx; i < numOfClntSock;i++)
		{
			// �ϳ��� �̺�Ʈ�� ���ؼ� �� �̺�Ʈ�� signaled�������� Ȯ���� ���� �ϴ� sigEventIdx�� ��ȯ���� ����
			// hEventArr�� ����ִ� �̺�Ʈ������Ʈ�� �޴����̱� ������ WSAWaitForMultipleEvents�Լ��� ȣ���Ͽ���
			// �ڵ����� signaled���� -> non-signaled���·� �ٲ��� �ʱ⶧���� �̷��� ȣ�� �� �� �ִ�.
			// (�ٲ�� ������ �Ẹ���� ���ϰ� ��ȣ�� ���������� ����� ��.)
			// �׸��� ��ٸ��� �ð��� 0�̹Ƿ� ȣ������ÿ� ��ȯ�� �̷� �񵿱�ó���� �����ϴ�.
			int sigEventIdx = WSAWaitForMultipleEvents(1, &hEventArr[i], TRUE, 0, FALSE);
			// �� ��ȯ���� �̿��Ͽ� �� �̺�Ʈ�� signaled�������� FAILED���� TIMEOUT���� �� �� �ִ�.
			if ((sigEventIdx == WSA_WAIT_FAILED || sigEventIdx == WSA_WAIT_TIMEOUT))
			{
				// �� �̺�Ʈ�� signaled���°� �ƴϹǷ� �ٽ� for���� ���� �ε����� �Ѿ�� ���� �̺�Ʈ�� ������. 
				continue;
			}else
			{
				// �� �̺�Ʈ�� signaled���� �Դϴ�. if���� �̿��Ͽ� �̺�Ʈ ���������� ó���� �ݴϴ�.

				// signaled������ �̺�Ʈ�� ��� �迭 �ε����� ���� 
				// �� �̺�Ʈ�� �߻��� �̺�Ʈ�� ������ ����!
				sigEventIdx = i;
				// ����ִ� netEvents ����ü �ּҸ� �־��ָ� �̺�Ʈ ������ ���������� ä���ִ� �Լ�
				// ������ �̺�Ʈ������Ʈ�� �ε����� �� �̺�Ʈ�� ����� ������ �ε����� ���߾����Ƿ� 
				// �Ȱ��� sigEventIdx�ε����� �̿��� �� �ִ�.
				// WSAEnumNetworkEvents(�̺�Ʈ�Ϳ���� ����, ���ϰ� ����� �̺�Ʈ, ������� ���� ����ִ� ����ü);
				WSAEnumNetworkEvents(hSockArr[sigEventIdx], hEventArr[sigEventIdx], &netEvents);
				// �̺�Ʈ&���� ������ ��� ����ü�� �̺�Ʈ������ �����û�� �̸�
				// �����û �̺�Ʈ�� �߻��� ���Ͽ� ���ؼ� ó��
				if (netEvents.lNetworkEvents & FD_ACCEPT)
				{
					// ���� ������ �����û �����̸�
					if (netEvents.iErrorCode[FD_ACCEPT_BIT] != 0)
					{
						puts("Accept Error");
						break;
					}
					// Ŭ���̾�Ʈ �ּ� ���� 
					clntAdrLen = sizeof(clntAdr);
					// �̺�Ʈ�� �߻��� ���Ͽ� ���ؼ� �����Ѵ�
					hClntSock = accept(hSockArr[sigEventIdx], (SOCKADDR*)&clntAdr, &clntAdrLen);
					// ���ο� �̺�Ʈ Ŀ�ο�����Ʈ�� ���� (accept�� ���� ���θ������ Ŭ���̾�Ʈ ���ϰ� ������ �̺�Ʈ��)
					newEvent = WSACreateEvent();
					// Ŭ���̾�Ʈ ���ϰ� �̺�Ʈ�� ���� (������ �̺�Ʈ ������ �Է¹��ۿ� ���� ������ �ִ°�? ���� ������ �ִ°�?)
					WSAEventSelect(hClntSock, newEvent, FD_READ | FD_CLOSE);

					// Ŭ���̾�Ʈ ���ϰ� �̺�Ʈ ������Ʈ�� ���� �ε����� �迭�� ����
					hEventArr[numOfClntSock] = newEvent;
					hSockArr[numOfClntSock] = hClntSock;
					// ������ �߰��� ���ϰ� �̺�Ʈ�� ���� �ε����� ������Ŵ(�ε��� �ߺ��� ����)
					numOfClntSock++;
					puts("connected new client...");
				}

				// �̺�Ʈ&���� ������ ��� ����ü�� �̺�Ʈ������ �Է¼��Ź��ۿ� �����Ͱ� ������(������ ���� ��)
				// �����͸� ������ ���Ͽ� ���ؼ� ó��
				if (netEvents.lNetworkEvents & FD_READ)
				{
					// ���� ������ ���� �����̸�
					if (netEvents.iErrorCode[FD_READ_BIT] != 0)
					{
						puts("Read Error");
						break;
					}
					// ���ڼ����̹Ƿ� ���۴��, ���Ŵ���� ���� ���� ���� �迭���� �̺�Ʈ�� �߻��� ���� �ε�����
					// �̺�Ʈ������ �ҷ��� recv, send�Լ��� ȣ���Ѵ�.
					strLen = recv(hSockArr[sigEventIdx], msg, sizeof(msg), 0);
					send(hSockArr[sigEventIdx], msg, strLen, 0);
				}

				// �̺�Ʈ&���� ������ ��� ����ü�� �̺�Ʈ������ ���� ��û��
				// �����û�� Ŭ���̾�Ʈ ���Ͽ� ���ؼ� ó��
				if (netEvents.lNetworkEvents & FD_CLOSE)
				{
					// ���� ������ ���� �����̸�
					if (netEvents.iErrorCode[FD_CLOSE_BIT] != 0)
					{
						puts("Close Error");
						break;
					}
					// �ش��ϴ� �̺�Ʈ�� ���Ͽ� ���� ���� ó��
					WSACloseEvent(hEventArr[sigEventIdx]);
					closesocket(hSockArr[sigEventIdx]);
					// ������ ���ϰ� �̺�Ʈ�� �����Ƿ� �ε����� �ϳ� ����
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
// ������ ������ �� �ε����� ä��
void CompressSockets(SOCKET hSockArr[], int idx, int total)
{
	int i;
	// ������ ������ �Ųٱ����� �ٷ� ���� �ִ� �������� ä�� (���ڸ��� �ڿ��� ���� �ɴ´�)
	for (i = idx; i < total; i++)
		hSockArr[i] = hSockArr[i + 1];
}
// ������ �̺�Ʈ�� �� �ε����� ä��
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