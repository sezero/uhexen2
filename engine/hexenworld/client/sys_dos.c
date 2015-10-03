/*
 * sys_dos.c -- DOS system interface code.
 * from quake1 source with adaptations for uhexen2.
 * $Id$
 *
 * Copyright (C) 1996-1997  Id Software, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "q_stdinc.h"
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <dos.h>
#include <io.h>
#include <dir.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dpmi.h>
#include <crt0.h>	/* for _crt0_startup_flags */
#include <sys/nearptr.h>
#include <conio.h>
#include <time.h>

#include "quakedef.h"
#include "dosisms.h"
#include "debuglog.h"
#include "sys_dxe.h"

#define MIN_MEM_ALLOC	0x1000000	/* minimum 16 mb */
#define STD_MEM_ALLOC	0x2000000	/* standart 32 mb */

/* 2000-07-16, DOSQuake/DJGPP mem detection fix by
 * Norberto Alfredo Bensa
 */
int	_crt0_startup_flags = _CRT0_FLAG_UNIX_SBRK;
#ifdef GLQUAKE /* need at least 1MB stack for 3dfx. */
unsigned int _stklen = 1048576;  /* FS: FIXME TUNE. */
#endif

int		end_of_memory;
static qboolean	lockmem, lockunlockmem, unlockmem;
static int	win95;

#define	KEYBUF_SIZE	256
static unsigned char	keybuf[KEYBUF_SIZE];
static int	keybuf_head = 0;
static int	keybuf_tail = 0;

static quakeparms_t	quakeparms;
static int		sys_checksum;

/* 2000-07-28, DOSQuake "time running too fast" fix
 * by Norberto Alfredo Bensa. Set USE_UCLOCK_TIME
 * to 0 if you want to use the old original code.
 * See Sys_DoubleTime() for information on uclock()
 */
#define	USE_UCLOCK_TIME		1

static void Sys_InitTime (void);
#if !USE_UCLOCK_TIME
static double		curtime = 0.0;
static double		lastcurtime = 0.0;
static double		oldtime = 0.0;
#endif	/* ! USE_UCLOCK_TIME */

cvar_t			sys_nostdout = {"sys_nostdout", "0", CVAR_NONE};
cvar_t			sys_throttle = {"sys_throttle", "0.02", CVAR_ARCHIVE};

float			fptest_temp;

extern char	start_of_memory __asm__("start");

//=============================================================================

// this is totally dependent on cwsdpmi putting the stack right after tge
// global data

// This does evil things in a Win95 DOS box!!!
#if 0
extern byte	end;
#define	CHECKBYTE	0xed
static void Sys_InitStackCheck (void)
{
	int		i;

	for (i = 0; i < 128*1024; i++)
		(&end)[i] = CHECKBYTE;
}

void Sys_StackCheck (void)
{
	int		i;

	for (i = 0; i < 128*1024; i++)
	{
		if ( (&end)[i] != CHECKBYTE )
			break;
	}

	Con_Printf ("%i undisturbed stack bytes\n", i);
	if (end != CHECKBYTE)
		Sys_Error ("System stack overflow!");
}
#endif

//=============================================================================

