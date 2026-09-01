#pragma once

#include "vstgui/contrib/keyboardview.h"
#include "vstgui/uidescription/icontroller.h"

#include <functional>

namespace Steinberg {
namespace Vst {
class IConnectionPoint;
class IMessage;
} // namespace Vst
} // namespace Steinberg

namespace EventHorizon {

using NewMessageFunc = std::function<Steinberg::Vst::IMessage* ()>;

class VST3KeyboardPlayerDelegate;

VSTGUI::IKeyboardViewPlayerDelegate* createKeyboardPlayerDelegate (
    Steinberg::Vst::IConnectionPoint* processorPeer, NewMessageFunc&& newMessage);

VSTGUI::IController* createKeyboardController (VSTGUI::IController* parent,
                                              VSTGUI::IKeyboardViewPlayerDelegate* player,
                                              VSTGUI::KeyboardViewRangeSelector::Range& range);

} // namespace EventHorizon
