/* dls_compat.h - Windows compat defs for libtimidity DLS code, based on MinGW headers.
 *
 * This file has no copyright assigned and is placed in the Public Domain.
 * It is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY. ALL WARRANTIES, EXPRESSED OR IMPLIED, ARE HEREBY DISCLAIMED.
 * This includes, but is not limited to, warranties of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.
 */
#ifndef DLS_COMPAT_H
#define DLS_COMPAT_H

#define FAR /* */

#define mmioFOURCC(A, B, C, D) \
  ( (uint32)(A) | ((uint32)(B) << 8) | ((uint32)(C) << 16) | ((uint32)(D) << 24) )

typedef struct _GUID {
  uint32 Data1;
  uint16 Data2;
  uint16 Data3;
  uint8  Data4[8];
} GUID;

/* no need supporting INITGUID */
#ifdef __cplusplus
#define DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) extern "C" const GUID name
#else
#define DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) extern     const GUID name
#endif

#pragma pack(push,1)
typedef struct _WAVEFORMAT {
  uint16 wFormatTag;
  uint16 nChannels;
  uint32 nSamplesPerSec;
  uint32 nAvgBytesPerSec;
  uint16 nBlockAlign;
} WAVEFORMAT;

typedef struct _PCMWAVEFORMAT {
  WAVEFORMAT wf;
  uint16 wBitsPerSample;
} PCMWAVEFORMAT;
#pragma pack(pop)

/* make sure that the two structures above are packed correctly: */
typedef int _chk_WAVEFORMAT[2*((int)sizeof(WAVEFORMAT) == 14) -1];
typedef int _chk_PCMWAVEFORMAT[2*((int)sizeof(PCMWAVEFORMAT) == 16) -1];

#endif /* DLS_COMPAT_H */
