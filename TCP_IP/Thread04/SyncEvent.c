#include<stdio.h>
#include<Windows.h>
#include<process.h>
#define STR_LEN 100

unsigned WINAPI NumberOfA(void* arg);
unsigned WINAPI NumberOfOthers(void* arg);

static char str[STR_LEN];
//static HANDLE hEvent;
static HANDLE semOne;

int main(int argc, char* argv[])
{
	HANDLE hThread1, hThread2;
	// �̺�Ʈ ���� (�⺻����, �޴����!, non-signaled����, �̸�����)
	//hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	semOne = CreateSemaphore(NULL, 0, 2, NULL);
	hThread1 = (HANDLE)_beginthreadex(NULL, 0, NumberOfA, NULL, 0, NULL);
	hThread2 = (HANDLE)_beginthreadex(NULL, 0, NumberOfOthers, NULL, 0, NULL);

	fputs("Input string: ", stdout);
	fgets(str, STR_LEN, stdin);
	//SetEvent(hEvent); // signaled���·� ����
	ReleaseSemaphore(semOne, 2, NULL);
	WaitForSingleObject(hThread1, INFINITE);
	WaitForSingleObject(hThread2, INFINITE);
	//ResetEvent(hEvent); // non-signaled���·� ����
	//CloseHandle(hEvent); // �޸� �ݳ�
	CloseHandle(semOne);
	return 0;
}

unsigned WINAPI NumberOfA(void* arg)
{
	int i, cnt = 0;
	//WaitForSingleObject(hEvent, INFINITE);
	WaitForSingleObject(semOne, INFINITE);
	for (i = 0; str[i] != 0; i++)
	{
		if (str[i] == 'A')
			cnt++;
	}
	printf("Num of A: %d \n", cnt);
	return 0;
}
unsigned WINAPI NumberOfOthers(void* arg)
{
	int i, cnt = 0;
	//WaitForSingleObject(hEvent, INFINITE);
	WaitForSingleObject(semOne, INFINITE);
	for (i = 0; str[i] != 0; i++)
	{
		if (str[i] != 'A')
			cnt++;
	}
	printf("Num of others: %d \n", cnt - 1);
	return 0;
}