#ifndef PROTO_AMIGAMESA_H
#define PROTO_AMIGAMESA_H
#include <exec/libraries.h>
extern struct Library *glBase;
#ifndef APIENTRY
#ifndef NOSAVEDS
#define APIENTRY __saveds
#else
#define APIENTRY
#endif
#endif
#include <inline/amigamesa_protos.h>
#endif
