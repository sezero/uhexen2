/*
 * entities.h
 * $Id: entities.h,v 1.9 2007-11-11 16:11:48 sezero Exp $
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
	char	message[1024];
	char	netname[1024];
	vec3_t	origin;
	float	angle;

	/* TYR - added fields */
	int	formula;
	float	atten;
	vec3_t	mangle;
	qboolean	use_mangle;
	int		lightcolor[3];

	int		light;
	int		style;
	char	target[32];
	char	targetname[32];
	struct epair_s	*epairs;
	struct entity_s	*targetent;
} entity_t;

/* Explanation of values added to struct entity_s
 *
 * formula:
 *    takes a value 0-3 (default 0)
 *    0 - Standard lighting formula like original light
 *    1 - light fades as 1/x
 *    2 - light fades as 1/(x^2)
 *    3 - Light stays same brightness reguardless of distance
 *
 * atten:
 *    Takes a float as a value (default 1.0).
 *    This reflects how fast a light fades with distance.
 *    For example a value of 2 will fade twice as fast, and a value of 0.5
 *    will fade half as fast. (Just like arghlite)
 *
 * mangle:
 *    If the entity is a light, then point the spotlight in this direction.
 *    If it is the worldspawn, then this is the sunlight mangle
 *
 * lightcolor:
 *    Stores the RGB values to determine the light color
 */

extern	entity_t	entities[MAX_MAP_ENTITIES];
extern	int			num_entities;

//extern int            numtexlights;
//extern entity_t       texlights[MAX_MAP_FACES];

void	SetKeyValue (entity_t *ent, const char *key, const char *value);
const char	*ValueForKey (entity_t *ent, const char *key);
float	FloatForKey (entity_t *ent, const char *key);
void	GetVectorForKey (entity_t *ent, const char *key, vec3_t vec);
entity_t *FindEntityWithKeyPair (const char *key, const char *value);

void	LoadEntities (void);
void	WriteEntitiesToString (void);

#endif	/* __ENTITIES_H__ */

