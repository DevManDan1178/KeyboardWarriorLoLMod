#pragma once

#include "HotkeyManager.h"
#include "Messages.h"

namespace MessagesUI {
    extern int categoryToggleStates;
    void messagesMenu(Messages &messages, HotkeyManager &hotkeyManager);
}