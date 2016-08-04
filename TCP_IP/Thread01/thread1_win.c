#include<stdio.h>
#include<Windows.h>
#include<process.h> // _beginthreadex, _endthreadex
unsigned WINAPI ThreadFunc(void* arg);

int main(int argc, char* argv[])
{
	HANDLE hThread; // 스레드 디스크립트 저장용
	DWORD wr; // 스레드의 커널 오브젝트의 상태가 어떤지 저장하기 위해
	unsigned threadID; // 스레드 고유 번호
	int param = 5;
	// 스레드 생성 반환값이 다르기 때문에 (HANDEL)로 형변활 필요
	// _beginthreadex(보안정보, 스택, 함수, 파라미터, 실행순서, 스레드 고유번호)
	hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadFunc, (void*)&param, 0, &threadID);
	// 스레드 생성 실패시 0 반환하므로 (성공하면 스레드핸들반환)
	if (hThread == 0)
	{
		puts("_beginthreadex() error");
		return -1;
	}
	// 스레드가 종료될때까지 영원히 기다림
	if ((wr = WaitForSingleObject(hThread, INFINITE)) == WAIT_FAILED)
	{
		puts("thread wait error");
		return -1;
	}
	//puts("하하하");
	// 스레드가 종료상태가 되어 스레드커널오브젝트의 상태가 signaled가 되었나? 아직 종료상태가 되지 않아 non-signaled인가?
	// 종료상태라면 wr에 WAIT_OBJECT_0이 저장되어 있으므로 signaled반환하게 됨.
	printf("wait result: %s \n", (wr == WAIT_OBJECT_0) ? "signaled" : "time-out");

	puts("end of main");
	return 0;
}

unsigned WINAPI ThreadFunc(void * arg)
{
	int i;
	int cnt = *((int*)arg);
	for (i = 0; i < cnt; i++)
	{
		Sleep(1000);
		puts("running thread");
	}
	return 0;
}
