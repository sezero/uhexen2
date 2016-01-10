/*
 * zone.c -- Memory management
 * $Id$
 *
 * Copyright (C) 1996-1997  Id Software, Inc.
 * Copyright (C) 1997-1998  Raven Software Corp.
 * Copyright (C) 2005-2012  O.Sezer <sezero@users.sourceforge.net>
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

#include "quakedef.h"

/* whether Z_Malloc should check zone
 * integrity before every allocation	*/
#define	Z_CHECKHEAP		0

/* whether we compile the debug report
 * commands. see in Memory_Init()	*/
#define	Z_DEBUG_COMMANDS	0

#define	ZONE_MINSIZE	0x40000
#define	ZONE_MAXSIZE	0x200000
#if defined(SERVERONLY)
#define	ZONE_DEFSIZE	0x40000
#else
#define	ZONE_DEFSIZE	0x60000
#endif	/* SERVERONLY */
#define	ZMAGIC		0x1d4a11
#define	ZMAGIC2		0xf382da
#define	HUNK_SENTINAL	0x1df001ed
#define	MINFRAGMENT	64

/* setup size for secondary zone: */
#define	MEM_STATIC_TEX	0x40000

#define	MEM_CODEC_MEM	0
#if defined(CODECS_USE_ZONE)
/* this setup assumes only one codec
 * would be active at a time. */
#if defined(USE_CODEC_MP3)
#if (MEM_CODEC_MEM < LIBMAD_NEEDMEM)
#undef	MEM_CODEC_MEM
#define	MEM_CODEC_MEM	LIBMAD_NEEDMEM
#endif
#endif	/* LIBMAD */
#if defined(USE_CODEC_VORBIS)
#if (MEM_CODEC_MEM < VORBIS_NEEDMEM)
#undef	MEM_CODEC_MEM
#define	MEM_CODEC_MEM	VORBIS_NEEDMEM
#endif
#endif	/* VORBIS */
#endif	/* CODECS_USE_ZONE */

#if defined(SERVERONLY)
#undef	MEM_STATIC_TEX
#define	MEM_STATIC_TEX	0
#undef	MEM_CODEC_MEM
#define	MEM_CODEC_MEM	0
#endif

#define	SECZONE_SIZE			\
	(MEM_STATIC_TEX + MEM_CODEC_MEM)

typedef struct memblock_s
{
	int	size;		/* including the header and possibly tiny fragments */
	int	tag;		/* a tag of 0 is a free block */
	int	magic;		/* should be ZMAGIC */
	int	pad;		/* pad to 64 bit boundary */
	struct	memblock_s	*next, *prev;
} memblock_t;

typedef struct memzone_s
{
	int		size;		/* total bytes malloced, including header */
	memblock_t	blocklist;	/* start / end cap for linked list */
	memblock_t	*rover;
} memzone_t;

typedef struct zonelist_s
{
	int		id, magic;
	const char		*name;
	memzone_t		*zone;
	struct zonelist_s	*next;
} zonelist_t;

#if defined (SERVERONLY)
#define Cache_FreeLow(x)
#define Cache_FreeHigh(x)
#else
static void Cache_FreeLow (int new_low_hunk);
static void Cache_FreeHigh (int new_high_hunk);
#endif


/*
==============================================================================

		ZONE MEMORY ALLOCATION

There is never any space between memblocks, and there will never be two
contiguous free memblocks.

The rover can be left pointing at a non-empty block

The zone calls are pretty much only used for small strings and structures,
all big things are allocated on the hunk.
==============================================================================
*/

static	zonelist_t	*zonelist;
static	char		mainzone[] = "MAINZONE";
#if (SECZONE_SIZE > 0)
static	char		sec_zone[] = "SEC_ZONE";
#endif


/*
========================
Z_Free
========================
*/
void Z_Free (void *ptr)
{
	zonelist_t	*z;
	memblock_t	*block, *other;

	if (!ptr)
		Sys_Error ("%s: NULL pointer", __thisfunc__);

	block = (memblock_t *) ( (byte *)ptr - sizeof(memblock_t));
	if (block->tag == 0)
		Sys_Error ("%s: freed a freed pointer", __thisfunc__);

	z = zonelist;
	while (z != NULL)
	{
		if (z->magic == block->magic)
			break;
		z = z->next;
	}
	if (z == NULL)
		Sys_Error ("%s: freed a pointer without ZMAGIC", __thisfunc__);

	block->tag = 0;		/* mark as free */

	other = block->prev;
	if (!other->tag)
	{	/* merge with previous free block */
		other->size += block->size;
		other->next = block->next;
		other->next->prev = other;
		if (block == z->zone->rover)
			z->zone->rover = other;
		block = other;
	}

	other = block->next;
	if (!other->tag)
	{	/* merge the next free block onto the end */
		block->size += other->size;
		block->next = other->next;
		block->next->prev = block;
		if (other == z->zone->rover)
			z->zone->rover = block;
	}
}


