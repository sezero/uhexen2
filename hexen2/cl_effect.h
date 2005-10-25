#define MAX_EFFECTS 256

// Types for various chunks
#define THINGTYPE_GREYSTONE		 1
#define THINGTYPE_WOOD			 2
#define THINGTYPE_METAL			 3
#define THINGTYPE_FLESH			 4
#define THINGTYPE_FIRE			 5
#define THINGTYPE_CLAY			 6
#define THINGTYPE_LEAVES		 7
#define THINGTYPE_HAY			 8
#define THINGTYPE_BROWNSTONE		 9
#define THINGTYPE_CLOTH			10
#define THINGTYPE_WOOD_LEAF		11
#define THINGTYPE_WOOD_METAL		12
#define THINGTYPE_WOOD_STONE		13
#define THINGTYPE_METAL_STONE		14
#define THINGTYPE_METAL_CLOTH		15
#define THINGTYPE_WEBS			16
#define THINGTYPE_GLASS			17
#define THINGTYPE_ICE			18
#define THINGTYPE_CLEARGLASS		19
#define THINGTYPE_REDGLASS		20
#define THINGTYPE_ACID			21
#define THINGTYPE_METEOR		22
#define THINGTYPE_GREENFLESH		23
#define THINGTYPE_BONE			24


struct EffectT
{
	int type;
	float expire_time;

	union
	{
		struct
		{
			vec3_t e_size, dir, min_org, max_org;
			float next_time,wait;
			int color, count , veer , flags;
		} Rain;
		struct
		{
			vec3_t pos,angle,movedir;
			vec3_t vforward,vup,vright;
			int color,cnt;
		} Fountain;
		struct
		{
			vec3_t origin;
			float radius;
		} Quake;
		struct
		{
			vec3_t origin;
			vec3_t velocity;
			int entity_index;
			float time_amount,framelength,frame;
		} Smoke;
		struct
		{
			vec3_t origin;
			int entity_index;
			float time_amount;
			int reverse;  // Forward animation has been run, now go backwards
		} Flash;
		struct
		{
			vec3_t origin;
			int entity_index[13];
			float time_amount;
			int stage;
			int color;
			float lifetime;
		} RD; // Rider Death
		struct
		{
			int entity_index[16];
			vec3_t origin;
			vec3_t velocity[16];
			float time_amount,framelength;
			float skinnum;
		} Teleporter;
		struct
		{
			vec3_t angle;
			vec3_t origin;
			vec3_t avelocity;
			vec3_t velocity;
			int entity_index;
			float time_amount;
		} Missile;
		struct
		{
			int	entity_index[16];
			vec3_t	velocity[16];
			vec3_t	avel[3];
			vec3_t	origin;
			unsigned char type;
			vec3_t	srcVel;
			unsigned char numChunks;
			float	time_amount;
			float	aveScale;
		}Chunk;

	} ef;
};

void CL_InitEffects(void);
void CL_ClearEffects(void);
void CL_EndEffect(void);
void CL_ParseEffect(void);
void CL_UpdateEffects (void);
void SV_ParseEffect(sizebuf_t *sb);
void SV_UpdateEffects(sizebuf_t *sb);
void SV_SaveEffects(FILE *FH);
void SV_LoadEffects(FILE *FH);

