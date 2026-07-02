#include <iomanip>
#include <fstream>
#include <iostream>
#include <string>

#include <chrono>
#include <atomic>
#include <mutex>
#include <condition_variable>


#include "Manipulator.h"
#include "TCPClient.h"
#include "UDPServer.h"
#include "framework/vreInput/vreInputManager.h"

using namespace makVre;






//※※※※※※
//AMMS(Active Manipulator)  -> 중형 트레드밀에서 사용
//PMMS(Passive Manipulator) -> 소형 트레드밀에서 사용 
//※※※※※※


//모든 메시지에 공통으로 붙는 최소한의 필드
//페이로드(payload) 해석을 위한 메타데이터, 프레임 경계 제외
#pragma pack(push, 1)
struct CommonHeader
{
	// STX,ETX같은 프레임 경계 마커(framing marker)(frame delimiters)는 제외

	BYTE deviceID;//발신 디바이스 ID
	BYTE msgID;//메시지 ID
	BYTE msgType;//예약
	uint16_t length;//전체 프레임 길이

};
#pragma pack(pop)


//UDP
#pragma region ManipulatorToMakIF


//Manipulator(PMMS,AMMS) --고장 MSG--> MAK Plugin(VRIS)
#pragma pack(push, 1)
struct IFManipulatorFaultDownMSG
{
	BYTE		STX			= 0x02;//Fixed
	BYTE		deviceID	= 0x00;//발신 디바이스 ID(0x31 = PMMS, 0x41 = AMMS)
	BYTE		msgID		= 0x00;//메시지 ID       (0x32 = PMMS 고장 발생, 0x42 = AMMS 고장 발생)
	BYTE		msgType		= 0x00;//Reserved
	uint16_t	length		= 0x00;//전체 프레임 길이 => 생성자에서 할당

	//ㅡㅡㅡㅡData(Payload)ㅡㅡㅡㅡ
	BYTE		faultDown	= 0;//고장 상태, 0 = Not Connected, 1 = Normal, 2 = Error

	BYTE		ETX			= 0x03;//Fixed

	IFManipulatorFaultDownMSG() :length(sizeof(IFManipulatorFaultDownMSG)) {}


	void debugPrint()
	{
		std::cout << "STX: "	<< static_cast<int>(STX)
			<< ", deviceID: "	<< static_cast<int>(deviceID)
			<< ", msgID: "		<< static_cast<int>(msgID)
			<< ", msgType: "	<< static_cast<int>(msgType)
			<< ", length: "		<< static_cast<int>(length)
			<< ", faultDown: "	<< static_cast<int>(faultDown)
			<< ", ETX: "		<< static_cast<int>(ETX);
	}

};
#pragma pack(pop)




//Manipulator(PMMS,AMMS) --장비 상태 MSG--> MAK Plugin(VRIS)
#pragma pack(push, 1)
struct IFManipulatorStatusMSG
{
	BYTE		STX					= 0x02;//Fixed
	BYTE		deviceID			= 0x00;//발신 디바이스 ID(0x31 = PMMS, 0x41 = AMMS)
	BYTE		msgID				= 0x00;//메시지 ID       (0x31 = PMMS 상태 정보, 0x41 = AMMS 상태 정보)
	BYTE		msgType				= 0x00;//Reserved
	uint16_t	length				= 0x00;//전체 프레임 길이 => 생성자에서 할당


	//ㅡㅡㅡㅡData(Payload)ㅡㅡㅡㅡ
	BYTE		opStatus			= 0; //동작 상태, 비주기, 0 = 전원 OFF, 1 = 전원ON, 2 = 모션 ON, 3 = 모션 ERROR, 4 = 비상 정지
	float		deviceOrientation	= 0x00;//매니퓰레이터 방향(=훈련자의 전방 각도)


	BYTE		ETX					= 0x03;//Fixed

	IFManipulatorStatusMSG() :length(sizeof(IFManipulatorStatusMSG)) {}

	void debugPrint()
	{
		std::cout << "STX: " << static_cast<int>(STX)
			<< ", deviceID: " << static_cast<int>(deviceID)
			<< ", msgID: " << static_cast<int>(msgID)
			<< ", msgType: " << static_cast<int>(msgType)
			<< ", length: " << static_cast<int>(length)
			<< ", opStatus: " << static_cast<int>(opStatus)
			<< ", deviceOrientation: " << deviceOrientation
			<< ", ETX: " << static_cast<int>(ETX);
	}

};
#pragma pack(pop)


#pragma endregion




//TCP
#pragma region MakToMonitoringIF

//MAK --장비 상태 MSG--> Monitoring
#pragma pack(push, 1)
struct IFMonitoringStatusMSG
{
	BYTE		STX			= 0x02;//Fixed
	BYTE		deviceID	= 0x71;//발신 디바이스 ID(0x71 = VRIS)
	BYTE		msgID		= 0x00;//메시지 ID       (0x31 = PMMS 상태 정보, 0x41 = AMMS 상태 정보)
	BYTE		msgType		= 0x00;//Reserved
	uint16_t	length		= 0x00;//전체 프레임 길이 => 생성자에서 할당

	//ㅡㅡㅡㅡData(Payload)ㅡㅡㅡㅡ
	BYTE		opStatus = 0;//0: 전원 OFF, 1: 전원 ON, 2: 모션 ON, 3: 모션 ERROR, 4: 비상 정지

	BYTE		ETX			= 0x03;//Fixed

	IFMonitoringStatusMSG() :length(sizeof(IFMonitoringStatusMSG)) {}


