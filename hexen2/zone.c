/*
	zone.c
	Memory management

	$Id: zone.c,v 1.16 2005-09-19 19:50:10 sezero Exp $
*/

#include "quakedef.h"

#define	DYNAMIC_SIZE	0x40000
#define	ZONEID		0x1d4a11
#define	HUNK_SENTINAL	0x1df001ed
#define MINFRAGMENT	64

typedef struct memblock_s
{
	int	size;		// including the header and possibly tiny fragments
	int	tag;		// a tag of 0 is a free block
	int	id;		// should be ZONEID
	struct	memblock_s	*next, *prev;
	int	pad;		// pad to 64 bit boundary
} memblock_t;

typedef struct
{
	int		size;		// total bytes malloced, including header
	memblock_t	blocklist;	// start / end cap for linked list
	memblock_t	*rover;
} memzone_t;

void Cache_FreeLow (int new_low_hunk);
void Cache_FreeHigh (int new_high_hunk);

#ifndef _WIN32
static void strlwr (char * str)
{
	while (*str) {*str = tolower (*str); str++; }
}
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

memzone_t	*mainzone;

void Z_ClearZone (memzone_t *zone, int size);


/*
========================
Z_ClearZone
========================
*/
void Z_ClearZone (memzone_t *zone, int size)
{
	memblock_t	*block;
	
// set the entire zone to one free block

	zone->blocklist.next = zone->blocklist.prev = block =
		(memblock_t *)( (byte *)zone + sizeof(memzone_t) );
	zone->blocklist.tag = 1;	// in use block
	zone->blocklist.id = 0;
	zone->blocklist.size = 0;
	zone->rover = block;
	
	block->prev = block->next = &zone->blocklist;
	block->tag = 0;			// free block
	block->id = ZONEID;
	block->size = size - sizeof(memzone_t);
}


/*
========================
Z_Free
========================
*/
void Z_Free (void *ptr)
{
	memblock_t	*block, *other;
	
	if (!ptr)
		Sys_Error ("Z_Free: NULL pointer");

	block = (memblock_t *) ( (byte *)ptr - sizeof(memblock_t));
	if (block->id != ZONEID)
		Sys_Error ("Z_Free: freed a pointer without ZONEID");
	if (block->tag == 0)
		Sys_Error ("Z_Free: freed a freed pointer");

	block->tag = 0;		// mark as free
	
	other = block->prev;
	if (!other->tag)
	{	// merge with previous free block
		other->size += block->size;
		other->next = block->next;
		other->next->prev = other;
		if (block == mainzone->rover)
			mainzone->rover = other;
		block = other;
	}
	
	other = block->next;
	if (!other->tag)
	{	// merge the next free block onto the end
		block->size += other->size;
		block->next = other->next;
		block->next->prev = block;
		if (other == mainzone->rover)
			mainzone->rover = block;
	}
}


/*
========================
Z_Malloc
========================
*/
void *Z_Malloc (int size)
{
	void	*buf;
	
//Z_CheckHeap ();	// DEBUG
	buf = Z_TagMalloc (size, 1);
	if (!buf)
		Sys_Error ("Z_Malloc: failed on allocation of %i bytes",size);
	memset (buf, 0, size);

	return buf;
}

void *Z_TagMalloc (int size, int tag)
{
	int		extra;
	memblock_t	*start, *rover, *new, *base;

	if (!tag)
		Sys_Error ("Z_TagMalloc: tried to use a 0 tag");

//
// scan through the block list looking for the first free block
// of sufficient size
//
	size += sizeof(memblock_t);	// account for size of block header
	size += 4;					// space for memory trash tester
	size = (size + 7) & ~7;		// align to 8-byte boundary
	
	base = rover = mainzone->rover;
	start = base->prev;
	
	do
	{
		if (rover == start)	// scaned all the way around the list
			return NULL;
		if (rover->tag)
			base = rover = rover->next;
		else
			rover = rover->next;
	} while (base->tag || base->size < size);
	
//
// found a block big enough
//
	extra = base->size - size;
	if (extra >  MINFRAGMENT)
	{	// there will be a free fragment after the allocated block
		new = (memblock_t *) ((byte *)base + size );
		new->size = extra;
		new->tag = 0;			// free block
		new->prev = base;
		new->id = ZONEID;
		new->next = base->next;
		new->next->prev = new;
		base->next = new;
		base->size = size;
	}
	
	base->tag = tag;				// no longer a free block
	
	mainzone->rover = base->next;	// next allocation will start looking here
	
	base->id = ZONEID;

// marker for memory trash testing
	*(int *)((byte *)base + base->size - 4) = ZONEID;

	return (void *) ((byte *)base + sizeof(memblock_t));
}


