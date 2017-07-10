#ifndef CLIB_CAMD_PROTOS_H
#define CLIB_CAMD_PROTOS_H

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifndef MIDI_CAMD_H
#include <midi/camd.h>
#endif

APTR LockCAMD(ULONG locknum);
void UnlockCAMD(APTR lock);

struct MidiNode *CreateMidiA(CONST struct TagItem *tags);
void DeleteMidi(struct MidiNode *mn);
BOOL SetMidiAttrsA(struct MidiNode *mn, CONST struct TagItem *tags);
ULONG GetMidiAttrsA(struct MidiNode *mn, CONST struct TagItem *tags);
struct MidiNode *NextMidi(struct MidiNode *mn);
struct MidiNode *FindMidi(CONST_STRPTR name);
void FlushMidi(struct MidiNode *mn);

struct MidiLink *AddMidiLinkA(struct MidiNode *mn, LONG, CONST struct TagItem *tags);
void RemoveMidiLink(struct MidiLink *ml);
BOOL SetMidiLinkAttrsA(struct MidiLink *ml, CONST struct TagItem *tags);
ULONG GetMidiLinkAttrsA(struct MidiLink *ml, CONST struct TagItem *tags);
struct MidiLink *NextClusterLink(struct MidiCluster *mc, struct MidiLink *ml, LONG type);
struct MidiLink *NextMidiLink(struct MidiNode *mn, struct MidiLink *ml, LONG type);
BOOL MidiLinkConnected(struct MidiLink *ml);

struct MidiCluster *NextCluster(struct MidiCluster *mc);
struct MidiCluster *FindCluster(CONST_STRPTR name);

void PutMidi(struct MidiLink *ml, LONG msgdata);
BOOL GetMidi(struct MidiNode *mn, MidiMsg *msg);
BOOL WaitMidi(struct MidiNode *mn, MidiMsg *msg);

void PutSysEx(struct MidiLink *mn, UBYTE *buffer);
ULONG GetSysEx(struct MidiNode *mn, UBYTE *buffer, ULONG length);
ULONG QuerySysEx(struct MidiNode *mn);
void SkipSysEx(struct MidiNode *mn);

UBYTE GetMidiErr(struct MidiNode *mn);
WORD MidiMsgType(MidiMsg *msg);
WORD MidiMsgLen(ULONG status);

void ParseMidi(struct MidiLink *ml, UBYTE *buffer, ULONG length);

struct MidiDeviceData *OpenMidiDevice(UBYTE *name);
void CloseMidiDevice(struct MidiDeviceData *mdd);

int RethinkCAMD(void);
void StartClusterNotify(struct ClusterNotifyNode *node);
void EndClusterNotify(struct ClusterNotifyNode *node);

#if !defined(USE_INLINE_STDARG)
struct MidiNode *CreateMidi(Tag, ...);
BOOL SetMidiAttrs(struct MidiNode *mn, Tag tag, ...);
ULONG GetMidiAttrs(struct MidiNode *mn, Tag tag, ...);
struct MidiLink *AddMidiLink(struct MidiNode *mn, LONG, Tag, ...);
BOOL SetMidiLinkAttrs(struct MidiLink *ml, Tag tag, ...);
ULONG GetMidiLinkAttrs(struct MidiLink *ml, Tag tag, ...);
#endif

#endif /* CLIB_CAMD_PROTOS_H */
