#ifndef IN_JOY_DINPUT_H
#define IN_JOY_DINPUT_H

#include "in_joy.h"

UINT joy_get_num_devs(void)
{
	return joyGetNumDevs();
}

MMRESULT joy_get_pos_ex(UINT uJoyID, LPJOYINFOEX pji)
{
	return joyGetPosEx(uJoyID, pji);
}

MMRESULT joy_get_dev_caps(UINT uJoyID, LPJOYCAPS pjc, UINT cbjc)
{
	return joyGetDevCaps(uJoyID, pjc, cbjc);
}

#endif
