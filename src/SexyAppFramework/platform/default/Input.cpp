/*
 * Portions of this file are based on the PopCap Games Framework
 * Copyright (C) 2005-2009 PopCap Games, Inc.
 *
 * Copyright (C) 2026 Zhou Qiankang <wszqkzqk@qq.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later AND LicenseRef-PopCap
 *
 * This file is part of PvZ-Portable.
 *
 * PvZ-Portable is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * PvZ-Portable is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with PvZ-Portable. If not, see <https://www.gnu.org/licenses/>.
 */

#include <SDL.h>
#include <cstdlib>

#include <algorithm>
#include <cmath>

#include "SexyAppBase.h"
#include "graphics/GLInterface.h"
#include "graphics/GLImage.h"
#include "widget/WidgetManager.h"
#include "misc/KeyCodes.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

EM_JS(void, WasmStartSoftKeyboard, (), {
	var input = document.getElementById('pvz-soft-keyboard');
	if (!input) return;

	if (!Module.wasmSoftKeyboardState) {
		var state = {
			active: false,
			pendingChars: [],
			pendingKeys: [],
			lastValue: ""
		};

		function syncInputValue() {
			if (!state.active) return;

			var nextValue = input.value || "";
			var lastChars = Array.from(state.lastValue); // diff by code point: one backspace deletes one code point
			var nextChars = Array.from(nextValue);
			var prefixLen = 0;
			while (prefixLen < lastChars.length && prefixLen < nextChars.length
				&& lastChars[prefixLen] === nextChars[prefixLen]) {
				prefixLen++;
			}

			for (var i = lastChars.length; i > prefixLen; --i) {
				state.pendingKeys.push(8);
			}

			for (const ch of nextChars.slice(prefixLen)) {
				var charCode = ch.codePointAt(0);
				if (charCode > 0 && charCode <= 0x7f) {
					state.pendingChars.push(charCode);
				} else if (charCode > 0x7f) {
					var bytes = new TextEncoder().encode(ch);
					for (var k = 0; k < bytes.length; ++k) {
						state.pendingChars.push(bytes[k]);
					}
				}
			}

			state.lastValue = nextValue;
		}

		input.addEventListener('input', syncInputValue);
		input.addEventListener('keydown', function(event) {
			if (!state.active) return;

			switch (event.key) {
				case 'Enter':
					state.pendingKeys.push(13);
					event.preventDefault();
					break;
				case 'Escape':
					state.pendingKeys.push(27);
					event.preventDefault();
					break;
				case 'Tab':
					state.pendingKeys.push(9);
					event.preventDefault();
					break;
				case 'Delete':
					state.pendingKeys.push(46);
					event.preventDefault();
					break;
				case 'ArrowLeft':
					state.pendingKeys.push(37);
					event.preventDefault();
					break;
				case 'ArrowRight':
					state.pendingKeys.push(39);
					event.preventDefault();
					break;
				case 'Home':
					state.pendingKeys.push(36);
					event.preventDefault();
					break;
				case 'End':
					state.pendingKeys.push(35);
					event.preventDefault();
					break;
			}
		});

		Module.wasmSoftKeyboardState = state;
	}

	var state = Module.wasmSoftKeyboardState;
	state.active = true;
	state.pendingChars.length = 0;
	state.pendingKeys.length = 0;
	state.lastValue = "";
	input.value = "";
	if (typeof input.focus === 'function') {
		input.focus();
	}
	if (typeof input.setSelectionRange === 'function') {
		input.setSelectionRange(0, 0);
	}
});

EM_JS(void, WasmStopSoftKeyboard, (), {
	var input = document.getElementById('pvz-soft-keyboard');
	var state = Module.wasmSoftKeyboardState;
	if (state) {
		state.active = false;
		state.pendingChars.length = 0;
		state.pendingKeys.length = 0;
		state.lastValue = "";
	}
	if (input) {
		input.value = "";
		if (typeof input.blur === 'function') {
			input.blur();
		}
	}
	if (Module.canvas && typeof Module.canvas.focus === 'function') {
		Module.canvas.focus();
	}
});

EM_JS(int, WasmPopSoftKeyboardChar, (), {
	var state = Module.wasmSoftKeyboardState;
	if (!state || state.pendingChars.length === 0) return 0;
	return state.pendingChars.shift();
});

EM_JS(int, WasmPopSoftKeyboardKey, (), {
	var state = Module.wasmSoftKeyboardState;
	if (!state || state.pendingKeys.length === 0) return 0;
	return state.pendingKeys.shift();
});

EM_JS(int, WasmHasSoftKeyboardEvents, (), {
	var state = Module.wasmSoftKeyboardState;
	if (!state) return 0;
	return (state.pendingChars.length + state.pendingKeys.length) > 0 ? 1 : 0;
});
#endif

using namespace Sexy;

