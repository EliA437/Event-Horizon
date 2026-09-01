#pragma once

#include "public.sdk/source/vst/vsteditcontroller.h"

namespace EventHorizon {

//------------------------------------------------------------------------
class EventHorizonController : public Steinberg::Vst::EditControllerEx1
{
public:
	EventHorizonController () = default;
	~EventHorizonController () SMTG_OVERRIDE = default;

	static Steinberg::FUnknown* createInstance (void* /*context*/)
	{
		return (Steinberg::Vst::IEditController*)new EventHorizonController;
	}

	Steinberg::tresult PLUGIN_API initialize (Steinberg::FUnknown* context) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API terminate () SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API setComponentState (Steinberg::IBStream* state) SMTG_OVERRIDE;
	Steinberg::IPlugView* PLUGIN_API createView (Steinberg::FIDString name) SMTG_OVERRIDE;

	DEFINE_INTERFACES
	END_DEFINE_INTERFACES (EditController)
	DELEGATE_REFCOUNT (EditController)
};

//------------------------------------------------------------------------
} // namespace EventHorizon
