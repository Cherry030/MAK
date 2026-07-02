#pragma once

#include "export.h"
#include "framework/vreInput/inputDevice.h"
#include "framework/vreInput/vreInputManager.h"
#include <thread>



//Forward declaration
namespace rfid_to_vris_packets {
	struct TraineeIdPacket;
}


namespace nesslab_frontend_plugins
{

	class RFIDPlugin : public makVre::DtInputDevice
	{
	public:
		RFIDPlugin();
		virtual ~RFIDPlugin();

		//! init is called once on app startup
		virtual bool init(makVre::DtVreInputManager& mgr) override;

		//! shutdown is called once during app shutdown
		virtual void shutdown() override;

		//! tick is called once for every frame. dt gives us the time
		//! elapsed (in seconds) since the last frame. Custom device
		//! input should be gathered between tick events and send to the
		//! VreInputManager for processing during tick().
		virtual void tick(double dt) override;

		// Required to compile, not currently used
		virtual void reportCurrentState() {};


	private:
		void InitPlugin();

		//Callback Function
		void OnDataReceived(const std::string& clientAddr, const uint8_t* data, int len);
		void OnDsmsConnected();
		void OnDsmsDisconnected();
		void OnTraineeIdPacket(const rfid_to_vris_packets::TraineeIdPacket& packet);


		void HandleDsmsReconnectThr();
		void SendDataToDsms();


		//Flag indicating traineeID was received from RFID.
		bool isTraineeIdReceived = false;
		std::thread netThread;
		std::thread reconnectThread;

	};
}