	void debugPrint()
	{
		std::cout << "STX: " << static_cast<int>(STX)
			<< ", deviceID: " << static_cast<int>(deviceID)
			<< ", msgID: " << static_cast<int>(msgID)
			<< ", msgType: " << static_cast<int>(msgType)
			<< ", length: " << static_cast<int>(length)
			<< ", opStatus: " << static_cast<int>(opStatus)
			<< ", ETX: " << static_cast<int>(ETX);
	}



};
#pragma pack(pop)




//MAK --고장 MSG--> Monitoring
#pragma pack(push, 1)
struct IFMonitoringFaultDownMSG
{
	BYTE		STX			= 0x02;//Fixed
	BYTE		deviceID	= 0x71;//발신 디바이스 ID(0x71 = VRIS)
	BYTE		msgID		= 0x00;//메시지 ID       (0x32 = PMMS 고장 발생, 0x42 = AMMS 고장 발생)
	BYTE		msgType		= 0x00;//Reserved
	uint16_t	length		= 0x00;//전체 프레임 길이 => 생성자에서 할당

	//ㅡㅡㅡㅡData(Payload)ㅡㅡㅡㅡ
	BYTE		faultDown	= 0;//고장 상태, 0 = Not Connected, 1 = Normal, 2 = Error

	BYTE		ETX			= 0x03;//Fixed

	IFMonitoringFaultDownMSG() :length(sizeof(IFMonitoringFaultDownMSG)) {}

	void debugPrint()
	{
		std::cout << "STX: " << static_cast<int>(STX)
			<< ", deviceID: " << static_cast<int>(deviceID)
			<< ", msgID: " << static_cast<int>(msgID)
			<< ", msgType: " << static_cast<int>(msgType)
			<< ", length: " << static_cast<int>(length)
			<< ", faultDown: " << static_cast<int>(faultDown)
			<< ", ETX: " << static_cast<int>(ETX);
	}


};
#pragma pack(pop)

#pragma endregion













//function declaration
//void sendManipulatorDeviceStatus(const BYTE& deviceStatus);//매니퓰레이터 장비 상태 데이터 전송


//Callback Function
void onDeviceDataReceived(BYTE* data, int len);//받은데이터 콜백
//void onManiStatusDisconnectCallback();



//UDP_Multicast(Manipulator -> MAK)
UDPServer*	manipUdpServer		= nullptr;
string		manipMulticastIP	= "127.0.0.1";
int			manipMulticastPort	= -1;
IFManipulatorFaultDownMSG manipFaultDownPacket;//매니퓰레이터 고장 메시지 패킷
IFManipulatorStatusMSG manipStatusPacket;      //매니퓰레이터 장비 상태 메시지 패킷



//TCP(MAK -> MonitoringPC(DSMS)
TCPClient*	monitoringTcpClient = nullptr;
string		monitoringIP		= "127.0.0.1";
int			monitoringPort		= -1;
IFMonitoringStatusMSG	 monitoringStatusPacket;	//모니터링 장비 상태 메시지 패킷
IFMonitoringFaultDownMSG monitoringFaultDownPacket;//모니터링 고장 메시지 패킷




//TCP Client 종료되었을때 재접속 대기 시간(초)
constexpr int retryDelaySec = 5000;



//테스트
std::atomic_bool isManiPluginDestroying = false;
std::mutex mtxMani;  // mutex
std::condition_variable cvMani;  // 조건 변수
thread ManiStatusDisConnectedThr;



static constexpr BYTE STX = 0x02;   // Start of Text
static constexpr BYTE ETX = 0x03;   // End of Text

//DeviceID
static constexpr BYTE pmmsDeviceID = 0x31;
static constexpr BYTE ammsDeviceID = 0x41;


//MSG_ID_Device
static constexpr BYTE pmmsStatusMsgID = 0x31;
static constexpr BYTE pmmsFaultDownMsgID = 0x32;
static constexpr BYTE ammsStatusMsgID = 0x41;
static constexpr BYTE ammsFaultDownMsgID = 0x42;



//MSG_ID_Monitoring
static constexpr BYTE dsmsPmmsStatusMsgID = 0x31;
static constexpr BYTE dsmsPmmsFaultDownMsgID = 0x32;
static constexpr BYTE dsmsAmmsStatusMsgID = 0x41;
static constexpr BYTE dsmsAmmsFaultDownMsgID = 0x42;



static constexpr int FRAME_HEADER_SIZE = sizeof(CommonHeader);
static constexpr int MIN_FRAME_SIZE = 1 //STX
									  + FRAME_HEADER_SIZE
									  + 1; //ETX



//Constructor
Manipulator::Manipulator()
{
	std::cout << "[ManipulatorPlugin][TRACE] Constructor " << std::endl;

	if (!extractConfigData()) return;


	isManiPluginDestroying = false;


	//매니퓰레이터 -> MAK
	manipUdpServer = new UDPServer(manipMulticastPort, UDPServer::PacketTransmissionModes::Multicast, manipMulticastIP);
	manipUdpServer->DataReceived = onDeviceDataReceived;
	manipUdpServer->UDPServerStart();


	//MAK -> 모니터링
	monitoringTcpClient = new TCPClient(monitoringPort, monitoringIP);
	monitoringTcpClient->ClientStart();


}




