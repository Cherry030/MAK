#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstdint>
#include <functional>
#include <string>
#include <thread>
#include <iostream>
#include <vector>

//Link with ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

//Visual Studio 2019 (v142)
//Windows SDK 10.0

class nesslabUdpClient
{
public:

	//패킷 전송 방식
	enum class PacketTransmissionMode : uint8_t
	{
		Unicast,
		Multicast, //224.0.0.0 ~ 239.255.255.255.255
		Broadcast  //255.255.255.255
	};
	 
	
	enum class ClientState : uint8_t
	{
		Stopped,
		Running 
	};


	//사용자 정의 에러((WSAGetLastError 코드X, errorCode 15000 이상 사용)
	enum class ClientError
	{
		InvalidPortRange = 15000,
		InvalidIpString,
		AlreadyRunning,
		InvalidThreadAccess,
		UndefinedSocketError
	};


	using DataCallback			= std::function<void(const std::uint8_t* data, int len)>;
	using StateChangedCallback	= std::function<void(ClientState state)>;
	using SocketErrorCallback	= std::function<void(int errorCode, const std::string& message)>;


	nesslabUdpClient() = delete;
	nesslabUdpClient(unsigned int port, const std::string& ip, PacketTransmissionMode transmissionMode);
	~nesslabUdpClient();


	bool Connect();
	void Disconnect();
	bool SendData(const std::uint8_t* data, int len);


	//Set CallbackFunction
	void SetOnDataReceived(DataCallback cb);
	void SetOnClientStateChanged(StateChangedCallback cb);
	void SetOnSocketError(SocketErrorCallback cb);


	unsigned int			GetServerPort() const { return serverPort; }
	std::string				GetServerIP()	 const { return serverIP; }
	PacketTransmissionMode	GetTransmissionMode() const { return transmissionMode; }

	
	//SocketOption,flag
	bool enableReuseAddr	  = true;//ReuseAddr(주소 재사용)
	bool enableMulticastLoop  = true;//멀티캐스트 패킷 루프백
	bool receiveSelfBroadcast = false;

private:

	void StartThr();
	void RecvLoop();


	// Endpoint
	std::string  serverIP	= "127.0.0.1";
	unsigned int serverPort = 0;

	PacketTransmissionMode transmissionMode = PacketTransmissionMode::Unicast;
	ClientState clientState{ ClientState::Stopped };


	//socket
	SOCKET      clientSocket = INVALID_SOCKET;
	sockaddr_in serverAddr{};

	std::thread      clientThr;

	//Callback Fuc
	DataCallback		 onDataReceivedCb;
	StateChangedCallback onClientStateChanged;//재연결이 필요하면 별도 스레드에서 Connect()
	SocketErrorCallback	 onSocketError;


	bool TryCreateSocketAndConfigure(int& errorCode);
	void NotifySocketError(int errorCode);
	void SetServerStateAndNotify(ClientState state);

	std::vector<std::string> GetLocalIPAddresses();

	std::vector<std::string> ipAddresses;

};
