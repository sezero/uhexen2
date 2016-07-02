#ifndef _VBCCINLINE_CAMD_H
#define _VBCCINLINE_CAMD_H

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

APTR __LockCAMD(__reg("a6") struct Library *, __reg("d0") ULONG locktype)="\tjsr\t-30(a6)";
#define LockCAMD(locktype) __LockCAMD(CamdBase, (locktype))

void __UnlockCAMD(__reg("a6") struct Library *, __reg("a0") APTR lock)="\tjsr\t-36(a6)";
#define UnlockCAMD(lock) __UnlockCAMD(CamdBase, (lock))

struct MidiNode * __CreateMidiA(__reg("a6") struct Library *, __reg("a0") struct TagItem * tags)="\tjsr\t-42(a6)";
#define CreateMidiA(tags) __CreateMidiA(CamdBase, (tags))

#if !defined(NO_INLINE_STDARG) && (__STDC__ == 1L) && (__STDC_VERSION__ >= 199901L)
struct MidiNode * __CreateMidi(__reg("a6") struct Library *, Tag tags, ...)="\tmove.l\ta0,-(a7)\n\tlea\t4(a7),a0\n\tjsr\t-42(a6)\n\tmovea.l\t(a7)+,a0";
#define CreateMidi(...) __CreateMidi(CamdBase, __VA_ARGS__)
#endif

void __DeleteMidi(__reg("a6") struct Library *, __reg("a0") struct MidiNode * mi)="\tjsr\t-48(a6)";
#define DeleteMidi(mi) __DeleteMidi(CamdBase, (mi))

BOOL __SetMidiAttrsA(__reg("a6") struct Library *, __reg("a0") struct MidiNode * mi, __reg("a1") struct TagItem * tags)="\tjsr\t-54(a6)";
#define SetMidiAttrsA(mi, tags) __SetMidiAttrsA(CamdBase, (mi), (tags))

#if !defined(NO_INLINE_STDARG) && (__STDC__ == 1L) && (__STDC_VERSION__ >= 199901L)
BOOL __SetMidiAttrs(__reg("a6") struct Library *, __reg("a0") struct MidiNode * mi, Tag tags, ...)="\tmove.l\ta1,-(a7)\n\tlea\t4(a7),a1\n\tjsr\t-54(a6)\n\tmovea.l\t(a7)+,a1";
#define SetMidiAttrs(mi, ...) __SetMidiAttrs(CamdBase, (mi), __VA_ARGS__)
#endif

ULONG __GetMidiAttrsA(__reg("a6") struct Library *, __reg("a0") struct MidiNode * mi, __reg("a1") struct TagItem * tags)="\tjsr\t-60(a6)";
#define GetMidiAttrsA(mi, tags) __GetMidiAttrsA(CamdBase, (mi), (tags))

#if !defined(NO_INLINE_STDARG) && (__STDC__ == 1L) && (__STDC_VERSION__ >= 199901L)
ULONG __GetMidiAttrs(__reg("a6") struct Library *, __reg("a0") struct MidiNode * mi, Tag tags, ...)="\tmove.l\ta1,-(a7)\n\tlea\t4(a7),a1\n\tjsr\t-60(a6)\n\tmovea.l\t(a7)+,a1";
#define GetMidiAttrs(mi, ...) __GetMidiAttrs(CamdBase, (mi), __VA_ARGS__)
#endif

struct MidiNode * __NextMidi(__reg("a6") struct Library *, __reg("a0") struct MidiNode * mi)="\tjsr\t-66(a6)";
#define NextMidi(mi) __NextMidi(CamdBase, (mi))

struct MidiNode * __FindMidi(__reg("a6") struct Library *, __reg("a1") STRPTR name)="\tjsr\t-72(a6)";
#define FindMidi(name) __FindMidi(CamdBase, (name))

void __FlushMidi(__reg("a6") struct Library *, __reg("a0") struct MidiNode * mi)="\tjsr\t-78(a6)";
#define FlushMidi(mi) __FlushMidi(CamdBase, (mi))

struct MidiLink * __AddMidiLinkA(__reg("a6") struct Library *, __reg("a0") struct MidiNode * mi, __reg("d0") LONG type, __reg("a1") struct TagItem * tags)="\tjsr\t-84(a6)";
#define AddMidiLinkA(mi, type, tags) __AddMidiLinkA(CamdBase, (mi), (type), (tags))

