# KeyboardWarriorLoL

KeyboardWarriorLoL is a League of Legends mod that allows players to send predefined chat messages using configurable hotkeys. 
Messages can be triggered at any time or unlocked dynamically based on detected in-game events such as the takedown of kills, objectives, and structures.

The mod supports two types of messages:

* **Default Messages** — Always available and can be sent at any time.
* **Event Messages** — Unlocked by specific in-game events and can only be sent after the corresponding event has been detected.

## Features

* Configure hotkeys for commonly used chat messages.
* Send messages quick without typing.
* Event-aware messaging system.
* Custom titles for every message slot.
* Support for multiple event-triggered message slots.
* Lightweight and easy to configure.

## Message Types

### Default Messages

Default messages are always available.

Example:

* "gj team"
* "group for dragon"
* "wait for me dont fight"

Pressing the assigned hotkey immediately sends the configured message.

### Event Messages

Event messages are tied to game events and are identified by an index.

An event message becomes available only after its associated event has occurred. 

Once available, pressing the assigned hotkey sends the configured message.

Supported events include:

- Game State
  - Game Start
    
- Kills
  - First Blood 
  - Assisted Kill
  - Solo Kill
  - Double Kill
  - Triple Kill
  - Quadra Kill
  - Pentakill
  - Ace
  - Death
    
- Objectives
  - Dragon
  - Baron
  - Rift Herald
  - Void Grubs
  - Atakhan
    
- Structures
  - First Turret
  - Turret
  - Inhibitor

#### Some Information About How Events Work
- Events are stored in a queue, where the current event can be skipped in favor of the next one

- **Kill events** (excluding First Blood, Ace and Death) will automatically be placed first in the queue (erasing all previous events).
  - Kill streak events will automatically update to the most recent streak. (Ex: getting a triple kill will automatically set the current event to `Triple Kill`)
  - Getting a kill with an event already active will set the kill as the active event instead of placing it after

- The **Quadra Kill** event expires after 30 seconds (the maximum possible amount of time before the timer for a pentakill expires)


### In-Game Workflow
The In-Game Overlay UI can be toggled between showing only on events or always showing.

When an event is detected, the overlay UI will show the current event, the hotkeys with the corresponding messages (by title), and a pending next event (if there is one).

Pressing a hotkey will type the message in the chat. 

\***The message is only properly typed if the chat is initially closed**\*

## Message Titles

Each message slot includes a customizable title.

Titles are used only within KeyboardWarriorLoL to help identify messages when configuring or viewing hotkey assignments. They do not affect the text sent to in-game chat.


This allows players to quickly distinguish between message slots without needing to read the full chat message when properly configured.

## Configuration

Each message slot can be configured with:

* Title
* Message Content

Example:
```text
Default Message 1
Title = "GLHF"
Content = "Good luck have fun"
```

Hotkey slots can be configured by clicking the hotkey button and pressing a valid hotkey.

Example:

```text
Default Hotkey 1
[Hotkey]   [Delete Hotkey]
```

## Usage

1. Launch League of Legends alongside KeyboardWarriorLoL.
2. Configure your message titles, messages, hotkeys, and mod configurations (if needed).
3. Test it in practice tool (get used to it).
4. Have fun with it.


Please use this mod responsibly.
- Don't use this to spam the chat.
- Treat people with respect.
- Follow Riot Games' rules and community guidelines.


## Disclaimer
*KeyboardWarriorLoL is an unofficial third-party tool and is not affiliated with or endorsed by Riot Games. Use of third-party software may violate Riot Games policies or Terms of Service. Users assume all risk associated with using this software, including potential account penalties.*


