#ifndef PROTO_GL_H
#define PROTO_GL_H
#include <exec/libraries.h>
extern struct Library *glBase;
#ifndef APIENTRY
#ifndef NOSAVEDS
#define APIENTRY __saveds
#else
#define APIENTRY
#endif
#endif
#include <inline/gl_protos.h>
#endif
