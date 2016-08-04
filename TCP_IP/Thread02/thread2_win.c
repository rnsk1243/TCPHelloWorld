#include<stdio.h>
#include<Windows.h>
#include<process.h>

#define NUM_THREAD 50
unsigned WINAPI threadInc(void* arg);
unsigned WINAPI threadDes(void* arg);

long long num = 0;
//CRITICAL_SECTION cs;
//HANDLE hMutex;
static HANDLE semOne;

int main(int argc, char* argv[])
{
	
	int i;
	semOne = CreateSemaphore(NULL, 1, 1, NULL);
	//InitializeCriticalSection(&cs);
	HANDLE tHandles[NUM_THREAD];
	//printf("sizeof long long: %d \n", sizeof(long long));
	//hMutex = CreateMutex(NULL, FALSE, NULL);
	for (i = 0; i < NUM_THREAD; i++)
	{
		// �����尡 ��������ڸ��� ������ �Լ� ������. ���� ��� �����尡 �� for���� ���� ����.
		if (i % 2)
			tHandles[i] = (HANDLE)_beginthreadex(NULL, 0, threadInc, NULL, 0, NULL);
		else
			tHandles[i] = (HANDLE)_beginthreadex(NULL, 0, threadDes, NULL, 0, NULL);
	}
	// ��� �����尡 ���� �ɶ����� main�Լ� ��������
	WaitForMultipleObjects(NUM_THREAD, tHandles, TRUE, INFINITE);
	// cs�� ����ϴ� �޸𸮰��� �ݳ�
	//DeleteCriticalSection(&cs);
	// ���ؽ� �ݳ�
	//CloseHandle(hMutex);
	CloseHandle(semOne);
	printf("result: %lld \n", num);
	return 0;
}

unsigned WINAPI threadInc(void* arg)
{
	//puts("����");
	int i;
	//EnterCriticalSection(&cs);
	//WaitForSingleObject(hMutex, INFINITE);
	WaitForSingleObject(semOne, INFINITE);
	for (i = 0; i < 5000000; i++)
	{
		num++;
	}
	ReleaseSemaphore(semOne, 1, NULL);
	//ReleaseMutex(hMutex);
	//LeaveCriticalSection(&cs);
	return 0;
}

unsigned WINAPI threadDes(void* arg)
{
	//puts("�Ǥ�");
	int i;
	//EnterCriticalSection(&cs);
	//WaitForSingleObject(hMutex, INFINITE);
	WaitForSingleObject(semOne, INFINITE);
	for (i = 0; i < 5000000; i++)
	{
		num--;
	}
	ReleaseSemaphore(semOne, 1, NULL);
	//ReleaseMutex(hMutex);
	//LeaveCriticalSection(&cs);
	return 0;
}