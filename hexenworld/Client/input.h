// input.h -- external (non-keyboard) input devices

void IN_Init (void);

void IN_Shutdown (void);

void IN_Commands (void);
// oportunity for devices to stick commands on the script buffer

void IN_Move (usercmd_t *cmd);
// add additional movement on top of the keyboard move cmd

void IN_ModeChanged (void);
// called whenever screen dimensions change

void IN_ClearStates (void);
// restores all button and position states to defaults


// for window manager events
void IN_ShowMouse (void);
void IN_DeactivateMouse (void);
void IN_HideMouse (void);
void IN_ActivateMouse (void);

extern	cvar_t		_enable_mouse;

void IN_SendKeyEvents (void);