static void *Z_TagMalloc (zonelist_t *z, int size, int tag)
{
	int		extra;
	memblock_t	*start, *rover, *newblock, *base;

	if (!tag)
		Sys_Error ("%s: tried to use a 0 tag", __thisfunc__);

/* scan through the block list looking for the first free block
 * of sufficient size
 */
	size += sizeof(memblock_t);	/* account for size of block header */
	size += 4;			/* space for memory trash tester */
	size = (size + 7) & ~7;		/* align to 8-byte boundary */

	base = rover = z->zone->rover;
	start = base->prev;

	do
	{
		if (rover == start)	/* scaned all the way around the list */
			return NULL;
		if (rover->tag)
			base = rover = rover->next;
		else
			rover = rover->next;
	} while (base->tag || base->size < size);

/* found a block big enough */
	extra = base->size - size;
	if (extra >  MINFRAGMENT)
	{	/* there will be a free fragment after the allocated block */
		newblock = (memblock_t *) ((byte *)base + size);
		newblock->size = extra;
		newblock->tag = 0;	/* free block */
		newblock->prev = base;
		newblock->magic = z->magic;
		newblock->next = base->next;
		newblock->next->prev = newblock;
		base->next = newblock;
		base->size = size;
	}

	base->tag = tag;		/* no longer a free block */

	z->zone->rover = base->next;	/* next allocation will start looking here */

	base->magic = z->magic;

/* marker for memory trash testing */
	*(int *)((byte *)base + base->size - 4) = z->magic;

	return (void *) ((byte *)base + sizeof(memblock_t));
}

/*
========================
Z_CheckHeap
========================
*/
#if Z_CHECKHEAP
static void Z_CheckHeap (memzone_t *zone)
{
	memblock_t	*block;

	for (block = zone->blocklist.next ; ; block = block->next)
	{
		if (block->next == &zone->blocklist)
			break;	/* all blocks have been hit */
		if ( (byte *)block + block->size != (byte *)block->next)
			Sys_Error ("%s: block size does not touch the next block", __thisfunc__);
		if ( block->next->prev != block)
			Sys_Error ("%s: next block doesn't have proper back link", __thisfunc__);
		if (!block->tag && !block->next->tag)
			Sys_Error ("%s: two consecutive free blocks", __thisfunc__);
	}
}
#endif	/* Z_CHECKHEAP */

/*
========================
Z_Malloc
========================
*/
void *Z_Malloc (int size, int zone_id)
{
	void	*buf;
	zonelist_t*	z;

	z = zonelist;
	while (z != NULL)
	{
		if (z->id & zone_id)
			break;
		z = z->next;
	}
	if (z == NULL)
		Sys_Error ("%s: Bad zone id %i", __thisfunc__, zone_id);

#if Z_CHECKHEAP
	Z_CheckHeap (z->zone);	/* DEBUG */
#endif
	buf = Z_TagMalloc (z, size, 1);
	if (!buf)
		Sys_Error ("%s: failed on allocation of %i bytes", __thisfunc__, size);
	memset (buf, 0, size);

	return buf;
}

void *Z_Realloc (void *ptr, int size, int zone_id)
{
	int		old_size;
	void		*old_ptr;
	zonelist_t	*z;
	memblock_t	*block;

	if (!ptr)
		return Z_Malloc (size, zone_id);

	block = (memblock_t *) ((byte *) ptr - sizeof (memblock_t));
	if (block->tag == 0)
		Sys_Error ("%s: realloced a freed pointer", __thisfunc__);

	z = zonelist;
	while (z != NULL)
	{
		if (z->magic == block->magic)
			break;
		z = z->next;
	}
	if (z == NULL)
		Sys_Error ("%s: realloced a pointer without ZMAGIC", __thisfunc__);

	old_size = block->size;
	old_size -= (4 + (int)sizeof(memblock_t));	/* see Z_TagMalloc() */
	old_ptr = ptr;

	Z_Free (ptr);
	z = zonelist;
	while (z != NULL)
	{
		if (z->id & zone_id)
			break;
		z = z->next;
	}
	if (z == NULL)
		Sys_Error ("%s: Bad zone id %i", __thisfunc__, zone_id);

	ptr = Z_TagMalloc (z, size, 1);
	if (!ptr)
		Sys_Error ("%s: failed on allocation of %i bytes", __thisfunc__, size);

	if (ptr != old_ptr)
		memmove (ptr, old_ptr, q_min(old_size, size));
	if (old_size < size)
		memset ((byte *)ptr + old_size, 0, size - old_size);

	return ptr;
}