static byte scantokey[128] =
{
//	0        1       2       3       4       5       6       7
//	8        9       A       B       C       D       E       F
	0  ,    27,     '1',    '2',    '3',    '4',    '5',    '6',
	'7',    '8',    '9',    '0',    '-',    '=', K_BACKSPACE, 9,	// 0
	'q',    'w',    'e',    'r',    't',    'y',    'u',    'i',
	'o',    'p',    '[',    ']',     13,   K_CTRL,  'a',    's',	// 1
	'd',    'f',    'g',    'h',    'j',    'k',    'l',    ';',
	'\'',   '`',  K_SHIFT,  '\\',   'z',    'x',    'c',    'v',	// 2
	'b',    'n',    'm',    ',',    '.',    '/',  K_SHIFT,  '*',
	K_ALT,  ' ',     0 ,    K_F1,   K_F2,   K_F3,   K_F4,  K_F5,	// 3
	K_F6,  K_F7,   K_F8,    K_F9,  K_F10,    0 ,     0 , K_HOME,
	K_UPARROW,K_PGUP,'-',K_LEFTARROW,'5',K_RIGHTARROW,'+',K_END,	// 4
	K_DOWNARROW,K_PGDN,K_INS,K_DEL,   0 ,    0 ,     0 ,  K_F11,
	K_F12,   0 ,     0 ,     0 ,      0 ,    0 ,     0 ,     0 ,	// 5
	0  ,     0 ,     0 ,     0 ,      0 ,    0 ,     0 ,     0 ,
	0  ,     0 ,     0 ,     0 ,      0 ,    0 ,     0 ,     0 ,	// 6
	0  ,     0 ,     0 ,     0 ,      0 ,    0 ,     0 ,     0 ,
	0  ,     0 ,     0 ,     0 ,      0 ,    0 ,     0 ,     0	// 7
};

#if 0	/* not used */
static byte shiftscantokey[128] =
{
//	0       1       2       3       4       5       6       7
//	8       9       A       B       C       D       E       F
	0  ,    27,     '!',    '@',    '#',    '$',    '%',    '^',
	'&',    '*',    '(',    ')',    '_',    '+', K_BACKSPACE, 9,	// 0
	'Q',    'W',    'E',    'R',    'T',    'Y',    'U',    'I',
	'O',    'P',    '{',    '}',    13 ,   K_CTRL,  'A',    'S',	// 1
	'D',    'F',    'G',    'H',    'J',    'K',    'L',    ':',
	'"' ,    '~', K_SHIFT,  '|',    'Z',    'X',    'C',    'V',	// 2
	'B',    'N',    'M',    '<',    '>',    '?',  K_SHIFT,  '*',
	K_ALT,  ' ',     0 ,    K_F1,   K_F2,   K_F3,   K_F4,  K_F5,	// 3
	K_F6,  K_F7,   K_F8,    K_F9,  K_F10,    0 ,     0 , K_HOME,
	K_UPARROW,K_PGUP,'_',K_LEFTARROW,'%',K_RIGHTARROW,'+',K_END,	// 4
	K_DOWNARROW,K_PGDN,K_INS,K_DEL,   0 ,    0 ,     0 ,  K_F11,
	K_F12,   0 ,     0 ,     0 ,      0 ,    0 ,     0 ,     0 ,	// 5
	0  ,     0 ,     0 ,     0 ,      0 ,    0 ,     0 ,     0 ,
	0  ,     0 ,     0 ,     0 ,      0 ,    0 ,     0 ,     0 ,	// 6
	0  ,     0 ,     0 ,     0 ,      0 ,    0 ,     0 ,     0 ,
	0  ,     0 ,     0 ,     0 ,      0 ,    0 ,     0 ,     0	// 7
};
#endif

static void TrapKey (void)
{
//	static int	ctrl = 0;
	keybuf[keybuf_head] = dos_inportb(0x60);
	dos_outportb(0x20, 0x20);
	/*
	if (scantokey[keybuf[keybuf_head] & 0x7f] == K_CTRL)
		ctrl = keybuf[keybuf_head] & 0x80;
	if (ctrl && scantokey[keybuf[keybuf_head] & 0x7f] == 'c')
		Sys_Error("ctrl-c hit\n");
	*/
	keybuf_head = (keybuf_head + 1) & (KEYBUF_SIZE - 1);
}


static void Sys_DetectWin95 (void)
{
	__dpmi_regs	r;

	r.x.ax = 0x160a;		/* Get Windows Version */
	__dpmi_int(0x2f, &r);

	if (r.x.ax || r.h.bh < 4)	/* Not windows or earlier than Win95 */
	{
		win95 = 0;
		lockmem = true;
		lockunlockmem = false;
		unlockmem = true;
	}
	else
	{
		win95 = 1;
		lockunlockmem = COM_CheckParm ("-winlockunlock");

		if (lockunlockmem)
			lockmem = true;
		else
			lockmem = COM_CheckParm ("-winlock");

		unlockmem = lockmem && !lockunlockmem;
	}
}

