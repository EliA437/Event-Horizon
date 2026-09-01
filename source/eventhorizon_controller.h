#pragma once

#include "ui/eventhorizon_keyboard_ui.h"
#include "public.sdk/source/vst/vsteditcontroller.h"
#include "vstgui/plugin-bindings/vst3editor.h"

namespace EventHorizon {

//------------------------------------------------------------------------
class EventHorizonController : public Steinberg::Vst::EditControllerEx1,
                               public VSTGUI::VST3EditorDelegate
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

	VSTGUI::IController* createSubController (VSTGUI::UTF8StringPtr name,
	                                        const VSTGUI::IUIDescription* description,
	                                        VSTGUI::VST3Editor* editor) SMTG_OVERRIDE;

	DEFINE_INTERFACES
	END_DEFINE_INTERFACES (EditController)
	DELEGATE_REFCOUNT (EditController)

private:
	VSTGUI::IKeyboardViewPlayerDelegate* playerDelegate {nullptr};
	VSTGUI::KeyboardViewRangeSelector::Range keyboardRange {};
};

//------------------------------------------------------------------------
} // namespace EventHorizon