// Map SDL_Keycode to internal KeyCode (Windows VK-compatible).
static KeyCode SDLKeyToKeyCode(SDL_Keycode theSDLKey)
{
	if (theSDLKey >= SDLK_a && theSDLKey <= SDLK_z)
		return static_cast<KeyCode>(theSDLKey - SDLK_a + 'A');

	if (theSDLKey >= SDLK_0 && theSDLKey <= SDLK_9)
		return static_cast<KeyCode>(theSDLKey);

	switch (theSDLKey)
	{
		case SDLK_BACKSPACE:    return KEYCODE_BACK;
		case SDLK_TAB:          return KEYCODE_TAB;
		case SDLK_CLEAR:        return KEYCODE_CLEAR;
		case SDLK_RETURN:       return KEYCODE_RETURN;
		case SDLK_AC_BACK:
		case SDLK_ESCAPE:       return KEYCODE_ESCAPE;
		case SDLK_SPACE:        return KEYCODE_SPACE;
		case SDLK_DELETE:       return KEYCODE_DELETE;

		case SDLK_LEFT:         return KEYCODE_LEFT;
		case SDLK_UP:           return KEYCODE_UP;
		case SDLK_RIGHT:        return KEYCODE_RIGHT;
		case SDLK_DOWN:         return KEYCODE_DOWN;

		case SDLK_INSERT:       return KEYCODE_INSERT;
		case SDLK_HOME:         return KEYCODE_HOME;
		case SDLK_END:          return KEYCODE_END;
		case SDLK_PAGEUP:       return KEYCODE_PRIOR;
		case SDLK_PAGEDOWN:     return KEYCODE_NEXT;

		case SDLK_LSHIFT:
		case SDLK_RSHIFT:       return KEYCODE_SHIFT;
		case SDLK_LCTRL:
		case SDLK_RCTRL:        return KEYCODE_CONTROL;
		case SDLK_LALT:
		case SDLK_RALT:         return KEYCODE_MENU;
		case SDLK_PAUSE:        return KEYCODE_PAUSE;
		case SDLK_CAPSLOCK:     return KEYCODE_CAPITAL;
		case SDLK_NUMLOCKCLEAR: return KEYCODE_NUMLOCK;
		case SDLK_SCROLLLOCK:   return KEYCODE_SCROLL;

		case SDLK_KP_0:         return KEYCODE_NUMPAD0;
		case SDLK_KP_1:         return KEYCODE_NUMPAD1;
		case SDLK_KP_2:         return KEYCODE_NUMPAD2;
		case SDLK_KP_3:         return KEYCODE_NUMPAD3;
		case SDLK_KP_4:         return KEYCODE_NUMPAD4;
		case SDLK_KP_5:         return KEYCODE_NUMPAD5;
		case SDLK_KP_6:         return KEYCODE_NUMPAD6;
		case SDLK_KP_7:         return KEYCODE_NUMPAD7;
		case SDLK_KP_8:         return KEYCODE_NUMPAD8;
		case SDLK_KP_9:         return KEYCODE_NUMPAD9;
		case SDLK_KP_MULTIPLY:  return KEYCODE_MULTIPLY;
		case SDLK_KP_PLUS:      return KEYCODE_ADD;
		case SDLK_KP_MINUS:     return KEYCODE_SUBTRACT;
		case SDLK_KP_PERIOD:    return KEYCODE_DECIMAL;
		case SDLK_KP_DIVIDE:    return KEYCODE_DIVIDE;
		case SDLK_KP_ENTER:     return KEYCODE_RETURN;

		case SDLK_F1:           return KEYCODE_F1;
		case SDLK_F2:           return KEYCODE_F2;
		case SDLK_F3:           return KEYCODE_F3;
		case SDLK_F4:           return KEYCODE_F4;
		case SDLK_F5:           return KEYCODE_F5;
		case SDLK_F6:           return KEYCODE_F6;
		case SDLK_F7:           return KEYCODE_F7;
		case SDLK_F8:           return KEYCODE_F8;
		case SDLK_F9:           return KEYCODE_F9;
		case SDLK_F10:          return KEYCODE_F10;
		case SDLK_F11:          return KEYCODE_F11;
		case SDLK_F12:          return KEYCODE_F12;

		default:                return KEYCODE_UNKNOWN;
	}
}

// Synthesize a minimal ASCII char stream from keydown so legacy KeyChar hotkeys still work.
static bool SDLSynthesizeAsciiCharFromKeyDown(const SDL_KeyboardEvent& theEvent, char& theChar)
{
	theChar = 0;

	SDL_Keycode aSym = theEvent.keysym.sym;
	SDL_Keymod aMods = static_cast<SDL_Keymod>(theEvent.keysym.mod);
	const bool aHasCtrl = (aMods & KMOD_CTRL) != 0;
	const bool aHasAlt = (aMods & KMOD_ALT) != 0;
	const bool aHasGui = (aMods & KMOD_GUI) != 0;
	const bool aHasShift = (aMods & KMOD_SHIFT) != 0;
	const bool aTextInputActive = SDL_IsTextInputActive(); // printable chars arrive via SDL_TEXTINPUT then

	if (aHasAlt || aHasGui)
		return false;

	if (aSym >= SDLK_a && aSym <= SDLK_z)
	{
		if (aHasCtrl)
		{
			theChar = static_cast<char>(aSym - SDLK_a + 1); // Ctrl+letter -> control code; SDL_TEXTINPUT is not guaranteed for Ctrl combos
			return true;
		}

		if (aTextInputActive)
			return false;

		theChar = static_cast<char>(aHasShift ? aSym - SDLK_a + 'A' : aSym);
		return true;
	}

	if (aHasCtrl || aTextInputActive)
		return false;

	switch (aSym)
	{
		case SDLK_KP_1: theChar = '1'; return true;
		case SDLK_KP_2: theChar = '2'; return true;
		case SDLK_KP_3: theChar = '3'; return true;
		case SDLK_KP_4: theChar = '4'; return true;
		case SDLK_KP_5: theChar = '5'; return true;
		case SDLK_KP_6: theChar = '6'; return true;
		case SDLK_KP_7: theChar = '7'; return true;
		case SDLK_KP_8: theChar = '8'; return true;
		case SDLK_KP_9: theChar = '9'; return true;
		case SDLK_KP_0: theChar = '0'; return true;
		case SDLK_KP_PLUS: theChar = '+'; return true;
		case SDLK_KP_MINUS: theChar = '-'; return true;
		case SDLK_KP_MULTIPLY: theChar = '*'; return true;
		case SDLK_KP_DIVIDE: theChar = '/'; return true;
		case SDLK_KP_PERIOD: theChar = '.'; return true;
		case SDLK_KP_EQUALS: theChar = '='; return true;
		case SDLK_1: theChar = aHasShift ? '!' : '1'; return true;
		case SDLK_2: theChar = aHasShift ? '@' : '2'; return true;
		case SDLK_3: theChar = aHasShift ? '#' : '3'; return true;
		case SDLK_4: theChar = aHasShift ? '$' : '4'; return true;
		case SDLK_5: theChar = aHasShift ? '%' : '5'; return true;
		case SDLK_6: theChar = aHasShift ? '^' : '6'; return true;
		case SDLK_7: theChar = aHasShift ? '&' : '7'; return true;
		case SDLK_8: theChar = aHasShift ? '*' : '8'; return true;
		case SDLK_9: theChar = aHasShift ? '(' : '9'; return true;
		case SDLK_0: theChar = aHasShift ? ')' : '0'; return true;
		case SDLK_MINUS: theChar = aHasShift ? '_' : '-'; return true;
		case SDLK_EQUALS: theChar = aHasShift ? '+' : '='; return true;
		case SDLK_LEFTBRACKET: theChar = aHasShift ? '{' : '['; return true;
		case SDLK_RIGHTBRACKET: theChar = aHasShift ? '}' : ']'; return true;
		case SDLK_BACKSLASH: theChar = aHasShift ? '|' : '\\'; return true;
		case SDLK_SEMICOLON: theChar = aHasShift ? ':' : ';'; return true;
		case SDLK_QUOTE: theChar = aHasShift ? '"' : '\''; return true;
		case SDLK_COMMA: theChar = aHasShift ? '<' : ','; return true;
		case SDLK_PERIOD: theChar = aHasShift ? '>' : '.'; return true;
		case SDLK_SLASH: theChar = aHasShift ? '?' : '/'; return true;
		case SDLK_BACKQUOTE: theChar = aHasShift ? '~' : '`'; return true;
		case SDLK_SPACE: theChar = ' '; return true;
		default: return false;
	}
}

