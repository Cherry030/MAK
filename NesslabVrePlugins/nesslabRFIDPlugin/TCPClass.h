#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <shared_mutex>
#include <queue>
#include <mutex>
#include <functional>

#pragma comment(lib, "Ws2_32.lib")

#define TCPPORT 6767//4000
#define RECV_EVENT_DATA_SIZE 256

typedef unsigned char DATAPACKET[RECV_EVENT_DATA_SIZE];

class ClientInfo
{
public:
    SOCKET cSock;
    bool isConnected = false;
    std::thread* parseThread = nullptr;
    //std::queue<std::vector<unsigned char>> recvQueue;
    /*
    //생성자
    ClientInfo(SOCKET sock, bool connected)
        : cSock(sock), isConnected(connected), parseThread(nullptr) {
    }
    */
};

class TCPClass {
public:
    TCPClass();              // 생성자
    ~TCPClass();             // 소멸자
    void Start();            // 서버 시작
    void CloseServer();      // 서버 종료
    bool HasData(); 
    std::vector<unsigned char> GetNextData();
    int tcpPort = 6767;
    //std::function<void(std::vector<unsigned char>)> DataReceived;
private:

    SOCKET sSocket, cSocket;
    sockaddr_in sAddr, cAddr;
    std::vector<ClientInfo> clientSockets;
    WSADATA wsaData;
    bool isConnected;
    std::thread* pTCPSocketThread;
    std::queue<std::vector<unsigned char>> recvDataQueue;
    std::shared_mutex queueMutex;

    void HandleClientConnection();  // 클라이언트 연결 처리
    //void RecvPacketFunc();          // 데이터 수신 처리
    void RecvPacketFunc(ClientInfo clientInfo);
};