
// refresh.h -- public interface to refresh functions

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
	int						keynum;			// for matching entities in different frames
	vec3_t					origin;
	vec3_t					angles;
	vec3_t					angleAdd;		// For clientside rotation stuff
	struct model_s			*model;			// NULL = no model
	int						frame;
	byte					*colormap, *sourcecolormap;
	byte					colorshade;
	int						skinnum;		// for Alias models
	int						scale;			// for Alias models
	int						drawflags;		// for Alias models
	int						abslight;		// for Alias models

	struct player_info_s	*scoreboard;	// identify player

	float					syncbase;

	struct efrag_s			*efrag;			// linked list of efrags (FIXME)
	int						visframe;		// last frame this entity was
											// found in an active leaf
											// only used for static objects
											
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
	float		xOrigin;			// should probably always be 0.5
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

extern	entity_t	r_worldentity;

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
void R_ParseRainEffect(void);
void R_DarkFieldParticles (entity_t *ent);
void R_DrawName(vec3_t origin, char *Name, int Red);
void R_ParticleExplosion (vec3_t org);
void R_ParticleExplosion2 (vec3_t org, int colorStart, int colorLength);
void R_BlobExplosion (vec3_t org);
void R_RunParticleEffect (vec3_t org, vec3_t dir, int color, int count);
void R_RunParticleEffect2 (vec3_t org, vec3_t dmin, vec3_t dmax, int color, int effect, int count);
void R_RunParticleEffect3 (vec3_t org, vec3_t box, int color, int effect, int count);
void R_RunParticleEffect4 (vec3_t org, float radius, int color, int effect, int count);
void R_SplashParticleEffect (vec3_t org, float radius, int color, int effect, int count);
void R_LavaSplash (vec3_t org);
void R_TeleportSplash (vec3_t org);
void R_RunQuakeEffect (vec3_t org, float distance);
void R_SunStaffTrail(vec3_t source, vec3_t dest);
void RiderParticle(int count, vec3_t origin);
void R_RocketTrail (vec3_t start, vec3_t end, int type);
void R_RainEffect (vec3_t org,vec3_t e_size,int x_dir, int y_dir,int color,int count);
void R_RainEffect2 (vec3_t org,vec3_t e_size,int x_dir, int y_dir,int color,int count);
void R_ColoredParticleExplosion (vec3_t org,int color,int radius,int counter);

void R_EntityParticles (entity_t *ent);
void R_SuccubusInvincibleParticles (entity_t *ent);
void R_BlobExplosion (vec3_t org);
void R_ParticleExplosion (vec3_t org);
void R_LavaSplash (vec3_t org);
void R_TeleportSplash (vec3_t org);
void R_TargetBallEffect (vec3_t org);
void R_BrightFieldSource (vec3_t org);

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

