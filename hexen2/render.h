/*
	refresh.h
	public interface to refresh functions

	$Header: /home/ozzie/Download/0000/uhexen2/hexen2/render.h,v 1.3 2004-12-18 13:59:25 sezero Exp $
*/

#define	MAXCLIPPLANES	11

#define	TOP_RANGE		16			// soldier uniform colors
#define	BOTTOM_RANGE	96

//=============================================================================

typedef struct efrag_s
{
	struct mleaf_s		*leaf;
	struct efrag_s		*leafnext;
	struct entity_s		*entity;
	struct efrag_s		*entnext;
} efrag_t;


typedef struct entity_s
{
	qboolean				forcelink;		// model changed

	int						update_type;

	entity_state3_t			baseline;		// to fill in defaults in updates

	double					msgtime;		// time of last update
	vec3_t					msg_origins[2];	// last two updates (0 is newest)	
	vec3_t					origin;
	vec3_t					msg_angles[2];	// last two updates (0 is newest)
	vec3_t					angles;
	struct model_s			*model;			// NULL = no model
	struct efrag_s			*efrag;			// linked list of efrags
	int						frame;
	float					syncbase;		// for client-side animations
	byte					*colormap, *sourcecolormap;
	byte					colorshade;
	int						effects;		// light, particals, etc
	int						skinnum;		// for Alias models
	int						scale;			// for Alias models
	int						drawflags;		// for Alias models
	int						abslight;		// for Alias models
	int						visframe;		// last frame this entity was
											//  found in an active leaf
											
	int						dlightframe;	// dynamic lighting
	int						dlightbits;
	
// FIXME: could turn these into a union
	int						trivial_accept;
	struct mnode_s			*topnode;		// for bmodels, first world node
											//  that splits bmodel, or NULL if
											//  not split
} entity_t;

// !!! if this is changed, it must be changed in asm_draw.h too !!!
typedef struct
{
	vrect_t		vrect;				// subwindow in video for refresh
									// FIXME: not need vrect next field here?
	vrect_t		aliasvrect;			// scaled Alias version
	int			vrectright, vrectbottom;	// right & bottom screen coords
	int			aliasvrectright, aliasvrectbottom;	// scaled Alias versions
	float		vrectrightedge;			// rightmost right edge we care about,
										//  for use in edge list
	float		fvrectx, fvrecty;		// for floating-point compares
	float		fvrectx_adj, fvrecty_adj; // left and top edges, for clamping
	int			vrect_x_adj_shift20;	// (vrect.x + 0.5 - epsilon) << 20
	int			vrectright_adj_shift20;	// (vrectright + 0.5 - epsilon) << 20
	float		fvrectright_adj, fvrectbottom_adj;
										// right and bottom edges, for clamping
	float		fvrectright;			// rightmost edge, for Alias clamping
	float		fvrectbottom;			// bottommost edge, for Alias clamping
	float		horizontalFieldOfView;	// at Z = 1.0, this many X is visible 
										// 2.0 = 90 degrees
	float		xOrigin;			// should probably allways be 0.5
	float		yOrigin;			// between be around 0.3 to 0.5

	vec3_t		vieworg;
	vec3_t		viewangles;
	
	int			ambientlight;
} refdef_t;


//
// refresh
//
extern	int		reinit_surfcache;


extern	refdef_t	r_refdef;
extern vec3_t	r_origin, vpn, vright, vup;

extern	struct texture_s	*r_notexture_mip;


void R_Init (void);
void R_InitTextures (void);
void R_InitEfrags (void);
void R_RenderView (void);		// must set r_refdef first
void R_ViewChanged (vrect_t *pvrect, int lineadj, float aspect);
								// called whenever r_refdef or vid change
void R_InitSky (struct texture_s *mt);	// called at level load

void R_AddEfrags (entity_t *ent);
void R_RemoveEfrags (entity_t *ent);

void R_NewMap (void);


void R_ParseParticleEffect (void);
void R_ParseParticleEffect2 (void);
void R_ParseParticleEffect3 (void);
void R_ParseParticleEffect4 (void);
void R_RunParticleEffect (vec3_t org, vec3_t dir, int color, int count);
void R_RocketTrail (vec3_t start, vec3_t end, int type);
void R_SunStaffTrail(vec3_t source, vec3_t dest);

#ifdef QUAKE2
void R_DarkFieldParticles (entity_t *ent);
#endif
void R_RainEffect (vec3_t org,vec3_t e_size,int x_dir, int y_dir,int color,int count);
void R_SnowEffect (vec3_t org1,vec3_t org2,int flags,vec3_t alldir,int count);
void R_ColoredParticleExplosion (vec3_t org,int color,int radius,int counter);

void R_EntityParticles (entity_t *ent);
void R_BlobExplosion (vec3_t org);
void R_ParticleExplosion (vec3_t org);
void R_ParticleExplosion2 (vec3_t org, int colorStart, int colorLength);
void R_LavaSplash (vec3_t org);
void R_TeleportSplash (vec3_t org);

void R_PushDlights (void);


//
// surface cache related
//
extern	int		reinit_surfcache;	// if 1, surface cache is currently empty and
extern qboolean	r_cache_thrash;	// set if thrashing the surface cache

int	D_SurfaceCacheForRes (int width, int height);
void D_FlushCaches (void);
void D_DeleteSurfaceCache (void);
void D_InitCaches (void *buffer, int size);
void R_SetVrect (vrect_t *pvrect, vrect_t *pvrectin, int lineadj);

//
// these prototypes were missing, causing ugly warnings. O.S.
//
void R_AnimateLight(void);
void R_ClearParticles (void);
void R_DarkFieldParticles (entity_t *ent);
void R_DrawParticles (void);
void R_InitParticles (void);
int R_LightPoint (vec3_t p);
void R_StoreEfrags (efrag_t **ppefrag);


/*
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2004/12/12 14:14:42  sezero
 * style changes to our liking
 *
 * Revision 1.1.1.1  2004/11/28 00:07:03  sezero
 * Initial import of AoT 1.2.0 code
 *
 * 4     3/05/98 7:54p Jmonroe
 * fixed startRain, optimized particle struct
 * 
 * 15    8/29/97 2:49p Rjohnson
 * Network updates
 * 
 * 14    8/17/97 3:28p Rjohnson
 * Fix for color selection
 * 
 * 13    7/28/97 2:44p Rjohnson
 * Fix for tinting
 * 
 * 12    7/24/97 4:37p Rjohnson
 * Color Tinting for models
 * 
 * 11    5/23/97 12:23p Bgokey
 * 
 * 10    5/14/97 2:40p Bgokey
 * 
 * 9     5/13/97 4:29p Bgokey
 * 
 * 8     4/28/97 11:58a Mgummelt
 * 
 * 7     4/22/97 3:50p Rjohnson
 * Added some more particle commands to cut back on the networking
 * 
 * 6     4/15/97 9:02p Bgokey
 * 
 * 5     2/19/97 11:51a Rjohnson
 * Id Updates
 */
