#pragma once

#include "export.h"
#include "vrvCore/DtWindow.h"
#include "vrePlayerStation/playerStation.h"
#include "vrePlayerStation/playerComponent.h"
#include "vrvCore/DtObserver.h"
#include "vrvCore/DtObserverObjectAgent.hpp"
#include "vrvCore/DtDisplay.h"
#include "vrvCore/DtWindowManager.h"
#include <vector>

//Forward Declaration
struct WindowPosition;

namespace nesslab_frontend_plugins {

	constexpr auto screenPluginType = "DtObserver";


	class ScreenWindowPlugin : public makVre::DtPlayerComponent
	{
	public:
		//CTOR
		ScreenWindowPlugin();
		//DTOR
		~ScreenWindowPlugin() override;

		//Base class abstract functions need to be defined
		bool initialize(makVre::DtPlayerStation* player, makVre::DtInitTable& config) override;
		void tick(double dt) override;
		void shutdown() override;
		//const std::string& type() override;//사용x => TODO: type 추가하는 순간 에러나는데 추후에 원인찾아보기
		//bool postInitialize() override;
		const char* type() const override;

	private:

		void InitPlugin();
		void PostInitPlugin();

		void WindowStart();
		void CreateWindowsOnScreens(std::vector<WindowPosition> windowPos);

		//update Screen Observer Transforms
		void SetScreenObserversTransform();
		
		//Aim Window
		void CreateWeaponAimWindow();

		void CreateObserver(const std::string& obName);

		//ConfigFile
		size_t	screenWindowCount = 6;
		bool	isFullscreen = false;
		bool	isWeaponAimWindowVisible = false;


		std::vector<makVrv::DtWindowConfiguration> screenWindows{};
		std::vector<makVrv::DtObserver*> screenObservers{};

	};

}