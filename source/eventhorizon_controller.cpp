#include "eventhorizon_controller.h"

#include "vstgui/plugin-bindings/vst3editor.h"

using namespace Steinberg;

namespace EventHorizon {

//------------------------------------------------------------------------
tresult PLUGIN_API EventHorizonController::initialize (FUnknown* context)
{
	tresult result = EditControllerEx1::initialize (context);
	if (result != kResultOk)
		return result;

	return result;
}

//------------------------------------------------------------------------
tresult PLUGIN_API EventHorizonController::terminate ()
{
	return EditControllerEx1::terminate ();
}

//------------------------------------------------------------------------
tresult PLUGIN_API EventHorizonController::setComponentState (IBStream* /*state*/)
{
	return kResultOk;
}

//------------------------------------------------------------------------
IPlugView* PLUGIN_API EventHorizonController::createView (FIDString name)
{
	if (FIDStringsEqual (name, Vst::ViewType::kEditor))
	{
		auto* view = new VSTGUI::VST3Editor (this, "view", "eventhorizon.uidesc");
		return view;
	}
	return nullptr;
}

//------------------------------------------------------------------------
} // namespace EventHorizon