//Destructor
Manipulator::~Manipulator()
{
	std::cout << "[ManipulatorPlugin][TRACE] Destructor " << std::endl;

	//	std::lock_guard<std::mutex> lock(mtxMani);  // mutex로 보호
	//isManiPluginDestroying = true;
	isManiPluginDestroying.store(true);//좀 더 찾아보기


	if (manipUdpServer != nullptr)
	{
		manipUdpServer->UDPServerStop();
		delete manipUdpServer;
		manipUdpServer = nullptr;
	}

	if (monitoringTcpClient != nullptr)
	{
		monitoringTcpClient->ClientStop();
		delete monitoringTcpClient;
		monitoringTcpClient = nullptr;
	}



	cvMani.notify_all();  // 대기 중인 스레드가 이 신호를 받고 종료



	//if (ManiStatusDisConnectedThr.joinable()) {
	//	cout << " ManiStatusDisConnectedThr.joinable() \n";
	//	ManiStatusDisConnectedThr.join();
	//}


}





//Config 파일에서 데이터 가져오기
bool Manipulator::extractConfigData()
{
	std::cout << "[ManipulatorPlugin][TRACE] Entering function: extractConfigData() " << std::endl;


	std::ifstream readFile;
	std::string filePath = "Config\\NesslabPluginConfig.txt";
	std::vector<string> result = {};

	readFile.open(filePath);


	if (readFile.is_open())
	{

		string tmp;
		while (getline(readFile, tmp))
		{
			//if (!tmp.empty())
			result.push_back(tmp);
		}

		readFile.close();

		//for (const auto line : result)
		//{
		//	std::cout << "File Data => " << line << std::endl;
		//}

		if (!result.empty())
		{
			//UDP_Multicast(Manipulator -> MAK)
			manipMulticastIP = result[60];
			manipMulticastPort = stoi(result[61]);

			//TCP(MAK -> Monitoring)
			monitoringIP = result[2];
			monitoringPort = stoi(result[64]);



			std::cout << "[ManipulatorPlugin][DEBUG] manipMulticastIP : " << manipMulticastIP << ", manipMulticastPort : " << manipMulticastPort << ", monitoringIP : " << monitoringIP << ", monitoringPort : " << monitoringPort << endl;


			return true;
		}
		else
		{
			std::cerr << "[ManipulatorPlugin][ERROR] No data found in file. " << filePath << std::endl;
		}
	}
	else
	{
		std::cerr << "[ManipulatorPlugin][ERROR] Unable to open file " << filePath << std::endl;
	}

	return false;
}






vector<BYTE> manipBuffer = {};


//햅틱 데이터 파싱
std::vector<BYTE> extractHapticBuffer()
{
	vector<BYTE> result = {};


	unsigned int stxIndex = -1;

	//STX
	for (unsigned int i = 0; i < manipBuffer.size(); ++i)
	{
		if (manipBuffer.at(i) == manipFaultDownPacket.STX)
		{
			stxIndex = i;
			break;
		}
	}

	if (stxIndex == -1)
	{
		cout << "[HapticPlugin][ERROR] STX not found in buffer. " << endl;
		manipBuffer.clear();
		return result;
	}
	else if (stxIndex > 0)
	{
		//STX 값의 Index값을 0으로 맞춰주기
		manipBuffer.erase(manipBuffer.begin(), manipBuffer.begin() + stxIndex + 1);
	}










	//Length
	//프레임 길이
	unsigned int length = manipBuffer[4] + manipBuffer[5] * 256;


	//버퍼가 데이터 크기 보다 작다면
	if (manipBuffer.size() < length)
	{
		cout << "[HapticPlugin][ERROR] Buffer size is too small.. BufferSize :" << manipBuffer.size() << endl;

		return result;
	}




	//프레임 길이 조건이랑 비교
	if (length != manipFaultDownPacket.length)
	{
		cout << "[HapticPlugin][ERROR] Invalid Length value.. Length :" << manipBuffer[4] << endl;

		manipBuffer.erase(manipBuffer.begin(), manipBuffer.begin() + 6);
		return result;
	}





	//0x31, 0x41 패시브, 액티브 매니퓰레이터
	////Device ID
	//if (maniBuffer[1] != manipulatorToMakIF._DEVICE_ID)
	//{
	//	cout << "[HapticPlugin][ERROR] Invalid DeviceID value.. DeviceID :" << maniBuffer[1] << endl;

	//	maniBuffer.erase(maniBuffer.begin(), maniBuffer.begin() + 1);
	//	return result;
	//}




	//ETX
	//size_t hapticSize = sizeof(_IF_Manipulator_To_MAK);
	//if (manipBuffer[hapticSize - 1] != manipFaultDownPacket.ETX)
	//{
	//	cout << "[HapticPlugin][ERROR] Invalid ETX value.. ETX :" << manipBuffer[hapticSize - 1] << endl;

	//	manipBuffer.erase(manipBuffer.begin(), manipBuffer.begin() + hapticSize);
	//	return result;
	//}



	result.assign(manipBuffer.begin(), manipBuffer.begin() + length);
	manipBuffer.erase(manipBuffer.begin(), manipBuffer.begin() + length);



	return result;
}



//STX, Device_ID, MSG_ID , Length, ETX
//공통 고정 데이터 STX,  ETX
//개별 데이터 : Device_ID MSG_ID, Length
//위에서 Device_ID,MSG_ID해주고
//파싱부분에서 STX,Length, ETX 비교해주면 될 듯?