/*
========================
Z_Print
========================
*/
void Z_Print (memzone_t *zone)
{
	memblock_t	*block;
	
	Con_Printf ("zone size: %i  location: %p\n",mainzone->size,mainzone);
	
	for (block = zone->blocklist.next ; ; block = block->next)
	{
		Con_Printf ("block:%p    size:%7i    tag:%3i\n",
			block, block->size, block->tag);
		
		if (block->next == &zone->blocklist)
			break;			// all blocks have been hit	
		if ( (byte *)block + block->size != (byte *)block->next)
			Con_Printf ("ERROR: block size does not touch the next block\n");
		if ( block->next->prev != block)
			Con_Printf ("ERROR: next block doesn't have proper back link\n");
		if (!block->tag && !block->next->tag)
			Con_Printf ("ERROR: two consecutive free blocks\n");
	}
}


/*
========================
Z_CheckHeap
========================
*/
void Z_CheckHeap (void)
{
	memblock_t	*block;
	
	for (block = mainzone->blocklist.next ; ; block = block->next)
	{
		if (block->next == &mainzone->blocklist)
			break;			// all blocks have been hit	
		if ( (byte *)block + block->size != (byte *)block->next)
			Sys_Error ("Z_CheckHeap: block size does not touch the next block\n");
		if ( block->next->prev != block)
			Sys_Error ("Z_CheckHeap: next block doesn't have proper back link\n");
		if (!block->tag && !block->next->tag)
			Sys_Error ("Z_CheckHeap: two consecutive free blocks\n");
	}
}

//============================================================================

#define HUNKNAME_LEN	20
typedef struct
{
	int		sentinal;
	int		size;		// including sizeof(hunk_t), -1 = not allocated
	char		name[HUNKNAME_LEN];
} hunk_t;

byte	*hunk_base;
int		hunk_size;

int		hunk_low_used;
int		hunk_high_used;

qboolean	hunk_tempactive;
int		hunk_tempmark;

/*
==============
Hunk_Check

Run consistancy and sentinal trahing checks
==============
*/
void Hunk_Check (void)
{
	hunk_t	*h;
	
	for (h = (hunk_t *)hunk_base ; (byte *)h != hunk_base + hunk_low_used ; )
	{
		if (h->sentinal != HUNK_SENTINAL)
			Sys_Error ("Hunk_Check: trashed sentinal");
		if (h->size < 16 || h->size + (byte *)h - hunk_base > hunk_size)
			Sys_Error ("Hunk_Check: bad size");
		h = (hunk_t *)((byte *)h+h->size);
	}
}

/*
==============
Hunk_Print

If "all" is specified, every single allocation is printed.
Otherwise, allocations with the same name will be totaled up before printing.
==============
*/
#define MEM_Printf(FH, fmt, args...) {\
	Con_Printf(fmt, ##args);\
	if ((FH))\
		fprintf((FH), fmt, ##args);\
}
void Hunk_Print (qboolean all, qboolean write_file)
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
		FH = fopen(va("%s/memory.txt", com_userdir),"w");

	h = (hunk_t *)hunk_base;
	endlow = (hunk_t *)(hunk_base + hunk_low_used);
	starthigh = (hunk_t *)(hunk_base + hunk_size - hunk_high_used);
	endhigh = (hunk_t *)(hunk_base + hunk_size);

	MEM_Printf(FH,"          :%8i total hunk size\n", hunk_size);
	MEM_Printf(FH,"-------------------------\n");

	while (1)
	{
	//
	// skip to the high hunk if done with low hunk
	//
		if ( h == endlow )
		{
			MEM_Printf(FH,"-------------------------\n");
			MEM_Printf(FH,"          :%8i REMAINING\n", hunk_size - hunk_low_used - hunk_high_used);
			MEM_Printf(FH,"-------------------------\n");
			h = starthigh;
		}
		
	//
	// if totally done, break
	//
		if ( h == endhigh )
			break;

	//
	// run consistancy checks
	//
		if (h->sentinal != HUNK_SENTINAL)
			Sys_Error ("Hunk_Check: trashed sentinal");
		if (h->size < 16 || h->size + (byte *)h - hunk_base > hunk_size)
			Sys_Error ("Hunk_Check: bad size");

		next = (hunk_t *)((byte *)h+h->size);
		count++;
		totalblocks++;
		sum += h->size;

	//
	// print the single block
	//
		if (all)
			MEM_Printf(FH,"%8p :%8i %8s\n",h, h->size, h->name);

	//
	// print the total
	//
		if (next == endlow || next == endhigh || 
			strncmp (h->name, next->name, HUNKNAME_LEN))
		{
			if (!all)
				MEM_Printf(FH,"          :%8i %8s (TOTAL)\n",sum, h->name);
			count = 0;
			sum = 0;
		}

		h = next;
	}

	MEM_Printf(FH,"-------------------------\n");
	MEM_Printf(FH,"%8i total blocks\n", totalblocks);
	if (FH) {
		fclose(FH);
		FH = NULL;
	}
}