char *Z_Strdup (const char *s)
{
	size_t sz = strlen(s) + 1;
	char *ptr = (char *) Z_Malloc (sz, Z_MAINZONE);
	memcpy (ptr, s, sz);
	return ptr;
}


/*============================================================================*/

#define HUNKNAME_LEN	24
typedef struct
{
	int		sentinal;
	int		size;		/* including sizeof(hunk_t), -1 = not allocated */
	char		name[HUNKNAME_LEN];
} hunk_t;

static byte	*hunk_base;
static int	hunk_size;

static int	hunk_low_used;
static int	hunk_high_used;

static qboolean	hunk_tempactive;
static int	hunk_tempmark;

/*
==============
Hunk_Check

Run consistancy and sentinal trashing checks
==============
*/
void Hunk_Check (void)
{
	hunk_t	*h;

	for (h = (hunk_t *)hunk_base ; (byte *)h != hunk_base + hunk_low_used ; )
	{
		if (h->sentinal != HUNK_SENTINAL)
			Sys_Error ("%s: trashed sentinal", __thisfunc__);
		if (h->size < (int) sizeof(hunk_t) || h->size + (byte *)h - hunk_base > hunk_size)
			Sys_Error ("%s: bad size", __thisfunc__);
		h = (hunk_t *)((byte *)h + h->size);
	}
}

/*
===================
Hunk_AllocName
===================
*/
void *Hunk_AllocName (int size, const char *name)
{
	hunk_t	*h;

#ifdef PARANOID
	Hunk_Check ();
#endif

	if (size < 0)
		Sys_Error ("%s: bad size: %i for %s", __thisfunc__, size, name);

	size = sizeof(hunk_t) + ((size + 15) & ~15);

	if (hunk_size - hunk_low_used - hunk_high_used < size)
		Sys_Error ("%s: failed on %i bytes for %s", __thisfunc__, size, name);

	h = (hunk_t *)(hunk_base + hunk_low_used);
	hunk_low_used += size;

	Cache_FreeLow (hunk_low_used);

	memset (h, 0, size);

	h->size = size;
	h->sentinal = HUNK_SENTINAL;
	q_strlcpy (h->name, name, HUNKNAME_LEN);

	return (void *)(h + 1);
}

/*
===================
Hunk_Alloc
===================
*/
void *Hunk_Alloc (int size)
{
	return Hunk_AllocName (size, "unknown");
}

int	Hunk_LowMark (void)
{
	return hunk_low_used;
}

void Hunk_FreeToLowMark (int mark)
{
	if (mark < 0 || mark > hunk_low_used)
		Sys_Error ("%s: bad mark %i", __thisfunc__, mark);
	memset (hunk_base + mark, 0, hunk_low_used - mark);
	hunk_low_used = mark;
}

int	Hunk_HighMark (void)
{
	if (hunk_tempactive)
	{
		hunk_tempactive = false;
		Hunk_FreeToHighMark (hunk_tempmark);
	}

	return hunk_high_used;
}

void Hunk_FreeToHighMark (int mark)
{
	if (hunk_tempactive)
	{
		hunk_tempactive = false;
		Hunk_FreeToHighMark (hunk_tempmark);
	}
	if (mark < 0 || mark > hunk_high_used)
		Sys_Error ("%s: bad mark %i", __thisfunc__, mark);
	memset (hunk_base + hunk_size - hunk_high_used, 0, hunk_high_used - mark);
	hunk_high_used = mark;
}


/*
===================
Hunk_HighAllocName
===================
*/
void *Hunk_HighAllocName (int size, const char *name)
{
	hunk_t	*h;

	if (size < 0)
		Sys_Error ("%s: bad size: %i", __thisfunc__, size);

	if (hunk_tempactive)
	{
		Hunk_FreeToHighMark (hunk_tempmark);
		hunk_tempactive = false;
	}

#ifdef PARANOID
	Hunk_Check ();
#endif

	size = sizeof(hunk_t) + ((size + 15) & ~15);

	if (hunk_size - hunk_low_used - hunk_high_used < size)
	{
		Con_Printf ("%s: failed on %i bytes\n", __thisfunc__, size);
		return NULL;
	}

	hunk_high_used += size;
	Cache_FreeHigh (hunk_high_used);

	h = (hunk_t *)(hunk_base + hunk_size - hunk_high_used);

	memset (h, 0, size);
	h->size = size;
	h->sentinal = HUNK_SENTINAL;
	q_strlcpy (h->name, name, HUNKNAME_LEN);

	return (void *)(h + 1);
}