#ifndef GLQUAKE
static int		minmem;

static void *dos_getmaxlockedmem (int *size)
{
	__dpmi_free_mem_info	meminfo;
	__dpmi_meminfo		info;
	int		working_size;
	void		*working_memory;
	int		last_locked;
	int		i, j, extra, allocsize;
	static const char msg[] = "Locking data...";
	byte		*x;
	unsigned long	ul;
 
// first lock all the current executing image so the locked count will
// be accurate.  It doesn't hurt to lock the memory multiple times
	last_locked = __djgpp_selector_limit + 1;
	info.size = last_locked - 4096;
	info.address = __djgpp_base_address + 4096;

	if (lockmem)
	{
		if (__dpmi_lock_linear_region(&info))
		{
			Sys_Error ("Lock of current memory at 0x%lx for %ldKb failed!\n",
						info.address, info.size / 1024);
		}
	}

	__dpmi_get_free_memory_information(&meminfo);

	if (!win95)	/* Not windows or earlier than Win95 */
	{
		ul = meminfo.maximum_locked_page_allocation_in_pages * 4096;
	}
	else
	{
		ul = meminfo.largest_available_free_block_in_bytes - LEAVE_FOR_CACHE;
	}

	if (ul > 0x7fffffff)
		ul = 0x7fffffff;	/* limit to 2GB */
	working_size = (int) ul;
	working_size &= ~0xffff;	/* Round down to 64K */
	working_size += 0x10000;

	do
	{
		working_size -= 0x10000;	/* Decrease 64K and try again */
		working_memory = sbrk(working_size);
	} while (working_memory == (void *)-1);

	extra = 0xfffc - ((unsigned)sbrk(0) & 0xffff);

	if (extra > 0)
	{
		sbrk(extra);
		working_size += extra;
	}

// now grab the memory
	info.address = last_locked + __djgpp_base_address;

	if (!win95)
	{
		info.size = __djgpp_selector_limit + 1 - last_locked;

		while (info.size > 0 && __dpmi_lock_linear_region(&info))
		{
			info.size -= 0x1000;
			working_size -= 0x1000;
			sbrk(-0x1000);
		}
	}
	else
	{	/* Win95 section */
		j = COM_CheckParm("-winmem");
	//	minmem = MIN_MEM_ALLOC;
		minmem = STD_MEM_ALLOC;

		if (j && j < com_argc - 1)
		{
			allocsize = ((int)(atoi(com_argv[j + 1]))) * 0x100000 +
							LOCKED_FOR_MALLOC;

			if (allocsize < (minmem + LOCKED_FOR_MALLOC))
				allocsize = minmem + LOCKED_FOR_MALLOC;
		}
		else
		{
			allocsize = minmem + LOCKED_FOR_MALLOC;
		}

		if (!lockmem)
		{
		// we won't lock, just sbrk the memory
			info.size = allocsize;
			goto UpdateSbrk;
		}

		// lock the memory down
		write (STDOUT_FILENO, msg, strlen (msg));

		for (j = allocsize; j > (minmem + LOCKED_FOR_MALLOC); j -= 0x100000)
		{
			info.size = j;

			if (!__dpmi_lock_linear_region(&info))
				goto Locked;

			write (STDOUT_FILENO, ".", 1);
		}

	// finally, try with the absolute minimum amount
		for (i = 0; i < 10; i++)
		{
			info.size = minmem + LOCKED_FOR_MALLOC;

			if (!__dpmi_lock_linear_region(&info))
				goto Locked;
		}

		Sys_Error ("Can't lock memory; %lu Mb lockable RAM required. "
				"Try shrinking smartdrv.", info.size / 0x100000);

Locked:

UpdateSbrk:

		info.address += info.size;
		info.address -= __djgpp_base_address + 4;	// ending point, malloc align
		working_size = info.address - (int)working_memory;
		sbrk(info.address - (int)sbrk(0));		// negative adjustment
	}

	if (lockunlockmem)
	{
		__dpmi_unlock_linear_region (&info);
		printf ("Locked and unlocked %d Mb data\n", working_size / 0x100000);
	}
	else if (lockmem)
	{
		printf ("Locked %d Mb data\n", working_size / 0x100000);
	}
	else
	{
		printf ("Allocated %d Mb data\n", working_size / 0x100000);
	}

// touch all the memory to make sure it's there. The 16-page skip is to
// keep Win 95 from thinking we're trying to page ourselves in (we are
// doing that, of course, but there's no reason we shouldn't)
	x = (byte *)working_memory;

	for (j = 0; j < 4; j++)
	{
		for (i = 0; i < (working_size - 16 * 0x1000); i += 4)
		{
			sys_checksum += *(int *)&x[i];
			sys_checksum += *(int *)&x[i + 16 * 0x1000];
		}
	}

// give some of what we locked back for malloc before returning.  Done
// by cheating and passing a negative value to sbrk
	working_size -= LOCKED_FOR_MALLOC;
	sbrk( -(LOCKED_FOR_MALLOC));
	*size = working_size;
	return working_memory;
}
#endif