/*
===================
Hunk_AllocName
===================
*/
void *Hunk_AllocName (int size, char *name)
{
	hunk_t	*h;
	
#ifdef PARANOID
	Hunk_Check ();
#endif

	if (size < 0)
		Sys_Error ("Hunk_Alloc: bad size: %i", size);
		
	size = sizeof(hunk_t) + ((size+15)&~15);

	if (hunk_size - hunk_low_used - hunk_high_used < size)
		Sys_Error ("Hunk_Alloc: failed on %i bytes",size);
	
	h = (hunk_t *)(hunk_base + hunk_low_used);
	hunk_low_used += size;

	Cache_FreeLow (hunk_low_used);

	memset (h, 0, size);
	
	h->size = size;
	h->sentinal = HUNK_SENTINAL;
	strncpy (h->name, name, HUNKNAME_LEN - 1);
	h->name[HUNKNAME_LEN - 1] = 0;

	return (void *)(h+1);
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
		Sys_Error ("Hunk_FreeToLowMark: bad mark %i", mark);
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
		Sys_Error ("Hunk_FreeToHighMark: bad mark %i", mark);
	memset (hunk_base + hunk_size - hunk_high_used, 0, hunk_high_used - mark);
	hunk_high_used = mark;
}


/*
===================
Hunk_HighAllocName
===================
*/
void *Hunk_HighAllocName (int size, char *name)
{
	hunk_t	*h;

	if (size < 0)
		Sys_Error ("Hunk_HighAllocName: bad size: %i", size);

	if (hunk_tempactive)
	{
		Hunk_FreeToHighMark (hunk_tempmark);
		hunk_tempactive = false;
	}

#ifdef PARANOID
	Hunk_Check ();
#endif

	size = sizeof(hunk_t) + ((size+15)&~15);

	if (hunk_size - hunk_low_used - hunk_high_used < size)
	{
		Con_Printf ("Hunk_HighAlloc: failed on %i bytes\n",size);
		return NULL;
	}

	hunk_high_used += size;
	Cache_FreeHigh (hunk_high_used);

	h = (hunk_t *)(hunk_base + hunk_size - hunk_high_used);

	memset (h, 0, size);
	h->size = size;
	h->sentinal = HUNK_SENTINAL;
	strncpy (h->name, name, HUNKNAME_LEN - 1);
	h->name[HUNKNAME_LEN - 1] = 0;

	return (void *)(h+1);
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

	size = (size+15)&~15;
	
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

/*
===============================================================================

CACHE MEMORY

===============================================================================
*/
#define CACHENAME_LEN	32
typedef struct cache_system_s
{
	int				size;		// including this header
	cache_user_t		*user;
	char			name[CACHENAME_LEN];
	struct cache_system_s	*prev, *next;
	struct cache_system_s	*lru_prev, *lru_next;	// for LRU flushing	
} cache_system_t;

cache_system_t *Cache_TryAlloc (int size, qboolean nobottom);

cache_system_t	cache_head;

/*
===========
Cache_Move
===========
*/
void Cache_Move ( cache_system_t *c)
{
	cache_system_t		*new;

// we are clearing up space at the bottom, so only allocate it late
	new = Cache_TryAlloc (c->size, true);
	if (new)
	{
//		Con_Printf ("cache_move ok\n");

		memcpy ( new+1, c+1, c->size - sizeof(cache_system_t) );
		new->user = c->user;
		memcpy (new->name, c->name, sizeof(new->name));
		Cache_Free (c->user);
		new->user->data = (void *)(new+1);
	}
	else
	{
//		Con_Printf ("cache_move failed\n");

		Cache_Free (c->user);		// tough luck...
	}
}

/*
============
Cache_FreeLow

Throw things out until the hunk can be expanded to the given point
============
*/
void Cache_FreeLow (int new_low_hunk)
{
	cache_system_t	*c;
	
	while (1)
	{
		c = cache_head.next;
		if (c == &cache_head)
			return;		// nothing in cache at all
		if ((byte *)c >= hunk_base + new_low_hunk)
			return;		// there is space to grow the hunk
		Cache_Move ( c );	// reclaim the space
	}
}

/*
============
Cache_FreeHigh

Throw things out until the hunk can be expanded to the given point
============
*/
void Cache_FreeHigh (int new_high_hunk)
{
	cache_system_t	*c, *prev;
	
	prev = NULL;
	while (1)
	{
		c = cache_head.prev;
		if (c == &cache_head)
			return;		// nothing in cache at all
		if ( (byte *)c + c->size <= hunk_base + hunk_size - new_high_hunk)
			return;		// there is space to grow the hunk
		if (c == prev)
			Cache_Free (c->user);	// didn't move out of the way
		else
		{
			Cache_Move (c);	// try to move it
			prev = c;
		}
	}
}

void Cache_UnlinkLRU (cache_system_t *cs)
{
	if (!cs->lru_next || !cs->lru_prev)
		Sys_Error ("Cache_UnlinkLRU: NULL link");

	cs->lru_next->lru_prev = cs->lru_prev;
	cs->lru_prev->lru_next = cs->lru_next;
	
	cs->lru_prev = cs->lru_next = NULL;
}

void Cache_MakeLRU (cache_system_t *cs)
{
	if (cs->lru_next || cs->lru_prev)
		Sys_Error ("Cache_MakeLRU: active link");

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
cache_system_t *Cache_TryAlloc (int size, qboolean nobottom)
{
	cache_system_t	*cs, *new;
	
// is the cache completely empty?

	if (!nobottom && cache_head.prev == &cache_head)
	{
		if (hunk_size - hunk_high_used - hunk_low_used < size)
			Sys_Error ("Cache_TryAlloc: out of hunk memory (failed to allocate %i bytes)", size);

		new = (cache_system_t *) (hunk_base + hunk_low_used);
		memset (new, 0, sizeof(*new));
		new->size = size;

		cache_head.prev = cache_head.next = new;
		new->prev = new->next = &cache_head;
		
		Cache_MakeLRU (new);
		return new;
	}
	
// search from the bottom up for space

	new = (cache_system_t *) (hunk_base + hunk_low_used);
	cs = cache_head.next;
	
	do
	{
		if (!nobottom || cs != cache_head.next)
		{
			if ( (byte *)cs - (byte *)new >= size)
			{	// found space
				memset (new, 0, sizeof(*new));
				new->size = size;
				
				new->next = cs;
				new->prev = cs->prev;
				cs->prev->next = new;
				cs->prev = new;
				
				Cache_MakeLRU (new);
	
				return new;
			}
		}

	// continue looking		
		new = (cache_system_t *)((byte *)cs + cs->size);
		cs = cs->next;

	} while (cs != &cache_head);
	
// try to allocate one at the very end
	if ( hunk_base + hunk_size - hunk_high_used - (byte *)new >= size)
	{
		memset (new, 0, sizeof(*new));
		new->size = size;
		
		new->next = &cache_head;
		new->prev = cache_head.prev;
		cache_head.prev->next = new;
		cache_head.prev = new;
		
		Cache_MakeLRU (new);

		return new;
	}
	
	return NULL;		// couldn't allocate
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
		Cache_Free ( cache_head.next->user );	// reclaim the space
}


/*
============
Cache_Print

============
*/
void Cache_Print (qboolean write_file)
{
	cache_system_t	*cd;
	FILE		*FH;
	int		count, sum;
	int num_mod, sum_mod;
	int num_wav, sum_wav;
	char temp[128];

	FH = NULL;
	if (write_file)
		FH = fopen(va("%s/cache.txt", com_userdir),"w");

	count = sum = 0;
	num_mod = sum_mod = 0;
	num_wav = sum_wav = 0;

	for (cd = cache_head.next ; cd != &cache_head ; cd = cd->next)
	{
		MEM_Printf(FH,"%8i : %s\n", cd->size, cd->name);

		count++;
		sum += cd->size;

		strcpy(temp,cd->name);
		strlwr(temp);
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
	if (FH) {
		fclose(FH);
		FH = NULL;
	}
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
Cache_Compact

============
*/
void Cache_Compact (void)
{
}

/*
============
Cache_Init

============
*/
void Cache_Init (void)
{
	cache_head.next = cache_head.prev = &cache_head;
	cache_head.lru_next = cache_head.lru_prev = &cache_head;

	Cmd_AddCommand ("flush", Cache_Flush);
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
		Sys_Error ("Cache_Free: not allocated");

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

// move to head of LRU
	Cache_UnlinkLRU (cs);
	Cache_MakeLRU (cs);
	
	return c->data;
}


/*
==============
Cache_Alloc
==============
*/
void *Cache_Alloc (cache_user_t *c, int size, char *name)
{
	cache_system_t	*cs;

	if (c->data)
		Sys_Error ("Cache_Alloc: already allocated");
	
	if (size <= 0)
		Sys_Error ("Cache_Alloc: size %i", size);

	size = (size + sizeof(cache_system_t) + 15) & ~15;

// find memory for it	
	while (1)
	{
		cs = Cache_TryAlloc (size, false);
		if (cs)
		{
			strncpy (cs->name, name, CACHENAME_LEN - 1);
			cs->name[CACHENAME_LEN - 1] = 0;
			c->data = (void *)(cs+1);
			cs->user = c;
			break;
		}
	
	// free the least recently used cahedat
		if (cache_head.lru_prev == &cache_head)
			Sys_Error ("Cache_Alloc: out of memory");
													// not enough memory at all
		Cache_Free ( cache_head.lru_prev->user );
	} 
	
	return Cache_Check (c);
}

//============================================================================

void Memory_Display_f(void)
{
	short NumItems,counter;
	qboolean all, write_file;

	all = true;
	write_file = false;

	NumItems = Cmd_Argc();
	for(counter=1;counter<NumItems;counter++)
	{
		if (Q_strcasecmp(Cmd_Argv(counter),"short") == 0)
			all = false;
		else if (Q_strcasecmp(Cmd_Argv(counter),"save") == 0)
			write_file = true;
	}

	Hunk_Print(all,write_file);
}

void Cache_Display_f(void)
{
	short NumItems,counter;
	qboolean write_file;

	write_file = false;

	NumItems = Cmd_Argc();
	for(counter=1;counter<NumItems;counter++)
	{
		if (Q_strcasecmp(Cmd_Argv(counter),"save") == 0)
			write_file = true;
	}

	Cache_Print(write_file);
}

#define NUM_GROUPS 18
char *MemoryGroups[NUM_GROUPS+1] =
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

void Memory_Stats_f(void)
{
	hunk_t	*h, *next, *endlow, *starthigh, *endhigh;
	int	count, sum, counter;
	int	GroupCount[NUM_GROUPS+1], GroupSum[NUM_GROUPS+1];
	short	NumItems;
	FILE	*FH;
	qboolean write_file;

	write_file = false;

	NumItems = Cmd_Argc();
	for(counter=1;counter<NumItems;counter++)
	{
		if (Q_strcasecmp(Cmd_Argv(counter),"save") == 0)
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
		if ( h == endlow )
		{
			h = starthigh;
		}
	//
	// if totally done, break
	//
		if ( h == endhigh )
			break;

		next = (hunk_t *)((byte *)h+h->size);

		for(counter=0;counter<NUM_GROUPS;counter++)
		{
			if (Q_strcasecmp(h->name,MemoryGroups[counter]) == 0)
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
		FH = fopen(va("%s/stats.txt", com_userdir),"w");

	MEM_Printf(FH,"Group           Count Size\n");
	MEM_Printf(FH,"--------------- ----- --------\n");
	for(counter=0;counter<NUM_GROUPS+1;counter++)
	{
		MEM_Printf(FH,"%-15s %-5i %i\n",MemoryGroups[counter],GroupCount[counter],GroupSum[counter]);
		count += GroupCount[counter];
		sum += GroupSum[counter];
	}
	MEM_Printf(FH,"--------------- ----- --------\n");
	MEM_Printf(FH,"%-15s %-5i %i\n","Total",count,sum);
	if (FH) {
		fclose(FH);
		FH = NULL;
	}
}


/*
========================
Memory_Init
========================
*/
void Memory_Init (void *buf, int size)
{
	int p;
	int zonesize = DYNAMIC_SIZE;

	hunk_base = buf;
	hunk_size = size;
	hunk_low_used = 0;
	hunk_high_used = 0;
	
	Cache_Init ();
	p = COM_CheckParm ("-zone");
	if (p) {
		if (p < com_argc-1) {
			zonesize = atoi (com_argv[p+1]) * 1024;
			if (zonesize < DYNAMIC_SIZE) {
				// no less than 256 KB default
				Sys_Printf ("Requested zone size (%i Kb) too little.\n", zonesize/1024);
				Sys_Printf ("Going with the default 256 KB size.\n");
				zonesize = DYNAMIC_SIZE; // 256 Kb
			} else if (zonesize > 1024*1024) {
				// no bigger than 1 MB
				Sys_Printf ("Requested zone size (%i Kb) too large.\n", zonesize/1024);
				Sys_Printf ("Will try going with a 1 MB size.\n");
				zonesize = 1024*1024;	// 4*DYNAMIC_SIZE
			}
		} else {
			Sys_Printf ("Memory_Init: No size specified after -zone. Ignoring.\n");
		}
	}
	mainzone = Hunk_AllocName ( zonesize, "zone" );
	Z_ClearZone (mainzone, zonesize);

	Cmd_AddCommand ("sys_memory", Memory_Display_f);
	Cmd_AddCommand ("sys_cache", Cache_Display_f);
	Cmd_AddCommand ("sys_stats", Memory_Stats_f);
}

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.15  2005/07/23 22:23:48  sezero
 * increased hexen2 minimum zone size to 256 KB to match that
 * of hexenworld's in order to handle the new changes
 *
 * Revision 1.14  2005/07/09 11:53:40  sezero
 * moved the local unix version of strlwr to zone.c, its only user.
 *
 * Revision 1.13  2005/07/09 10:45:19  sezero
 * macroized the repetitive console+file printing routines for sys_*
 * console commands, fixed some string length oversights, performed
 * usual style clean-up.
 *
 * Revision 1.12  2005/07/09 07:26:27  sezero
 * increased hexen2 zone size to 128 kb
 *
 * Revision 1.11  2005/07/06 08:35:23  sezero
 * style/whitespace changes and removal of weird alien stuff
 *
 * Revision 1.10  2005/07/02 13:13:27  sezero
 * cleaned-up file saving in zone.c
 *
 * Revision 1.9  2005/06/15 06:16:26  sezero
 * strlwr extern in zone.c is for unix only
 *
 * Revision 1.8  2005/05/19 11:34:45  sezero
 * save cache.txt and stats.txt into com_userdir
 *
 * Revision 1.7  2005/05/17 22:56:19  sezero
 * cleanup the "stricmp, strcmpi, strnicmp, Q_strcasecmp, Q_strncasecmp" mess:
 * Q_strXcasecmp will now be used throughout the code which are implementation
 * dependant defines for __GNUC__ (strXcasecmp) and _WIN32 (strXicmp)
 *
 * Revision 1.6  2005/05/17 17:39:54  sezero
 * (Re-)added the parms.userdir to all sys_win.c. The platform conditionals
 * around some of the com_userdir code of late are now unnecessary.
 *
 * Revision 1.5  2005/04/15 20:25:11  sezero
 * save memory.txt into com_userdir
 *
 * Revision 1.4  2005/04/13 12:22:41  sezero
 * - Removed useless -minmemory cmdline argument
 * - Removed useless parms->memsize < minimum_memory check in Host_Init
 * - Added lower/upper boundaries (8mb/96mb) for -heapsize argument
 * - Added lower (48kb for hexen2, 256kb for hw)/upper (1mb) boundaries
 *   for -zone argument (DYNAMIC_SIZE definitions/zonesize requirements
 *   are different for hexen2 and hexenworld)
 * - We won't die if no size is specified after -zone, but will ignore
 * - Added null string terminations to hexen2 zone.c, so as to prevent
 *   garbage on sys_memory console command (found this in Pa3PyX)
 *
 * Revision 1.3  2004/12/18 13:59:25  sezero
 * Clean-up and kill warnings 8:
 * Missing prototypes.
 *
 * Revision 1.2  2004/12/12 14:14:43  sezero
 * style changes to our liking
 *
 * Revision 1.1.1.1  2004/11/28 00:08:32  sezero
 * Initial import of AoT 1.2.0 code
 *
 * 2     3/01/98 8:20p Jmonroe
 * removed the slow "quake" version of common functions
 * 
 * 3     8/31/97 3:45p Rjohnson
 * Memory changes
 * 
 * 2     3/06/97 10:29a Rjohnson
 * Added memory statistics
 */
