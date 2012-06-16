/*
 * r_part.h -- exported functions from r_part.c
 * $Id: r_part.h,v 1.4 2008-05-14 08:37:20 sezero Exp $
 *
 * Copyright (C) 1996-1997  Id Software, Inc.
 * Copyright (C) 1997-1998  Raven Software Corp.
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

#ifndef __R_PART_H
#define __R_PART_H

void R_DrawParticles (void);
void R_InitParticles (void);
void R_ClearParticles (void);

void R_ParseParticleEffect (void);
void R_ParseParticleEffect2 (void);
void R_ParseParticleEffect3 (void);
void R_ParseParticleEffect4 (void);

void R_RunParticleEffect (vec3_t org, vec3_t dir, int color, int count);
void R_RunParticleEffect2 (vec3_t org, vec3_t dmin, vec3_t dmax, int color, ptype_t effect, int count);
void R_RunParticleEffect3 (vec3_t org, vec3_t box, int color, ptype_t effect, int count);
void R_RunParticleEffect4 (vec3_t org, float radius, int color, ptype_t effect, int count);
					/* for ptype_t, d_iface.h or glquake.h must be included before. */

void R_ParticleExplosion (vec3_t org);
void R_ParticleExplosion2 (vec3_t org, int colorStart, int colorLength);
void R_ColoredParticleExplosion (vec3_t org, int color, int radius, int counter);
void R_BlobExplosion (vec3_t org);
void R_TargetBallEffect (vec3_t org);

void R_RocketTrail (vec3_t start, vec3_t end, int type);
void R_SunStaffTrail (vec3_t source, vec3_t dest);

void R_LavaSplash (vec3_t org);
void R_SplashParticleEffect (vec3_t org, float radius, int color, ptype_t effect, int count);
void R_TeleportSplash (vec3_t org);

void R_ParseRainEffect (void);
void R_RainEffect (vec3_t org, vec3_t e_size, int x_dir, int y_dir, int color, int count);
void R_RainEffect2(vec3_t org, vec3_t e_size, int x_dir, int y_dir, int color, int count);

void R_RunQuakeEffect (vec3_t org, float distance);

void RiderParticle (int count, vec3_t origin);
void R_DarkFieldParticles (entity_t *ent);
void R_BrightFieldSource (vec3_t org);
void R_SuccubusInvincibleParticles (entity_t *ent);

void R_EntityParticles (entity_t *ent);

/*
 * NOTES: R_EntityParticles, R_ParticleExplosion2
 *	and R_BlobExplosion actually are not used.
 */

#endif	/* __R_PART_H */

