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
* Description:	Header file for the MGLDOS binding for the MSDOS environment.
*
*
****************************************************************************/

#ifndef	__MGLDOS_H
#define	__MGLDOS_H

#ifndef MGLDOS
#define	MGLDOS
#endif

/*------------------------- Function Prototypes ---------------------------*/

#ifdef	__cplusplus
extern "C" {			/* Use "C" linkage when in C++ mode	*/
#endif

/* Disable/enable event handling (call before calling MGL_init */

void 	MGLAPI MGL_useEvents(ibool use);

/* Suspend/resume event handling */

void	MGLAPI MGL_suspend(void);
void	MGLAPI MGL_resume(void);

/* Install user supplied event filter callback */

void	MGLAPI MGL_setUserEventFilter(ibool (*userEventFilter)(event_t *evt));

#ifdef	__cplusplus
}						/* End of "C" linkage for C++	*/
#endif

#endif	/* __MGLDOS_H */
