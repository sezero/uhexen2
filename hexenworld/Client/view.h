// view.h

extern	cvar_t		v_gamma;

#ifdef GLQUAKE
extern	float	v_blend[4];
#endif

#if 0
extern	byte		gammatable[256];	// palette is sent through this
extern	unsigned short	ramps[3][256];
#endif

void V_Init (void);
void V_RenderView (void);
void V_CalcBlend (void);
float V_CalcRoll (vec3_t angles, vec3_t velocity);
void V_UpdatePalette (void);