static void parseBuffer() 
{
	size_t offset = 0;
	while (manipBuffer.size() - offset >= MIN_FRAME_SIZE)
	{

		//헤더 저장
		CommonHeader hdr;
		std::memcpy(&hdr, manipBuffer.data() + offset, FRAME_HEADER_SIZE);

		//// 네트워크 바이트 오더라면 ntohs/ntohl 처리
		//uint16_t msgID = ntohs(hdr.msgID);
		//uint32_t length = ntohl(hdr.length);




		//STX
		unsigned int stxIndex = -1;
		for (unsigned int i = 0; i < manipBuffer.size(); ++i)
		{
			if (manipBuffer.at(i) == STX)
			{
				stxIndex = i;
				break;
			}
		}

		if (stxIndex == -1)
		{
			cout << "[HapticPlugin][ERROR] STX not found in buffer. " << endl;
			manipBuffer.clear();
			break;
		}
		else if (stxIndex > 0)
		{
			//STX 값의 Index값을 0으로 맞춰주기
			manipBuffer.erase(manipBuffer.begin(), manipBuffer.begin() + stxIndex + 1);
		}





		//Length
		//버퍼가 전체 프레임 길이보다 작다면 메시지 더 받을 때까지 대기
		if (manipBuffer.size() - offset < hdr.length)
		{		
			cout << "[HapticPlugin][ERROR] Buffer size is too small.. BufferSize :" << manipBuffer.size() << endl;

			break;
		}



		//ETX
		if (manipBuffer[hdr.length - 1] != ETX)
		{
			cout << "[HapticPlugin][ERROR] Invalid ETX value.. ETX :" << manipBuffer[hdr.length - 1] << endl;

			manipBuffer.erase(manipBuffer.begin(), manipBuffer.begin() + hdr.length);
			break;
		}
		

		//구조체 or 페이로드 잘라서 사용

		
		//페이로드 포인터
		//const uint8_t* payload = buffer.data() + offset + HEADER_SIZE;

		//메시지 처리
		//dispatchMessage(msgID, payload, length);


		//버퍼에서 소비한 부분 제거
		//offset += HEADER_SIZE + length;


		

		//발신 디바이스 ID




	}
	// 남은 데이터만 복사해서 버퍼 재구성
	
	/*
	임시 제거
	buffer.erase(buffer.begin(), buffer.begin() + offset);

	*/

}


/*
IFManipulatorFaultDownMSG manipFaultDownPacket;//매니퓰레이터 고장 메시지 패킷
IFManipulatorStatusMSG manipStatusPacket;      //매니퓰레이터 장비 상태 메시지 패킷
IFMonitoringStatusMSG	 monitoringStatusPacket;	//모니터링 장비 상태 메시지 패킷
IFMonitoringFaultDownMSG monitoringFaultDownPacket;//모니터링 고장 메시지 패킷


*/


//메시지 처리
void dispatchMessage(const BYTE deviceId, const BYTE msgID, const BYTE* data, const size_t len)
{
	//Passive Manipulator
	if (deviceId == pmmsDeviceID)
	{
		//MSG ID
		//pmms 상태
		if (msgID == pmmsStatusMsgID)
		{
			//패킷에 복사
			//memcpy(&manipStatusPacket, data, len);

			//	BYTE		opStatus			= 0; //동작 상태, 비주기, 0 = 전원 OFF, 1 = 전원ON, 2 = 모션 ON, 3 = 모션 ERROR, 4 = 비상 정지
			//float		deviceOrientation = 0x00;//매니퓰레이터 방향(=훈련자의 전방 각도)
			
			manipStatusPacket.deviceID = data[1];
			manipStatusPacket.msgID = data[2];
			manipStatusPacket.msgType = data[3];

			/*
			
			 // big → host 오더 변환
			float tmp;
			std::memcpy(&tmp, data + 5, sizeof(tmp));  // 4바이트 복사
			tmp = ntohl(tmp); // big → host 오더 변환
			memcpy(&manipStatusPacket.deviceOrientation, &tmp, sizeof(tmp));//복사
				
			*/

			//Payload
			manipStatusPacket.opStatus = data[4];
			memcpy(&manipStatusPacket.deviceOrientation, data + 5, sizeof(float));//deviceOrientation
			



			
			//출력
			manipStatusPacket.debugPrint();



		
			//sendDataToMonitoring

		}
		//pmms 고장
		else if (msgID == pmmsFaultDownMsgID)
		{
			//패킷에 복사
			memcpy(&manipFaultDownPacket, data, len);

			//출력
			manipFaultDownPacket.debugPrint();
		}
		else
			cerr << "[ManipulatorPlugin][ERROR] MSG ID does not match, MSG : " << (int)msgID << endl;


	}
	//Active Manipulator
	else if (deviceId == ammsDeviceID)
	{
		//MSG ID
		//amms 상태
		if (msgID == ammsStatusMsgID)
		{
			//패킷에 복사
			memcpy(&manipStatusPacket, data, len);

			//출력
			manipStatusPacket.debugPrint();

		}
		//amms 고장
		else if (msgID == ammsFaultDownMsgID)
		{
			//패킷에 복사
			memcpy(&manipFaultDownPacket, data, len);

			//출력
			manipFaultDownPacket.debugPrint();

		}
		else
			cerr << "[ManipulatorPlugin][ERROR] MSG ID does not match, MSG : " << (int)msgID << endl;

	}
	else
		cerr << "[ManipulatorPlugin][ERROR] Device ID does not match, DeviceID : " << (int)deviceId << endl;
}





