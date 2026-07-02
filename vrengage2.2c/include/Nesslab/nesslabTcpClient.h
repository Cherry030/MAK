#pragma once

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <thread>
#include <functional>
#include <string>
#include <cstdint>

#pragma comment(lib, "ws2_32.lib")


class nesslabTcpClient
{
public:

	enum class ConnectionState : uint8_t
	{
		Disconnected,
		Connecting,
		Connected
	};

	//TcpClient 사용자 정의 에러((WSAGetLastError 코드X, errorCode 15000 이상 사용)
	enum class ClientError
	{
		InvalidPortRange = 15000,
		InvalidIpString,
		AlreadyRunning,
		InvalidThreadAccess,
		UndefinedSocketError
	};


	using DataReceivedCallback = std::function<void(const uint8_t* data, int len)>;
	using ConnectionChangedCallback = std::function<void(ConnectionState connectionState)>;
	using SocketErrorCallback = std::function<void(int errorCode, const std::string& message)>;




	//Constructor 
	nesslabTcpClient() = delete;
	nesslabTcpClient(unsigned int port, const std::string& ip);
	~nesslabTcpClient();


	bool Connect();
	void Disconnect();
	bool SendData(const uint8_t* data, int len);


	// Set Callbacks
	void SetOnDataReceived(DataReceivedCallback cb);
	void SetOnConnectionChanged(ConnectionChangedCallback cb);
	void SetOnSocketError(SocketErrorCallback cb);


	const std::string& GetServerIP()		 const { return serverIP; }
	unsigned int		GetServerPort()		 const { return serverPort; }
	ConnectionState		GetConnectionState() const { return connectionState; }


private:

	//Endpoint
	const std::string	serverIP{ "" };
	const unsigned int	serverPort{ 0 };

	// Socket & thread
	SOCKET		clientSocket{ INVALID_SOCKET };
	std::thread clientThr;

	ConnectionState connectionState{ ConnectionState::Disconnected };

	//Callback
	DataReceivedCallback	  onDataReceived;
	ConnectionChangedCallback onConnectionChanged;//재연결이 필요하면 별도 스레드에서 Connect()
	SocketErrorCallback		  onSocketError;


	bool TryCreateSocketAndConnect(int& errorCode);
	void RunClientThread();
	void ReceiveLoop();
	void NotifySocketError(int errorCode);
	void SetConnectionStateAndNotify(ConnectionState state);
};
