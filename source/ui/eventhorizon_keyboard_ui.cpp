#include "eventhorizon_keyboard_ui.h"

#include "../eventhorizon_messaging.h"

#include "base/source/fobject.h"
#include "pluginterfaces/base/smartpointer.h"
#include "pluginterfaces/vst/ivstevents.h"
#include "pluginterfaces/vst/ivstmessage.h"
#include "vstgui/lib/iviewlistener.h"
#include "vstgui/uidescription/delegationcontroller.h"

#include <cassert>
#include <cmath>
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
                           public IKeyboardViewKeyRangeChangedListener,
                           public KeyboardViewPlayerDelegate
{
public:
	KeyboardController (IController* parent, IKeyboardViewPlayerDelegate* inPlayer,
	                    KeyboardViewRangeSelector::Range& range)
	: DelegationController (parent), player (inPlayer), selectedRange (range)
	{
	}

	~KeyboardController () noexcept override
	{
		if (player)
		{
			for (auto& e : noteOnIds)
				player->onNoteOff (e.second, e.first);
		}
		if (keyboard)
			keyboard->unregisterViewListener (this);
		if (rangeSelector)
		{
			rangeSelector->unregisterViewListener (this);
			rangeSelector->unregisterKeyRangeChangedListener (this);
		}
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
		else if (auto kbsv = dynamic_cast<KeyboardViewRangeSelector*> (view))
		{
			assert (rangeSelector == nullptr);
			rangeSelector = kbsv;
			rangeSelector->registerViewListener (this);
			rangeSelector->registerKeyRangeChangedListener (this);
			if (selectedRange.length > 0)
				rangeSelector->setSelectionRange (selectedRange);
		}
		return controller->verifyView (view, attributes, description);
	}

	void viewAttached (CView* view) override
	{
		if (view == rangeSelector)
			updateKeyboard ();
	}

	void viewWillDelete (CView* view) override
	{
		if (view == rangeSelector)
			rangeSelector = nullptr;
		else if (view == keyboard)
			keyboard = nullptr;
		view->unregisterViewListener (this);
	}

private:
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
		if (rangeSelector)
			rangeSelector->setKeyPressed (note, true);
		return noteID;
	}

	void onNoteOff (NoteIndex note, int32_t noteID) override
	{
		if (player)
		{
			player->onNoteOff (note, noteID);
			noteOnIds.erase (noteID);
		}
		if (rangeSelector)
			rangeSelector->setKeyPressed (note, false);
		if (keyboard)
			keyboard->setKeyPressed (note, false);
	}

	void onNoteModulation (int32_t noteID, double xPos, double yPos) override
	{
		if (player)
			player->onNoteModulation (noteID, xPos, yPos);
	}

	void onKeyRangeChanged (KeyboardViewRangeSelector*) override
	{
		if (!keyboard || !rangeSelector)
			return;
		auto range = rangeSelector->getSelectionRange ();
		while (!keyboard->isWhiteKey (range.position))
			range.position--;
		rangeSelector->setSelectionRange (range);
		updateKeyboard ();
		selectedRange = rangeSelector->getSelectionRange ();
	}

	void updateKeyboard ()
	{
		if (!keyboard || !rangeSelector)
			return;
		auto range = rangeSelector->getSelectionRange ();
		CCoord whiteKeyWidth =
		    std::floor (keyboard->getViewSize ().getWidth () / rangeSelector->getNumWhiteKeysSelected ());
		if (range.position + range.length >
		    rangeSelector->getNumKeys () + rangeSelector->getKeyRangeStart ())
		{
			range.length -= 1;
			rangeSelector->setSelectionRange (range);
		}
		keyboard->setKeyRange (range.position, range.length);
		keyboard->setWhiteKeyWidth (whiteKeyWidth);
		keyboard->setBlackKeyWidth (whiteKeyWidth / 1.5);
		keyboard->setBlackKeyHeight (keyboard->getHeight () / 2.);
	}

	KeyboardView* keyboard {nullptr};
	KeyboardViewRangeSelector* rangeSelector {nullptr};
	IKeyboardViewPlayerDelegate* player {nullptr};
	KeyboardViewRangeSelector::Range& selectedRange;
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
                                      KeyboardViewRangeSelector::Range& range)
{
	return new KeyboardController (parent, inPlayer, range);
}

//------------------------------------------------------------------------
} // namespace EventHorizon
