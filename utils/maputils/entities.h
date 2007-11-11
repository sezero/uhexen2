/*
	entities.h
	$Id: entities.h,v 1.5 2007-11-11 16:11:48 sezero Exp $
*/

#ifndef __ENTITIES_H__
#define __ENTITIES_H__

#define DEFAULTLIGHTLEVEL	300

typedef struct epair_s
{
	struct epair_s	*next;
	char	key[MAX_KEY];
	char	value[MAX_VALUE];
} epair_t;

typedef struct entity_s
{
	char	classname[64];
	vec3_t	origin;
	float	angle;
	int		light;
	int		style;
	char	target[32];
	char	targetname[32];
	struct epair_s	*epairs;
	struct entity_s	*targetent;
} entity_t;

extern	entity_t	entities[MAX_MAP_ENTITIES];
extern	int			num_entities;

void	SetKeyValue (entity_t *ent, const char *key, const char *value);
const char	*ValueForKey (entity_t *ent, const char *key);
float	FloatForKey (entity_t *ent, const char *key);
void	GetVectorForKey (entity_t *ent, const char *key, vec3_t vec);

void	LoadEntities (void);
void	WriteEntitiesToString (void);

#endif	/* __ENTITIES_H__ */

