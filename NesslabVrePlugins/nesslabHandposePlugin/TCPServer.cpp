#include "TCPServer.h"


TCPServer::TCPServer()
{
	Connected = NULL;
	clientDisconnectCallback = NULL;
	DataReceived = NULL;
	serverSocket = NULL;

	this->serverPort = serverPort;
}

TCPServer::TCPServer(int port)
{
	Connected = NULL;
	clientDisconnectCallback = NULL;
	DataReceived = NULL;
	serverSocket = NULL;

	this->serverPort = port;
}


TCPServer::~TCPServer()
{
}


void TCPServer::ServerStart()
{
	std::thread serverThr(&TCPServer::StartThr, this);
	serverThr.detach();
}



void TCPServer::StartThr()
{
	WSADATA wsaData;
	int result;


	result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0)
	{
		cout << "[TCP Server] WSAStartup failed :" << WSAGetLastError() << endl;
		return;
	}



	serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serverSocket == INVALID_SOCKET)
	{
		cout << "[TCP Server] Error at socket(): %ld\n", WSAGetLastError();
		WSACleanup();
		return;
	}


	// 주소 구조체 설정, 소켓 정보
	SOCKADDR_IN listenAddr = {};
	listenAddr.sin_family = AF_INET;
	listenAddr.sin_port = htons(serverPort);
	listenAddr.sin_addr.s_addr = INADDR_ANY;//접속받을 소켓 ANY
	//inet_pton(AF_INET, "127.0.0.1", &(serverAddr.sin_addr.s_addr));


	//bind
	result = bind(serverSocket, (sockaddr*)&listenAddr, sizeof(listenAddr));
	if (result == SOCKET_ERROR)
	{
		cout << "[TCP Server] bind failed with error: " << WSAGetLastError() << endl;
		closesocket(serverSocket);
		WSACleanup();
		return;
	}


	//listen(클라이언트의 접근 요청에 수신 대기열 생성)
	result = listen(serverSocket, SOMAXCONN);
	if (result == SOCKET_ERROR)
	{
		cout << "[TCP Server] Listen failed with error: " << WSAGetLastError() << endl;
		closesocket(serverSocket);
		WSACleanup();
		return;
	}


	//accept(클라이언트 연결 대기)
	while (true)
	{
		cout << "[TCP Server] waiting clients...\n";

		SOCKADDR_IN clientAddr = {};
		int clientAddrSize = sizeof(clientAddr);

		SOCKET clientSocket = accept(serverSocket, (SOCKADDR*)&clientAddr, &clientAddrSize);

		if (clientSocket == INVALID_SOCKET)
		{
			cout << "[TCP Server] accept failed: " << WSAGetLastError() << endl;
			//WSACleanup();
			break;
		}

		string addrStr = AddrToStr(clientAddr);

		//클라이언트 생성 콜백
		if (Connected != NULL)
			Connected(addrStr);

		dicClientSockets.insert({ addrStr,clientSocket });

		std::thread receiveMSG(&TCPServer::RecvThr, this, addrStr, clientSocket);
		receiveMSG.detach();
	}


	if (serverClosedCallback != NULL)
		serverClosedCallback();


	//ServerStop();
	cout << "[TCP Server] 서버 완전 종료 \n";
}


void TCPServer::RecvThr(string clientAddr, SOCKET clientSocket)
{
	while (true)
	{
		char recvbuf[BUFLEN] = {};
		int result = recv(clientSocket, recvbuf, BUFLEN, 0);


		if (result == 0)
		{
			//연결이 정상적으로 닫힌 경우
			cout << "[TCP Server] Connection Closed. " << clientAddr << endl;
			break;
		}
		else if (result < 0)
		{
			//오류
			cout << "[TCP Server] TCP recv() failed with error code: " << WSAGetLastError() << "\n";
			break;
		}


		if (DataReceived != NULL)
			DataReceived((BYTE*)recvbuf, result);
	}

	//현재 클라이언트 소켓 제거
	closesocket(clientSocket);
	dicClientSockets.erase(clientAddr);


	//클라이언트 제거 콜백
	if (clientDisconnectCallback != NULL)
		clientDisconnectCallback(clientAddr);
}



void TCPServer::ServerStop()
{
	cout << "[TCP Server] TcpServer Stop.." << endl;

	DataReceived = NULL;

	if (!dicClientSockets.empty())
	{
		//연결된 전체 소켓 닫아주기
		for (auto itr = dicClientSockets.begin(); itr != dicClientSockets.end(); itr++) {
			shutdown(itr->second, SD_BOTH);
			closesocket(itr->second);
		}
		dicClientSockets.clear();
	}

	if (serverSocket != NULL)
	{
		//Server
		int result = shutdown(serverSocket, SD_BOTH);//closesocket(itr->second);
		closesocket(serverSocket);
		WSACleanup();

		serverSocket = NULL;

		if (result == -1)
			cout << "[TCP Server] shutdown failed: " << WSAGetLastError();
	}

}


//데이터 전송(전체 클라이언트)
BOOL TCPServer::SendData(BYTE* data, int len)
{
	for (auto itr = dicClientSockets.begin(); itr != dicClientSockets.end(); itr++)
		return SendData(data, len, itr->first, itr->second);
}


//데이터 전송(특정 클라이언트)
BOOL TCPServer::SendData(BYTE* data, int len, string clientAddr)
{
	if (dicClientSockets.find(clientAddr) != dicClientSockets.end())
		return SendData(data, len, clientAddr, dicClientSockets[clientAddr]);
}


//데이터 전송
BOOL TCPServer::SendData(BYTE* data, int len, string clientAddr, SOCKET clientSock)
{
	if (clientSock == -1 || clientSock == NULL)
	{
		//cout << "ClientSocketError clientSOkcet : " << clientSocket << endl;
		return false;
	}


	bool iResult = send(clientSock, (char*)data, len, 0);

	if (iResult == -1)
	{
		std::cerr << "[TCP Server][ERROR] Send failed: " << WSAGetLastError() << "\n";

		return false;
	}

	return true;
}


string TCPServer::AddrToStr(sockaddr_in addr)
{
	//ip
	char ipstr[16];
	PCSTR result = inet_ntop(addr.sin_family, &addr.sin_addr, ipstr, 16);

	//port
	char portstr[6];
	sprintf_s(portstr, "%d", addr.sin_port);

	//ip:port
	string ipportstr = ipstr;
	ipportstr.append({ ':','\0' });
	ipportstr.append(portstr);


	return ipportstr;
}