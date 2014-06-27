/*
 * entities.c
 * $Id: entities.c,v 1.19 2007-12-14 16:41:23 sezero Exp $
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
#include "compiler.h"
#include "arch_def.h"
#include "cmdlib.h"
#include "mathlib.h"
#include "bspfile.h"
#include "entities.h"
#include "tyrlite.h"


entity_t	entities[MAX_MAP_ENTITIES];
int			num_entities;
int			num_lights;
int			num_clights;


/*
==============================================================================

ENTITY FILE PARSING

If a light has a targetname, generate a unique style in the 32-63 range
==============================================================================
*/

static int		numlighttargets;
static char	lighttargets[32][64];

static int LightStyleForTargetname (char *targetname, qboolean alloc)
{
	int		i;

	for (i = 0 ; i < numlighttargets ; i++)
		if (!strcmp (lighttargets[i], targetname))
			return 32 + i;
	if (!alloc)
		return -1;
	strcpy (lighttargets[i], targetname);
	numlighttargets++;
	return numlighttargets-1 + 32;
}


/*
==================
MatchTargets
==================
*/
static void MatchTargets (void)
{
	int		i, j;

	for (i = 0 ; i < num_entities ; i++)
	{
		if (!entities[i].target[0])
			continue;

		for (j = 0 ; j < num_entities ; j++)
			if (!strcmp(entities[j].targetname, entities[i].target))
			{
				entities[i].targetent = &entities[j];
				break;
			}
		if (j == num_entities)
		{
			printf ("WARNING: entity at (%i,%i,%i) (%s) has unmatched target\n",
					(int)entities[i].origin[0], (int)entities[i].origin[1],
					(int)entities[i].origin[2], entities[i].classname);
			continue;
		}

	// set the style on the source ent for switchable lights
		if (entities[j].style)
		{
			char	s[16];

			entities[i].style = entities[j].style;
			sprintf (s,"%i", entities[i].style);
			SetKeyValue (&entities[i], "style", s);
		}
	}
}


