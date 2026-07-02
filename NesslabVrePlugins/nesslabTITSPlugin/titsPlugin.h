#pragma once

#include "export.h"

#include "vrePlayerStation/playerComponent.h"

#include "titsAccessory.h"


namespace nesslab_frontend_plugins
{
	// Component type string used for registration and in role configuration.
	constexpr auto titsPluginType = "DtTITS";

	class TITSPlugin : public makVre::DtPlayerComponent
	{

	public:
		TITSPlugin();
		virtual ~TITSPlugin() override;

		virtual bool initialize(makVre::DtPlayerStation* player, makVre::DtInitTable& config);
		virtual void tick(double dt);
		virtual void shutdown();
		virtual const char* type() const override;

		virtual bool postInitialize() override;

		void setHelmetPatchState(uint8_t state);
		void setBodyPatchState(uint8_t state);
		void setHandPatchState(uint8_t state);
	protected:

		std::string helmetAttrName;
		std::string bodyAttrName;
		std::string handAttrName;
	};
}
namespace makVrv
{
	class DtDe;
}
;