int Sys_mkdir (const char *path, qboolean crash)
{
	int rc = mkdir (path, 0777);
	if (rc != 0 && errno == EEXIST)
		rc = 0;
	if (rc != 0 && crash)
		Sys_Error("Unable to create directory %s", path);
	return rc;
}

int Sys_rmdir (const char *path)
{
	return rmdir(path);
}

int Sys_unlink (const char *path)
{
	return remove(path);
}

int Sys_rename (const char *oldp, const char *newp)
{
	return rename(oldp, newp);
}

long Sys_filesize (const char *path)
{
	struct ffblk	f;

	if (findfirst(path, &f, FA_ARCH | FA_RDONLY) != 0)
		return -1;

	return (long) f.ff_fsize;
}

int Sys_FileType (const char *path)
{
	int attr = _chmod(path, 0);
	/* Root directories on some non-local drives
	   (e.g. CD-ROM) as well as devices may fail
	   _chmod, but we are not interested in such
	   cases.  */
	if (attr == -1)
		return FS_ENT_NONE;
	if (attr & _A_SUBDIR)
		return FS_ENT_DIRECTORY;
	if (attr & _A_VOLID)	/* we shouldn't hit this! */
		return FS_ENT_DIRECTORY;

	return FS_ENT_FILE;
}

#define	COPY_READ_BUFSIZE		8192	/* BUFSIZ */
int Sys_CopyFile (const char *frompath, const char *topath)
{
	char	buf[COPY_READ_BUFSIZE];
	int	in, out;
	long	remaining, count;
	struct ftime	ft;

	in = open (frompath, O_RDONLY | O_BINARY);
	if (in < 0)
	{
		Con_Printf ("%s: unable to open %s\n", __thisfunc__, frompath);
		return 1;
	}
	remaining = filelength (in);
	if (remaining < 0)
	{
		Con_Printf ("%s: %s failed filelength()\n", __thisfunc__, frompath);
		close (in);
		return 1;
	}
	out = open (topath, O_CREAT | O_WRONLY | O_TRUNC | O_BINARY, 0666);
	if (out < 0)
	{
		Con_Printf ("%s: unable to create %s\n", __thisfunc__, topath);
		close (in);
		return 1;
	}

	while (remaining)
	{
		if (remaining < sizeof(buf))
			count = remaining;
		else	count = sizeof(buf);

		if (read(in, buf, count) < 0)
			break;
		if (write(out, buf, count) < 0)
			break;

		remaining -= count;
	}

	if (remaining == 0) {
	/* restore the file's timestamp */
		if (getftime(in, &ft) == 0)
			setftime(out, &ft);
	}

	close (in);
	close (out);

	return remaining;
}

/*
=================================================
simplified findfirst/findnext implementation:
Sys_FindFirstFile and Sys_FindNextFile return
filenames only.
=================================================
*/
static struct ffblk	finddata;
static int		findhandle = -1;
static char	findstr[MAX_OSPATH];

