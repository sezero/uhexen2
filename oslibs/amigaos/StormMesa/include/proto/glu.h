#ifndef PROTO_GLU_H
#define PROTO_GLU_H
#include <exec/libraries.h>
extern struct Library *gluBase;
#ifndef APIENTRY
#ifndef NOSAVEDS
#define APIENTRY __saveds
#else
#define APIENTRY
#endif
#endif
#include <inline/glu_protos.h>
#endif