//데이터 받으면 콜백
void onDeviceDataReceived(BYTE* data, int len)
{
	//받은 데이터 출력(hex byte)
	std::cout << "[ManipulatorPlugin][DEBUG] Received Data from Manipulator Device" << endl;
	std::cout << "Data Length : " << len << ", Data : ";
	std::cout << std::hex << std::uppercase;//16진수 + 대문자
	for (int i = 0; i < len; ++i)
	{
		cout << std::setw(2) << std::setfill('0') << static_cast<int>(data[i]) << " ";
	}
	std::cout << std::dec << endl;

	

	//Vector 타입 버퍼에 저장
	//insert(vector 삽입할 위치, 배열 삽입 시작위치, 배열 삽입 끝위치)
	manipBuffer.insert(manipBuffer.end(), data, data + len);


	////Vector 타입 버퍼에 저장
	//for (int i = 0; i < len; ++i)
	//	manipBuffer.push_back(data[i]);



	//최소 데이터 크기보다 크면 파싱
	while (manipBuffer.size() >= MIN_FRAME_SIZE)
	{
		vector<BYTE> result = extractHapticBuffer();


		if (result.size() == 0)
			continue;


		BYTE DEVICE_ID = result[1];
		BYTE MSGID = result[2];
		BYTE opStatus = 0x00;


		//PMMS(Passive Manipulator)
		if (DEVICE_ID == 0x31)
		{
			//BYTE FaultDown = 0;//고장 상태, 비주기, 0 = Not Connected, 1 = Normal, 2 = Error // [6]
			//BYTE OpStatus = 0; //동작 상태, 비주기, 0 = 전원 OFF, 1 = 전원ON, 2 = 모션 ON, 3 = 모션 ERROR, 4 = 비상 정지 [7]

			if (MSGID == 0x31)
			{
				//Passive Manipulator 고장 발생
				cout << "[ManipulatorPlugin][DEBUG] Passive Manipulator Fault" << endl;

				BYTE faultDown = result[6];
				cout << "faultDown => " << (int)faultDown << endl;


				switch (faultDown)
				{
				case 0://Not Connected
					opStatus = 0x00;
						break;
				case 1://Normal
					opStatus = 0x01;
					break;
				case 2://Error
					opStatus = 0x02;
					break;
				default:
					break;
				}

				//sendManipulatorDeviceStatus(opStatus);
				

			}
			else if (MSGID == 0x32)
			{
				//Passive Manipulator 상태 정보 

				cout << "[ManipulatorPlugin][DEBUG] Passive Manipulator Status" << endl;

				BYTE OpStatus = result[7];
				cout << "OpStatus => " << (int)OpStatus << endl;

				switch (OpStatus)
				{
				case 0://전원 OFF
					opStatus = 0x00;
					break;
				case 1://전원 ON
					opStatus = 0x01;
					break;
				case 2://모션 ON
					opStatus = 0x01;
					break;
				case 3://모션 ERROR
					opStatus = 0x02;
					break;
				case 4://비상 정지
					opStatus = 0x02;
					break;
				default:
					break;
				}

				//sendManipulatorDeviceStatus(opStatus);
			}
			else
			{
				cout << "[ManipulatorPlugin][ERROR] MSGID != 0x31, 0x32     MSGID : " <<(int)MSGID << endl;
			}



		}
		//AMMS(Active Manipulator)
		else if (DEVICE_ID == 0x41)
		{
			if (MSGID == 0x41)
			{

				//Active Manipulator 고장 발생
				cout << "[ManipulatorPlugin][DEBUG] Active Manipulator Fault" << endl;

				BYTE faultDown = result[6];
				cout << "faultDown => " << (int)faultDown << endl;

				switch (faultDown)
				{
				case 0://Not Connected
					opStatus = 0x00;
					break;
				case 1://Normal
					opStatus = 0x01;
					break;
				case 2://Error
					opStatus = 0x02;
					break;
				default:
					break;
				}


				//sendManipulatorDeviceStatus(opStatus);
			}
			else if (MSGID == 0x42)
			{

				//Active Manipulator 상태 정보 
				cout << "[ManipulatorPlugin][DEBUG] Active Manipulator Status" << endl;


				BYTE OpStatus = result[7];
				cout << "OpStatus => " << (int)OpStatus << endl;


				switch (OpStatus)
				{
				case 0://전원 OFF
					opStatus = 0x00;
					break;
				case 1://전원 ON
					opStatus = 0x01;
					break;
				case 2://모션 ON
					opStatus = 0x01;
					break;
				case 3://모션 ERROR
					opStatus = 0x02;
					break;
				case 4://비상 정지
					opStatus = 0x02;
					break;
				default:
					break;
				}

				//sendManipulatorDeviceStatus(opStatus);
			}
			else
			{
				cout << "[ManipulatorPlugin][ERROR] MSGID != 0x31, 0x32     MSGID : " << (int)MSGID << endl;
			}

		}
	}
}




//monitoringFaultDownPacket

/*
static constexpr BYTE dsmsPmmsStatusMsgID = 0x31;
static constexpr BYTE dsmsPmmsFaultDownMsgID = 0x32;
static constexpr BYTE dsmsAmmsStatusMsgID = 0x41;
static constexpr BYTE dsmsAmmsFaultDownMsgID = 0x42;
*/

