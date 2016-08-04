#include<stdio.h>
#include<Windows.h>
#include<process.h> // _beginthreadex, _endthreadex
unsigned WINAPI ThreadFunc(void* arg);

int main(int argc, char* argv[])
{
	HANDLE hThread; // ������ ��ũ��Ʈ �����
	DWORD wr; // �������� Ŀ�� ������Ʈ�� ���°� ��� �����ϱ� ����
	unsigned threadID; // ������ ���� ��ȣ
	int param = 5;
	// ������ ���� ��ȯ���� �ٸ��� ������ (HANDEL)�� ����Ȱ �ʿ�
	// _beginthreadex(��������, ����, �Լ�, �Ķ����, �������, ������ ������ȣ)
	hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadFunc, (void*)&param, 0, &threadID);
	// ������ ���� ���н� 0 ��ȯ�ϹǷ� (�����ϸ� �������ڵ��ȯ)
	if (hThread == 0)
	{
		puts("_beginthreadex() error");
		return -1;
	}
	// �����尡 ����ɶ����� ������ ��ٸ�
	if ((wr = WaitForSingleObject(hThread, INFINITE)) == WAIT_FAILED)
	{
		puts("thread wait error");
		return -1;
	}
	//puts("������");
	// �����尡 ������°� �Ǿ� ������Ŀ�ο�����Ʈ�� ���°� signaled�� �Ǿ���? ���� ������°� ���� �ʾ� non-signaled�ΰ�?
	// ������¶�� wr�� WAIT_OBJECT_0�� ����Ǿ� �����Ƿ� signaled��ȯ�ϰ� ��.
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
