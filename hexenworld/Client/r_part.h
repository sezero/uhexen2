/*
	r_part.h
	exported functions from r_part.c

	$Header: /home/ozzie/Download/0000/uhexen2/hexenworld/Client/r_part.h,v 1.1 2007-07-08 12:00:10 sezero Exp $
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

#endif	/* __R_PART_H */