// ---------------------------------------------------------------------------
// Game controller support.
//
// PvZ is a mouse-driven game; there is no native gamepad path. This layer
// drives a virtual cursor from the left stick / D-pad and synthesizes the
// same MouseMove/MouseDown/MouseUp calls the mouse path uses, so no game
// logic needs to know a controller exists. Clean-room; only public SDL2
// GameController APIs are used.
// ---------------------------------------------------------------------------
namespace {

SDL_GameController*	gController = nullptr;
bool				gCursorValid = false;
float				gCursorX = 0.0f;
float				gCursorY = 0.0f;
int					gDrawX = 0;			// last cursor pos, in game-logical space (for drawing the sprite)
int					gDrawY = 0;
float				gStickX = 0.0f;		// normalized [-1,1], deadzoned
float				gStickY = 0.0f;
Uint32				gLastAdvanceTick = 0;
bool				gAHeld = false;		// A held -> auto-repeat clicks (sweep to collect sun)
Uint32				gARepeatTick = 0;	// next time an auto-repeat click fires
bool				gCursorBoostEnabled = true;		// L3 can be switched off, for thumbs that rest on the stick
bool				gCursorBoost = false;	// L3 held -> the cursor sprints
bool				gGameSpeedUp = false;	// R2 held -> 2x game speed
bool				gGameSlowDown = false;	// L2 held -> the game eases off
bool				gOnBoard = false;	// cursor is over a lawn cell -> draw selector box, not arrow
int					gBoxX = 0, gBoxY = 0, gBoxW = 0, gBoxH = 0;	// selector box rect (game space)
int					gLastDpadX = 0, gLastDpadY = 0;	// previous D-pad direction (edge detection)
Uint32				gDpadRepeatTick = 0;			// next time a held D-pad step fires

const Uint32		kDpadDelay = 260;	// ms before a held D-pad direction starts repeating
const Uint32		kDpadInterval = 110;	// ms between repeated D-pad cell steps
const float			kSnapSpeed = 18.0f;	// how fast the cursor settles onto a cell when input stops
const float			kJellyPull = 9.0f;	// free-mode magnetism toward a cell centre (jelly/detent feel)

// Runtime controller settings. Canonical values live here; the game persists
// them to the registry (via the SexyAppBase getters/setters below) and exposes
// them in the controller options dialog. Env vars still override at startup for
// on-device tuning without touching a save.
const float			kCursorSpeed = 700.0f;			// px/s at full deflection, on the game's fixed 800x600 canvas.
											// The port this borrows from reads as 400, but it steps a hardcoded
											// 1/60 dt on a 100Hz update, so it really moves about 667.
float				gSensitivity = 1.0f;			// [0.5, 2.0]
float				gSunRadius = 220.0f;			// auto-collect radius px, [60, 640]
bool				gFreeCursor = false;			// false = confine the cursor to the lawn during normal play; true = let it roam the screen
bool				gSwapAB = false;				// swap A and B, for pads whose face buttons are labelled the other way round
bool				gSwapXY = false;				// swap X and Y, for pads whose face buttons are labelled the other way round
const int			kStickDeadzone = 6553;	// ~0.2 * 32767
const int			kTriggerThreshold = 16384;	// half pull counts as pressed
const float			kBoostFactor = 2.5f;	// cursor speed multiplier while the sprint is on

const Uint32		kARepeatDelay = 300;	// ms before A begins repeating
const Uint32		kARepeatInterval = 60;	// ms between auto-repeat clicks

float Clampf(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }

void ControllerLoadConfig()
{
	const char* aEnv;
	if ((aEnv = getenv("PVZ_CURSOR_SENSITIVITY")) != nullptr && aEnv[0] != '\0')
		gSensitivity = Clampf((float)atof(aEnv), 0.5f, 2.0f);
	if ((aEnv = getenv("PVZ_SUN_RADIUS")) != nullptr && aEnv[0] != '\0')
		gSunRadius = Clampf((float)atof(aEnv), 60.0f, 640.0f);
}

void ControllerOpenFirst()
{
	if (gController != nullptr)
		return;
	for (int i = 0; i < SDL_NumJoysticks(); ++i)
	{
		if (SDL_IsGameController(i))
		{
			gController = SDL_GameControllerOpen(i);
			if (gController != nullptr)
				break;
		}
	}
}

// Cursor pixels per second, including the R2/L3 sprint modifier.
float CursorSpeed()
{
	return kCursorSpeed * gSensitivity * (gCursorBoost ? kBoostFactor : 1.0f);
}

float NormalizeAxis(Sint16 theValue)
{
	if (theValue > -kStickDeadzone && theValue < kStickDeadzone)
		return 0.0f;
	float aNorm = theValue / 32767.0f;
	if (aNorm > 1.0f) aNorm = 1.0f;
	if (aNorm < -1.0f) aNorm = -1.0f;
	return aNorm;
}

} // namespace

void SexyAppBase::InitInput()
{
	SDL_Init(SDL_INIT_EVENTS);

	if (SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) == 0)
	{
		ControllerLoadConfig();
		ControllerOpenFirst();
	}
}

