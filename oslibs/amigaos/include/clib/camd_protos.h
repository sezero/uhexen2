#ifndef CLIB_CAMD_PROTOS_H
#define CLIB_CAMD_PROTOS_H
/************************************************************************
*     C. A. M. D.       (Commodore Amiga MIDI Driver)                   *
*************************************************************************
*                                                                       *
* Design & Development  - Roger B. Dannenberg                           *
*                       - Jean-Christophe Dhellemmes                    *
*                       - Bill Barton                                   *
*                       - Darius Taghavy                                *
*                       - Talin & Joe Pearce                            *
*                                                                       *
* Copyright 1990 by Commodore Business Machines                         *
*************************************************************************
*
* clib/camd_protos.h    - camd.library function prototypes
*
************************************************************************/

#ifndef EXEC_TYPES_H
  #include <exec/types.h>
#endif

#ifndef MIDI_CAMD_H
  #include <midi/camd.h>
#endif

    /* ---- Locks */
APTR LockCAMD (ULONG locknum);
void UnlockCAMD (APTR lock);

    /* ---- MidiNode */
struct MidiNode *CreateMidiA (struct TagItem *);
struct MidiNode *CreateMidi (Tag, ...);
void DeleteMidi (struct MidiNode *);
BOOL SetMidiAttrsA (struct MidiNode *, struct TagItem *);
BOOL SetMidiAttrs (struct MidiNode *, Tag tag, ...);
ULONG GetMidiAttrsA (struct MidiNode *, struct TagItem *);
ULONG GetMidiAttrs (struct MidiNode *, Tag tag, ...);
struct MidiNode *NextMidi (struct MidiNode *);
struct MidiNode *FindMidi (STRPTR);
void FlushMidi (struct MidiNode *);

    /* ---- MidiLink */
struct MidiLink *AddMidiLinkA (struct MidiNode *, LONG, struct TagItem *);
struct MidiLink *AddMidiLink (struct MidiNode *, LONG, Tag, ...);
void RemoveMidiLink (struct MidiLink *);
BOOL SetMidiLinkAttrsA (struct MidiLink *, struct TagItem *);
BOOL SetMidiLinkAttrs (struct MidiLink *, Tag tag, ...);
ULONG GetMidiLinkAttrsA (struct MidiLink *, struct TagItem *);
ULONG GetMidiLinkAttrs (struct MidiLink *, Tag tag, ...);
struct MidiLink *NextClusterLink (struct MidiCluster *, struct MidiLink *, LONG);
struct MidiLink *NextMidiLink (struct MidiNode *, struct MidiLink *, LONG);
BOOL MidiLinkConnected (struct MidiLink *);

    /* ---- MidiCluster */
struct MidiCluster *NextCluster(struct MidiCluster *);
struct MidiCluster *FindCluster(STRPTR);

    /* ---- Message */
void PutMidi (struct MidiLink *, LONG);
BOOL GetMidi (struct MidiNode *, MidiMsg *);
BOOL WaitMidi (struct MidiNode *, MidiMsg *);

void PutSysEx (struct MidiLink *, UBYTE *);
ULONG GetSysEx (struct MidiNode *, UBYTE *, ULONG);
ULONG QuerySysEx (struct MidiNode *);
void SkipSysEx (struct MidiNode *);

UBYTE GetMidiErr (struct MidiNode *);
WORD MidiMsgType (MidiMsg *);
WORD MidiMsgLen (ULONG StatusByte);

void ParseMidi (struct MidiLink *, UBYTE *Buffer, ULONG Length);

    /* ---- Devices */
struct MidiDeviceData *OpenMidiDevice (UBYTE *Name);
void CloseMidiDevice (struct MidiDeviceData *);

    /* ---- External Support */
int RethinkCAMD(void);
void StartClusterNotify(struct ClusterNotifyNode *);
void EndClusterNotify(struct ClusterNotifyNode *);

#endif /* CLIB_CAMD_PROTOS_H */

