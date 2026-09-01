#include "eventhorizon_controller.h"

#include "ui/eventhorizon_keyboard_ui.h"

#include "vstgui/plugin-bindings/vst3editor.h"

#include <string_view>

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
	if (playerDelegate)
	{
		delete playerDelegate;
		playerDelegate = nullptr;
	}
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
VSTGUI::IController* EventHorizonController::createSubController (VSTGUI::UTF8StringPtr _name,
                                                                 const VSTGUI::IUIDescription* /*description*/,
                                                                 VSTGUI::VST3Editor* editor)
{
	std::string_view name (_name);
	if (name == "KeyboardController")
	{
		if (playerDelegate == nullptr)
		{
			playerDelegate = createKeyboardPlayerDelegate (
			    getPeer (), [this] () { return allocateMessage (); });
		}
		return createKeyboardController (editor, playerDelegate, keyboardStartNote);
	}
	return nullptr;
}

//------------------------------------------------------------------------
} // namespace EventHorizon