/*
=================
Hunk_TempAlloc

Return space from the top of the hunk
=================
*/
void *Hunk_TempAlloc (int size)
{
	void	*buf;

	size = (size + 15) & ~15;

	if (hunk_tempactive)
	{
		Hunk_FreeToHighMark (hunk_tempmark);
		hunk_tempactive = false;
	}

	hunk_tempmark = Hunk_HighMark ();

	buf = Hunk_HighAllocName (size, "temp");

	hunk_tempactive = true;

	return buf;
}

char *Hunk_Strdup (const char *s, const char *name)
{
	size_t sz = strlen(s) + 1;
	char *ptr = (char *) Hunk_AllocName (sz, name);
	memcpy (ptr, s, sz);
	return ptr;
}

/*
===============================================================================

CACHE MEMORY

===============================================================================
*/
#if !defined(SERVERONLY)	/* CACHE not used in dedicated server apps */

#define CACHENAME_LEN	32
typedef struct cache_system_s
{
	int			size;		/* including this header */
	cache_user_t		*user;
	char			name[CACHENAME_LEN];
	struct cache_system_s	*prev, *next;
	struct cache_system_s	*lru_prev, *lru_next;	/* for LRU flushing */
} cache_system_t;

static cache_system_t *Cache_TryAlloc (int size, qboolean nobottom);

static cache_system_t	cache_head;

/*
===========
Cache_Move
===========
*/
static void Cache_Move ( cache_system_t *c)
{
	cache_system_t		*new_cs;

/* we are clearing up space at the bottom, so only allocate it late */
	new_cs = Cache_TryAlloc (c->size, true);
	if (new_cs)
	{
	/*	Con_Printf ("cache_move ok\n");*/
		memcpy (new_cs+1, c+1, c->size - sizeof(cache_system_t));
		new_cs->user = c->user;
		memcpy (new_cs->name, c->name, sizeof(new_cs->name));
		Cache_Free (c->user);
		new_cs->user->data = (void *)(new_cs + 1);
	}
	else
	{
	/*	Con_Printf ("cache_move failed\n");*/
		Cache_Free (c->user);	/* tough luck... */
	}
}

/*
============
Cache_FreeLow

Throw things out until the hunk can be expanded to the given point
============
*/
static void Cache_FreeLow (int new_low_hunk)
{
	cache_system_t	*c;

	while (1)
	{
		c = cache_head.next;
		if (c == &cache_head)
			return;		/* nothing in cache at all */
		if ((byte *)c >= hunk_base + new_low_hunk)
			return;		/* there is space to grow the hunk */
		Cache_Move ( c );	/* reclaim the space */
	}
}

/*
============
Cache_FreeHigh

Throw things out until the hunk can be expanded to the given point
============
*/
static void Cache_FreeHigh (int new_high_hunk)
{
	cache_system_t	*c, *prev;

	prev = NULL;
	while (1)
	{
		c = cache_head.prev;
		if (c == &cache_head)
			return;		/* nothing in cache at all */
		if ( (byte *)c + c->size <= hunk_base + hunk_size - new_high_hunk)
			return;		/* there is space to grow the hunk */
		if (c == prev)
			Cache_Free (c->user);	/* didn't move out of the way */
		else
		{
			Cache_Move (c);	/* try to move it */
			prev = c;
		}
	}
}

static void Cache_UnlinkLRU (cache_system_t *cs)
{
	if (!cs->lru_next || !cs->lru_prev)
		Sys_Error ("%s: NULL link", __thisfunc__);

	cs->lru_next->lru_prev = cs->lru_prev;
	cs->lru_prev->lru_next = cs->lru_next;

	cs->lru_prev = cs->lru_next = NULL;
}

static void Cache_MakeLRU (cache_system_t *cs)
{
	if (cs->lru_next || cs->lru_prev)
		Sys_Error ("%s: active link", __thisfunc__);

	cache_head.lru_next->lru_prev = cs;
	cs->lru_next = cache_head.lru_next;
	cs->lru_prev = &cache_head;
	cache_head.lru_next = cs;
}

