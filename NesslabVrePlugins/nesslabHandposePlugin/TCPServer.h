#pragma once

#include <iostream>
#include <thread>
#include <map>
#include <sstream>
#include <WinSock2.h>//WinSock 2 DLL and WinSock 2 applications 정의
#include <WS2tcpip.h>//윈도우 소켓 2 개발에 필요한 헤더

#pragma comment(lib, "Ws2_32.lib")

#define BUFLEN 1024

using namespace std;


//플러그인 만들 때 스레드로 처리해서 본 프로그램 동작에 영향 안가도록 처리 신경써주기
class TCPServer
{
	//typedef void (*DataReceiveCallback)(BYTE* data, int len, string client);
	typedef void (*DataReceiveCallback)(BYTE* data, int len);//데이터 받으면 호출
	typedef void (*ConnectCallback)(string client);//클라이언트 접속
	typedef void (*onClientDisconnect)(string client);//클라이언트 종료
	typedef void (*onServerClosed)();//서버 종료


public:

	TCPServer();
	TCPServer(int serverPort);
	~TCPServer();

	void ServerStart();
	void ServerStop();
	BOOL SendData(BYTE* data, int len);
	BOOL SendData(BYTE* data, int len, string clientAddr);
	BOOL SendData(BYTE* data, int len, string clientAddr, SOCKET clientSock);

	string AddrToStr(sockaddr_in addr);


	DataReceiveCallback DataReceived;
	ConnectCallback Connected;
	onClientDisconnect clientDisconnectCallback;
	onServerClosed serverClosedCallback;


	//key => 포트+IP(=>중복방지), value => 클라이언트 소켓
	map< string, SOCKET> dicClientSockets;
	int serverPort = 5010;


private:
	SOCKET serverSocket;

	void StartThr();
	void RecvThr(string clientAddr, SOCKET clientSocket);
};