// Advance the virtual cursor once per frame from current stick/D-pad state and
// push a MouseMove. Returns true if a controller is driving the cursor.
bool SexyAppBase::UpdateControllerCursor()
{
	if (gController == nullptr)
		return false;

	if (!ControllerInGame())
	{
		gGameSpeedUp = false;
		gGameSlowDown = false;
	}



	Uint32 aNow = SDL_GetTicks();
	if (gLastAdvanceTick == 0)
		gLastAdvanceTick = aNow;
	float aDt = (aNow - gLastAdvanceTick) / 1000.0f;
	gLastAdvanceTick = aNow;
	if (aDt < 0.0f || aDt > 0.25f)
		aDt = 0.0f;		// first frame or a stall: no motion, but still service A-repeat

	if (!gCursorValid)
	{
		gCursorX = mWidth * 0.5f;
		gCursorY = mHeight * 0.5f;
		gCursorValid = true;
	}

	int aDpadX = (SDL_GameControllerGetButton(gController, SDL_CONTROLLER_BUTTON_DPAD_RIGHT) ? 1 : 0)
			   - (SDL_GameControllerGetButton(gController, SDL_CONTROLLER_BUTTON_DPAD_LEFT) ? 1 : 0);
	int aDpadY = (SDL_GameControllerGetButton(gController, SDL_CONTROLLER_BUTTON_DPAD_DOWN) ? 1 : 0)
			   - (SDL_GameControllerGetButton(gController, SDL_CONTROLLER_BUTTON_DPAD_UP) ? 1 : 0);
	bool aStickActive = (gStickX != 0.0f || gStickY != 0.0f);

	// Probe the cell under the cursor (the game reports cell centre + size).
	int aCX, aCY, aCW, aCH;
	bool aCellHere = ControllerBoardCell((int)gCursorX, (int)gCursorY, aCX, aCY, aCW, aCH);

	if (aCellHere)
	{
		if (aDpadX != 0 || aDpadY != 0)
		{
			// Snappy D-pad: one discrete cell step per press (auto-repeat when held).
			bool aEdge = (aDpadX != gLastDpadX || aDpadY != gLastDpadY);
			if (aEdge || (Sint32)(aNow - gDpadRepeatTick) >= 0)
			{
				gCursorX += aDpadX * aCW;
				gCursorY += aDpadY * aCH;
				Uint32 aRepeat = aEdge ? kDpadDelay : kDpadInterval;
				if (gCursorBoost)
					aRepeat = (Uint32)(aRepeat / kBoostFactor);
				gDpadRepeatTick = aNow + aRepeat;
			}
		}
		else
		{
			// The stick slides with magnetism toward whichever cell it is
			// over, and the cursor settles onto the nearest cell centre once
			// input stops.
			float aMvX = gStickX;
			float aMvY = gStickY;
			if (aMvX > 1.0f) aMvX = 1.0f; if (aMvX < -1.0f) aMvX = -1.0f;
			if (aMvY > 1.0f) aMvY = 1.0f; if (aMvY < -1.0f) aMvY = -1.0f;
			if ((aMvX != 0.0f || aMvY != 0.0f) && aDt > 0.0f)
			{
				gCursorX += aMvX * CursorSpeed() * aDt;
				gCursorY += aMvY * CursorSpeed() * aDt;

				// Jelly magnetism: while sliding, pull toward the cell centre the
				// cursor is now over -- strong near the centre, ~0 at the edge, so
				// it feels free but "sucks in" to cells (iOS/PSV detent feel).
				// It fades out as the stick goes over, so lining a shot up still
				// gets the detent while crossing the lawn is not dragged at.
				int jcx, jcy, jcw, jch;
				float aPush = sqrtf(aMvX * aMvX + aMvY * aMvY);
				if (aPush > 1.0f) aPush = 1.0f;
				float aPullScale = 1.0f - aPush;
				if (aPullScale > 0.0f &&
					ControllerBoardCell((int)gCursorX, (int)gCursorY, jcx, jcy, jcw, jch))
				{
					float dx = jcx - gCursorX, dy = jcy - gCursorY;
					float nx = (jcw > 0) ? dx / (jcw * 0.5f) : 0.0f;
					float ny = (jch > 0) ? dy / (jch * 0.5f) : 0.0f;
					float s = 1.0f - (nx * nx + ny * ny);	// 1 at centre, 0 at edge
					if (s > 0.0f)
					{
						float f = kJellyPull * s * aPullScale * aDt;
						if (f > 1.0f) f = 1.0f;
						gCursorX += dx * f;
						gCursorY += dy * f;
					}
				}
			}
			else if (aDt > 0.0f)
			{
				// Input stopped: settle onto the nearest cell centre. Both modes
				// do this, so a plant always lands on the cell you stopped over.
				float aLerp = kSnapSpeed * aDt;
				if (aLerp > 1.0f) aLerp = 1.0f;
				gCursorX += (aCX - gCursorX) * aLerp;
				gCursorY += (aCY - gCursorY) * aLerp;
			}
		}
	}
	else if (aDt > 0.0f)
	{
		// Off the lawn (menus/dialogs): free pointer, stick + D-pad both move it.
		float aFx = gStickX + aDpadX;
		float aFy = gStickY + aDpadY;
		if (aFx > 1.0f) aFx = 1.0f; if (aFx < -1.0f) aFx = -1.0f;
		if (aFy > 1.0f) aFy = 1.0f; if (aFy < -1.0f) aFy = -1.0f;
		gCursorX += aFx * CursorSpeed() * aDt;
		gCursorY += aFy * CursorSpeed() * aDt;
	}

	if (gCursorX < 0.0f) gCursorX = 0.0f;
	if (gCursorY < 0.0f) gCursorY = 0.0f;
	if (gCursorX > mWidth)  gCursorX = mWidth;
	if (gCursorY > mHeight) gCursorY = mHeight;

	// Snap mode keeps the cursor on the lawn: seeds are chosen with the
	// shoulder buttons and the shovel and menu have their own buttons, so
	// there is nothing to reach outside it, and it cannot wander off.
	int aLawnL, aLawnT, aLawnR, aLawnB;
	if (!gFreeCursor && ControllerLawnBounds((int)gCursorX, aLawnL, aLawnT, aLawnR, aLawnB))
	{
		if (gCursorX < aLawnL) gCursorX = (float)aLawnL;
		if (gCursorX > aLawnR) gCursorX = (float)aLawnR;
		if (gCursorY < aLawnT) gCursorY = (float)aLawnT;
		if (gCursorY > aLawnB) gCursorY = (float)aLawnB;
	}
	gLastDpadX = aDpadX;
	gLastDpadY = aDpadY;

	// Re-probe after moving. The box is centred on the cursor itself, not on the
	// cell centre -- so in free mode it slides with the cursor (and only lands on
	// a cell once the cursor settles there), while in snappy mode the cursor jumps
	// cell-to-cell so the box does too.
	int aBoxCX, aBoxCY, aBoxCW, aBoxCH;
	gOnBoard = ControllerBoardCell((int)gCursorX, (int)gCursorY, aBoxCX, aBoxCY, aBoxCW, aBoxCH);
	if (gOnBoard)
	{
		gBoxW = aBoxCW;
		gBoxH = aBoxCH;
		gBoxX = (int)gCursorX - gBoxW / 2;
		gBoxY = (int)gCursorY - gBoxH / 2;
	}

	int x = (int)gCursorX;
	int y = (int)gCursorY;
	gDrawX = x;
	gDrawY = y;

	if (gOnBoard)
		ControllerAutoCollect(x, y);

	mMouseIn = true;
	mLastUserInputTick = mLastTimerTime;
	mWidgetManager->MouseMove(x, y);

	// Holding A auto-repeats a click ONLY over the lawn, so sweeping collects the
	// sun it passes. In menus/dialogs a hold must not re-fire, or it double-toggles
	// checkboxes / re-clicks buttons (the reference build fixed the same bug).
	if (gAHeld && gOnBoard && (Sint32)(aNow - gARepeatTick) >= 0)
	{
		mWidgetManager->MouseDown(x, y, 1);
		mWidgetManager->MouseUp(x, y, 1);
		gARepeatTick = aNow + kARepeatInterval;
	}
	return true;
}

