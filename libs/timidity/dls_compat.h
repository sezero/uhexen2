#ifndef DLS_COMPAT_H
#define DLS_COMPAT_H
/* Some typedefs so the public dls headers don't need to be modified */

#define FAR /* */

typedef uint8  BYTE;
typedef sint16 SHORT;
typedef uint16 USHORT;
typedef uint16 WORD;
typedef sint32 LONG;
typedef uint32 ULONG;
typedef uint32 DWORD;

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
#define DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) extern "C" const GUID A
#else
#define DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) extern     const GUID A
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

#endif /* DLS_COMPAT_H */