const char *Sys_FindFirstFile (const char *path, const char *pattern)
{
	if (findhandle == 0)
		Sys_Error ("Sys_FindFirst without FindClose");

	q_snprintf (findstr, sizeof(findstr), "%s/%s", path, pattern);
	memset (&finddata, 0, sizeof(finddata));

	findhandle = findfirst(findstr, &finddata, FA_ARCH | FA_RDONLY);
	if (findhandle == 0)
		return finddata.ff_name;

	return NULL;
}

const char *Sys_FindNextFile (void)
{
	if (findhandle != 0)
		return NULL;

	if (findnext(&finddata) == 0)
		return finddata.ff_name;

	return NULL;
}

void Sys_FindClose (void)
{
	findhandle = -1;
}

void Sys_Sleep (unsigned long msecs)
{
	usleep (msecs * 1000);
}


static void Sys_Init (void)
{
	MaskExceptions ();

	Sys_SetFPCW ();

#if !USE_UCLOCK_TIME
	dos_outportb(0x43, 0x34);	// set system timer to mode 2
	dos_outportb(0x40, 0);			// for Sys_DoubleTime()
	dos_outportb(0x40, 0);
#endif	/* ! USE_UCLOCK_TIME */

	Sys_InitTime ();

	_go32_interrupt_stack_size = 4 * 1024;
	_go32_rmcb_stack_size = 4 * 1024;

	Sys_InitDXE3();
}

void Sys_Shutdown (void)
{
	if (!isDedicated)
		dos_restoreintr(9);

	if (unlockmem)
	{
		dos_unlockmem (&start_of_memory, end_of_memory - (int)&start_of_memory);
		dos_unlockmem (quakeparms.membase, quakeparms.memsize);
	}
}


#define	SC_UPARROW	0x48
#define	SC_DOWNARROW	0x50
#define	SC_LEFTARROW	0x4b
#define	SC_RIGHTARROW	0x4d
#define	SC_LEFTSHIFT	0x2a
#define	SC_RIGHTSHIFT	0x36

void Sys_SendKeyEvents (void)
{
	int	k, next;
	int	outkey;

// get key events

	while (keybuf_head != keybuf_tail)
	{
		k = keybuf[keybuf_tail++];
		keybuf_tail &= (KEYBUF_SIZE - 1);

		if (k == 0xe0)
			continue;		// special / pause keys
		next = keybuf[(keybuf_tail - 2) & (KEYBUF_SIZE - 1)];
		// Pause generates e1 1d 45 e1 9d c5 when pressed, and
		// nothing when released. e1 is generated only for the
		// pause key.
		if (next == 0xe1)
			continue;		// pause key bullshit
		if (k == 0xc5 && next == 0x9d)
		{
			Key_Event (K_PAUSE, true);
			continue;
		}

		// extended keyboard shift key bullshit
		if ( (k & 0x7f) == SC_LEFTSHIFT || (k & 0x7f) == SC_RIGHTSHIFT )
		{
			if (keybuf[(keybuf_tail - 2) & (KEYBUF_SIZE - 1)] == 0xe0)
				continue;
			k &= 0x80;
			k |= SC_RIGHTSHIFT;
		}

		if (k == 0xc5 && keybuf[(keybuf_tail - 2) & (KEYBUF_SIZE - 1)] == 0x9d)
			continue;	// more pause bullshit

		outkey = scantokey[k & 0x7f];

		if (k & 0x80)
			Key_Event (outkey, false);
		else
			Key_Event (outkey, true);
	}
}


char *Sys_GetClipboardData (void)
{
	return NULL;
}


// =======================================================================
// General routines
// =======================================================================

void Sys_PrintTerm (const char *msgtxt)
{
	const unsigned char	*p;

	if (sys_nostdout.integer)
		return;

	for (p = (const unsigned char *) msgtxt; *p; p++)
		putc (*p, stdout);
}

