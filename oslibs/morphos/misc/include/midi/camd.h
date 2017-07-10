#ifndef MIDI_CAMD_H
#define MIDI_CAMD_H

#ifndef EXEC_LISTS_H
#include <exec/lists.h>
#endif

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifndef UTILITY_TAGITEM_H
#include <utility/tagitem.h>
#endif

enum
{
	CD_Linkages,
	CD_NLocks
};

typedef union
{
	ULONG l[2];
	UBYTE b[4];
} MidiMsg;

#define mm_Msg    l[0]
#define mm_Time   l[1]
#define mm_Status b[0]
#define mm_Data1  b[1]
#define mm_Data2  b[2]
#define mm_Port   b[3]
#define mm_Data   b

#define voicemsg(m,a) (((m)->mm_Status & MS_StatBits) == (a))
#define sysmsg(m)     ((m)->mm_Status >= MS_System)
#define noteon(m)     (voicemsg(m,MS_NoteOn) && (m)->mm_Data2)
#define realtime(m)   ((m)->mm_Status >= MS_RealTime)
#define modemsg(m)    (voicemsg(m,MS_Ctrl) && (m)->mm_Data1 >= MM_Min)

struct MidiCluster
{
	struct Node mcl_Node;
	WORD        mcl_Participants;
	struct List mcl_Receivers;
	struct List mcl_Senders;
	WORD        mcl_PublicParticipants;
	UWORD       mcl_Flags;
};

struct MidiLink
{
	struct Node         ml_Node;
	WORD                ml_Pad;
	struct MinNode      ml_OwnerNode;
	struct MidiNode    *ml_MidiNode;
	struct MidiCluster *ml_Location;
	char               *ml_ClusterComment;
	UBYTE               ml_Flags;
	UBYTE               ml_PortID;
	UWORD               ml_ChannelMask;
	ULONG               ml_EventTypeMask;

	union SysExFilter
	{
		UBYTE            b[4];
		ULONG            sxf_Packed;
	} ml_SysExFilter;

	APTR                ml_ParserData;
	APTR                ml_UserData;
};

#define sxf_Mode b[0]
#define sxf_ID1  b[1]
#define sxf_ID2  b[2]
#define sxf_ID3  b[3]

enum
{
	MLTYPE_Receiver,
	MLTYPE_Sender,
	MLTYPE_NTypes
};

#define MLF_Sender      (1 << 0)
#define MLF_PartChange  (1 << 1)
#define MLF_PrivateLink (1 << 2)
#define MLF_DeviceLink  (1 << 3)

#define MLINK_Base         (TAG_USER + 65)

#define MLINK_Location     (MLINK_Base + 0)
#define MLINK_ChannelMask  (MLINK_Base + 1)
#define MLINK_EventMask    (MLINK_Base + 2)
#define MLINK_UserData     (MLINK_Base + 3)
#define MLINK_Comment      (MLINK_Base + 4)
#define MLINK_PortID       (MLINK_Base + 5)
#define MLINK_Private      (MLINK_Base + 6)
#define MLINK_Priority     (MLINK_Base + 7)
#define MLINK_SysExFilter  (MLINK_Base + 8)
#define MLINK_SysExFilterX (MLINK_Base + 9)
#define MLINK_Parse        (MLINK_Base + 10)
#define MLINK_Reserved     (MLINK_Base + 11)
#define MLINK_ErrorCode    (MLINK_Base + 12)
#define MLINK_Name         (MLINK_Base + 13)

#define SXF_ModeBits  0x04
#define SXF_CountBits 0x03

#define SXFM_Off      0x00
#define SXFM_1Byte    0x00
#define SXFM_3Byte    0x04


struct MidiNode
{
	struct Node     mi_Node;
	UWORD           mi_ClientType;
	struct Image   *mi_Image;
	struct MinList  mi_OutLinks;
	struct MinList  mi_InLinks;
	struct Task    *mi_SigTask;
	struct Hook    *mi_ReceiveHook;
	struct Hook    *mi_ParticipantHook;
	BYTE            mi_ReceiveSigBit;
	BYTE            mi_ParticipantSigBit;
	UBYTE           mi_ErrFilter;
	UBYTE           mi_Alignment[1];
	ULONG          *mi_TimeStamp;
	ULONG           mi_MsgQueueSize;
	ULONG           mi_SysExQueueSize;
};

#define CCType_Sequencer        (1 << 0)
#define CCType_SampleEditor     (1 << 1)
#define CCType_PatchEditor      (1 << 2)
#define CCType_Notator          (1 << 3)
#define CCType_EventProcessor   (1 << 4)
#define CCType_EventFilter      (1 << 5)
#define CCType_EventRouter      (1 << 6)
#define CCType_ToneGenerator    (1 << 7)
#define CCType_EventGenerator   (1 << 8)
#define CCType_GraphicAnimator  (1 << 9)