#if !defined(NO_INLINE_STDARG) && (__STDC__ == 1L) && (__STDC_VERSION__ >= 199901L)
struct MidiLink * __AddMidiLink(__reg("a6") struct Library *, __reg("a0") struct MidiNode * mi, __reg("d0") LONG type, Tag tags, ...)="\tmove.l\ta1,-(a7)\n\tlea\t4(a7),a1\n\tjsr\t-84(a6)\n\tmovea.l\t(a7)+,a1";
#define AddMidiLink(mi, type, ...) __AddMidiLink(CamdBase, (mi), (type), __VA_ARGS__)
#endif

void __RemoveMidiLink(__reg("a6") struct Library *, __reg("a0") struct MidiLink * ml)="\tjsr\t-90(a6)";
#define RemoveMidiLink(ml) __RemoveMidiLink(CamdBase, (ml))

BOOL __SetMidiLinkAttrsA(__reg("a6") struct Library *, __reg("a0") struct MidiLink * ml, __reg("a1") struct TagItem * tags)="\tjsr\t-96(a6)";
#define SetMidiLinkAttrsA(ml, tags) __SetMidiLinkAttrsA(CamdBase, (ml), (tags))

#if !defined(NO_INLINE_STDARG) && (__STDC__ == 1L) && (__STDC_VERSION__ >= 199901L)
BOOL __SetMidiLinkAttrs(__reg("a6") struct Library *, __reg("a0") struct MidiLink * ml, Tag tags, ...)="\tmove.l\ta1,-(a7)\n\tlea\t4(a7),a1\n\tjsr\t-96(a6)\n\tmovea.l\t(a7)+,a1";
#define SetMidiLinkAttrs(ml, ...) __SetMidiLinkAttrs(CamdBase, (ml), __VA_ARGS__)
#endif

ULONG __GetMidiLinkAttrsA(__reg("a6") struct Library *, __reg("a0") struct MidiLink * ml, __reg("a1") struct TagItem * tags)="\tjsr\t-102(a6)";
#define GetMidiLinkAttrsA(ml, tags) __GetMidiLinkAttrsA(CamdBase, (ml), (tags))

#if !defined(NO_INLINE_STDARG) && (__STDC__ == 1L) && (__STDC_VERSION__ >= 199901L)
ULONG __GetMidiLinkAttrs(__reg("a6") struct Library *, __reg("a0") struct MidiLink * ml, Tag tags, ...)="\tmove.l\ta1,-(a7)\n\tlea\t4(a7),a1\n\tjsr\t-102(a6)\n\tmovea.l\t(a7)+,a1";
#define GetMidiLinkAttrs(ml, ...) __GetMidiLinkAttrs(CamdBase, (ml), __VA_ARGS__)
#endif

struct MidiLink * __NextClusterLink(__reg("a6") struct Library *, __reg("a0") struct MidiCluster * mc, __reg("a1") struct MidiLink * ml, __reg("d0") LONG type)="\tjsr\t-108(a6)";
#define NextClusterLink(mc, ml, type) __NextClusterLink(CamdBase, (mc), (ml), (type))

struct MidiLink * __NextMidiLink(__reg("a6") struct Library *, __reg("a0") struct MidiNode * mi, __reg("a1") struct MidiLink * ml, __reg("d0") LONG type)="\tjsr\t-114(a6)";
#define NextMidiLink(mi, ml, type) __NextMidiLink(CamdBase, (mi), (ml), (type))

BOOL __MidiLinkConnected(__reg("a6") struct Library *, __reg("a0") struct MidiLink * ml)="\tjsr\t-120(a6)";
#define MidiLinkConnected(ml) __MidiLinkConnected(CamdBase, (ml))

struct MidiCluster * __NextCluster(__reg("a6") struct Library *, __reg("a0") struct MidiCluster * mc)="\tjsr\t-126(a6)";
#define NextCluster(mc) __NextCluster(CamdBase, (mc))

struct MidiCluster * __FindCluster(__reg("a6") struct Library *, __reg("a0") STRPTR name)="\tjsr\t-132(a6)";
#define FindCluster(name) __FindCluster(CamdBase, (name))

