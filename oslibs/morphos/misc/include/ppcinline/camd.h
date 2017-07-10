/* Automatically generated header! Do not edit! */

#ifndef _PPCINLINE_CAMD_H
#define _PPCINLINE_CAMD_H

#ifndef __PPCINLINE_MACROS_H
#include <ppcinline/macros.h>
#endif /* !__PPCINLINE_MACROS_H */

#ifndef CAMD_BASE_NAME
#define CAMD_BASE_NAME CamdBase
#endif /* !CAMD_BASE_NAME */

#define NextMidi(__p0) \
	LP1(66, struct MidiNode *, NextMidi, \
		struct MidiNode *, __p0, a0, \
		, CAMD_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define StartClusterNotify(__p0) \
	LP1NR(222, StartClusterNotify, \
		struct ClusterNotifyNode *, __p0, a0, \
		, CAMD_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define GetMidiLinkAttrsA(__p0, __p1) \
	LP2(102, ULONG , GetMidiLinkAttrsA, \
		struct MidiLink *, __p0, a0, \
		CONST struct TagItem *, __p1, a1, \
		, CAMD_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PutSysEx(__p0, __p1) \
	LP2NR(156, PutSysEx, \
		struct MidiLink *, __p0, a0, \
		UBYTE *, __p1, a1, \
		, CAMD_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define GetMidiAttrsA(__p0, __p1) \
	LP2(60, ULONG , GetMidiAttrsA, \
		struct MidiNode *, __p0, a0, \
		CONST struct TagItem *, __p1, a1, \
		, CAMD_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MidiLinkConnected(__p0) \
	LP1(120, BOOL , MidiLinkConnected, \
		struct MidiLink *, __p0, a0, \
		, CAMD_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define OpenMidiDevice(__p0) \
	LP1(204, struct MidiDeviceData *, OpenMidiDevice, \
		UBYTE *, __p0, a0, \
		, CAMD_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define DeleteMidi(__p0) \
	LP1NR(48, DeleteMidi, \
		struct MidiNode *, __p0, a0, \
		, CAMD_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MidiMsgType(__p0) \
	LP1(186, WORD , MidiMsgType, \
		MidiMsg *, __p0, a0, \
		, CAMD_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define GetSysEx(__p0, __p1, __p2) \
	LP3(162, ULONG , GetSysEx, \
		struct MidiNode *, __p0, a0, \
		UBYTE *, __p1, a1, \
		ULONG , __p2, d0, \
		, CAMD_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SetMidiLinkAttrsA(__p0, __p1) \
	LP2(96, BOOL , SetMidiLinkAttrsA, \
		struct MidiLink *, __p0, a0, \
		CONST struct TagItem *, __p1, a1, \
		, CAMD_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define FindMidi(__p0) \
	LP1(72, struct MidiNode *, FindMidi, \
		CONST_STRPTR , __p0, a1, \
		, CAMD_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define RemoveMidiLink(__p0) \
	LP1NR(90, RemoveMidiLink, \
		struct MidiLink *, __p0, a0, \
		, CAMD_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define AddMidiLinkA(__p0, __p1, __p2) \
	LP3(84, struct MidiLink *, AddMidiLinkA, \
		struct MidiNode *, __p0, a0, \
		LONG , __p1, d0, \
		CONST struct TagItem *, __p2, a1, \
		, CAMD_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define LockCAMD(__p0) \
	LP1(30, APTR , LockCAMD, \
		ULONG , __p0, d0, \
		, CAMD_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define NextClusterLink(__p0, __p1, __p2) \
	LP3(108, struct MidiLink *, NextClusterLink, \
		struct MidiCluster *, __p0, a0, \
		struct MidiLink *, __p1, a1, \
		LONG , __p2, d0, \
		, CAMD_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MidiMsgLen(__p0) \
	LP1(192, WORD , MidiMsgLen, \
		ULONG , __p0, d0, \
		, CAMD_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CreateMidiA(__p0) \
	LP1(42, struct MidiNode *, CreateMidiA, \
		CONST struct TagItem *, __p0, a0, \
		, CAMD_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SkipSysEx(__p0) \
	LP1NR(174, SkipSysEx, \
		struct MidiNode *, __p0, a0, \
		, CAMD_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ParseMidi(__p0, __p1, __p2) \
	LP3NR(198, ParseMidi, \
		struct MidiLink *, __p0, a0, \
		UBYTE *, __p1, a1, \
		ULONG , __p2, d0, \
		, CAMD_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SetMidiAttrsA(__p0, __p1) \
	LP2(54, BOOL , SetMidiAttrsA, \
		struct MidiNode *, __p0, a0, \
		CONST struct TagItem *, __p1, a1, \
		, CAMD_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define FlushMidi(__p0) \
	LP1NR(78, FlushMidi, \
		struct MidiNode *, __p0, a0, \
		, CAMD_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define GetMidiErr(__p0) \
	LP1(180, UBYTE , GetMidiErr, \
		struct MidiNode *, __p0, a0, \
		, CAMD_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define QuerySysEx(__p0) \
	LP1(168, ULONG , QuerySysEx, \
		struct MidiNode *, __p0, a0, \
		, CAMD_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CloseMidiDevice(__p0) \
	LP1NR(210, CloseMidiDevice, \
		struct MidiDeviceData *, __p0, a0, \
		, CAMD_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define RethinkCAMD() \
	LP0(216, int , RethinkCAMD, \
		, CAMD_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define NextMidiLink(__p0, __p1, __p2) \
	LP3(114, struct MidiLink *, NextMidiLink, \
		struct MidiNode *, __p0, a0, \
		struct MidiLink *, __p1, a1, \
		LONG , __p2, d0, \
		, CAMD_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define FindCluster(__p0) \
	LP1(132, struct MidiCluster *, FindCluster, \
		CONST_STRPTR , __p0, a0, \
		, CAMD_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PutMidi(__p0, __p1) \
	LP2NR(138, PutMidi, \
		struct MidiLink *, __p0, a0, \
		LONG , __p1, d0, \
		, CAMD_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define GetMidi(__p0, __p1) \
	LP2(144, BOOL , GetMidi, \
		struct MidiNode *, __p0, a0, \
		MidiMsg *, __p1, a1, \
		, CAMD_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define UnlockCAMD(__p0) \
	LP1NR(36, UnlockCAMD, \
		APTR , __p0, a0, \
		, CAMD_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define WaitMidi(__p0, __p1) \
	LP2(150, BOOL , WaitMidi, \
		struct MidiNode *, __p0, a0, \
		MidiMsg *, __p1, a1, \
		, CAMD_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EndClusterNotify(__p0) \
	LP1NR(228, EndClusterNotify, \
		struct ClusterNotifyNode *, __p0, a0, \
		, CAMD_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define NextCluster(__p0) \
	LP1(126, struct MidiCluster *, NextCluster, \
		struct MidiCluster *, __p0, a0, \
		, CAMD_BASE_NAME, 0, 0, 0, 0, 0, 0)

#if defined(USE_INLINE_STDARG) && !defined(__STRICT_ANSI__)

#include <stdarg.h>

#define SetMidiLinkAttrs(__p0, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	SetMidiLinkAttrsA(__p0, (CONST struct TagItem *)_tags);})

#define GetMidiAttrs(__p0, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	GetMidiAttrsA(__p0, (CONST struct TagItem *)_tags);})

#define AddMidiLink(__p0, __p1, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	AddMidiLinkA(__p0, __p1, (CONST struct TagItem *)_tags);})

#define SetMidiAttrs(__p0, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	SetMidiAttrsA(__p0, (CONST struct TagItem *)_tags);})

#define CreateMidi(...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	CreateMidiA((CONST struct TagItem *)_tags);})

#define GetMidiLinkAttrs(__p0, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	GetMidiLinkAttrsA(__p0, (CONST struct TagItem *)_tags);})

#endif

#endif /* !_PPCINLINE_CAMD_H */
