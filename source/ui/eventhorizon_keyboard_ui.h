#pragma once

#include "vstgui/contrib/keyboardview.h"
#include "vstgui/uidescription/icontroller.h"

#include <cstdint>
#include <functional>

namespace Steinberg {
namespace Vst {
class IConnectionPoint;
class IMessage;
} // namespace Vst
} // namespace Steinberg

namespace EventHorizon {

using NewMessageFunc = std::function<Steinberg::Vst::IMessage* ()>;

static constexpr int16_t kKeyboardDefaultStartNote = 60; // C4
static constexpr uint8_t kKeyboardNumKeys = 24; // two octaves
static constexpr int16_t kKeyboardMinStartNote = 0;
static constexpr int16_t kKeyboardMaxStartNote = 104; // 127 - 23

enum KeyboardControlTags : int32_t
{
	kOctaveUpTag = 1001,
	kOctaveDownTag = 1002
};

class VST3KeyboardPlayerDelegate;

VSTGUI::IKeyboardViewPlayerDelegate* createKeyboardPlayerDelegate (
    Steinberg::Vst::IConnectionPoint* processorPeer, NewMessageFunc&& newMessage);

VSTGUI::IController* createKeyboardController (VSTGUI::IController* parent,
                                              VSTGUI::IKeyboardViewPlayerDelegate* player,
                                              int16_t& startNote);

} // namespace EventHorizon