// True if a controller is driving the cursor; returns its draw position (in
// screen-image space) so the caller can blit a cursor sprite there.
bool SexyAppBase::GetControllerCursor(int& theX, int& theY)
{
	if (gController == nullptr || !gCursorValid)
		return false;
	theX = gDrawX;
	theY = gDrawY;
	return true;
}

bool SexyAppBase::IsControllerActive()
{
	return gController != nullptr;
}


float SexyAppBase::GetControllerSensitivity()          { return gSensitivity; }
void  SexyAppBase::SetControllerSensitivity(float v)   { gSensitivity = Clampf(v, 0.5f, 2.0f); }
float SexyAppBase::GetControllerSunRadius()            { return gSunRadius; }
void  SexyAppBase::SetControllerSunRadius(float v)     { gSunRadius = Clampf(v, 60.0f, 640.0f); }
bool  SexyAppBase::GetControllerFreeCursor()           { return gFreeCursor; }
void  SexyAppBase::SetControllerFreeCursor(bool v)     { gFreeCursor = v; }
bool  SexyAppBase::IsControllerGameSpeedUp()           { return gGameSpeedUp; }
bool  SexyAppBase::IsControllerGameSlowDown()          { return gGameSlowDown; }
bool  SexyAppBase::GetControllerCursorBoostEnabled()   { return gCursorBoostEnabled; }
void  SexyAppBase::SetControllerCursorBoostEnabled(bool v) { gCursorBoostEnabled = v; if (!v) gCursorBoost = false; }
bool  SexyAppBase::GetControllerSwapAB()               { return gSwapAB; }
void  SexyAppBase::SetControllerSwapAB(bool v)         { gSwapAB = v; }
bool  SexyAppBase::GetControllerSwapXY()               { return gSwapXY; }
void  SexyAppBase::SetControllerSwapXY(bool v)         { gSwapXY = v; }

// If the gamepad cursor is over a lawn cell, returns its selector-box rect
// (game space) so the caller draws a cell box instead of the pointer arrow.
bool SexyAppBase::GetControllerBox(int& theX, int& theY, int& theW, int& theH)
{
	if (gController == nullptr || !gCursorValid || !gOnBoard)
		return false;
	theX = gBoxX;
	theY = gBoxY;
	theW = gBoxW;
	theH = gBoxH;
	return true;
}