void __PutMidi(__reg("a6") struct Library *, __reg("a0") struct MidiLink * ml, __reg("d0") LONG msgdata)="\tjsr\t-138(a6)";
#define PutMidi(ml, msgdata) __PutMidi(CamdBase, (ml), (msgdata))

BOOL __GetMidi(__reg("a6") struct Library *, __reg("a0") struct MidiNode * mi, __reg("a1") MidiMsg * msg)="\tjsr\t-144(a6)";
#define GetMidi(mi, msg) __GetMidi(CamdBase, (mi), (msg))

BOOL __WaitMidi(__reg("a6") struct Library *, __reg("a0") struct MidiNode * mi, __reg("a1") MidiMsg * msg)="\tjsr\t-150(a6)";
#define WaitMidi(mi, msg) __WaitMidi(CamdBase, (mi), (msg))

void __PutSysEx(__reg("a6") struct Library *, __reg("a0") struct MidiLink * ml, __reg("a1") UBYTE * buffer)="\tjsr\t-156(a6)";
#define PutSysEx(ml, buffer) __PutSysEx(CamdBase, (ml), (buffer))

ULONG __GetSysEx(__reg("a6") struct Library *, __reg("a0") struct MidiNode * mi, __reg("a1") UBYTE * buffer, __reg("d0") ULONG len)="\tjsr\t-162(a6)";
#define GetSysEx(mi, buffer, len) __GetSysEx(CamdBase, (mi), (buffer), (len))

ULONG __QuerySysEx(__reg("a6") struct Library *, __reg("a0") struct MidiNode * mi)="\tjsr\t-168(a6)";
#define QuerySysEx(mi) __QuerySysEx(CamdBase, (mi))

void __SkipSysEx(__reg("a6") struct Library *, __reg("a0") struct MidiNode * mi)="\tjsr\t-174(a6)";
#define SkipSysEx(mi) __SkipSysEx(CamdBase, (mi))

UBYTE __GetMidiErr(__reg("a6") struct Library *, __reg("a0") struct MidiNode * mi)="\tjsr\t-180(a6)";
#define GetMidiErr(mi) __GetMidiErr(CamdBase, (mi))

WORD __MidiMsgType(__reg("a6") struct Library *, __reg("a0") MidiMsg * msg)="\tjsr\t-186(a6)";
#define MidiMsgType(msg) __MidiMsgType(CamdBase, (msg))

WORD __MidiMsgLen(__reg("a6") struct Library *, __reg("d0") ULONG status)="\tjsr\t-192(a6)";
#define MidiMsgLen(status) __MidiMsgLen(CamdBase, (status))

void __ParseMidi(__reg("a6") struct Library *, __reg("a0") struct MidiLink * ml, __reg("a1") UBYTE * buffer, __reg("d0") ULONG length)="\tjsr\t-198(a6)";
#define ParseMidi(ml, buffer, length) __ParseMidi(CamdBase, (ml), (buffer), (length))

struct MidiDeviceData * __OpenMidiDevice(__reg("a6") struct Library *, __reg("a0") UBYTE * name)="\tjsr\t-204(a6)";
#define OpenMidiDevice(name) __OpenMidiDevice(CamdBase, (name))

void __CloseMidiDevice(__reg("a6") struct Library *, __reg("a0") struct MidiDeviceData * mdd)="\tjsr\t-210(a6)";
#define CloseMidiDevice(mdd) __CloseMidiDevice(CamdBase, (mdd))

int __RethinkCAMD(__reg("a6") struct Library *)="\tjsr\t-216(a6)";
#define RethinkCAMD() __RethinkCAMD(CamdBase)

void __StartClusterNotify(__reg("a6") struct Library *, __reg("a0") struct ClusterNotifyNode * node)="\tjsr\t-222(a6)";
#define StartClusterNotify(node) __StartClusterNotify(CamdBase, (node))

void __EndClusterNotify(__reg("a6") struct Library *, __reg("a0") struct ClusterNotifyNode * node)="\tjsr\t-228(a6)";
#define EndClusterNotify(node) __EndClusterNotify(CamdBase, (node))

#endif /*  _VBCCINLINE_CAMD_H  */