//모니터링 PC에 장비 고장 메시지 데이터 전송
//void SendData_To_Monitoring(const BYTE deviceID, const BYTE msgID)
//static void sendDataToMonitoring(BYTE* data, size_t len)
static void sendDataToMonitoring(const BYTE deviceID, const BYTE msgID)
{

	
	//bool result = false;

	//모니터링 매니퓰레이터 - 상태 메시지
	if (msgID == dsmsPmmsStatusMsgID || msgID == dsmsAmmsStatusMsgID)
	{
		monitoringStatusPacket.deviceID = deviceID;
		monitoringStatusPacket.msgID = msgID;
		monitoringStatusPacket.opStatus = manipStatusPacket.opStatus;


		//uint8_t byteArray[sizeof(monitoringStatusPacket)];
		//std::memcpy(&monitoringStatusPacket, &monitoringStatusPacket, sizeof(monitoringStatusPacket));
		//result = monitoringTcpClient->SendData(byteArray, sizeof(monitoringStatusPacket));;

	}
	//모니터링 매니퓰레이터 - 고장 상태
	else if (msgID == dsmsPmmsFaultDownMsgID || msgID == dsmsAmmsFaultDownMsgID)
	{
		monitoringFaultDownPacket.deviceID = deviceID;
		monitoringFaultDownPacket.msgID = msgID;
		monitoringFaultDownPacket.faultDown = manipFaultDownPacket.faultDown;


		//uint8_t byteArray[sizeof(monitoringFaultDownPacket)];
		//std::memcpy(&monitoringStatusPacket, &monitoringStatusPacket, sizeof(monitoringFaultDownPacket));
		//result = monitoringTcpClient->SendData(byteArray, sizeof(monitoringFaultDownPacket));;
	}
	else
	{
		cerr << "[ManipulatorPlugin][ERROR] Invalid monitoring message ID, msgID:" << (int)msgID << endl;
		return;
	}
	

	/*
	임시 제거
	const unsigned int size = sizeof(_IF_MAK_TO_Monitoring);
	BYTE byteArray[size];
	std::memcpy(&byteArray, &makToMonitoringIF, size);

*/




	//uint8_t byteArray[sizeof(monitoringStatusPacket)];
	//std::memcpy(&monitoringStatusPacket, &monitoringStatusPacket, sizeof(monitoringStatusPacket));
	//result = monitoringTcpClient->SendData(byteArray, sizeof(monitoringStatusPacket));;



	//if (result)
	//{
	//	//데이터 보내기 성공
	//	std::cout << "[ManipulatorPlugin][DEBUG] Send data to MonitroingPC : ";
	//	std::cout << std::hex << uppercase;
	//	for (int i = 0; i < len; ++i)
	//		cout << std::setw(2) << std::setfill('0') << static_cast<int>(data[i]) << " ";
	//	std::cout << dec << std::endl;

	//}
	//else
	//	std::cout << "[ManipulatorPlugin][DEBUG] Failed to send debugging data to the monitoring PC. ";




	/*
	//if(deviceID == )
			//데이터 보내기 실패


	////소켓이랑 연결되어있는지 확인해주는 코드 필요하면 넣기
	////참고 if (actionTcpClient == nullptr || !isTCPServerConnected) return;

	makToMonitoringIF.OpStatus = OpStatus;


	const unsigned int size = sizeof(_IF_MAK_TO_Monitoring);
	BYTE byteArray[size];
	std::memcpy(&byteArray, &makToMonitoringIF, size);


	BYTE chkSum = 0;
	for (int i = 23; i < size - 2; ++i)
		chkSum += byteArray[i];

	byteArray[size - 2] = chkSum;


	//보낸 데이터 출력
	std::cout << "[ManipulatorPlugin][DEBUG] Send data to MonitroingPC : ";
	std::cout << std::hex << uppercase;
	for (int i = 0; i < size; ++i)
		cout << std::setw << std::setfill('0') << static_cast<int>(byteArray[i]) << " ";
	std::cout << dec << std::endl;



	monitoringTcpClient->SendData(byteArray, size);
	*/
}


/*
//TCP Server 재접속 시도 쓰레드
void onMani_Monitoring_DisconnectThr()
{
	if (isManiPluginDestroying) return;


	std::cout << "[ManipulatorPlugin][TRACE] Entering function: onManiStatusDisconnectThr() " << std::endl;


	std::unique_lock<std::mutex> lock(mtxMani);



	// retryDelaySec초 동안 대기하는데, notify_one 호출이 있으면 먼저 깨어남
	auto status = cvMani.wait_for(lock, std::chrono::seconds(retryDelaySec));


	if (status == std::cv_status::timeout)
	{
		// retryDelaySec초 동안 아무런 종료 호출(notify_one)이 없었으므로 성공으로 간주



		// 대기중에 플러그인 종료되었는지 확인
		if (isManiPluginDestroying)
			return;




		//매니퓰레이터 상태 TCP Client 재접속
		if (mani_status_tcpClient != nullptr)
		{

			delete mani_status_tcpClient;

			mani_status_tcpClient = new TCPClient(mani_status_Port, mani_status_IP);
			mani_status_tcpClient->onDisconnect = onManiStatusDisconnectCallback;
			mani_status_tcpClient->ClientStart();
		}

	}
	else {
		// 다른 스레드에 의해 notify_one()으로 깨어난 경우
		std::cout << "종료 호출 받음" << std::endl;
		return;
	}
}



//이름 변경해주기
//상태 프로그램쪽 TCP 클라이언트 종료 콜백
void onMani_Monitoring_DisconnectCallback()
{
	if (isManiPluginDestroying) return;


	std::cout << "[ManipulatorPlugin][TRACE] Entering function: onMani_Monitoring_DisconnectCallback() " << std::endl;


	//상태 프로그램 재접속 시도
	ManiStatusDisConnectedThr = thread(&onMani_Monitoring_DisconnectThr);
	ManiStatusDisConnectedThr.detach();
}
*/