// Translate a controller button into the mouse/key event the game expects.
// Returns true if the event was a controller event and was handled.
bool SexyAppBase::HandleControllerEvent(const SDL_Event& theEvent)
{
	switch (theEvent.type)
	{
		case SDL_CONTROLLERDEVICEADDED:
			ControllerOpenFirst();
			return true;

		case SDL_CONTROLLERDEVICEREMOVED:
			if (gController != nullptr &&
				theEvent.cdevice.which ==
					SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(gController)))
			{
				SDL_GameControllerClose(gController);
				gController = nullptr;
				ControllerOpenFirst();
			}
			return true;

		case SDL_CONTROLLERAXISMOTION:
			if (theEvent.caxis.axis == SDL_CONTROLLER_AXIS_LEFTX)
				gStickX = NormalizeAxis(theEvent.caxis.value);
			else if (theEvent.caxis.axis == SDL_CONTROLLER_AXIS_LEFTY)
				gStickY = NormalizeAxis(theEvent.caxis.value);
			// The triggers set the pace while held: L2 slower, R2 faster. Held
			// rather than toggled, so the game's speed is never left somewhere
			// unexpected, and the pace returns the moment a finger lifts.
			if (theEvent.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT)
				gGameSlowDown = theEvent.caxis.value > kTriggerThreshold;
			if (theEvent.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT)
				gGameSpeedUp = theEvent.caxis.value > kTriggerThreshold;
			return true;

		case SDL_CONTROLLERBUTTONDOWN:
		case SDL_CONTROLLERBUTTONUP:
		{
			bool aDown = theEvent.type == SDL_CONTROLLERBUTTONDOWN;
			int x = (int)gCursorX;
			int y = (int)gCursorY;

			// Some handhelds label the face buttons the other way round, so
			// swap the pair before anything acts on it.
			Uint8 aButton = theEvent.cbutton.button;
			if (gSwapAB)
			{
				if (aButton == SDL_CONTROLLER_BUTTON_A)
					aButton = SDL_CONTROLLER_BUTTON_B;
				else if (aButton == SDL_CONTROLLER_BUTTON_B)
					aButton = SDL_CONTROLLER_BUTTON_A;
			}
			if (gSwapXY)
			{
				if (aButton == SDL_CONTROLLER_BUTTON_X)
					aButton = SDL_CONTROLLER_BUTTON_Y;
				else if (aButton == SDL_CONTROLLER_BUTTON_Y)
					aButton = SDL_CONTROLLER_BUTTON_X;
			}


			if (aButton == SDL_CONTROLLER_BUTTON_A)
			{
				// A: commit the highlighted seed (KEYCODE_GAMEPAD_PLANT lets the
				// game pick it up first), then click at the cursor -- which plants
				// the now-held seed, or collects sun/coins under the cursor. Holding
				// A auto-repeats only the click, so a hold sweep-collects sun
				// without re-triggering a plant.
				if (gCursorValid && aDown)
				{
					mMouseIn = true;
					mLastUserInputTick = mLastTimerTime;
					mWidgetManager->KeyDown(KEYCODE_GAMEPAD_PLANT);
					mWidgetManager->MouseMove(x, y);
					mWidgetManager->MouseDown(x, y, 1);
					mWidgetManager->MouseUp(x, y, 1);
					gAHeld = true;
					gARepeatTick = SDL_GetTicks() + kARepeatDelay;
				}
				else if (!aDown)
				{
					gAHeld = false;
				}
				return true;
			}

			if (aButton == SDL_CONTROLLER_BUTTON_B)
			{
				// B in-game: Board decides -- cancel a held item, else grab the
				// shovel. Elsewhere: right-click at the cursor.
				if (gCursorValid && aDown)
				{
					mMouseIn = true;
					mLastUserInputTick = mLastTimerTime;
					if (ControllerInGame())
					{
						mWidgetManager->KeyDown(KEYCODE_GAMEPAD_SHOVEL);
					}
					else
					{
						mWidgetManager->MouseMove(x, y);
						mWidgetManager->MouseDown(x, y, -1);
						mWidgetManager->MouseUp(x, y, -1);
					}
				}
				return true;
			}

			// Start/Select: escape. In-game that pauses (options dialog), in
			// menus and dialogs it backs out -- same as the keyboard key.
			if (aDown &&
				(theEvent.cbutton.button == SDL_CONTROLLER_BUTTON_START ||
				 theEvent.cbutton.button == SDL_CONTROLLER_BUTTON_BACK))
			{
				mLastUserInputTick = mLastTimerTime;
				mWidgetManager->KeyDown(KEYCODE_ESCAPE);
				return true;
			}

			// X: context action (open store / whack hammer / slot machine lever).
			if (aDown && aButton == SDL_CONTROLLER_BUTTON_X)
			{
				mWidgetManager->KeyDown(KEYCODE_GAMEPAD_CONTEXT);
				return true;
			}

			// Y: Zen Garden helper (wake Stinky).
			if (aDown && aButton == SDL_CONTROLLER_BUTTON_Y)
			{
				mWidgetManager->KeyDown(KEYCODE_GAMEPAD_ZEN);
				return true;
			}

			// Shoulder buttons cycle the held seed packet. L1/L2 -> previous,
			// R1/R2 -> next. Emitted as synthetic keycodes so the game layer
			// (Board::KeyDown) owns the seed logic.
			if (aDown &&
				(theEvent.cbutton.button == SDL_CONTROLLER_BUTTON_LEFTSHOULDER))
			{
				mWidgetManager->KeyDown(KEYCODE_GAMEPAD_PREV_SEED);
				return true;
			}
			if (aDown &&
				(theEvent.cbutton.button == SDL_CONTROLLER_BUTTON_RIGHTSHOULDER))
			{
				mWidgetManager->KeyDown(KEYCODE_GAMEPAD_NEXT_SEED);
				return true;
			}

			// L3 sprints the cursor while held, the same bargain as the triggers:
			// a finger is on it, so it can never be left switched on.
			if (theEvent.cbutton.button == SDL_CONTROLLER_BUTTON_LEFTSTICK)
			{
				gCursorBoost = aDown && gCursorBoostEnabled;
				return true;
			}
			return true;
		}
	}
	return false;
}

static void RecordDemoMousePosition(SexyAppBase* theApp, int theX, int theY)
{
	// stream coords are unsigned 12-bit; wrap so out-of-bounds clicks stay out of bounds
	theX &= 4095;
	theY &= 4095;

	int aDiffX = theX - theApp->mLastDemoMouseX;
	int aDiffY = theY - theApp->mLastDemoMouseY;

	if ((abs(aDiffX) < 32) && (abs(aDiffY) < 32))
	{
		if ((aDiffX != 0) || (aDiffY != 0))
		{
			theApp->WriteDemoTimingBlock();
			theApp->mDemoBuffer.WriteNumBits(1, 1);
			theApp->mDemoBuffer.WriteNumBits(0, 1);
			theApp->mDemoBuffer.WriteNumBits(aDiffX, 6);
			theApp->mDemoBuffer.WriteNumBits(aDiffY, 6);
		}
	}
	else
	{
		theApp->WriteDemoTimingBlock();
		theApp->mDemoBuffer.WriteNumBits(0, 1);
		theApp->mDemoBuffer.WriteNumBits(DEMO_MOUSE_POSITION, 5);
		theApp->mDemoBuffer.WriteNumBits(theX, 12);
		theApp->mDemoBuffer.WriteNumBits(theY, 12);
	}

	theApp->mLastDemoMouseX = theX;
	theApp->mLastDemoMouseY = theY;
}

