#include "eventhorizon_keyboard_ui.h"

#include "../eventhorizon_messaging.h"

#include "base/source/fobject.h"
#include "pluginterfaces/base/smartpointer.h"
#include "pluginterfaces/vst/ivstevents.h"
#include "pluginterfaces/vst/ivstmessage.h"
#include "vstgui/contrib/keyboardview.h"
#include "vstgui/lib/ccolor.h"
#include "vstgui/lib/controls/cbuttons.h"
#include "vstgui/lib/controls/ccontrol.h"
#include "vstgui/lib/iviewlistener.h"
#include "vstgui/uidescription/delegationcontroller.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstring>
#include <map>

using namespace Steinberg;
using namespace VSTGUI;

namespace EventHorizon {

//------------------------------------------------------------------------
class VST3KeyboardPlayerDelegate : public IKeyboardViewPlayerDelegate
{
public:
	VST3KeyboardPlayerDelegate (Vst::IConnectionPoint* processorPeer, NewMessageFunc&& newMessage)
	: newMessage (std::move (newMessage)), processor (processorPeer)
	{
	}

	int32_t onNoteOn (NoteIndex note, double /*xPos*/, double /*yPos*/) override
	{
		if (noteIDCounter < Vst::kNoteIDUserRangeLowerBound)
			noteIDCounter = Vst::kNoteIDUserRangeUpperBound;
		auto newNoteID = --noteIDCounter;
		Vst::Event evt {};
		evt.type = Vst::Event::EventTypes::kNoteOnEvent;
		evt.flags = Vst::Event::EventFlags::kIsLive;
		evt.noteOn.channel = 0;
		evt.noteOn.pitch = note;
		evt.noteOn.tuning = 0.f;
		evt.noteOn.velocity = 1.f;
		evt.noteOn.length = 0;
		evt.noteOn.noteId = newNoteID;
		sendEvent (evt);
		return newNoteID;
	}

	void onNoteOff (NoteIndex note, int32_t noteID) override
	{
		Vst::Event evt {};
		evt.type = Vst::Event::EventTypes::kNoteOffEvent;
		evt.flags = Vst::Event::EventFlags::kIsLive;
		evt.noteOff.channel = 0;
		evt.noteOff.pitch = note;
		evt.noteOff.velocity = 0.f;
		evt.noteOff.noteId = noteID;
		evt.noteOff.tuning = 0.f;
		sendEvent (evt);
	}

	void onNoteModulation (int32_t /*noteID*/, double /*xPos*/, double /*yPos*/) override {}

private:
	void sendEvent (const Vst::Event& evt)
	{
		if (!processor)
			return;
		if (auto message = Steinberg::owned (newMessage ()))
		{
			message->setMessageID (MsgIDEvent);
			if (auto attr = message->getAttributes ())
				attr->setBinary (MsgIDEvent, &evt, sizeof (Vst::Event));
			processor->notify (message);
		}
	}