/*
//MAK -> Monitoring, 장비 상태 정보 메시지
struct _IF_MAK_TO_Monitoring
{
	const BYTE	_STX = 0x02;//Fixed
	BYTE		_DEVICE_ID = 0x00;//발신 디바이스 ID
	BYTE		_MSG_ID = 0x00;//메시지 ID
	BYTE		MSG_TYPE = 0x00;//Reserved
	uint16_t	LENGTH = 0;//전체 프레임 길이 0x00(0)
	uint16_t	SEAT_ID = 0;//Seat ID
	byte		TRAINEE_ID[16] = {};//훈련자 ID
	BYTE		RESERVED_1 = 0x00;//Reserved
	BYTE		RESERVED_2 = 0x00;//Reserved


	BYTE OpStatus = 0; //동작 상태, 비주기, 0 = 전원 OFF, 1 = 전원ON, 2 = 모션 ON, 3 = 모션 ERROR, 4 = 비상 정지

	BYTE CHECKSUM = 0x00;

	const BYTE ETX = 0x03;//Fixed

};



//MAK -> Monitoring, 매니퓰레이터 - 상태 메시지
#pragma pack(push, 1)
struct _IF_MAK_TO_Monitoring_DeviceStatus
{
	BYTE	 _STX = 0x02;//Fixed
	BYTE	 _DEVICE_ID = 0x02;//발신 디바이스 ID
	BYTE	 _MSG_ID = 0x00;//메시지 ID
	BYTE	 MSG_TYPE = 0x99;//Reserved
	uint16_t LENGTH = 29;//전체 프레임 길이


	////ㅡㅡㅡㅡData(Payload)ㅡㅡㅡㅡ
	uint16_t SEAT_ID = 0x02;//Seat ID(임시값)
	byte	 TRAINEE_ID[16] = { 1,1,1,1 ,1 ,1 ,1 ,1 ,1 ,1,1,1,1,1,1,1 };//훈련자 ID
	BYTE	 RESERVED_1 = 0x00;//Reserved
	BYTE	 RESERVED_2 = 0x00;//Reserved

	//BYTE	 FaultDown = 0;//0: not connected, 1:normal, 2:error
	BYTE	 OpStatus = 0;//0: not connected, 1:normal, 2:error

	BYTE CHECKSUM = 0x00;

	const BYTE ETX = 0x03;//Fixed
};
#pragma pack(pop)








//TCP(MAK -> MonitoringPC(DSMS)
TCPClient* monitoringTcpClient = nullptr;
string		monitoringIP = "127.0.0.1";
int			monitoringPort = -1;
//_IF_MAK_TO_Monitoring makToMonitoringIF;


	//TCP_Client
	//monitoringTcpClient = new TCPClient(monitoringPort, monitoringIP);
	//monitoringTcpClient->ClientStart();




	//장비 상태 모니터링 PC로 데이터 전송(10초에 한 번씩 전송)
	//std::thread sendThread(SendStatusData_To_Monitoring);
	//sendThread.detach();
	//sendThread.join();

	//SendStatusData_To_Monitoring();



		////UDP_Multicast(Manipulator -> MAK)
	//multicastIP = "234.2.3.25";
	//multicastPort = 2325;

	////TCP(MAK -> Monitoring)
	//monitoringIP = "127.0.0.1";
	//monitoringPort = 1002;



//TCP 서버에 연결성공 여부 콜백 함수 //모니터링PC 서버랑 연결 여부
void ConnectedCallback(bool isConnected)
{
	//isServerConnected = isConnected;

	//if (isConnected)
	//{
	//	std::cout << "[ManipulatorPlugin][DEBUG] Connected to MonitoringPC server.\n";

	//	BYTE Manipulator[10] = { 1,1,1,1,1,1,1,1,1,1 };
	//	//BYTE Manipulator[10] = { 2,2,2,2,2,2,2,2,2,2 };



	//	//데이터 출력(훈련자 ID)
	//	std::cout << "======================== \n";
	//	std::cout << "[ManipulatorPlugin][DEBUG] Manipulator : ";
	//	for (auto ID : Manipulator)
	//	{
	//		std::cout << (int)ID << " ";
	//	}
	//	std::cout << "======================== \n";


	//	//Config 파일에 훈련자 ID 저장
	//	setManipulator(Manipulator);


	//	//모니터링 PC로 데이터 전송
	//	SendData_To_Monitoring(Manipulator);



	//}
	//else
	//{
	//	std::cout << "[ManipulatorPlugin][ERROR] Failed to connect to MonitoringPC server. \n";
	//}
}









//TCP 서버에 종료되면 콜백 함수
void onDisconnectCallback()
{
	//isServerConnected = FALSE;


	//traineeTcpClient = new TCPClient(MonitoringServer_Port, MonitoringServer_IP);
	//traineeTcpClient->isConnected = ConnectedCallback;
	//traineeTcpClient->onDisconnect = onDisconnectCallback;
	////traineeTcpClient->isLooping = TRUE;//연결 실패시 재접속시도
	//traineeTcpClient->isLooping = TRUE;//재접속 반복
	//traineeTcpClient->ClientStart();
}

//모니터링 PC에 장비 상태 정보 메시지 전송 // 10초에 한 번씩 전송
void SendStatusData_To_Monitoring()
{
	//소켓이랑 연결되어있는지 확인해주는 코드 필요하면 넣기
	//참고 if (actionTcpClient == nullptr || !isTCPServerConnected) return;

	//while (isRunning)


	while (isRunning)
	{
		//if (!check) continue;//데이터 수신 받으면 데이터 보내기 시작

		const unsigned int size = sizeof(_IF_MAK_TO_Monitoring_DeviceStatus);
		BYTE byteArray[size];
		std::memcpy(&byteArray, &deviceStatusToMonitoringIF, size);


		BYTE chkSum = 0;
		for (int i = 5; i < size - 2; ++i)
			chkSum += byteArray[i];

		byteArray[size - 2] = chkSum;


		//보낸 데이터 출력
		//std::cout << "[ManipulatorPlugin][DEBUG] Send Device Operation Status data to MonitroingPC : ";
		std::cout << "[ManipulatorPlugin][DEBUG] Send data to MonitroingPC : ";
		std::cout << std::hex << uppercase;
		for (int i = 0; i < size; ++i)
			cout << std::setw(2) << std::setfill('0') << static_cast<int>(byteArray[i]) << " ";
		std::cout << dec << std::endl;


		monitoringTcpClient->SendData(byteArray, size);




		//상태데이터 전송
		//SendData(byteArray[size - 3]);


		std::this_thread::sleep_for(std::chrono::seconds(10));
	}

	std::cout << "[ManipulatorPlugin][DEBUG] Thread exiting..." << std::endl;
}




//모니터링 PC에 장비 고장 메시지 데이터 전송
void SendData_To_Monitoring(const BYTE& OpStatus)
{
	//소켓이랑 연결되어있는지 확인해주는 코드 필요하면 넣기
	//참고 if (actionTcpClient == nullptr || !isTCPServerConnected) return;

	makToMonitoringIF.OpStatus = OpStatus;


	const unsigned int size = sizeof(_IF_MAK_TO_Monitoring);
	BYTE byteArray[size];
	std::memcpy(&byteArray, &makToMonitoringIF, size);


	BYTE chkSum = 0;
	for (int i = 23; i < size - 2; ++i)
		chkSum += byteArray[i];

	byteArray[size - 2] = chkSum;


	//보낸 데이터 출력
	std::cout << "[ManipulatorPlugin][DEBUG] Send data to MonitroingPC : ";
	std::cout << std::hex << uppercase;
	for (int i = 0; i < size; ++i)
		cout << std::setw << std::setfill('0') << static_cast<int>(byteArray[i]) << " ";
	std::cout << dec << std::endl;



	monitoringTcpClient->SendData(byteArray, size);
}


*/