/*
==================
LoadEntities
==================
*/
void LoadEntities (void)
{
	const char	*data;
	entity_t	*entity;
	char		key[64];
	epair_t		*epair;
	vec_t		v[3];
	int		i;

	data = dentdata;
//
// start parsing
//
	num_entities = 0;
	num_lights = 0;

// go through all the entities
	while (1)
	{
	// parse the opening brace
		data = COM_Parse (data);
		if (!data)
			break;
		if (com_token[0] != '{')
			COM_Error ("%s: found %s when expecting {", __thisfunc__, com_token);

		if (num_entities == MAX_MAP_ENTITIES)
			COM_Error ("%s: MAX_MAP_ENTITIES", __thisfunc__);
		entity = &entities[num_entities];
		num_entities++;
		entity->use_mangle = false;

	// go through all the keys in this entity
		while (1)
		{
			int		c;

		// parse key
			data = COM_Parse (data);
			if (!data)
				COM_Error ("%s: EOF without closing brace", __thisfunc__);
			if (!strcmp(com_token,"}"))
				break;
			strcpy (key, com_token);

		// parse value
			data = COM_Parse (data);
			if (!data)
				COM_Error ("%s: EOF without closing brace", __thisfunc__);
			c = com_token[0];
			if (c == '}')
				COM_Error ("%s: closing brace without data", __thisfunc__);

			epair = (epair_t *) SafeMalloc (sizeof(epair_t));
			strcpy (epair->key, key);
			strcpy (epair->value, com_token);
			epair->next = entity->epairs;
			entity->epairs = epair;

			if (!strcmp(key, "classname"))
				strcpy (entity->classname, com_token);
			else if (!strcmp (key, "message"))
				strcpy (entity->message, com_token);
			else if (!strcmp (key, "netname"))
				strcpy (entity->netname, com_token);
			else if (!strcmp(key, "target"))
				strcpy (entity->target, com_token);
			else if (!strcmp(key, "targetname"))
				strcpy (entity->targetname, com_token);
			else if (!strcmp(key, "origin"))
			{
				if (sscanf(com_token, "%lf %lf %lf",
						&entity->origin[0],
						&entity->origin[1],
						&entity->origin[2]) != 3)
					COM_Error ("%s: not 3 values for origin", __thisfunc__);
			}
			else if (!strncmp(key, "light", 5))
			{
				entity->light = atof(com_token);
			}
			else if (!strncmp (key, "_light", 6))
			{
				entity->light = atof(com_token);
			}
			else if (!strcmp(key, "style"))
			{
				entity->style = atoi(com_token);
				if ((unsigned int)entity->style > 254)
					COM_Error ("Bad light style %i (must be 0-254)", entity->style);
			}
			else if (!strcmp(key, "angle"))
				entity->angle = atof(com_token);
			else if (!strcmp(key, "wait"))
				entity->atten = atof(com_token);
			else if (!strcmp(key, "delay"))
				entity->formula = atoi(com_token);
			else if (!strcmp(key, "mangle"))
			{
				if (sscanf(com_token, "%lf %lf %lf", &v[0], &v[1], &v[2]) != 3)
					COM_Error ("%s: not 3 values for mangle", __thisfunc__);

				/* Precalculate the direction vector		*/
				entity->use_mangle = true;
				entity->mangle[0] = cos(v[0]*Q_PI/180)*cos(v[1]*Q_PI/180);
				entity->mangle[1] = sin(v[0]*Q_PI/180)*cos(v[1]*Q_PI/180);
				entity->mangle[2] = sin(v[1]*Q_PI/180);
			}
			else if (!strcmp(key, "_color")/* don't work with hipnotic particle fields || !strcmp(key, "color")*/)
			{
				if (sscanf(com_token, "%lf %lf %lf", &v[0], &v[1], &v[2]) != 3)
					COM_Error ("%s: not 3 values for color", __thisfunc__);
				for (i = 0 ; i < 3 ; i++)
					entity->lightcolor[i] = v[i];
			}
			else if (!strcmp(key, "_sunlight"))
				sunlight = atof(com_token);
			else if (!strcmp(key, "_sun_mangle"))
			{
				if (sscanf(com_token, "%lf %lf %lf", &v[0], &v[1], &v[2]) != 3)
					COM_Error ("%s: not 3 values for _sun_mangle", __thisfunc__);

				/* Precalculate sun vector and			*/
				/* make it too large to fit into the map	*/
				sunmangle[0] = cos(v[0]*Q_PI/180)*cos(v[1]*Q_PI/180);
				sunmangle[1] = sin(v[0]*Q_PI/180)*cos(v[1]*Q_PI/180);
				sunmangle[2] = sin(v[1]*Q_PI/180);
				VectorNormalize(sunmangle);
				VectorScale(sunmangle, -16384, sunmangle);
			}
			else if (!strcmp(key, "_sunlight_color"))
			{
				if (sscanf(com_token, "%lf %lf %lf",
						&sunlight_color[0],
						&sunlight_color[1],
						&sunlight_color[2]) != 3)
					COM_Error ("%s: not 3 values for _sunlight_color", __thisfunc__);
			}
			else if (!strcmp(key, "_minlight_color"))
			{
				if (sscanf(com_token, "%lf %lf %lf",
						&minlight_color[0],
						&minlight_color[1],
						&minlight_color[2]) != 3)
					COM_Error ("%s: not 3 values for _minlight_color", __thisfunc__);
			}
		}

	// all fields have been parsed

		if (!strncmp (entity->classname, "light", 5))
		{
			num_lights++;

			if (!entity->light)
				entity->light = DEFAULTLIGHTLEVEL;
			if (entity->atten <= 0.0)
				entity->atten = 1.0;
			if (entity->formula < 0 || entity->formula > 3)
				entity->formula = 0;

			// set some colors based on hexen2 entity classname
			if (!q_strncasecmp(entity->classname, "light_flame", 11) ||	/* _large_yellow, _small_yellow */
			    !q_strncasecmp(entity->classname, "light_torch", 11) ||	/* _castle, _rome, _meso, _egypt, _walltorch */
			    !q_strcasecmp (entity->classname, "light_gem"))
			{
				// make it orange
				entity->lightcolor[0] = 255;
				entity->lightcolor[1] = 128;
				entity->lightcolor[2] = 64;
				num_clights++;
			}
			else
			{
				// we don't need any color info to begin with
				entity->lightcolor[0] = 0;
				entity->lightcolor[1] = 0;
				entity->lightcolor[2] = 0;
			}

			if (entity->targetname[0] && !entity->style)
			{
				char	s[16];

				entity->style = LightStyleForTargetname (entity->targetname, true);
				sprintf (s,"%i", entity->style);
				SetKeyValue (entity, "style", s);
			}
		}

	// check for light value in worldspawn
		if (!strcmp (entity->classname, "worldspawn"))
		{
			// mapname according to the message field requires line
			// number entity->message to be read from strings.txt
			if (strlen(entity->netname))
				printf ("Map name : \"%s\"\n\n", entity->netname);
			else if (strlen(entity->message) && atoi(entity->message) > 0)
				printf ("Map name : at line %d in strings.txt\n\n", atoi(entity->message));
			else
				printf ("Map name : -- unknown --\n\n");

			if (entity->light > 0 && !worldminlight)
			{
				worldminlight = entity->light;
				printf("using minlight value %i from worldspawn.\n", worldminlight);
			}
			else if (worldminlight)
			{
				printf("Using minlight value %i from command line.\n", worldminlight);
			}

			worldminlight = 0;
				// FIXME: Why nullify the -light cmdline arg?? (was added in MHColour)
		}
	}

	printf ("%d entities read, %d are lights.\n", num_entities, num_lights);

	MatchTargets ();
}


const char *ValueForKey (entity_t *ent, const char *key)
{
	epair_t	*ep;

	for (ep = ent->epairs ; ep ; ep = ep->next)
	{
		if (!strcmp (ep->key, key) )
			return ep->value;
	}

	return "";
}

void SetKeyValue (entity_t *ent, const char *key, const char *value)
{
	epair_t	*ep;

	for (ep = ent->epairs ; ep ; ep = ep->next)
	{
		if (!strcmp (ep->key, key) )
		{
			strcpy (ep->value, value);
			return;
		}
	}

	ep = (epair_t *) SafeMalloc (sizeof(*ep));
	ep->next = ent->epairs;
	ent->epairs = ep;
	strcpy (ep->key, key);
	strcpy (ep->value, value);
}

float FloatForKey (entity_t *ent, const char *key)
{
	const char	*k;

	k = ValueForKey (ent, key);
	return (float)atof(k);
}

void GetVectorForKey (entity_t *ent, const char *key, vec3_t vec)
{
	const char	*k;

	k = ValueForKey (ent, key);
	sscanf (k, "%lf %lf %lf", &vec[0], &vec[1], &vec[2]);
}

entity_t *FindEntityWithKeyPair (const char *key, const char *value)
{
	epair_t	*ep;
	entity_t	*ent;
	int		i;

	for (i = 0 ; i < num_entities ; i++)
	{
		ent = &entities[i];
		for (ep = ent->epairs ; ep ; ep = ep->next)
		{
			if (!strcmp (ep->key, key))
			{
				if (!strcmp (ep->value, value))
					return ent;
				break;
			}
		}
	}
	return NULL;
}

