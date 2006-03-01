
//**************************************************************************
//**
//** loadtri.h
//**
//**************************************************************************

#define MAXTRIANGLES	2048

typedef struct
{
	vec3_t	verts[3];
} triangle_t;

void LoadTriangleList(char *fileName, triangle_t **triList, int *triangleCount);

extern char	InputFileName[1024];

extern float	FixHTRRotateX;
extern float	FixHTRRotateY;
extern float	FixHTRRotateZ;

extern float	FixHTRTranslateX;
extern float	FixHTRTranslateY;
extern float	FixHTRTranslateZ;
