#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <winsock2.h>
#include <windows.h>

#define BUF_SIZE 1000
#define READ	3
#define	WRITE	5

typedef struct    // socket info
{
	SOCKET hClntSock;
	SOCKADDR_IN clntAdr;
} PER_HANDLE_DATA, * LPPER_HANDLE_DATA;

//���Ͽ��� �����͸� ���� �Ŀ� ���⿡�ִ� �����鿡 ��� �����־��ִ���
typedef struct    // buffer info
{
	OVERLAPPED overlapped;
	WSABUF wsaBuf;
	char buffer[BUF_SIZE];
	int rwMode;    // READ or WRITE
} PER_IO_DATA, * LPPER_IO_DATA;


#pragma comment(lib , "ws2_32.lib")

DWORD WINAPI EchoThreadMain(LPVOID CompletionPortIO);
void ErrorHandling(char* message);

int main(int argc, char* argv[])
{
	WSADATA	wsaData;
	HANDLE hComPort;
	SYSTEM_INFO sysInfo;
	LPPER_IO_DATA ioInfo;
	LPPER_HANDLE_DATA handleInfo;

	SOCKET hServSock;
	SOCKADDR_IN servAdr;
	int recvBytes, i, flags = 0;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error!");

	hComPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	GetSystemInfo(&sysInfo);
	//�����ִ� �ֵ����� IO�ѱ�Ŵϱ� �����ִ¾ֵ� �������� �����ָ鼭 �³����� ���ؾ�����(EchoThreadMain) �Ѱ��ְ� ����϶���.  
	// ��û���鿬�����´ϱ� �ڵ����ٰ�(hComPort) �̰Ž����带 ��� ����.
	for (i = 0; i < sysInfo.dwNumberOfProcessors; i++)
		_beginthreadex(NULL, 0, EchoThreadMain, (LPVOID)hComPort, 0, NULL);

	hServSock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family = AF_INET; 
	servAdr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAdr.sin_port = htons(8080);

	bind(hServSock, (SOCKADDR*)&servAdr, sizeof(servAdr));
	listen(hServSock, 5);

	while (1)
	{
		SOCKET hClntSock;
		SOCKADDR_IN clntAdr;
		int addrLen = sizeof(clntAdr);

		hClntSock = accept(hServSock, (SOCKADDR*)&clntAdr, &addrLen);

		handleInfo = (LPPER_HANDLE_DATA)malloc(sizeof(PER_HANDLE_DATA));
		handleInfo->hClntSock = hClntSock;
		memcpy(&(handleInfo->clntAdr), &clntAdr, addrLen);

		CreateIoCompletionPort((HANDLE)hClntSock, hComPort, (DWORD)handleInfo, 0);

		ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
		memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
		ioInfo->wsaBuf.len = BUF_SIZE;
		ioInfo->wsaBuf.buf = ioInfo->buffer;
		ioInfo->rwMode = READ;

		WSARecv(handleInfo->hClntSock, &(ioInfo->wsaBuf),
			1, &recvBytes, &flags, &(ioInfo->overlapped), NULL);
	}
	return 0;
}

DWORD WINAPI EchoThreadMain(LPVOID pComPort)
{
	HANDLE hComPort = (HANDLE)pComPort;
	SOCKET sock;
	DWORD bytesTrans;
	LPPER_HANDLE_DATA handleInfo;
	LPPER_IO_DATA ioInfo;
	DWORD flags = 0;

	//get��û�ޱ����� ������
	char buf[2048];
	char method[100];
	char ct[100];
	char filename[100];

	while (1)
	{
		GetQueuedCompletionStatus(hComPort, &bytesTrans,
			(LPDWORD)&handleInfo, (LPOVERLAPPED*)&ioInfo, INFINITE);
		sock = handleInfo->hClntSock;

		if (ioInfo->rwMode == READ)
		{
			puts("message received!");
			if (bytesTrans == 0)    // EOF ���� ��
			{
				closesocket(sock);
				free(handleInfo); free(ioInfo);
				continue;
			}
			recv(sock, buf, 2048, 0);
		/*	printf("���۹� ó�������� %s : ", buf);*/
			if (strstr(buf, "HTTP/") == NULL)
			{
				
			}

			printf("reveived message (http��û�� ioInfo->buffer ����������Ȯ�ο�  : %s ", ioInfo->buffer);
			
			memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));

			
			ioInfo->rwMode = WRITE;
			printf("���ڿ����� : %ld", bytesTrans);
		/*	ioInfo->wsaBuf.buf = """ HTTP/1.0 200 OK\r\nServer:simple web server\r\nContent-length:2048\r\nContent-type: text/html \r\n\r\n Body : HELLO!!!!!!!!!!!!!!!!!!!!! """;
			ioInfo->wsaBuf.len = strlen("HTTP/1.0 200 OK\r\nServer:simple web server\r\nContent-length:2048\r\nContent-type: text/plain \r\n\r\n HELLO!!!!!!!!!!!!!!!!!!!");*/
			send(sock, "HTTP/1.0 200 OK\r\n", strlen("HTTP/1.0 200 OK\r\n"), 0);
			send(sock, "Server:simple web server\r\n", strlen("Server:simple web server\r\n"), 0);
			send(sock, "Content-length:2048\r\n", strlen("Content-length:2048\r\n"), 0);
			send(sock, "Content-type: application/json \r\n\r\n", strlen("Content-type: application/json \r\n\r\n"), 0);
				/*send(sock, " <!DOCTYPE html>< html lang = \"en\">< head >< meta charset = \"UTF-8\">< meta http - equiv = \"X-UA-Compatible\" content = \"IE=edge\">< meta name = \"viewport\" content=\"width=device-width, initial-scale=1.0\">< title > Document< / title>< / head><body><h3>HELLO!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!< / h3>< / body>< / html> "
				, strlen(" <!DOCTYPE html>< html lang = \"en\">< head >< meta charset = \"UTF-8\">< meta http - equiv = \"X-UA-Compatible\" content = \"IE=edge\">< meta name = \"viewport\" content=\"width=device-width, initial-scale=1.0\">< title > Document< / title>< / head><body><h3>HELLO!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!< / h3>< / body>< / html> "),
				0);*/
			send(sock, "{'message' : 'Hello!!!!!!!!!'}", strlen("{ 'message' : 'Hello!!!!!!!!!'}"), 0);
			WSASend(sock, &(ioInfo->wsaBuf),
				1, NULL, 0, &(ioInfo->overlapped), NULL);


			ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
			memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
			ioInfo->wsaBuf.len = BUF_SIZE;
			ioInfo->wsaBuf.buf = ioInfo->buffer;
			ioInfo->rwMode = READ;
			WSARecv(sock, &(ioInfo->wsaBuf),
				1, NULL, &flags, &(ioInfo->overlapped), NULL);
		}
		else
		{
			puts("message sent!");
			
			closesocket(sock);
			free(ioInfo);
		}
	}
	return 0;
}

void ErrorHandling(char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}