/*
============
Cache_TryAlloc

Looks for a free block of memory between the high and low hunk marks
Size should already include the header and padding
============
*/
static cache_system_t *Cache_TryAlloc (int size, qboolean nobottom)
{
	cache_system_t	*cs, *new_cs;

/* is the cache completely empty? */
	if (!nobottom && cache_head.prev == &cache_head)
	{
		if (hunk_size - hunk_high_used - hunk_low_used < size)
			Sys_Error ("%s: out of hunk memory (failed to allocate %i bytes)", __thisfunc__, size);

		new_cs = (cache_system_t *) (hunk_base + hunk_low_used);
		memset (new_cs, 0, sizeof(*new_cs));
		new_cs->size = size;

		cache_head.prev = cache_head.next = new_cs;
		new_cs->prev = new_cs->next = &cache_head;

		Cache_MakeLRU (new_cs);
		return new_cs;
	}

/* search from the bottom up for space */
	new_cs = (cache_system_t *) (hunk_base + hunk_low_used);
	cs = cache_head.next;

	do
	{
		if (!nobottom || cs != cache_head.next)
		{
			if ((byte *)cs - (byte *)new_cs >= size)
			{	/* found space */
				memset (new_cs, 0, sizeof(*new_cs));
				new_cs->size = size;

				new_cs->next = cs;
				new_cs->prev = cs->prev;
				cs->prev->next = new_cs;
				cs->prev = new_cs;

				Cache_MakeLRU (new_cs);

				return new_cs;
			}
		}

	/* continue looking */
		new_cs = (cache_system_t *)((byte *)cs + cs->size);
		cs = cs->next;

	} while (cs != &cache_head);

/* try to allocate one at the very end */
	if (hunk_base + hunk_size - hunk_high_used - (byte *)new_cs >= size)
	{
		memset (new_cs, 0, sizeof(*new_cs));
		new_cs->size = size;

		new_cs->next = &cache_head;
		new_cs->prev = cache_head.prev;
		cache_head.prev->next = new_cs;
		cache_head.prev = new_cs;

		Cache_MakeLRU (new_cs);

		return new_cs;
	}

	return NULL;	/* couldn't allocate */
}

/*
============
Cache_Flush

Throw everything out, so new data will be demand cached
============
*/
void Cache_Flush (void)
{
	while (cache_head.next != &cache_head)
		Cache_Free ( cache_head.next->user );	/* reclaim the space */
}


/*
============
Cache_Report

============
*/
void Cache_Report (void)
{
	Con_DPrintf ("%4.1f megabyte data cache\n", (hunk_size - hunk_high_used - hunk_low_used) / (float)(1024*1024) );
}

/*
============
Cache_Init

============
*/
static void Cache_Init (void)
{
	cache_head.next = cache_head.prev = &cache_head;
	cache_head.lru_next = cache_head.lru_prev = &cache_head;
}

/*
==============
Cache_Free

Frees the memory and removes it from the LRU list
==============
*/
void Cache_Free (cache_user_t *c)
{
	cache_system_t	*cs;

	if (!c->data)
		Sys_Error ("%s: not allocated", __thisfunc__);

	cs = ((cache_system_t *)c->data) - 1;

	cs->prev->next = cs->next;
	cs->next->prev = cs->prev;
	cs->next = cs->prev = NULL;

	c->data = NULL;

	Cache_UnlinkLRU (cs);
}


/*
==============
Cache_Check
==============
*/
void *Cache_Check (cache_user_t *c)
{
	cache_system_t	*cs;

	if (!c->data)
		return NULL;

	cs = ((cache_system_t *)c->data) - 1;

/* move to head of LRU */
	Cache_UnlinkLRU (cs);
	Cache_MakeLRU (cs);

	return c->data;
}


/*
==============
Cache_Alloc
==============
*/
void *Cache_Alloc (cache_user_t *c, int size, const char *name)
{
	cache_system_t	*cs;

	if (c->data)
		Sys_Error ("%s: %s is already allocated", __thisfunc__, name);

	if (size <= 0)
		Sys_Error ("%s: bad size %i for %s", __thisfunc__, size, name);

	size = (size + sizeof(cache_system_t) + 15) & ~15;

/* find memory for it */
	while (1)
	{
		cs = Cache_TryAlloc (size, false);
		if (cs)
		{
			q_strlcpy (cs->name, name, CACHENAME_LEN);
			c->data = (void *)(cs + 1);
			cs->user = c;
			break;
		}

	/* free the least recently used cahedat */
		if (cache_head.lru_prev == &cache_head)	/* not enough memory at all */
			Sys_Error ("%s: out of memory", __thisfunc__);
		Cache_Free ( cache_head.lru_prev->user );
	}

	return Cache_Check (c);
}
#endif	/* ! SERVERONLY */


/*
==============================================================================

CONSOLE COMMANDS

==============================================================================
*/

