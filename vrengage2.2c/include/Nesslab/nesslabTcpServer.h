#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <cstdint>
#include <thread>
#include <map>
#include <functional>
#include <vector>

#pragma comment(lib, "Ws2_32.lib")


class nesslabTcpServer
{
public:

	enum class ServerState : uint8_t {
		Stopped,
		Running
	};


	//사용자 정의 에러((WSAGetLastError 코드X, errorCode 15000 이상 사용)
	enum class ServerError {
		InvalidPortRange = 15000,
		AlreadyRunning,
		InvalidThreadAccess,
		UndefinedSocketError
	};


	enum class ClientConnectionState : uint8_t {
		Disconnected,
		Connected
	};


	using DataReceivedCallback = std::function<void(const std::string& client, const uint8_t* data, int len)>;
	using ClientConnectionCallback = std::function<void(ClientConnectionState state, const std::string& clientAddr)>;//Connected/DisConnected
	using StateChangedCallback = std::function<void(ServerState serverState)>;
	using SocketErrorCallback = std::function<void(int errorCode, const std::string& message)>;


	nesslabTcpServer() = delete;
	nesslabTcpServer(unsigned int serverPort);
	~nesslabTcpServer();

	void ServerStart();
	void ServerStop();

	bool SendData(const uint8_t* data, int len);
	bool SendData(const uint8_t* data, int len, const std::string& clientAddr);


	// Callback setters
	void SetOnDataReceived(DataReceivedCallback cb);
	void SetOnClientConnectionChanged(ClientConnectionCallback cb);
	void SetOnServerStateChanged(StateChangedCallback cb);
	void SetOnSocketError(SocketErrorCallback cb);


	ServerState	GetServerState() const { return serverState; }
	uint16_t	GetPort()		 const { return serverPort; }


	//SocketOption
	bool enableReuseAddr = true;//ReuseAddr(주소 재사용)


private:

	struct ConnectedClient {
		SOCKET sock = INVALID_SOCKET;
		std::thread thr;
	};


	SOCKET			serverSocket = INVALID_SOCKET;
	const uint16_t	serverPort = 0;
	std::thread		serverThr;
	ServerState		serverState = ServerState::Stopped;


	// key=>"ip:port"
	std::map<std::string, ConnectedClient> connectedClients;

	// Callback
	DataReceivedCallback		onDataReceived;
	ClientConnectionCallback	onClientConnectionChanged;
	StateChangedCallback		onServerStateChanged;
	SocketErrorCallback			onSocketError;

	void RunClientThread();
	bool TryCreateSocketAndConnect(int& errorCode);
	void ClientRecvLoop(const std::string& clientAddr, SOCKET clientSocket);

	std::string AddrToStr(const sockaddr_in& addr);
	void NotifySocketError(int errorCode);
	void SetServerStateAndNotify(ServerState state);
	void CleanupDisconnectedClients();

};