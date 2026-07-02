#include "TCPClass.h"
#include <iostream>

TCPClass::TCPClass() {
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "[ERROR] Can't load 'winsock.dll'\n";
        exit(EXIT_FAILURE);
    }
    //isConnected = false;
    //pTCPSocketThread = nullptr;
}

TCPClass::~TCPClass() {
    CloseServer();
}

void TCPClass::Start() {
    sSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sSocket == INVALID_SOCKET) {
        std::cerr << "[ERROR] Listen Socket Error\n";
        return;
    }

    memset(&sAddr, 0, sizeof(sAddr));
    sAddr.sin_family = AF_INET;
    //sAddr.sin_port = htons(TCPPORT);
    sAddr.sin_port = htons(tcpPort);
    sAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

    if (bind(sSocket, (sockaddr*)&sAddr, sizeof(sAddr)) == SOCKET_ERROR) {
        std::cerr << "[ERROR] BIND ERROR!\n";
        closesocket(sSocket);
        return;
    }

    if (listen(sSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "[ERROR] LISTEN ERROR!\n";
        closesocket(sSocket);
        return;
    }

    std::cout << "[INFO] 서버 리스닝 시작!\n";

    std::thread clientThread(&TCPClass::HandleClientConnection, this);
    clientThread.detach();
}
void TCPClass::HandleClientConnection() {
    while (true) {
        int sizeCAddr = sizeof(cAddr);

        SOCKET tmpSock = accept(sSocket, (sockaddr*)&cAddr, &sizeCAddr);
        if (tmpSock == INVALID_SOCKET) {
            std::cerr << "[ERROR] ACCEPT ERROR\n";
            continue;
        }

        ClientInfo tmpInfo;
        tmpInfo.cSock = tmpSock;
        tmpInfo.isConnected = true;
        if (tmpInfo.parseThread)
        {
            tmpInfo.parseThread->join();
            delete tmpInfo.parseThread;
            tmpInfo.parseThread = nullptr;
        }
        tmpInfo.parseThread = new std::thread(&TCPClass::RecvPacketFunc, this, tmpInfo);

        clientSockets.push_back(tmpInfo);

        /*
        cSocket = accept(sSocket, (sockaddr*)&cAddr, &sizeCAddr);
        if (cSocket == INVALID_SOCKET) {
            std::cerr << "[ERROR] ACCEPT ERROR\n";
            continue;
        }

        isConnected = true;
        std::cout << "[INFO] 클라이언트 연결됨!\n";

        if (pTCPSocketThread) {
            pTCPSocketThread->join();
            delete pTCPSocketThread;
            pTCPSocketThread = nullptr;
        }

        pTCPSocketThread = new std::thread(&TCPClass::RecvPacketFunc, this);
        */
    }
}
/*
void TCPClass::HandleClientConnection() {
    while (true) {
        int sizeCAddr = sizeof(cAddr);
        cSocket = accept(sSocket, (sockaddr*)&cAddr, &sizeCAddr);

        if (cSocket == INVALID_SOCKET) {
            std::cerr << "[ERROR] ACCEPT ERROR\n";
            continue;
        }

        isConnected = true;

        if (pTCPSocketThread == nullptr) {
            pTCPSocketThread = new std::thread(&TCPClass::RecvPacketFunc, this);
        }
    }
}
*/
/*
void TCPClass::RecvPacketFunc() {
    int recvLen;
    unsigned char recvBuffer[RECV_EVENT_DATA_SIZE] = { 0 };

    while (isConnected) 
    {
        recvLen = recv(cSocket, reinterpret_cast<char*>(recvBuffer), RECV_EVENT_DATA_SIZE, 0);

        if (recvLen > 0) 
        {
            std::cout << "[Client] 받은 데이터: " << recvLen << " 바이트\n";

            std::unique_lock<std::shared_mutex> lock(queueMutex);
            recvEventQueue.push(std::vector<unsigned char>(recvBuffer, recvBuffer + recvLen));
            std::cout << "[Queue] 현재 큐 크기: " << recvEventQueue.size() << "\n";

            
            //if (DataReceived) {
            //    DataReceived(std::vector<unsigned char>(recvBuffer, recvBuffer + recvLen));
            //}
            
        }
        else if (recvLen == 0 || recvLen == SOCKET_ERROR) {
            std::cerr << "[ERROR] 클라이언트 연결 종료 또는 recv() 실패! 오류 코드: " << WSAGetLastError() << "\n";
            isConnected = false;
            break;
        }
    }
}
*/
void TCPClass::RecvPacketFunc(ClientInfo clientInfo) {
    int recvLen;
    unsigned char recvBuffer[RECV_EVENT_DATA_SIZE] = { 0 };

    while (clientInfo.isConnected)
    {
        recvLen = recv(clientInfo.cSock, reinterpret_cast<char*>(recvBuffer), RECV_EVENT_DATA_SIZE, 0);

        if (recvLen > 0)
        {
            std::cout << "[Client] 받은 데이터: " << recvLen << " 바이트\n";

            std::unique_lock<std::shared_mutex> lock(queueMutex);
            recvDataQueue.push(std::vector<unsigned char>(recvBuffer, recvBuffer + recvLen));
            std::cout << "[Queue] 현재 큐 크기: " << recvDataQueue.size() << "\n";

        }
        else if (recvLen == 0 || recvLen == SOCKET_ERROR) {
            std::cerr << "[ERROR] 클라이언트 연결 종료 또는 recv() 실패! 오류 코드: " << WSAGetLastError() << "\n";
            clientInfo.isConnected = false;
            break;
        }
    }
}
bool TCPClass::HasData() {
    std::shared_lock<std::shared_mutex> lock(queueMutex);
    return !recvDataQueue.empty();
}

std::vector<unsigned char> TCPClass::GetNextData() {
    std::unique_lock<std::shared_mutex> lock(queueMutex);
    std::cout << "[Queue] GetNextData 크기: " << recvDataQueue.size() << "\n";
    if (!recvDataQueue.empty()) {
        std::vector<unsigned char> data = recvDataQueue.front();
        recvDataQueue.pop();
        return data;
    }
    return {};
}



void TCPClass::CloseServer() {
    closesocket(sSocket);
    //closesocket(cSocket);
    //WSACleanup();

    /*
    if (pTCPSocketThread) {
        pTCPSocketThread->join();
        delete pTCPSocketThread;
        pTCPSocketThread = nullptr;
    }
    */
    for (int i = 0; i < clientSockets.size(); i++)
    {
        closesocket(clientSockets[i].cSock);
        if (clientSockets[i].parseThread) {
            clientSockets[i].parseThread->join();
            delete clientSockets[i].parseThread;
            clientSockets[i].parseThread = nullptr;
        }
    }

    WSACleanup();
}