#if Z_DEBUG_COMMANDS

#if defined(__GNUC__) && \
  !(defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L)
#define MEM_Printf(FH, fmt, args...)		\
    do {					\
	if ((FH)) fprintf((FH), fmt, ##args);	\
	else Con_Printf(fmt, ##args);		\
    } while (0)
#else
#define MEM_Printf(FH, ...)			\
    do {					\
	if ((FH)) fprintf((FH), __VA_ARGS__);	\
	else Con_Printf(__VA_ARGS__);		\
    } while (0)
#endif

/*
==============
Hunk_Print

If "all" is specified, every single allocation is printed.
Otherwise, allocations with the same name will be totaled up before printing.
==============
*/
static void Hunk_Print (qboolean all, qboolean write_file)
{
	hunk_t	*h, *next, *endlow, *starthigh, *endhigh;
	int		count, sum;
	int		totalblocks;
	FILE	*FH;

	count = 0;
	sum = 0;
	totalblocks = 0;

	FH = NULL;
	if (write_file)
		FH = fopen(FS_MakePath(FS_USERDIR,NULL,"memory.txt"), "w");

	h = (hunk_t *)hunk_base;
	endlow = (hunk_t *)(hunk_base + hunk_low_used);
	starthigh = (hunk_t *)(hunk_base + hunk_size - hunk_high_used);
	endhigh = (hunk_t *)(hunk_base + hunk_size);

	MEM_Printf(FH,"          :%8i total hunk size\n", hunk_size);
	MEM_Printf(FH,"-------------------------\n");

	while (1)
	{
	/* skip to the high hunk if done with low hunk */
		if (h == endlow)
		{
			MEM_Printf(FH,"-------------------------\n");
			MEM_Printf(FH,"          :%8i REMAINING\n", hunk_size - hunk_low_used - hunk_high_used);
			MEM_Printf(FH,"-------------------------\n");
			h = starthigh;
		}

	/* if totally done, break */
		if (h == endhigh)
			break;

	/* run consistancy checks */
		if (h->sentinal != HUNK_SENTINAL)
		{
			Sys_Error ("%s: trashed sentinal", __thisfunc__);
		}
		if (h->size < (int)sizeof(hunk_t) ||
		    h->size + (byte *)h - hunk_base > hunk_size)
		{
			Sys_Error ("%s: bad size", __thisfunc__);
		}

		next = (hunk_t *)((byte *)h + h->size);
		count++;
		totalblocks++;
		sum += h->size;

	/* print the single block */
		if (all)
		{
			MEM_Printf(FH,"%8p :%8i %8s\n",h, h->size, h->name);
		}

	/* print the total */
		if (next == endlow || next == endhigh || 
		    strncmp (h->name, next->name, HUNKNAME_LEN - 1))
		{
			if (!all)
			{
				MEM_Printf(FH,"          :%8i %8s (TOTAL)\n",sum, h->name);
			}
			count = 0;
			sum = 0;
		}

		h = next;
	}

	MEM_Printf(FH,"-------------------------\n");
	MEM_Printf(FH,"%8i total blocks\n", totalblocks);
	if (FH)
	{
		fclose(FH);
		Con_Printf ("Wrote to memory.txt\n");
	}
}

static void Memory_Display_f(void)
{
	int		num_args, counter;
	qboolean	all, write_file;

	all = true;
	write_file = false;

	num_args = Cmd_Argc();
	for (counter = 1; counter < num_args; counter++)
	{
		if (q_strcasecmp(Cmd_Argv(counter),"short") == 0)
			all = false;
		else if (q_strcasecmp(Cmd_Argv(counter),"save") == 0)
			write_file = true;
	}

	Hunk_Print(all, write_file);
}

# if !defined(SERVERONLY)
static void Cache_Print (qboolean write_file)
{
	cache_system_t	*cd;
	FILE		*FH;
	int		count, sum;
	int num_mod, sum_mod;
	int num_wav, sum_wav;
	char temp[128];

	FH = NULL;
	if (write_file)
		FH = fopen(FS_MakePath(FS_USERDIR,NULL,"cache.txt"), "w");

	count = sum = 0;
	num_mod = sum_mod = 0;
	num_wav = sum_wav = 0;

	for (cd = cache_head.next ; cd != &cache_head ; cd = cd->next)
	{
		MEM_Printf(FH,"%8i : %s\n", cd->size, cd->name);

		count++;
		sum += cd->size;

		q_strlcpy (temp, cd->name, sizeof(temp));
		q_strlwr(temp);
		if (strstr(temp,".mdl"))
		{
			num_mod++;
			sum_mod += cd->size;
		}
		else if (strstr(temp,".wav"))
		{
			num_wav++;
			sum_wav += cd->size;
		}
	}

	MEM_Printf(FH,"--------   ------------------\n");
	MEM_Printf(FH,"%8i : Total of %i items\n",sum,count);
	MEM_Printf(FH,"%8i : Total .MDL of %i items\n",sum_mod,num_mod);
	MEM_Printf(FH,"%8i : Total .WAV of %i items\n",sum_wav,num_wav);
	if (FH)
	{
		fclose(FH);
		Con_Printf ("Wrote to cache.txt\n");
	}
}

static void Cache_Display_f(void)
{
	int		num_args, counter;
	qboolean	write_file;

	write_file = false;

	num_args = Cmd_Argc();
	for (counter = 1; counter < num_args; counter++)
	{
		if (q_strcasecmp(Cmd_Argv(counter),"save") == 0)
			write_file = true;
	}

	Cache_Print(write_file);
}
# endif	/* SERVERONLY */

/*
========================
Z_Print
========================
*/
static void Z_Print (memzone_t *zone, FILE *f)
{
	memblock_t	*block;

	MEM_Printf (f, "zone size: %i  location: %p\n", zone->size, zone);

	for (block = zone->blocklist.next ; ; block = block->next)
	{
		MEM_Printf (f, "block: %p    size: %7i    tag: %3i\n",
				block, block->size, block->tag);

		if (block->next == &zone->blocklist)
			break;	/* all blocks have been hit */
		if ( (byte *)block + block->size != (byte *)block->next)
		{
			MEM_Printf (f, "ERROR: block size does not touch the next block\n");
		}
		if ( block->next->prev != block)
		{
			MEM_Printf (f, "ERROR: next block doesn't have proper back link\n");
		}
		if (!block->tag && !block->next->tag)
		{
			MEM_Printf (f, "ERROR: two consecutive free blocks\n");
		}
	}
}

static void Zone_Display_f(void)
{
	FILE		*FH;
	zonelist_t	*z;

	if ((Cmd_Argc() == 2) &&
	    (q_strcasecmp(Cmd_Argv(1),"save") == 0))
	{
		FH = fopen(FS_MakePath(FS_USERDIR,NULL,"zone.txt"), "w");
	}
	else
	{
		FH = NULL;
	}

	z = zonelist;
	while (z != NULL)
	{
		MEM_Printf(FH,"-------------------------\n");
		MEM_Printf(FH," %s :\n", z->name);
		MEM_Printf(FH,"-------------------------\n");
		Z_Print (z->zone, FH);
		MEM_Printf(FH,"\n");
		z = z->next;
	}
	if (FH)
	{
		fclose (FH);
		Con_Printf ("Wrote to zone.txt\n");
	}
}

#define NUM_GROUPS 18
static const char *MemoryGroups[NUM_GROUPS+1] =
{
	"texture",
	"light",
	"vis",
	"entities",
	"vertexes",
	"submodels",
	"edges",
	"faces",
	"nodes",
	"leafs",
	"clipnodes",
	"hull0",
	"marksurfaces",
	"surfedges",
	"planes",
	"video",
	"progs.dat",
	"edicts",
	"misc"
};

static void Memory_Stats_f(void)
{
	hunk_t	*h, *next, *endlow, *starthigh, *endhigh;
	int	num_args, count, sum, counter;
	int	GroupCount[NUM_GROUPS+1], GroupSum[NUM_GROUPS+1];
	FILE	*FH;
	qboolean write_file;

	write_file = false;

	num_args = Cmd_Argc();
	for (counter = 1; counter < num_args; counter++)
	{
		if (q_strcasecmp(Cmd_Argv(counter),"save") == 0)
			write_file = true;
	}

	memset(GroupCount,0,sizeof(GroupCount));
	memset(GroupSum,0,sizeof(GroupSum));

	h = (hunk_t *)hunk_base;
	endlow = (hunk_t *)(hunk_base + hunk_low_used);
	starthigh = (hunk_t *)(hunk_base + hunk_size - hunk_high_used);
	endhigh = (hunk_t *)(hunk_base + hunk_size);

	while (1)
	{
		if (h == endlow)
		{
			h = starthigh;
		}
		/* if totally done, break */
		if (h == endhigh)
			break;

		next = (hunk_t *)((byte *)h + h->size);

		for (counter = 0; counter < NUM_GROUPS; counter++)
		{
			if (q_strcasecmp(h->name,MemoryGroups[counter]) == 0)
			{
				GroupCount[counter]++;
				GroupSum[counter] += h->size;
				break;
			}
		}
		if (counter >= NUM_GROUPS)
		{
			GroupCount[NUM_GROUPS]++;
			GroupSum[NUM_GROUPS] += h->size;
		}

		h = next;
	}

	count = 0;
	sum = 0;

	FH = NULL;
	if (write_file)
		FH = fopen(FS_MakePath(FS_USERDIR,NULL,"stats.txt"), "w");

	MEM_Printf(FH,"Group           Count Size\n");
	MEM_Printf(FH,"--------------- ----- --------\n");
	for (counter = 0; counter < NUM_GROUPS+1; counter++)
	{
		MEM_Printf(FH,"%-15s %-5i %i\n",MemoryGroups[counter],GroupCount[counter],GroupSum[counter]);
		count += GroupCount[counter];
		sum += GroupSum[counter];
	}
	MEM_Printf(FH,"--------------- ----- --------\n");
	MEM_Printf(FH,"%-15s %-5i %i\n","Total",count,sum);
	if (FH)
	{
		fclose(FH);
		Con_Printf ("Wrote to stats.txt\n");
	}
}
#endif	/* Z_DEBUG_COMMANDS */

/*============================================================================*/


/*
========================
Memory_Init
========================
*/
static void Memory_InitZone (const char *name, int id, int magic, int size)
{
	zonelist_t	*z;
	memblock_t	*block;

	z = (zonelist_t *) Hunk_AllocName (sizeof(zonelist_t), name);
	z->id = id;
	z->magic = magic;
	z->name = name;
	z->zone = (memzone_t *) Hunk_AllocName (size, name);

/* set the entire zone to one free block */
	z->zone->blocklist.next = z->zone->blocklist.prev = block =
		(memblock_t *)( (byte *)z->zone + sizeof(memzone_t) );
	z->zone->blocklist.tag = 1;	/* in use block */
	z->zone->blocklist.magic = 0;
	z->zone->blocklist.size = 0;
	z->zone->rover = block;

	block->prev = block->next = &z->zone->blocklist;
	block->tag = 0;			/* free block */
	block->magic = magic;
	block->size = size - sizeof(memzone_t);

/* add to linked list */
	z->next = zonelist;
	zonelist = z;
}

void Memory_Init (void *buf, int size)
{
	int p;
	int zonesize = ZONE_DEFSIZE;

	hunk_base = (byte *) buf;
	hunk_size = size;
	hunk_low_used = 0;
	hunk_high_used = 0;

#if !defined(SERVERONLY)
	Cache_Init ();
#endif	/* SERVERONLY */

	p = COM_CheckParm ("-zone");
	if (p && p < com_argc-1)
	{
		zonesize = atoi (com_argv[p+1]) * 1024;
		if (zonesize < ZONE_MINSIZE && !COM_CheckParm ("-forcemem"))
		{
			Sys_Printf ("Requested zone size (%d Kb) too little, using %d Kb.\n",
					zonesize / 1024, (ZONE_MINSIZE / 1024));
			Sys_Printf ("If you are sure, use the -forcemem switch.\n");
			zonesize = ZONE_MINSIZE;
		}
		else if (zonesize > ZONE_MAXSIZE && !COM_CheckParm ("-forcemem"))
		{
			Sys_Printf ("Requested zone size (%d Kb) too large, using %d Kb.\n",
					zonesize / 1024, (ZONE_MAXSIZE / 1024));
			Sys_Printf ("If you are sure, use the -forcemem switch.\n");
			zonesize = ZONE_MAXSIZE;
		}
	}
	Memory_InitZone (mainzone, Z_MAINZONE, ZMAGIC, zonesize);

#if (SECZONE_SIZE > 0)
	zonesize = 0;
	if (!isDedicated)
	{
		zonesize += MEM_STATIC_TEX;
		zonesize += MEM_CODEC_MEM;
	}
	if (zonesize > 0)
		Memory_InitZone (sec_zone, Z_SECZONE, ZMAGIC2, zonesize);
#endif	/* SECONDARY ZONE */

#if !defined(SERVERONLY)
	Cmd_AddCommand ("flush", Cache_Flush);
#endif	/* SERVERONLY */
#if Z_DEBUG_COMMANDS
	Cmd_AddCommand ("sys_memory", Memory_Display_f);
	Cmd_AddCommand ("sys_zone", Zone_Display_f);
	Cmd_AddCommand ("sys_stats", Memory_Stats_f);
#if !defined(SERVERONLY)
	Cmd_AddCommand ("sys_cache", Cache_Display_f);
#endif	/* SERVERONLY */
#endif	/* Z_DEBUG_COMMANDS */
}

