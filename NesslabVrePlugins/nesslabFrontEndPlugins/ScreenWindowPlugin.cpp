#include "ScreenWindowPlugin.h"
#include "Nesslab/nesslabCommon.h"
#include "vrvCore/DtDe.h"
#include "vrvCore/DtDriverManager.h"
#include "vrvCore/DtInputDriver.h"

#define M_PI        3.14159265358979323846
constexpr double  observerHeight = 1.6f;


using namespace makVre;
using namespace makVrv;


struct DisplayInfo {
	RECT  rcMonitor;//위치,해상도
	std::string displayName;
};


struct WindowPosition
{
	int posX;
	int posY;
};


//WeaponPosePlugin.cpp에서 사용
bool gInitScreenPlugin = false;

std::string weaponAimObsName{ "WeaponAimingPointOb" };//실제 조준 처리하는 옵저버
std::string aimOriginObsName{ "aimOriginObserver" };  //무기 조준점 옵저버 원점



/*
	TODO: 경사진곳에서 위아래 적을 어떻게 처리할지 고민해보기

*/


namespace nesslab_frontend_plugins {


		//콜백 함수: 모니터가 하나 열거될 때마다 호출
		BOOL CALLBACK OnMonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM data) {


			//std::vector<DisplayInfo>& monitorInfos = *reinterpret_cast<std::vector<DisplayInfo>*>(data);
			std::vector<DisplayInfo>* displays = reinterpret_cast<std::vector<DisplayInfo>*>(data);


			MONITORINFOEX monitor = {};
			monitor.cbSize = sizeof(monitor);

			if (GetMonitorInfo(hMonitor, &monitor)) {
				displays->push_back({ monitor.rcMonitor, monitor.szDevice });
			}

			return TRUE;
		}



		std::vector<DisplayInfo> GetAllMonitorOrigins() {

			std::vector<DisplayInfo> displays{};

			// 모든 모니터 열거 → 각 모니터에 대해 MonitorEnumProc 호출(콜백), dwData = MonitorEnumProc 함수에 직접 전달하는 사용자 데이터 
			EnumDisplayMonitors(nullptr, nullptr, OnMonitorEnumProc, reinterpret_cast<LPARAM>(&displays));//displays 주소 전달

			std::cout << "[ScreenPlugin][Debug] displayCount : " << displays.size() << "\n";

			if (!displays.empty())
			{
				//모니터 좌상단 위치값이 낮은 순으로 정렬
				std::sort(displays.begin(), displays.end(),
					[](const DisplayInfo& displayA, const DisplayInfo& displayB) {
						return displayA.rcMonitor.left < displayB.rcMonitor.left;
					}
				);
				
				for (const DisplayInfo& display : displays) {
					std::cout << "[ScreenPlugin][Debug] Display Name: " << display.displayName
						<< ", Display Position X: " << display.rcMonitor.left << ", Y: " << display.rcMonitor.top
						<< ", Display Resolution : " << (display.rcMonitor.right - display.rcMonitor.left) << "x" << (display.rcMonitor.bottom - display.rcMonitor.top) << std::endl;
				}
			}

			return displays;
		}




		ScreenWindowPlugin::ScreenWindowPlugin()
			:DtPlayerComponent()
		{
			std::cout << "[ScreenPlugin][Trace] Entering function: Constructor()" << std::endl;
		}


		ScreenWindowPlugin::~ScreenWindowPlugin()
		{
			std::cout << "[ScreenPlugin][Trace] Entering function: Destructor()" << std::endl;
		}

		void ScreenWindowPlugin::shutdown()
		{
			std::cout << "[ScreenPlugin][Trace] Entering function: shutdown()" << std::endl;

			//base class shutdown
			DtPlayerComponent::shutdown();
		}



		const char* ScreenWindowPlugin::type() const
		{
			return screenPluginType;
		}



		//bool compareByLeft(const DisplayInfo& a, const DisplayInfo& b) {
		//	return a.rcMonitor.left < b.rcMonitor.left;
		//}


		static uint8_t tickCount = 0;

		bool ScreenWindowPlugin::initialize(makVre::DtPlayerStation* player, makVre::DtInitTable& config)
		{
			//do base class init.  Shouldn't fail
			if (!DtPlayerComponent::initialize(player, config))
			{
				return false;
			}

			std::cout << "[ScreenPlugin][Trace] Entering function: initialize()" << std::endl;


			InitPlugin();


			return true;
		}


		void ScreenWindowPlugin::InitPlugin()
		{

			gInitScreenPlugin = false;
			tickCount = 0;

			//Get ConfigFile Data
			{
				using namespace nesslab_common;

				//Screen
				screenWindowCount = GetPrivateProfileInt("Screen", "screenWindowCount", INI_INT_NOT_FOUND_DEFAULT, CONFIG_FILE_PATH.c_str());

				//FullScreen(Bool)
				int result = GetPrivateProfileInt("Screen", "fullscreenEnabled", INI_INT_NOT_FOUND_DEFAULT, CONFIG_FILE_PATH.c_str());
				isFullscreen = (result == 1) ? true : false;

				//WeaponAimWindow(Bool)
				result = GetPrivateProfileInt("Screen", "showWeaponAimWindow", INI_INT_NOT_FOUND_DEFAULT, CONFIG_FILE_PATH.c_str());
				isWeaponAimWindowVisible = (result == 1) ? true : false;


				if (screenWindowCount == INI_INT_NOT_FOUND_DEFAULT)
					LOG_WARN(LOG_WARN_CHANNEL_NAME) << "[ScreenPlugin][Error] Failed to read data from INI file. Using default value.\n";


				std::cout << "[ScreenPlugin][Debug] screenWindowCount: " << screenWindowCount << ", isFullscreen: " << (bool)isFullscreen << ", isWeaponAimWindowVisible_: " << (bool)isWeaponAimWindowVisible << std::endl;

			}


			//Create Screen Window
			WindowStart();


			//Create Weapon Aiming point Window
			if (isWeaponAimWindowVisible)
				CreateWeaponAimWindow();

		}

		void ScreenWindowPlugin::PostInitPlugin()
		{
			//스크린 옵저버 위치+회전 처리
			SetScreenObserversTransform();

			//조준점 옵저버의 원점 옵저버
			CreateObserver(aimOriginObsName);

			//실제 조준처리하는 옵저버 생성
			CreateObserver(weaponAimObsName);

			//스크린 초기화 완료 플래그 -> WeaponPosePlugin에서 사용
			gInitScreenPlugin = true;

		}





		void ScreenWindowPlugin::tick(double dt)
		{
	

			if (tickCount == 0)
			{
				/*
				//Create Screen Window
				WindowStart();


				//Create Weapon Aiming point Window
				if (isWeaponAimWindowVisible)
					CreateWeaponAimWindow();
				*/

				tickCount++;

			}
			else if (tickCount == 1)
			{
				//한 프레임 이후에 처리 필요(엔진멈춤 현상) -> SetScreenObserversTransform 함수에서 옵저버 주소를 못읽어오는 듯

				PostInitPlugin();

				tickCount++;
			}	
		}


		void ScreenWindowPlugin::WindowStart()
		{
			if (screenWindowCount == 0) return;

			std::vector<WindowPosition> windowPos = {};
			std::vector<DisplayInfo>	displays = GetAllMonitorOrigins();

			std::cout << "[ScreenPlugin][Debug] displaySize(): " << displays.size() << std::endl;


			//TODO: 모니터 7대는 스크린 설치하면 테스트 + display.rcMonitor.left == 0가 Main Monitor인지 확인
			//풀스크린이면 화면 하나에 하나씩 배치
			if (displays.size() == 7 || isFullscreen)
			{

				for (DisplayInfo& display : displays)
				{
					if (display.rcMonitor.left == 0) continue;

					//windowPos.push_back({ display.rcMonitor.left, display.rcMonitor.right });
					windowPos.push_back({ display.rcMonitor.left, 0 });
				}

				CreateWindowsOnScreens(windowPos);

			}
			else if (!displays.empty())
			{

				int winPosX = displays.at(0).rcMonitor.left;
				int winPosY = displays.at(0).rcMonitor.top;


				int horizontalWindowCount = screenWindowCount / 2;//가로 방향으로 생성되는 윈도우 개수
				//int verticalWndowCount	= screenWindowCount/2;


				//홀수
				horizontalWindowCount = (screenWindowCount % 2 == 1) ? horizontalWindowCount + 1 : horizontalWindowCount;


				const int addWinSizeX = (displays.at(0).rcMonitor.right - displays.at(0).rcMonitor.left) / horizontalWindowCount;
				const int addWinSizeY = (displays.at(0).rcMonitor.bottom - displays.at(0).rcMonitor.top) / 2;


				//절반은 화면위
				for (int i = 0; i < horizontalWindowCount; ++i)
				{
					//windowPos.push_back({ winPosX, winPosY });
					windowPos.push_back({ winPosX, 0 });
					winPosX += addWinSizeX;
				}

				winPosX = displays.at(0).rcMonitor.left;
				winPosY += addWinSizeY;


				//절반은 화면아래 
				for (int i = horizontalWindowCount; i < screenWindowCount; ++i)
				{
					//windowPos.push_back({ winPosX, winPosY });
					windowPos.push_back({ winPosX, 0 });
					winPosX += addWinSizeX;
				}


				CreateWindowsOnScreens(windowPos);
			}
		}



		void ScreenWindowPlugin::CreateWindowsOnScreens(std::vector<WindowPosition> windowPos)
		{

			if (screenWindowCount == 0 || windowPos.size() != screenWindowCount)
			{
				LOG_WARN(nesslab_common::LOG_WARN_CHANNEL_NAME) << "[ScreenPlugin][Error] The number of windows to create does not match the number of windowPosition entries." ;
				return;
			}

			//QHD, [0]=Width, [1]=Height
			constexpr std::array<int, 2> displayResolution{ 2560, 1440 };



			//WindowManager
			makVrv::DtWindowManager& myWinmanager = myDe->display()->windowManager();
			std::string displayName = myDe->display()->configuration().name();

			int winRowCount = screenWindowCount / 2;
			winRowCount = (screenWindowCount % 2 == 1) ? winRowCount + 1 : winRowCount;

			const int winSizeX = displayResolution[0] / winRowCount;
			const int winSizeY = displayResolution[1] / 2;
			int projBounds = (360 / screenWindowCount) / 2;

			
			//※카메라가 2대보다 작으면 각이 커져서 화면이 안보는거 방지
			if (screenWindowCount <= 2)
			{
				projBounds = 60;
			}


			for (int i = 0; i < screenWindowCount; i++)
			{

				std::string windowName	 = "nesslabWindow"	+ std::to_string(i);
				std::string channelName	 = "nesslabCh"		+ std::to_string(i);
				std::string observerName = "nesslabOb"		+ std::to_string(i);


				if (!myWinmanager.findWindow(windowName))
				{

					//Configuration
					DtWindowConfiguration	myWindowConfig(windowName);
					DtChannelConfiguration	myChannelConfig(channelName);
					DtObserverConfiguration myObserverConfig(observerName);


					//WindowConfiguration
					myWindowConfig.setWindowType(makVrv::DtWindowConfiguration::Window);//FullscreenWindow, FramelessWindow
					myWindowConfig.setPosition(windowPos.at(i).posX, windowPos.at(i).posY);
					myWindowConfig.setSize(winSizeX, winSizeY);//qhd 2560, 1440



					//ChannelConfiguration
					//myChannelConfig.setViewport(1, 99, 1, 99);
					myChannelConfig.setObserverName(observerName);//Set observer
					myChannelConfig.setProjectionResizePolicy(makVrv::DtChannelConfiguration::ProjectionResizePolicy::VERTICAL);//ReSize VERTICAL


					std::cout << "[ScreenPlugin][Debug] projBounds=> " << projBounds << std::endl;

					//투영범위
					myChannelConfig.setProjection(makVrv::DtChannelConfiguration::FieldOfViewAngles,
						-projBounds,	// left   
						projBounds,		// right  
						-45,			// bottom  
						45,				// top    
						0.01,			// NEAR_Z
						1000			// FAR_Z
					);



					//Add Observer //※추가안하면 흰색 화면만 나옴
					myDe->driverManager().inputDriver().addObserverConfiguration(myObserverConfig);

				

					//VRF -> appData폴더에서 default_Sensor 검색하면 Sensor 확인가능(ex IR, Visual Camera....)
					DtObserver* observer = myDe->driverManager().inputDriver().findObserverByName(myChannelConfig.observerName());
					if (observer != nullptr)
					{

						//Set Observer Type
						observer->setAttachType(DtObserverObject::AttachTypeTether);

						//AttachOb
						DtObserver::ElementList attachList;
						attachList.push_back(myPlayer->elementId());
						//※무기변경시 옵저버 위치변경 방지(MAK Support PS-12193)
						observer->setUsesBoundingVolumeForAttachedOffsets(false);
						observer->setPrimaryAttachment(attachList);


						observer->setAttachedLocation(0, 0, 0);
						observer->setAttachedOrientation(0, 0, 0);

						
						screenObservers.push_back(observer);
					}


					//add channelConfiguration
					myWindowConfig.channelConfigurations().add(myChannelConfig);


					//Create Window
					if (myWinmanager.addWindow(displayName, myWindowConfig))
						std::cout << "[ScreenPlugin][Debug] Create Screen Window " << std::endl;

				}
				else
				{
					
					//윈도우가 이미 존재하면 옵저버 설정만 변경(캐릭터가 변경되어서 setPrimaryAttachment 필요)
					DtObserver* observer = myDe->driverManager().inputDriver().findObserverByName(observerName);
					if (observer != nullptr)
					{
						//Set Observer Type
						observer->setAttachType(DtObserverObject::AttachTypeTether);

						//AttachOb
						DtObserver::ElementList attachList;
						attachList.push_back(myPlayer->elementId());
						observer->setUsesBoundingVolumeForAttachedOffsets(false);
						observer->setPrimaryAttachment(attachList);

						observer->setAttachedLocation(0, 0, 0);
						observer->setAttachedOrientation(0, 0, 0);

						screenObservers.push_back(observer);
					}
				}


				if (isFullscreen)
					myWinmanager.findWindow(windowName)->toggleFullScreen();

			}
		}



		//Create Weapon aiming point Window
		void ScreenWindowPlugin::CreateWeaponAimWindow()
		{
			int projBounds = 30;		

			//WindowManager
			makVrv::DtWindowManager& myWinmanager = myDe->display()->windowManager();
			std::string displayName = myDe->display()->configuration().name();

			std::string windowName   = "WeaponAimingPointWin";
			std::string channelName  = "WeaponAimingPointCh";
			std::string observerName = "WeaponAimingPointOb";
			
			if (!myWinmanager.findWindow(windowName))
			{

				//Configuration
				DtWindowConfiguration	myWindowConfig(windowName);
				DtChannelConfiguration	myChannelConfig(channelName);
				DtObserverConfiguration myObserverConfig(observerName);


				//WindowConfiguration
				myWindowConfig.setWindowType(makVrv::DtWindowConfiguration::Window);
				myWindowConfig.setPosition(0, 0);
				myWindowConfig.setSize(700, 700);


				//ChannelConfiguration
				myChannelConfig.setObserverName(observerName);//Set observer
				myChannelConfig.setProjectionResizePolicy(makVrv::DtChannelConfiguration::ProjectionResizePolicy::FIXED);//ReSize Fixed

				//투영범위
				myChannelConfig.setProjection(makVrv::DtChannelConfiguration::FieldOfViewAngles,
					-projBounds,	// left   
					projBounds,		// right  
					-45,			// bottom  
					45,				// top 
					0.01,			// NEAR_Z
					1000			// FAR_Z
				);
				

				//Add Observer //※추가안하면 흰색 화면만 나옴
				myDe->driverManager().inputDriver().addObserverConfiguration(myObserverConfig);


				DtObserver* observer = myDe->driverManager().inputDriver().findObserverByName(myChannelConfig.observerName());
				if (observer != nullptr)
				{
					//Set Observer Type
					observer->setAttachType(DtObserverObject::AttachTypeTether);

					//AttachOb
					DtObserver::ElementList attachList;
					attachList.push_back(myPlayer->elementId());
					observer->setUsesBoundingVolumeForAttachedOffsets(false);
					observer->setPrimaryAttachment(attachList);

					observer->setAttachedLocation(0, 0, 0);
					observer->setAttachedOrientation(0, 0, 0);

				}


				//Add channelConfiguration
				myWindowConfig.channelConfigurations().add(myChannelConfig);


				//Create Window
				if (myWinmanager.addWindow(displayName, myWindowConfig))
					std::cout << "[ScreenPlugin][Debug] Create Weapon Aim Window " << std::endl;

			}

		}




		//Update Screen Observer Transforms
		void ScreenWindowPlugin::SetScreenObserversTransform()
		{
			if (screenWindowCount == 0) return;

			double topoOri = 0;

			//Screen Observers
			for (auto observer : screenObservers)
			{
				if (observer != nullptr)
				{				
					observer->agent().findObject()->setTopographicOrientation(topoOri, 0, 0);//Ori
					observer->agent().findObject()->moveAttachedUp(observerHeight);//Pos

					topoOri += (2 * M_PI / screenWindowCount);
				}
			}

		}
	

		//weaponAimObs, aimOriginObs
		void ScreenWindowPlugin::CreateObserver(const std::string& obName)
		{

			DtObserver* observer = myDe->driverManager().inputDriver().findObserverByName(obName);
			if (observer == nullptr)
			{
				DtObserverConfiguration myObserverConfig(obName);

				//Add Observer 
				myDe->driverManager().inputDriver().addObserverConfiguration(myObserverConfig);

				observer = myDe->driverManager().inputDriver().findObserverByName(obName);
			}

			
			//Set Observer Type
			observer->setAttachType(DtObserverObject::AttachTypeTether);

			//AttachObs
			DtObserver::ElementList attachList;
			attachList.push_back(myPlayer->elementId());
			observer->setUsesBoundingVolumeForAttachedOffsets(false);
			observer->setPrimaryAttachment(attachList);


			observer->setAttachedLocation(0, 0, 0);
			observer->setAttachedOrientation(0, 0, 0);


		}

}