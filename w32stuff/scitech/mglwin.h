/****************************************************************************
*
*						MegaGraph Graphics Library
*
*  ========================================================================
*
*    The contents of this file are subject to the SciTech MGL Public
*    License Version 1.0 (the "License"); you may not use this file
*    except in compliance with the License. You may obtain a copy of
*    the License at http://www.scitechsoft.com/mgl-license.txt
*
*    Software distributed under the License is distributed on an
*    "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
*    implied. See the License for the specific language governing
*    rights and limitations under the License.
*
*    The Original Code is Copyright (C) 1991-1998 SciTech Software, Inc.
*
*    The Initial Developer of the Original Code is SciTech Software, Inc.
*    All Rights Reserved.
*
*  ========================================================================
*
*
* Language:		ANSI C
* Environment:	IBM PC (MS DOS)
*
* Description:	Header file for the MGLWIN bindings for MS Windows using
*				WinG in a window and WinDirect for full screen. The MGLWIN
*				binding only targets Win32 applications, so cannot be used
*				for 16 bit Windows development.
*
*
****************************************************************************/

#ifndef	__MGLWIN_H
#define	__MGLWIN_H

#ifndef MGLWIN
#define	MGLWIN
#endif

/*---------------------- Macros and type definitions ----------------------*/

#ifdef	DECLARE_HANDLE
/* {secret} */
typedef	HDC			MGL_HDC;
/* {secret} */
typedef	HINSTANCE	MGL_HINSTANCE;
/* {secret} */
typedef	HPALETTE	MGL_HPALETTE;
/* {secret} */
typedef	HGLRC		MGL_HGLRC;
/* {secret} */
typedef	WNDPROC		MGL_WNDPROC;
#else
/* {secret} */
typedef	void		*MGL_HDC;
/* {secret} */
typedef	void		*MGL_HINSTANCE;
/* {secret} */
typedef	void		*MGL_HPALETTE;
/* {secret} */
typedef	void		*MGL_HGLRC;
/* {secret} */
typedef long 		(__stdcall* MGL_WNDPROC)(MGL_HWND, uint, uint, long);
#endif
#ifdef	__DDRAW_INCLUDED__
/* {secret} */
typedef	LPDIRECTDRAW			MGL_LPDD;
/* {secret} */
typedef	LPDIRECTDRAWSURFACE		MGL_LPDDSURF;
/* {secret} */
typedef	LPDIRECTDRAWPALETTE		MGL_LPDDPAL;
#else
/* {secret} */
typedef	void		*MGL_LPDD;
/* {secret} */
typedef	void		*MGL_LPDDSURF;
/* {secret} */
typedef	void		*MGL_LPDDPAL;
#endif

/*------------------------- Function Prototypes ---------------------------*/

#ifdef	__cplusplus
extern "C" {			/* Use "C" linkage when in C++ mode	*/
#endif

/* Function to register a fullscreen window with the MGL. If you wish
 * for the MGL to use your own window for fullscreen modes, you can
 * register it with this function. Note that when the MGL goes into
 * fullscreen modes, the attributes, size and position of the window are
 * modified to make it into a fullscreen Window necessary to cover the
 * entire desktop, and the state of the window will be restore to the original
 * format on return to normal GDI mode.
 *
 * Note that if you are using a common window for Windowed mode and fullscreen
 * modes of your application, you will need to ensure that certain messages
 * that you window normally handles in windowed modes are ignored when in
 * fullscreen modes.
 */

void 	MGLAPI MGL_registerFullScreenWindow(MGL_HWND hwndFullScreen);

/* Obtain the handle to the MGL fullscreen window when in fullscreen modes */

MGL_HWND MGLAPI MGL_getFullScreenWindow(void);

/* Tell the MGL what your applications main window is */

void	MGLAPI MGL_setMainWindow(MGL_HWND hwnd);

/* Tell the MGL your applications instance handle (call before all funcs!) */

void	MGLAPI MGL_setAppInstance(MGL_HINSTANCE hInstApp);

/* Function to register a fullscreen event handling window procedure.
 * If you wish to do your own event handling, you can register your window
 * procedure with the MGL using this function and it will be called
 * when there are messages to be handled. You can still call the MGL_event()
 * functions even if you have registered an event handling procedure.
 */

void 	MGLAPI MGL_registerEventProc(MGL_WNDPROC userWndProc);

/* Device context management */

MGLDC	* MGLAPI MGL_createWindowedDC(MGL_HWND hwnd);

/* Get the DirectDraw object's for a fullscreen video mode. If the device
 * context passed in is not a DirectDraw fullscreen device context, this
 * function will return NULL. Note that we return the value a void pointer
 * (so we dont need to include the DirectDraw headers), so you will need to
 * cast it to an LPDIRECTDRAW or LPDIRECTDRAWSURFACE before you can use them.
 */

MGL_LPDD 		MGLAPI MGL_getDirectDrawObject(MGLDC *dc);
MGL_LPDDSURF 	MGLAPI MGL_getDirectDrawPrimarySurface(MGLDC *dc);
MGL_LPDDSURF 	MGLAPI MGL_getDirectDrawOffscreenSurface(MGLDC *dc);
MGL_LPDDSURF 	MGLAPI MGL_getDirectDrawActiveSurface(MGLDC *dc);
MGL_LPDDPAL		MGLAPI MGL_getDirectDrawPalette(MGLDC *dc);

/* Get a Windows HDC for the MGL device context. You can use this returned
 * HDC to get GDI to draw to the device context surface, such as rendering
 * and using TrueType fonts with the MGL. If a Windows compatible HDC is not
 * available, this function will return NULL.
 */

MGL_HDC	MGLAPI MGL_getWinDC(MGLDC *dc);

/* Associate a Window manager DC with the MGLDC for painting */

ibool	MGLAPI MGL_setWinDC(MGLDC *dc,MGL_HDC hdc);

/* Resize the Windowed device context (call for WM_SIZE) */

void	MGLAPI MGL_resizeWinDC(MGLDC *dc);

/* Activate the WindowDC's palette */

ibool	MGLAPI MGL_activatePalette(MGLDC *dc,ibool unrealize);

/* Let the MGL know when your application is being activated or deactivated.
 * This function only needs to be called when running in Windowed modes and
 * you have set the system palette to SYSPAL_NOSTATIC mode, to ensure
 * that the MGL can properly re-map your application palette when your
 * app is not active and allow Windows to re-map your bitmap colors on the
 * fly. This function should be passed a pointer to the currently active
 * MGL Windowed DC and a flag to indicate whether the app is in the background
 * or not.   
 */

void	MGLAPI MGL_appActivate(MGLDC *winDC,ibool active);

#ifdef	__cplusplus
}						/* End of "C" linkage for C++	*/
#endif

#endif	/* __MGLWIN_H */