#define MIDI_Base         (TAG_USER + 65)
#define MIDI_Name         (MIDI_Base + 0)
#define MIDI_SignalTask   (MIDI_Base + 1)
#define MIDI_RecvHook     (MIDI_Base + 2)
#define MIDI_PartHook     (MIDI_Base + 3)
#define MIDI_RecvSignal   (MIDI_Base + 4)
#define MIDI_PartSignal   (MIDI_Base + 5)
#define MIDI_MsgQueue     (MIDI_Base + 6)
#define MIDI_SysExSize    (MIDI_Base + 7)
#define MIDI_TimeStamp    (MIDI_Base + 8)
#define MIDI_ErrFilter    (MIDI_Base + 9)
#define MIDI_ClientType   (MIDI_Base + 10)
#define MIDI_Image        (MIDI_Base + 11)
#define MIDI_ErrorCode    (MIDI_Base + 12)

#define CME_NoMem         801
#define CME_NoSignals     802
#define CME_NoTimer       803
#define CME_BadPrefs      804
#define CME_NoUnit(unit) (820 + (unit))

#define CMB_Note          0
#define CMB_Prog          1
#define CMB_PitchBend     2

#define CMB_CtrlMSB       3
#define CMB_CtrlLSB       4
#define CMB_CtrlSwitch    5
#define CMB_CtrlByte      6
#define CMB_CtrlParam     7
#define CMB_CtrlUndef     8

#define CMB_Mode          9
#define CMB_ChanPress     10
#define CMB_PolyPress     11

#define CMB_RealTime      12
#define CMB_SysCom        13
#define CMB_SysEx         14

#define CMF_Note          (1L << CMB_Note)
#define CMF_Prog          (1L << CMB_Prog)
#define CMF_PitchBend     (1L << CMB_PitchBend)

#define CMF_CtrlMSB       (1L << CMB_CtrlMSB)
#define CMF_CtrlLSB       (1L << CMB_CtrlLSB)
#define CMF_CtrlSwitch    (1L << CMB_CtrlSwitch)
#define CMF_CtrlByte      (1L << CMB_CtrlByte)
#define CMF_CtrlParam     (1L << CMB_CtrlParam)
#define CMF_CtrlUndef     (1L << CMB_CtrlUndef)

#define CMF_Mode          (1L << CMB_Mode)
#define CMF_ChanPress     (1L << CMB_ChanPress)
#define CMF_PolyPress     (1L << CMB_PolyPress)

#define CMF_RealTime      (1L << CMB_RealTime)
#define CMF_SysCom        (1L << CMB_SysCom)
#define CMF_SysEx         (1L << CMB_SysEx)

#define CMF_Ctrl          (CMF_CtrlMSB | CMF_CtrlLSB | CMF_CtrlSwitch | CMF_CtrlByte | CMF_CtrlParam | CMF_CtrlUndef)
#define CMF_Channel       (CMF_Note | CMF_Prog | CMF_PitchBend | CMF_Ctrl | CMF_Mode | CMF_ChanPress | CMF_PolyPress)
#define CMF_All           (CMF_Channel | CMF_RealTime | CMF_SysCom | CMF_SysEx)


#define CMEB_MsgErr       0
#define CMEB_BufferFull   1
#define CMEB_SysExFull    2
#define CMEB_ParseMem     3
#define CMEB_RecvErr      4
#define CMEB_RecvOverflow 5
#define CMEB_SysExTooBig  6

#define CMEF_MsgErr       (1L << CMEB_MsgErr)
#define CMEF_BufferFull   (1L << CMEB_BufferFull)
#define CMEF_SysExFull    (1L << CMEB_SysExFull)
#define CMEF_ParseMem     (1L << CMEB_ParseMem)
#define CMEF_RecvErr      (1L << CMEB_RecvErr)
#define CMEF_RecvOverflow (1L << CMEB_RecvOverflow)
#define CMEF_SysExTooBig  (1L << CMEB_SysExTooBig)

#define CMEF_All          (CMEF_MsgErr | CMEF_BufferFull | CMEF_SysExFull | CMEF_SysExTooBig | CMEF_ParseMem | CMEF_RecvErr | CMEF_RecvOverflow)

enum
{
	CMSG_Recv,
	CMSG_Link
};

struct cmLink
{
	ULONG cml_MethodID;
	ULONG cml_Action;
};

enum
{
	CACT_Link,
	CACT_Unlink
};

struct ClusterNotifyNode
{
	struct MinNode  cnn_Node;
	struct Task	   *cnn_Task;
	BYTE	          cnn_SigBit;
	UBYTE	          pad[3];
};

#define PackSysExFilter0()            ((ULONG)(SXFM_Off << 24))
#define PackSysExFilter1(id1)         ((ULONG)(SXFM_1Byte | 1) << 24 | (ULONG)(id1) << 16)
#define PackSysExFilter2(id1,id2)     ((ULONG)(SXFM_1Byte | 2) << 24 | (ULONG)(id1) << 16 | (id2) << 8)
#define PackSysExFilter3(id1,id2,id3) ((ULONG)(SXFM_1Byte | 3) << 24 | (ULONG)(id1) << 16 | (id2) << 8 | (id3))
#define PutMidiMsg(ml,msg)            PutMidi((ml),(msg)->l[0])

#endif /* MIDI_CAMD_H */
