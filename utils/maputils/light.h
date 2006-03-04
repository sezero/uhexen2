// structure of the light lump

#include "cmdlib.h"
#include "mathlib.h"
#include "bspfile.h"
#include "entities.h"
#include "threads.h"

#define	ON_EPSILON	0.1

#define	MAXLIGHTS	1024

extern	float		scaledist;
extern	float		scalecos;
extern	float		rangescale;

//extern int		c_culldistplane, c_proper;
extern	byte		*filebase;
extern	vec3_t		bsp_origin;
extern	float		minlights[MAX_MAP_FACES];

extern	qboolean	extrasamples;

//void	TransformSample (vec3_t in, vec3_t out);
//void	RotateSample (vec3_t in, vec3_t out);
//void	LoadNodes (char *file);

byte	*GetFileSpace (int size);

qboolean TestLine (vec3_t start, vec3_t stop);
void	LightFace (int surfnum);
void	LightLeaf (dleaf_t *leaf);
void	MakeTnodes (dmodel_t *bm);