static void Sys_AtExit (void)
{
// shutdown only once (so Sys_Error can call this function to shutdown, then
// print the error message, then call exit without exit calling this function
// again)
	Sys_Shutdown();
}


void Sys_Quit (void)
{
	Cvar_SetROM ("sys_nostdout", "0");	// enable printing to terminal
	Host_Shutdown ();

	exit (0);
}

#define ERROR_PREFIX	"\nFATAL ERROR: "
void Sys_Error (const char *error, ...)
{
	va_list		argptr;
	char		text[MAX_PRINTMSG];
	const char	text2[] = ERROR_PREFIX;
	const unsigned char	*p;

	va_start (argptr, error);
	q_vsnprintf (text, sizeof(text), error, argptr);
	va_end (argptr);

	if (con_debuglog)
	{
		LOG_Print (ERROR_PREFIX);
		LOG_Print (text);
		LOG_Print ("\n\n");
	}

	Cvar_SetROM ("sys_nostdout", "0");	// enable printing to terminal
	Host_Shutdown ();

	for (p = (const unsigned char *) text2; *p; p++)
		putc (*p, stderr);
	for (p = (const unsigned char *) text ; *p; p++)
		putc (*p, stderr);
	putc ('\n', stderr);
	putc ('\n', stderr);

// Sys_AtExit is called by exit to shutdown the system
	exit (1);
}

/*
================
Sys_MakeCodeWriteable
================
*/
void Sys_MakeCodeWriteable (unsigned long startaddr, unsigned long length)
{
	// it's always writeable
}


/*
================
Sys_DoubleTime
================
*/
double Sys_DoubleTime (void)
{
#if USE_UCLOCK_TIME
/*
From DJGPP uclock() man page :

uclock() returns the number of uclock ticks since an arbitrary time,
actually, since the first call to uclock(), which itself returns zero.
The number of tics per second is UCLOCKS_PER_SEC (declared in time.h
as 1193180.)

uclock() is provided for very high-resulution timing.  uclock_t is a
64-bit integer.  It is currently accurate to better than 1 microsecond
(actually about 840 nanoseconds).  You cannot time across two midnights
with this implementation, giving a maximum useful period of 48 hours
and an effective limit of 24 hours.  Casting to a 32-bit integer limits
its usefulness to about an hour before 32 bits will wrap.

Also note that uclock reprograms the interval timer in your PC to act
as a rate generator rather than a square wave generator.  I've had no
problems running in this mode all the time, but if you notice strange
things happening with the clock (losing time) after using uclock, check
to see if this is the cause of the problem.

Windows 3.X doesn't allow to reprogram the timer so the values returned
by uclock() there are incorrect.  DOS and Windows 9X don't have this
problem.  
Windows NT, 2000 and XP attempt to use the rdtsc feature of newer CPUs
instead of the interval timer because the timer tick and interval timer
are not coordinated.  During calibration the SIGILL signal handler is
replaced to protect against systems which do not support or allow rdtsc.
If rdtsc is available, uclock will keep the upper bits of the returned
value consistent with the bios tick counter by re-calibration if needed.
If rdtsc is not available, these systems fall back to interval timer
usage, which may show an absolute error of 65536 uclock ticks in the
values and not be monotonically increasing.
*/
	return (double) uclock() / (double) UCLOCKS_PER_SEC;

#else
	int		r;
	unsigned	t, tick;
	double		ft, time;
	static int	sametimecount;

	Sys_PushFPCW_SetHigh ();

	t = *(unsigned short*)real2ptr(0x46c) * 65536;

	dos_outportb(0x43, 0);	// latch time
	r = dos_inportb(0x40);
	r |= dos_inportb(0x40) << 8;
	r = (r - 1) & 0xffff;

	tick = *(unsigned short*)real2ptr(0x46c) * 65536;
	if ((tick != t) && (r & 0x8000))
		t = tick;

	ft = (double) (t + (65536 - r)) / 1193200.0;
	time = ft - oldtime;
	oldtime = ft;

	if (time < 0)
	{
		if (time > -3000.0)
			time = 0.0;
		else
			time += 3600.0;
	}

	curtime += time;

	if (curtime == lastcurtime)
	{
		sametimecount++;

		if (sametimecount > 100000)
		{
			curtime += 1.0;
			sametimecount = 0;
		}
	}
	else
	{
		sametimecount = 0;
	}

	lastcurtime = curtime;

	Sys_PopFPCW ();

	return curtime;
#endif	/* ! USE_UCLOCK_TIME */
}