	int32_t noteIDCounter = Vst::kNoteIDUserRangeUpperBound;
	NewMessageFunc newMessage;
	Vst::IConnectionPoint* processor {nullptr};
};

//------------------------------------------------------------------------
class KeyboardController : public DelegationController,
                           public ViewListenerAdapter,
                           public KeyboardViewPlayerDelegate
{
public:
	KeyboardController (IController* parent, IKeyboardViewPlayerDelegate* inPlayer, int16_t& inStartNote)
	: DelegationController (parent), player (inPlayer), startNote (inStartNote)
	{
	}

	~KeyboardController () noexcept override
	{
		releaseAllNotes ();
		if (keyboard)
			keyboard->unregisterViewListener (this);
	}

	IControlListener* getControlListener (UTF8StringPtr controlTagName) override
	{
		if (controlTagName)
		{
			if (strcmp (controlTagName, "OctaveUp") == 0 ||
			    strcmp (controlTagName, "OctaveDown") == 0)
				return this;
		}
		return controller->getControlListener (controlTagName);
	}

	CView* verifyView (CView* view, const UIAttributes& attributes,
	                   const IUIDescription* description) override
	{
		if (auto kb = dynamic_cast<KeyboardView*> (view))
		{
			assert (keyboard == nullptr);
			keyboard = kb;
			keyboard->registerViewListener (this);
			keyboard->setDelegate (this);
		}
		else if (auto* button = dynamic_cast<CTextButton*> (view))
		{
			const auto tag = button->getTag ();
			if (tag == kOctaveUpTag || tag == kOctaveDownTag)
			{
				button->setListener (this);
				button->setTextColor (CColor (210, 210, 215));
				button->setTextColorHighlighted (CColor (255, 255, 255));
				button->setFrameColor (CColor (90, 90, 100));
				button->setFrameColorHighlighted (CColor (120, 120, 135));
				button->setTextAlignment (kCenterText);
			}
		}
		return controller->verifyView (view, attributes, description);
	}

	void viewAttached (CView* view) override
	{
		if (view == keyboard)
			updateKeyboard ();
	}

	void viewWillDelete (CView* view) override
	{
		if (view == keyboard)
		{
			keyboard->unregisterViewListener (this);
			keyboard = nullptr;
		}
	}

	void valueChanged (CControl* control) override
	{
		if (!control)
			return;

		switch (control->getTag ())
		{
			case kOctaveUpTag:
				if (control->getValue () >= 0.5f)
					changeOctave (12);
				return;
			case kOctaveDownTag:
				if (control->getValue () >= 0.5f)
					changeOctave (-12);
				return;
			default:
				if (controller)
					controller->valueChanged (control);
				break;
		}
	}

	void controlBeginEdit (CControl* control) override
	{
		if (isOctaveControl (control))
			return;
		if (controller)
			controller->controlBeginEdit (control);
	}

	void controlEndEdit (CControl* control) override
	{
		if (isOctaveControl (control))
			return;
		if (controller)
			controller->controlEndEdit (control);
	}

private:
	static bool isOctaveControl (const CControl* control)
	{
		if (!control)
			return false;
		const auto tag = control->getTag ();
		return tag == kOctaveUpTag || tag == kOctaveDownTag;
	}

	int32_t onNoteOn (NoteIndex note, double xPos, double yPos) override
	{
		int32_t noteID = note;
		if (player)
		{
			noteID = player->onNoteOn (note, xPos, yPos);
			noteOnIds[noteID] = note;
		}
		if (keyboard)
			keyboard->setKeyPressed (note, true);
		return noteID;
	}

	void onNoteOff (NoteIndex note, int32_t noteID) override
	{
		if (player)
		{
			player->onNoteOff (note, noteID);
			noteOnIds.erase (noteID);
		}
		if (keyboard)
			keyboard->setKeyPressed (note, false);
	}

	void onNoteModulation (int32_t noteID, double xPos, double yPos) override
	{
		if (player)
			player->onNoteModulation (noteID, xPos, yPos);
	}

	void releaseAllNotes ()
	{
		const auto activeNotes = noteOnIds;
		for (const auto& e : activeNotes)
		{
			if (keyboard)
				keyboard->setKeyPressed (e.second, false);
			if (player)
				player->onNoteOff (e.second, e.first);
		}
		noteOnIds.clear ();
	}

	void changeOctave (int16_t semitones)
	{
		const int16_t nextStart =
		    static_cast<int16_t> (std::clamp (static_cast<int> (startNote) + semitones,
		                                      static_cast<int> (kKeyboardMinStartNote),
		                                      static_cast<int> (kKeyboardMaxStartNote)));
		if (nextStart == startNote)
			return;
		releaseAllNotes ();
		startNote = nextStart;
		updateKeyboard ();
	}

	void updateKeyboard ()
	{
		if (!keyboard)
			return;

		keyboard->setKeyRange (startNote, kKeyboardNumKeys);

		const auto numWhiteKeys = keyboard->getNumWhiteKeys ();
		if (numWhiteKeys > 0)
		{
			const CCoord whiteKeyWidth =
			    std::floor (keyboard->getViewSize ().getWidth () / static_cast<CCoord> (numWhiteKeys));
			keyboard->setWhiteKeyWidth (whiteKeyWidth);
			keyboard->setBlackKeyWidth (whiteKeyWidth / 1.5);
		}
		keyboard->setBlackKeyHeight (keyboard->getHeight () * 0.6);
		keyboard->invalid ();
	}

	KeyboardView* keyboard {nullptr};
	IKeyboardViewPlayerDelegate* player {nullptr};
	int16_t& startNote;
	std::map<int32_t, NoteIndex> noteOnIds;
};

//------------------------------------------------------------------------
VSTGUI::IKeyboardViewPlayerDelegate* createKeyboardPlayerDelegate (
    Vst::IConnectionPoint* processorPeer, NewMessageFunc&& newMessage)
{
	return new VST3KeyboardPlayerDelegate (processorPeer, std::move (newMessage));
}

//------------------------------------------------------------------------
IController* createKeyboardController (IController* parent, IKeyboardViewPlayerDelegate* inPlayer,
                                      int16_t& startNote)
{
	return new KeyboardController (parent, inPlayer, startNote);
}

//------------------------------------------------------------------------
} // namespace EventHorizon
