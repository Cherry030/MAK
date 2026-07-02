#pragma once


#include <winsock2.h>
#include <ws2tcpip.h>


#include <cstdint>
#include <functional>
#include <string>
#include <thread>
#include <iostream>
#include <sstream>
#include <vector>

#pragma comment(lib, "Ws2_32.lib")


class nesslabUdpServer
{
public:

    enum class PacketTransmissionMode : uint8_t
    {
        Unicast,   // 1:1
        Multicast, // 224.0.0.0 ~ 239.255.255.255
        Broadcast  // 255.255.255.255
    };

    enum class ServerState : uint8_t
    {
        Stopped,
        Running
    };


    //사용자 정의 에러((WSAGetLastError 코드X, errorCode 15000 이상 사용)
    enum class ServerError
    {
        InvalidPortRange = 15000,
        InvalidIpString,
        AlreadyRunning,
        InvalidThreadAccess,
        UndefinedSocketError
    };


    using DataReceiveCallback  = std::function<void(unsigned char* data, int len)>;
    using StateChangedCallback = std::function<void(ServerState serverState)>;
    using SocketErrorCallback  = std::function<void(int errorCode, const std::string& message)>;


    nesslabUdpServer() = delete;
    nesslabUdpServer(unsigned int port, PacketTransmissionMode transmissionMode);//Unicast
    nesslabUdpServer(unsigned int port, PacketTransmissionMode transmissionMode, const std::string& destIP);//Multicast,BroadCast
    ~nesslabUdpServer();


    bool ServerStart();
    void ServerStop();

    //마지막으로 데이터 받은 addr로 전송(Unicast)
    bool SendData(const std::uint8_t* data, int len);
    //특정 sendAddr로 전송
    bool SendData(const std::uint8_t* data, int len, const sockaddr_in& sendAddr);

    
    //Set Callback Func
    void SetOnDataReceived(DataReceiveCallback cb);
    void SetOnStateChanged(StateChangedCallback cb);
    void SetOnSocketError(SocketErrorCallback cb);


    unsigned int GetServerPort()                    const { return serverPort; }
    PacketTransmissionMode GetTransmissionMode()   const { return transmissionMode; }
    std::string GetMulticastIP()                    const { return destIP; }

    // "ip:port"
    std::string AddrToStr(const sockaddr_in& addr) const;

    //SocketOption,flag
    bool enableReuseAddr      = true;//ReuseAddr(주소 재사용)
    bool enableMulticastLoop  = true;//멀티캐스트 패킷 루프백
    bool receiveSelfBroadcast = false;


private:
    void StartThr();
    void RecvLoop();
    

    SOCKET          serverSocket = INVALID_SOCKET;
    unsigned int    serverPort = 5010;
    std::thread     serverThr;
    ServerState     serverState{ ServerState::Stopped };
    PacketTransmissionMode transmissionMode = PacketTransmissionMode::Unicast;
    
    std::string destIP = "";
    sockaddr_in sendAddr{};
    std::vector<std::string> ipAddresses;
 
    //Callback
    DataReceiveCallback  onDataReceived;
    StateChangedCallback onServerStateChanged;
    SocketErrorCallback	 onSocketError;
  
    bool TryCreateSocketAndConfigure(int& errorCode);
    void SetServerStateAndNotify(ServerState state);
    void NotifySocketError(int errorCode);
    std::vector<std::string> GetLocalIPAddresses();
    
};