/*
================
Sys_InitTime
================
*/
static void Sys_InitTime (void)
{
#if !USE_UCLOCK_TIME
	int		j;

	Sys_DoubleTime ();

	oldtime = curtime;

	j = COM_CheckParm("-starttime");
	if (j && j < com_argc - 1)
	{
		curtime = (double) (atof(com_argv[j+1]));
	}
	else
	{
		curtime = 0.0;
	}
	lastcurtime = curtime;
#endif	/* ! USE_UCLOCK_TIME */
}

char *Sys_DateTimeString (char *buf)
{
	static char strbuf[24];
	struct _dosdate_t d;
	struct _dostime_t t;
	unsigned int val;

	if (!buf) buf = strbuf;

	_dos_getdate(&d);
	_dos_gettime(&t);

	val = d.month;
	buf[0] = val / 10 + '0';
	buf[1] = val % 10 + '0';
	buf[2] = '/';
	val = d.day;
	buf[3] = val / 10 + '0';
	buf[4] = val % 10 + '0';
	buf[5] = '/';
	val = d.year / 100;
	buf[6] = val / 10 + '0';
	buf[7] = val % 10 + '0';
	val = d.year % 100;
	buf[8] = val / 10 + '0';
	buf[9] = val % 10 + '0';

	buf[10] = ' ';

	val = t.hour;
	buf[11] = val / 10 + '0';
	buf[12] = val % 10 + '0';
	buf[13] = ':';
	val = t.minute;
	buf[14] = val / 10 + '0';
	buf[15] = val % 10 + '0';
	buf[16] = ':';
	val = t.second;
	buf[17] = val / 10 + '0';
	buf[18] = val % 10 + '0';

	buf[19] = '\0';

	return buf;
}


/*
================
Sys_GetMemory
================
*/
static void Sys_GetMemory (void)
{
	int		j, tsize;

	j = COM_CheckParm("-mem");
	if (j && j < com_argc - 1)
	{
		quakeparms.memsize = (int) (atof(com_argv[j + 1]) * 1024 * 1024);
		quakeparms.membase = malloc (quakeparms.memsize);
	}
#ifdef GLQUAKE
	else
	{
		/* 16 mb is usually enough. Leave rest of mem for gl driver */
		quakeparms.memsize = (int) 0x1000000;
		quakeparms.membase = malloc (quakeparms.memsize);
	}
#else
	else
	{
		quakeparms.membase = dos_getmaxlockedmem (&quakeparms.memsize);
	}
#endif

	printf("malloc'd: %d\n", quakeparms.memsize);

	j = COM_CheckParm ("-heapsize");
	if (j && j < com_argc - 1)
	{
		tsize = atoi (com_argv[j + 1]) * 1024;

		if (tsize < quakeparms.memsize)
			quakeparms.memsize = tsize;
	}
}