// Records input events into the demo stream, mirroring the formats read by ProcessDemo
static void RecordDemoEvent(SexyAppBase* theApp, const SDL_Event& theEvent)
{
	switch (theEvent.type)
	{
		case SDL_APP_WILLENTERBACKGROUND:
		case SDL_APP_DIDENTERFOREGROUND:
			theApp->WriteDemoTimingBlock();
			theApp->mDemoBuffer.WriteNumBits(0, 1);
			theApp->mDemoBuffer.WriteNumBits(DEMO_SIZE, 5);
			theApp->mDemoBuffer.WriteBoolean(theEvent.type == SDL_APP_WILLENTERBACKGROUND);
			break;

		case SDL_WINDOWEVENT:
			switch (theEvent.window.event)
			{
				case SDL_WINDOWEVENT_MINIMIZED:
				case SDL_WINDOWEVENT_RESTORED:
					theApp->WriteDemoTimingBlock();
					theApp->mDemoBuffer.WriteNumBits(0, 1);
					theApp->mDemoBuffer.WriteNumBits(DEMO_SIZE, 5);
					theApp->mDemoBuffer.WriteBoolean(theEvent.window.event == SDL_WINDOWEVENT_MINIMIZED);
					break;

				case SDL_WINDOWEVENT_FOCUS_GAINED:
				case SDL_WINDOWEVENT_FOCUS_LOST:
					theApp->WriteDemoTimingBlock();
					theApp->mDemoBuffer.WriteNumBits(0, 1);
					theApp->mDemoBuffer.WriteNumBits(DEMO_ACTIVATE_APP, 5);
					theApp->mDemoBuffer.WriteNumBits(theEvent.window.event == SDL_WINDOWEVENT_FOCUS_GAINED ? 1 : 0, 1);
					break;
			}
			break;

		case SDL_MOUSEMOTION:
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
		{
			int x = (theEvent.type == SDL_MOUSEMOTION) ? theEvent.motion.x : theEvent.button.x;
			int y = (theEvent.type == SDL_MOUSEMOTION) ? theEvent.motion.y : theEvent.button.y;
			theApp->mWidgetManager->RemapMouse(x, y);

			RecordDemoMousePosition(theApp, x, y);

			if (theEvent.type != SDL_MOUSEMOTION)
			{
				bool down = theEvent.type == SDL_MOUSEBUTTONDOWN;
				int aBtnNum =
					(theEvent.button.button == SDL_BUTTON_LEFT) ? 1 :
					(theEvent.button.button == SDL_BUTTON_RIGHT) ? -1 :
					3;
				if (down && theEvent.button.clicks == 2)
					aBtnNum = (theEvent.button.button == SDL_BUTTON_LEFT) ? 2 : -2;

				theApp->WriteDemoTimingBlock();
				theApp->mDemoBuffer.WriteNumBits(1, 1);
				theApp->mDemoBuffer.WriteNumBits(1, 1);
				theApp->mDemoBuffer.WriteNumBits(down ? 1 : 0, 1);
				theApp->mDemoBuffer.WriteNumBits(aBtnNum, 3);
			}

			if (!theApp->mMouseIn)
			{
				theApp->WriteDemoTimingBlock();
				theApp->mDemoBuffer.WriteNumBits(0, 1);
				theApp->mDemoBuffer.WriteNumBits(DEMO_MOUSE_ENTER, 5);
			}
			break;
		}

		case SDL_MOUSEWHEEL:
			theApp->WriteDemoTimingBlock();
			theApp->mDemoBuffer.WriteNumBits(0, 1);
			theApp->mDemoBuffer.WriteNumBits(DEMO_MOUSE_WHEEL, 5);
			theApp->mDemoBuffer.WriteNumBits(std::clamp(theEvent.wheel.y, -128, 127), 8);
			break;

		case SDL_KEYDOWN:
		{
			if (theApp->mAllowAltEnter &&
				theEvent.key.repeat == 0 &&
				(theEvent.key.keysym.sym == SDLK_RETURN || theEvent.key.keysym.sym == SDLK_KP_ENTER) &&
				(theEvent.key.keysym.mod & KMOD_ALT))
				break; // screen-mode toggle, not delivered to the widget manager

			theApp->WriteDemoTimingBlock();
			theApp->mDemoBuffer.WriteNumBits(0, 1);
			theApp->mDemoBuffer.WriteNumBits(DEMO_KEY_DOWN, 5);
			theApp->mDemoBuffer.WriteNumBits(static_cast<int>(SDLKeyToKeyCode(theEvent.key.keysym.sym)), 8);

			char aChar = 0;
			if (SDLSynthesizeAsciiCharFromKeyDown(theEvent.key, aChar))
			{
				theApp->WriteDemoTimingBlock();
				theApp->mDemoBuffer.WriteNumBits(0, 1);
				theApp->mDemoBuffer.WriteNumBits(DEMO_KEY_CHAR, 5);
				theApp->mDemoBuffer.WriteNumBits(0, 1);
				theApp->mDemoBuffer.WriteNumBits(aChar, 8);
			}
			break;
		}

		case SDL_KEYUP:
			theApp->WriteDemoTimingBlock();
			theApp->mDemoBuffer.WriteNumBits(0, 1);
			theApp->mDemoBuffer.WriteNumBits(DEMO_KEY_UP, 5);
			theApp->mDemoBuffer.WriteNumBits(static_cast<int>(SDLKeyToKeyCode(theEvent.key.keysym.sym)), 8);
			break;

		case SDL_TEXTINPUT:
			if (theEvent.text.text[0] != 0) // delivered via KeyText, so record the whole UTF-8 string
			{
				theApp->WriteDemoTimingBlock();
				theApp->mDemoBuffer.WriteNumBits(0, 1);
				theApp->mDemoBuffer.WriteNumBits(DEMO_KEY_TEXT, 5);
				theApp->mDemoBuffer.WriteString(theEvent.text.text);
			}
			break;
	}
}

bool SexyAppBase::StartTextInput(std::string& theInput)
{
	(void)theInput;
	SDL_StartTextInput();

#ifdef __EMSCRIPTEN__
	WasmStartSoftKeyboard();
#endif

	return false;
}

void SexyAppBase::StopTextInput()
{
	SDL_StopTextInput();

#ifdef __EMSCRIPTEN__
	WasmStopSoftKeyboard();
#endif
}

void SexyAppBase::SetTextInputRect(const Rect& theRect)
{
	const Rect& aLogical = mWidgetManager->mMouseDestRect;
	const Rect& aPresent = mWidgetManager->mMouseSourceRect; // presentation/window pixels, inverse of RemapMouse
	if (aLogical.mWidth <= 0 || aLogical.mHeight <= 0)
		return;

	SDL_Rect aRect;
	aRect.x = (theRect.mX - aLogical.mX) * aPresent.mWidth / aLogical.mWidth + aPresent.mX;
	aRect.y = (theRect.mY - aLogical.mY) * aPresent.mHeight / aLogical.mHeight + aPresent.mY;
	aRect.w = theRect.mWidth * aPresent.mWidth / aLogical.mWidth;
	aRect.h = theRect.mHeight * aPresent.mHeight / aLogical.mHeight;
	SDL_SetTextInputRect(&aRect);
}

