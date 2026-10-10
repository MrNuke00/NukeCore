#pragma once
#include <Platform/PlatformCore.h>

//cross platform 

enum class eKeyCode : uint32 {
	None = 0,

	// Keyboard - Letters
	A,
	B,
	C,
	D,
	E,
	F,
	G,
	H,
	I,
	J,
	K,
	L,
	M,
	N,
	O,
	P,
	Q,
	R,
	S,
	T,
	U,
	V,
	W,
	X,
	Y,
	Z,
	
	// Keyboard - Numbers (Top Row)	Num0,
	Num1,
	Num2,
	Num3,
	Num4,
	Num5,
	Num6,
	Num7,
	Num8,
	Num9,
	
	// Keyboard - Function Keys	F1..F24
	F1,
	F2,
	F3,
	F4,
	F5,
	F6,
	F7,
	F8,
	F9,
	F10,
	F11,
	F12,
	F13,
	F14,
	F15,
	F16,
	F17,
	F18,
	F19,
	F20,
	F21,
	F22,
	F23,
	F24,
	
	// Keyboard - Control & Navigation
	Escape,
	Enter,
	Space,
	Backspace,
	Tab,
	CapsLock,
	PrintScreen,
	ScrollLock,
	Pause,
	Insert,
	Delete,
	Home,
	End,
	PageUp,
	PageDown,
	
	
	
	// Keyboard - Arrows
	Up		,
	Down	,
	Left	,
	Right	,
	
	
	// Keyboard - Modifiers
	LeftShift		,
	RightShift		,
	LeftControl		,
	RightControl	,
	LeftAlt			,
	RightAlt		,
	LeftSuper		,
	RightSuper		,
	
	// Windows / Command key
	Menu,	// Context Menu key
	
	// Keyboard - Symbols & Punctuation
	Grave,			// ` ~
	Minus,			// - _
	Equals,			// = +
	LeftBracket,	// [ {
	RightBracket,	// ] }
	Backslash,		// \ |
	Semicolon,		// ; :
	Apostrophe,		// ' "
	Comma,			// , <
	Period,			// . >
	Slash,			// / ?
	
	
	// Keyboard - Numpad
	Numpad0,
	Numpad1,
	Numpad2,
	Numpad3,
	Numpad4,
	Numpad5,
	Numpad6,
	Numpad7,
	Numpad8,
	Numpad9,
	NumpadAdd,
	NumpadSubtract,
	NumpadMultiply,
	NumpadDivide,
	NumpadEnter,
	NumpadDecimal,
	NumLock,
	
	// Mouse
	MouseLeft,
	MouseRight,
	MouseMiddle,
	MouseX1,	// Mouse Button 4 (Browser Back)
	MouseX2,	// Mouse Button 5 (Browser Forward)
	
	// Gamepad (Standard Xbox + PlayStation layout)
    GamepadA,           // Xbox A      / PS Cross
    GamepadB,           // Xbox B      / PS Circle
    GamepadX,           // Xbox X      / PS Square
    GamepadY,           // Xbox Y      / PS Triangle
    GamepadLeftBumper,  // LB          / L1
    GamepadRightBumper, // RB          / R1
    GamepadLeftTrigger, // LT          / L2
    GamepadRightTrigger,// RT          / R2
    GamepadBack,        // View/Select / Share
    GamepadStart,       // Menu/Start  / Options
    GamepadGuide,       // Xbox Button / PS Button
    GamepadLeftThumb,   // L3 (Left Stick Click)
    GamepadRightThumb,  // R3 (Right Stick Click)
    GamepadDPadUp,      // D-Pad Up
    GamepadDPadDown,    // D-Pad Down
    GamepadDPadLeft,    // D-Pad Left
    GamepadDPadRight,   // D-Pad Right
	
	// Metadata
	Count,
	Unknown = UINT32_MAX
};




