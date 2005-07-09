#ifndef DEMOBUILD
#define	AOT_USERDIR	".hexen2"
#else
#define	AOT_USERDIR	".hexen2demo"
#endif

#define	DEST_H2		0
#define	DEST_HW		1

#define	RES_320		0
#define	RES_400		1
#define	RES_512		2
#define	RES_640		3
#define	RES_800		4
#define	RES_1024	5
#define	RES_1280	6
#define	RES_1600	7
#define	RES_MAX		7

#define	MAX_H2GAMES	3	// max.# of entries in the h2game_names table
#define	MAX_HWGAMES	6	// max.# of entries in the hwgame_names table
#if defined(__linux__)
#define	MAX_SOUND	4	// max.# of entries in the snddrv_names table
#else
#define	MAX_SOUND	3	// ... this time excluding alsa
#endif
#define	MAX_RATES	6	// max.# of entries in the snd_rates table

#define HEAP_MINSIZE	8192	// minimum heap memory size in KB
#define HEAP_DEFAULT	32768	// default heap memory size in KB
#define HEAP_MAXSIZE	98304	// maximum heap memory size in KB
#define ZONE_MINSIZE	128	// minimum zone memory size in KB
#define ZONE_DEFAULT	128	// default zone memory size in KB
#define ZONE_MAXSIZE	1024	// maximum zone memory size in KB