/* 장비 상태 중간 프로그램으로 보내는 코드

//매니퓰레이터 장비 상태 -> 장비상태 관리프로그램
struct _IF_Manipulator_To_EquipmentStatusManager
{
	BYTE STX = 0x02;
	BYTE MSG_ID = 0x01;//0 = 트레드밀, 1= 매니퓰레이터, 2 = 행동/자세 인식 장비, 3 = 모의총기 장비, 4 = 햅틱 슈트 장비, 7 = 기동보조장치 
	BYTE LEN = 5;
	BYTE deviceStatus = 0x00;//

	BYTE ETX = 0x03;

	_IF_Manipulator_To_EquipmentStatusManager() :LEN(sizeof(_IF_Manipulator_To_EquipmentStatusManager)) {}
};



//MAK -> 상태 통합 프로그램 -> 모니터링 PC
TCPClient*	mani_status_tcpClient = nullptr;
string		mani_status_IP	= "127.0.0.1";
int			mani_status_Port = -1;
_IF_Manipulator_To_EquipmentStatusManager manipulatorStatusPacket;






//상태데이터 전송 MAK -> 상태 통합 프로그램 -> 모니터링PC
void sendManipulatorDeviceStatus(const BYTE& deviceStatus)
{
	BYTE data[5] = { manipulatorStatusPacket.STX, manipulatorStatusPacket.MSG_ID ,manipulatorStatusPacket.LEN, deviceStatus, manipulatorStatusPacket .ETX};


	BOOL result = mani_status_tcpClient->SendData(data, sizeof(data));

	if (result)
	{
		std::cout << "[ManipulatorPlugin][DEBUG]  Send data to EquipmentStatusManager : ";
		std::cout << std::hex << std::uppercase;
		for (int i = 0; i < sizeof(data); i++)
		{
			cout << std::setw(2) << std::setfill('0') << static_cast<int>(data[i]) << " ";
		}
		std::cout << std::dec << std::endl;
	}
	else
		std::cerr << "[ManipulatorPlugin][Error] Failed to send data to the EquipmentStatusManager ";
}

//TCP Server 재접속 시도 쓰레드
void onManiStatusDisconnectThr()
{
	if (isManiPluginDestroying) return;


	std::cout << "[ManipulatorPlugin][TRACE] Entering function: onManiStatusDisconnectThr() " << std::endl;


	std::unique_lock<std::mutex> lock(mtxMani);



	// retryDelaySec초 동안 대기하는데, notify_one 호출이 있으면 먼저 깨어남
	auto status = cvMani.wait_for(lock, std::chrono::seconds(retryDelaySec));


	if (status == std::cv_status::timeout)
	{
		// retryDelaySec초 동안 아무런 종료 호출(notify_one)이 없었으므로 성공으로 간주



		// 대기중에 플러그인 종료되었는지 확인
		if (isManiPluginDestroying)
			return;




		//매니퓰레이터 상태 TCP Client 재접속
		if (mani_status_tcpClient != nullptr)
		{

			delete mani_status_tcpClient;

			mani_status_tcpClient = new TCPClient(mani_status_Port, mani_status_IP);
			mani_status_tcpClient->onDisconnect = onManiStatusDisconnectCallback;
			mani_status_tcpClient->ClientStart();
		}

	}
	else {
		// 다른 스레드에 의해 notify_one()으로 깨어난 경우
		std::cout << "종료 호출 받음" << std::endl;
		return;
	}
}



//이름 변경해주기
//상태 프로그램쪽 TCP 클라이언트 종료 콜백
void onManiStatusDisconnectCallback()
{
	if (isManiPluginDestroying) return;


	std::cout << "[ManipulatorPlugin][TRACE] Entering function: onManiStatusDisconnectCallback() " << std::endl;


	//상태 프로그램 재접속 시도
	ManiStatusDisConnectedThr = thread(&onManiStatusDisconnectThr);
	ManiStatusDisConnectedThr.detach();
}


*/