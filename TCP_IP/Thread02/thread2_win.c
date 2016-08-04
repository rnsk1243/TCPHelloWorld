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
		// 스레드가 만들어지자마자 정해준 함수 실행함. 각각 모든 스레드가 다 for문을 돌며 실행.
		if (i % 2)
			tHandles[i] = (HANDLE)_beginthreadex(NULL, 0, threadInc, NULL, 0, NULL);
		else
			tHandles[i] = (HANDLE)_beginthreadex(NULL, 0, threadDes, NULL, 0, NULL);
	}
	// 모든 스레드가 종료 될때까지 main함수 정지상태
	WaitForMultipleObjects(NUM_THREAD, tHandles, TRUE, INFINITE);
	// cs가 사용하던 메모리공간 반납
	//DeleteCriticalSection(&cs);
	// 뮤텍스 반납
	//CloseHandle(hMutex);
	CloseHandle(semOne);
	printf("result: %lld \n", num);
	return 0;
}

unsigned WINAPI threadInc(void* arg)
{
	//puts("ㅎㅎ");
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
	//puts("ㅗㅗ");
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