bool SexyAppBase::ProcessDeferredMessages(bool singleMessage)
{
#ifdef __EMSCRIPTEN__
	if (!mPlayingDemoBuffer)
	{
		int aPendingKey = WasmPopSoftKeyboardKey();
		if (aPendingKey != 0)
		{
			if ((mRecordingDemoBuffer) && (!mShutdown))
			{
				WriteDemoTimingBlock();
				mDemoBuffer.WriteNumBits(0, 1);
				mDemoBuffer.WriteNumBits(DEMO_KEY_DOWN, 5);
				mDemoBuffer.WriteNumBits(aPendingKey, 8);
			}
			mLastUserInputTick = mLastTimerTime;
			mWidgetManager->KeyDown(static_cast<KeyCode>(aPendingKey));
			return WasmHasSoftKeyboardEvents() || SDL_HasEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
		}

		int aPendingChar = WasmPopSoftKeyboardChar();
		if (aPendingChar != 0)
		{
			std::string aPendingText;
			do
			{
				aPendingText += static_cast<char>(aPendingChar);
				aPendingChar = WasmPopSoftKeyboardChar();
			}
			while (aPendingChar != 0);

			if ((mRecordingDemoBuffer) && (!mShutdown))
			{
				WriteDemoTimingBlock();
				mDemoBuffer.WriteNumBits(0, 1);
				mDemoBuffer.WriteNumBits(DEMO_KEY_TEXT, 5);
				mDemoBuffer.WriteString(aPendingText);
			}
			mLastUserInputTick = mLastTimerTime;
			mWidgetManager->KeyText(aPendingText);
			return WasmHasSoftKeyboardEvents() || SDL_HasEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
		}
	}
	else
	{
		while (WasmPopSoftKeyboardKey() != 0) {} // discard queued soft-keyboard input during playback
		while (WasmPopSoftKeyboardChar() != 0) {}
	}
#endif

	SDL_Event event;
	if (SDL_PollEvent(&event))
	{
		if ((mRecordingDemoBuffer) && (!mShutdown))
			RecordDemoEvent(this, event);

		if (mPlayingDemoBuffer)
		{
			// Input is replayed from the demo stream; only window-management events are handled
			switch (event.type)
			{
				case SDL_QUIT:
					CloseRequestAsync();
					break;

				case SDL_WINDOWEVENT:
					if (event.window.event == SDL_WINDOWEVENT_CLOSE)
						CloseRequestAsync();
					else if (event.window.event == SDL_WINDOWEVENT_RESIZED)
					{
						mGLInterface->UpdateViewport();
						mWidgetManager->Resize(mScreenBounds, mGLInterface->mPresentationRect);
						mWidgetManager->MarkAllDirty();
					}
					break;
			}

			return SDL_HasEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
		}

		if (HandleControllerEvent(event))
			return SDL_HasEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);

		switch(event.type)
		{
			case SDL_QUIT:
				CloseRequestAsync();
				break;

			case SDL_APP_WILLENTERBACKGROUND:
				mMinimized = true;
				RehupFocus();
				break;

			case SDL_APP_DIDENTERFOREGROUND:
				mMinimized = false;
				RehupFocus();
				mWidgetManager->MarkAllDirty();
				break;

			case SDL_WINDOWEVENT:
				switch(event.window.event)
				{
					case SDL_WINDOWEVENT_CLOSE:
						CloseRequestAsync();
						break;

					case SDL_WINDOWEVENT_RESIZED:
						mGLInterface->UpdateViewport();
						mWidgetManager->Resize(mScreenBounds, mGLInterface->mPresentationRect);
						mWidgetManager->MarkAllDirty();
						break;

					case SDL_WINDOWEVENT_MINIMIZED:
						mMinimized = true;
						RehupFocus();
						break;

					case SDL_WINDOWEVENT_RESTORED:
						mMinimized = false;
						RehupFocus();
						mWidgetManager->MarkAllDirty();
						break;

					case SDL_WINDOWEVENT_FOCUS_GAINED:
					case SDL_WINDOWEVENT_FOCUS_LOST:
						mActive = event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED;
						RehupFocus();
						break;
				}
				break;

			case SDL_MOUSEWHEEL:
			{
				mLastUserInputTick = mLastTimerTime;
				mWidgetManager->MouseWheel(event.wheel.y);
				break;
			}

			case SDL_MOUSEMOTION:
			{
				if (!mMouseIn)
					mMouseIn = true;

				int x = event.motion.x;
				int y = event.motion.y;
				mWidgetManager->RemapMouse(x, y);

				mLastUserInputTick = mLastTimerTime;

				mWidgetManager->MouseMove(x, y);
				break;
			}

			case SDL_MOUSEBUTTONDOWN:
			{
				if (!mMouseIn)
					mMouseIn = true;

				int x = event.button.x;
				int y = event.button.y;
				mWidgetManager->RemapMouse(x, y);

				mLastUserInputTick = mLastTimerTime;

				mWidgetManager->MouseMove(x, y);
				int btn =
					(event.button.button == SDL_BUTTON_LEFT) ? 1 :
					(event.button.button == SDL_BUTTON_RIGHT) ? -1 :
					3;
				if (event.button.clicks == 2)
					btn = (event.button.button == SDL_BUTTON_LEFT) ? 2 : -2;

				mWidgetManager->MouseDown(x, y, btn);
				break;
			}

			case SDL_MOUSEBUTTONUP:
			{
				if (!mMouseIn)
					mMouseIn = true;

				int x = event.button.x;
				int y = event.button.y;
				mWidgetManager->RemapMouse(x, y);

				mLastUserInputTick = mLastTimerTime;

				mWidgetManager->MouseMove(x, y);
				int btn =
					(event.button.button == SDL_BUTTON_LEFT) ? 1 :
					(event.button.button == SDL_BUTTON_RIGHT) ? -1 :
					3;

				mWidgetManager->MouseUp(x, y, btn);
				break;
			}

			case SDL_KEYDOWN:
			{
				mLastUserInputTick = mLastTimerTime;

				if (mAllowAltEnter &&
					event.key.repeat == 0 &&
					(event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER) &&
					(event.key.keysym.mod & KMOD_ALT))
				{
					SwitchScreenMode(!mIsWindowed);
					break;
				}

				mWidgetManager->KeyDown(SDLKeyToKeyCode(event.key.keysym.sym));

				char aSynthesizedChar = 0;
				if (SDLSynthesizeAsciiCharFromKeyDown(event.key, aSynthesizedChar))
					mWidgetManager->KeyChar(aSynthesizedChar);

				break;
			}

			case SDL_KEYUP:
				mLastUserInputTick = mLastTimerTime;
				mWidgetManager->KeyUp(SDLKeyToKeyCode(event.key.keysym.sym));
				break;

			case SDL_TEXTINPUT:
				mLastUserInputTick = mLastTimerTime;
				mWidgetManager->KeyText(std::string_view(event.text.text));
				break;
		}
	}
	else
	{
		// No event this poll cycle: advance the controller-driven cursor once.
		UpdateControllerCursor();
	}

	return SDL_HasEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
}
