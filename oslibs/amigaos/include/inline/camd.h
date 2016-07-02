/* Automatically generated header (sfdc 1.10)! Do not edit! */

#ifndef _INLINE_CAMD_H
#define _INLINE_CAMD_H

#ifndef _SFDC_VARARG_DEFINED
#define _SFDC_VARARG_DEFINED
#ifdef __HAVE_IPTR_ATTR__
typedef APTR _sfdc_vararg __attribute__((iptr));
#else
typedef ULONG _sfdc_vararg;
#endif /* __HAVE_IPTR_ATTR__ */
#endif /* _SFDC_VARARG_DEFINED */

#ifndef __INLINE_MACROS_H
#include <inline/macros.h>
#endif /* !__INLINE_MACROS_H */

#ifndef CAMD_BASE_NAME
#define CAMD_BASE_NAME CamdBase
#endif /* !CAMD_BASE_NAME */

#define LockCAMD(___locktype) \
      LP1(0x1e, APTR, LockCAMD , ULONG, ___locktype, d0,\
      , CAMD_BASE_NAME)

#define UnlockCAMD(___lock) \
      LP1NR(0x24, UnlockCAMD , APTR, ___lock, a0,\
      , CAMD_BASE_NAME)

#define CreateMidiA(___tags) \
      LP1(0x2a, struct MidiNode *, CreateMidiA , struct TagItem *, ___tags, a0,\
      , CAMD_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define CreateMidi(___tags, ...) \
    ({_sfdc_vararg _tags[] = { ___tags, __VA_ARGS__ }; CreateMidiA((struct TagItem *) _tags); })
#endif /* !NO_INLINE_STDARG */

#define DeleteMidi(___mi) \
      LP1NR(0x30, DeleteMidi , struct MidiNode *, ___mi, a0,\
      , CAMD_BASE_NAME)

#define SetMidiAttrsA(___mi, ___tags) \
      LP2(0x36, BOOL, SetMidiAttrsA , struct MidiNode *, ___mi, a0, struct TagItem *, ___tags, a1,\
      , CAMD_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define SetMidiAttrs(___mi, ___tags, ...) \
    ({_sfdc_vararg _tags[] = { ___tags, __VA_ARGS__ }; SetMidiAttrsA((___mi), (struct TagItem *) _tags); })
#endif /* !NO_INLINE_STDARG */

#define GetMidiAttrsA(___mi, ___tags) \
      LP2(0x3c, ULONG, GetMidiAttrsA , struct MidiNode *, ___mi, a0, struct TagItem *, ___tags, a1,\
      , CAMD_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define GetMidiAttrs(___mi, ___tags, ...) \
    ({_sfdc_vararg _tags[] = { ___tags, __VA_ARGS__ }; GetMidiAttrsA((___mi), (struct TagItem *) _tags); })
#endif /* !NO_INLINE_STDARG */

#define NextMidi(___mi) \
      LP1(0x42, struct MidiNode *, NextMidi , struct MidiNode *, ___mi, a0,\
      , CAMD_BASE_NAME)

#define FindMidi(___name) \
      LP1(0x48, struct MidiNode *, FindMidi , STRPTR, ___name, a1,\
      , CAMD_BASE_NAME)

#define FlushMidi(___mi) \
      LP1NR(0x4e, FlushMidi , struct MidiNode *, ___mi, a0,\
      , CAMD_BASE_NAME)

#define AddMidiLinkA(___mi, ___type, ___tags) \
      LP3(0x54, struct MidiLink *, AddMidiLinkA , struct MidiNode *, ___mi, a0, LONG, ___type, d0, struct TagItem *, ___tags, a1,\
      , CAMD_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define AddMidiLink(___mi, ___type, ___tags, ...) \
    ({_sfdc_vararg _tags[] = { ___tags, __VA_ARGS__ }; AddMidiLinkA((___mi), (___type), (struct TagItem *) _tags); })
#endif /* !NO_INLINE_STDARG */

#define RemoveMidiLink(___ml) \
      LP1NR(0x5a, RemoveMidiLink , struct MidiLink *, ___ml, a0,\
      , CAMD_BASE_NAME)

#define SetMidiLinkAttrsA(___ml, ___tags) \
      LP2(0x60, BOOL, SetMidiLinkAttrsA , struct MidiLink *, ___ml, a0, struct TagItem *, ___tags, a1,\
      , CAMD_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define SetMidiLinkAttrs(___ml, ___tags, ...) \
    ({_sfdc_vararg _tags[] = { ___tags, __VA_ARGS__ }; SetMidiLinkAttrsA((___ml), (struct TagItem *) _tags); })
#endif /* !NO_INLINE_STDARG */

#define GetMidiLinkAttrsA(___ml, ___tags) \
      LP2(0x66, ULONG, GetMidiLinkAttrsA , struct MidiLink *, ___ml, a0, struct TagItem *, ___tags, a1,\
      , CAMD_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define GetMidiLinkAttrs(___ml, ___tags, ...) \
    ({_sfdc_vararg _tags[] = { ___tags, __VA_ARGS__ }; GetMidiLinkAttrsA((___ml), (struct TagItem *) _tags); })
#endif /* !NO_INLINE_STDARG */

#define NextClusterLink(___mc, ___ml, ___type) \
      LP3(0x6c, struct MidiLink *, NextClusterLink , struct MidiCluster *, ___mc, a0, struct MidiLink *, ___ml, a1, LONG, ___type, d0,\
      , CAMD_BASE_NAME)

#define NextMidiLink(___mi, ___ml, ___type) \
      LP3(0x72, struct MidiLink *, NextMidiLink , struct MidiNode *, ___mi, a0, struct MidiLink *, ___ml, a1, LONG, ___type, d0,\
      , CAMD_BASE_NAME)

#define MidiLinkConnected(___ml) \
      LP1(0x78, BOOL, MidiLinkConnected , struct MidiLink *, ___ml, a0,\
      , CAMD_BASE_NAME)

#define NextCluster(___mc) \
      LP1(0x7e, struct MidiCluster *, NextCluster , struct MidiCluster *, ___mc, a0,\
      , CAMD_BASE_NAME)

#define FindCluster(___name) \
      LP1(0x84, struct MidiCluster *, FindCluster , STRPTR, ___name, a0,\
      , CAMD_BASE_NAME)

#define PutMidi(___ml, ___msgdata) \
      LP2NR(0x8a, PutMidi , struct MidiLink *, ___ml, a0, LONG, ___msgdata, d0,\
      , CAMD_BASE_NAME)

#define GetMidi(___mi, ___msg) \
      LP2(0x90, BOOL, GetMidi , struct MidiNode *, ___mi, a0, MidiMsg *, ___msg, a1,\
      , CAMD_BASE_NAME)

#define WaitMidi(___mi, ___msg) \
      LP2(0x96, BOOL, WaitMidi , struct MidiNode *, ___mi, a0, MidiMsg *, ___msg, a1,\
      , CAMD_BASE_NAME)

#define PutSysEx(___ml, ___buffer) \
      LP2NR(0x9c, PutSysEx , struct MidiLink *, ___ml, a0, UBYTE *, ___buffer, a1,\
      , CAMD_BASE_NAME)

#define GetSysEx(___mi, ___buffer, ___len) \
      LP3(0xa2, ULONG, GetSysEx , struct MidiNode *, ___mi, a0, UBYTE *, ___buffer, a1, ULONG, ___len, d0,\
      , CAMD_BASE_NAME)

#define QuerySysEx(___mi) \
      LP1(0xa8, ULONG, QuerySysEx , struct MidiNode *, ___mi, a0,\
      , CAMD_BASE_NAME)

#define SkipSysEx(___mi) \
      LP1NR(0xae, SkipSysEx , struct MidiNode *, ___mi, a0,\
      , CAMD_BASE_NAME)

#define GetMidiErr(___mi) \
      LP1(0xb4, UBYTE, GetMidiErr , struct MidiNode *, ___mi, a0,\
      , CAMD_BASE_NAME)

#define MidiMsgType(___msg) \
      LP1(0xba, WORD, MidiMsgType , MidiMsg *, ___msg, a0,\
      , CAMD_BASE_NAME)

#define MidiMsgLen(___status) \
      LP1(0xc0, WORD, MidiMsgLen , ULONG, ___status, d0,\
      , CAMD_BASE_NAME)

#define ParseMidi(___ml, ___buffer, ___length) \
      LP3NR(0xc6, ParseMidi , struct MidiLink *, ___ml, a0, UBYTE *, ___buffer, a1, ULONG, ___length, d0,\
      , CAMD_BASE_NAME)

#define OpenMidiDevice(___name) \
      LP1(0xcc, struct MidiDeviceData *, OpenMidiDevice , UBYTE *, ___name, a0,\
      , CAMD_BASE_NAME)

#define CloseMidiDevice(___mdd) \
      LP1NR(0xd2, CloseMidiDevice , struct MidiDeviceData *, ___mdd, a0,\
      , CAMD_BASE_NAME)

#define RethinkCAMD() \
      LP0(0xd8, int, RethinkCAMD ,\
      , CAMD_BASE_NAME)

#define StartClusterNotify(___node) \
      LP1NR(0xde, StartClusterNotify , struct ClusterNotifyNode *, ___node, a0,\
      , CAMD_BASE_NAME)

#define EndClusterNotify(___node) \
      LP1NR(0xe4, EndClusterNotify , struct ClusterNotifyNode *, ___node, a0,\
      , CAMD_BASE_NAME)

#endif /* !_INLINE_CAMD_H */
