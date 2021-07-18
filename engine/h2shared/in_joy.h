#ifndef IN_JOY
#define IN_JOY

#include <Windows.h>
#include <joystickapi.h>

UINT joy_get_num_devs(void);

MMRESULT joy_get_pos_ex(UINT uJoyID, LPJOYINFOEX pji);

MMRESULT joy_get_dev_caps(UINT uJoyID, LPJOYCAPS pjc, UINT cbjc);

#endif
