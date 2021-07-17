#ifndef IN_JOY_XINPUT_H
#define IN_JOY_XINPUT_H

#include <stdlib.h>
#include <Windows.h>
#include <joystickapi.h>
#include <Xinput.h>

#pragma comment(lib, "Xinput.lib")

UINT WINAPI joyGetNumDevs(void)
{
	XINPUT_STATE state;

	ZeroMemory(&state, sizeof(XINPUT_STATE));

	const DWORD result = XInputGetState(0, &state);

	return result == ERROR_SUCCESS ? 1 : 0;
}

MMRESULT WINAPI joyGetPosEx(UINT uJoyID, LPJOYINFOEX pji)
{
	if (uJoyID != 0)
		return MMSYSERR_BADDEVICEID;

	XINPUT_STATE state;

	ZeroMemory(&state, sizeof(XINPUT_STATE));

	if (XInputGetState(0, &state) != ERROR_SUCCESS)
		return JOYERR_UNPLUGGED;

	const XINPUT_GAMEPAD pad = state.Gamepad;

	const SHORT lx = pad.sThumbLX;
	const SHORT ly = pad.sThumbLY;
	const SHORT rx = pad.sThumbRX;
	const SHORT ry = pad.sThumbRY;

	pji->dwXpos = 32768;
	pji->dwYpos = 32768;
	pji->dwZpos = 32768;
	pji->dwRpos = 32768;

	if (abs(lx) > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
		pji->dwXpos += lx;

	if (abs(ly) > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
		pji->dwYpos += ly;

	if (abs(rx) > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
		pji->dwZpos += rx;

	if (abs(ry) > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
		pji->dwRpos += ry;

	const BYTE lt = pad.bLeftTrigger;

	if (lt > XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
		pji->dwUpos = 32768 + (int)(lt / 255.0 * 32767);

	const BYTE rt = pad.bRightTrigger;

	if (rt > XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
		pji->dwUpos = 32768 + (int)(rt / 255.0 * 32767);

	pji->dwButtons = 0;

	if (pad.wButtons & XINPUT_GAMEPAD_A)
		pji->dwButtons |= JOY_BUTTON1;

	if (pad.wButtons & XINPUT_GAMEPAD_B)
		pji->dwButtons |= JOY_BUTTON2;

	if (pad.wButtons & XINPUT_GAMEPAD_X)
		pji->dwButtons |= JOY_BUTTON3;

	if (pad.wButtons & XINPUT_GAMEPAD_Y)
		pji->dwButtons |= JOY_BUTTON4;

	if (pad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER)
		pji->dwButtons |= JOY_BUTTON5;

	if (pad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER)
		pji->dwButtons |= JOY_BUTTON6;

	if (pad.wButtons & XINPUT_GAMEPAD_BACK)
		pji->dwButtons |= JOY_BUTTON7;

	if (pad.wButtons & XINPUT_GAMEPAD_START)
		pji->dwButtons |= JOY_BUTTON8;

	if (pad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB)
		pji->dwButtons |= JOY_BUTTON9;

	if (pad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB)
		pji->dwButtons |= JOY_BUTTON10;

	if (lt > XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
		pji->dwButtons |= JOY_BUTTON11;

	if (rt > XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
		pji->dwButtons |= JOY_BUTTON12;

	pji->dwPOV = JOY_POVCENTERED;

	if (pad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT)
		pji->dwPOV = JOY_POVLEFT;

	if (pad.wButtons & XINPUT_GAMEPAD_DPAD_UP)
		pji->dwPOV = JOY_POVFORWARD;

	if (pad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)
		pji->dwPOV = JOY_POVRIGHT;

	if (pad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN)
		pji->dwPOV = JOY_POVBACKWARD;

	return JOYERR_NOERROR;
}

MMRESULT WINAPI joyGetDevCaps(UINT uJoyID, LPJOYCAPS pjc, UINT cbjc)
{
	if (uJoyID != 0)
		return MMSYSERR_NODRIVER;

	pjc->wNumButtons = 12;
	pjc->wCaps = JOYCAPS_HASPOV;

	return JOYERR_NOERROR;
}

#endif // IN_JOY_XINPUT_H
