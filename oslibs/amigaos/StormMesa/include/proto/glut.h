#ifndef PROTO_GLUT_H
#define PROTO_GLUT_H
#include <exec/libraries.h>
extern struct Library *glutBase;
#ifndef APIENTRY
#ifndef NOSAVEDS
#define APIENTRY __saveds
#else
#define APIENTRY
#endif
#endif
#include <inline/glut_protos.h>
#endif