/*
================
Sys_PageInProgram

walks the text, data, and bss to make sure it's all paged in so that the
actual physical memory detected by Sys_GetMemory is correct.
================
*/
static void Sys_PageInProgram (void)
{
	int		i, j;

	end_of_memory = (int)sbrk(0);

	if (lockmem)
	{
		if (dos_lockmem ((void *)&start_of_memory, end_of_memory - (int)&start_of_memory))
			Sys_Error ("Couldn't lock text and data");
	}

	if (lockunlockmem)
	{
		dos_unlockmem((void *)&start_of_memory, end_of_memory - (int)&start_of_memory);
		printf ("Locked and unlocked %d Mb image\n",
				(end_of_memory - (int)&start_of_memory) / 0x100000);
	}
	else if (lockmem)
	{
		printf ("Locked %d Mb image\n",
				(end_of_memory - (int)&start_of_memory) / 0x100000);
	}
	else
	{
		printf ("Loaded %d Mb image\n",
				(end_of_memory - (int)&start_of_memory) / 0x100000);
	}

// touch the entire image, doing the 16-page skip so Win95 doesn't think we're
// trying to page ourselves in
	for (j = 0; j < 4; j++)
	{
		for (i = (int)&start_of_memory; i < (end_of_memory - 16 * 0x1000); i += 4)
		{
			sys_checksum += *(int *)i;
			sys_checksum += *(int *)(i + 16 * 0x1000);
		}
	}
}


/*
================
Sys_NoFPUExceptionHandler
================
*/
static void Sys_NoFPUExceptionHandler (int whatever)
{
	const char	err[] = "\nError: Hexen II requires a floating-point processor\n";
	const unsigned char	*p;

	for (p = (const unsigned char *) err; *p; p++)
		putc (*p, stderr);

	exit (1);
}

/*
================
Sys_DefaultExceptionHandler
================
*/
static void Sys_DefaultExceptionHandler (int whatever)
{
}


static int Sys_GetBasedir (char *argv0, char *dst, size_t dstsize)
{
	char	*tmp;

	if (getcwd(dst, dstsize - 1) == NULL)
		return -1;

	tmp = dst;
	while (*tmp != 0)
		tmp++;
	while (*tmp == 0 && tmp != dst)
	{
		--tmp;
		if (tmp != dst && (*tmp == '/' || *tmp == '\\'))
			*tmp = 0;
	}

	return 0;
}


static void PrintVersion (void)
{
	printf ("Hammer of Thyrion, release %s (%s)\n", HOT_VERSION_STR, HOT_VERSION_REL_DATE);
	printf ("running on %s engine %4.2f (%s)\n", ENGINE_NAME, ENGINE_VERSION, PLATFORM_STRING);
	printf ("More info / sending bug reports:  http://uhexen2.sourceforge.net\n");
}

/*
================
main
================
*/
static char	cwd[MAX_OSPATH];

int main (int argc, char **argv)
{
	double		time, oldtime, newtime;

	PrintVersion();

// make sure there's an FPU
	signal(SIGNOFP, Sys_NoFPUExceptionHandler);
	signal(SIGABRT, Sys_DefaultExceptionHandler);
	signal(SIGALRM, Sys_DefaultExceptionHandler);
	signal(SIGKILL, Sys_DefaultExceptionHandler);
	signal(SIGQUIT, Sys_DefaultExceptionHandler);
	signal(SIGINT, Sys_DefaultExceptionHandler);

	if (fptest_temp >= 0.0)
		fptest_temp += 0.1;

	memset (cwd, 0, sizeof(cwd));
	if (Sys_GetBasedir(argv[0], cwd, sizeof(cwd)) != 0)
		Sys_Error ("Couldn't determine current directory");

	/* initialize the host params */
	memset (&quakeparms, 0, sizeof(quakeparms));
	quakeparms.basedir = cwd;
	quakeparms.userdir = cwd;
	quakeparms.argc = argc;
	quakeparms.argv = argv;
	host_parms = &quakeparms;

	LOG_Init (&quakeparms);

	COM_ValidateByteorder ();

	Sys_DetectWin95 ();
	Sys_PageInProgram ();
	Sys_GetMemory ();

	atexit (Sys_AtExit);	// in case we crash

	Sys_Init ();

	dos_registerintr(9, TrapKey);

//	Sys_InitStackCheck ();

	Host_Init();

//	Sys_StackCheck ();
//	Con_Printf ("Top of stack: 0x%x\n", &time);

	oldtime = Sys_DoubleTime ();
	while (1)
	{
		newtime = Sys_DoubleTime ();
		time = newtime - oldtime;

		Host_Frame (time);

//		Sys_StackCheck ();

		oldtime = newtime;
	}
}

