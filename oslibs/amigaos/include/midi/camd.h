#ifndef MIDI_CAMD_H
#define MIDI_CAMD_H
/************************************************************************
*     C. A. M. D.       (Commodore Amiga MIDI Driver)                   *
*************************************************************************
*                                                                       *
* Design & Development  - Roger B. Dannenberg                           *
*                       - Jean-Christophe Dhellemmes                    *
*                       - Bill Barton                                   *
*                       - Darius Taghavy                                *
*                                                                       *
* Copyright 1990 by Commodore Business Machines                         *
*************************************************************************
*
* camd.h      - General CAMD include files.
*             - General CAMD definitions.
*
************************************************************************/

#ifndef EXEC_LISTS_H
  #include <exec/lists.h>
#endif

#ifndef EXEC_TYPES_H
  #include <exec/types.h>
#endif

#ifndef UTILITY_TAGITEM_H
  #include <utility/tagitem.h>
#endif

#if 0       /* !!! old */

/***************************************************************
*
*   Library Name and Version
*
***************************************************************/

#define CamdName    "camd.library"
#define CamdVersion 2

#endif


/***************************************************************
*
*   CAMD internal lists that can be locked
*
***************************************************************/

enum {
    CD_Linkages,                                /* internal linages */
    CD_NLocks
};

#if 0       /* !!! old */
/***************************************************************
*
*   MIDI Port Definitions
*
*   The default Unit Ports are:
*
*              ports
*       unit  in  out
*       ----  --  ---
*        0     1   0
*        1     3   2
*        2     5   4
*        3     6   5
*
*   These are the values returned by CMP_Out() and CMP_In().
*
*   User ports are allocated starting at CMP_Max and descending.
*
***************************************************************/

#define CMP_Out(unit)   ((unit) * 2)            /* Default port # of MIDI Output */
#define CMP_In(unit)    (CMP_Out(unit) + 1)     /* Default port # of MIDI Input */
#define CMP_Max         31                      /* highest port # */
#endif

/***************************************************************
*
*   MidiMsg
*
***************************************************************/

typedef union
{
    ULONG l[2];
    UBYTE b[4];
} MidiMsg;

    /* MidiMsg field definitions */
#define mm_Msg    l[0]
#define mm_Time   l[1]
#define mm_Status b[0]
#define mm_Data1  b[1]
#define mm_Data2  b[2]
#define mm_Port   b[3]
#define mm_Data   b

    /* MidiMsg macros for quick message type detection.  MidiMsgType()
       generally gives more precise results. */

#define voicemsg(m,a)   ( ((m)->mm_Status & MS_StatBits) == (a) )
#define sysmsg(m)       ( (m)->mm_Status >= MS_System )
#define noteon(m)       ( voicemsg(m,MS_NoteOn) && (m)->mm_Data2 )
#define realtime(m)     ( (m)->mm_Status >= MS_RealTime )
#define modemsg(m)      ( voicemsg(m,MS_Ctrl) && (m)->mm_Data1 >= MM_Min )

/***************************************************************
*
*   MidiCluster -- a meeting place for linkages
*
*   All fields are READ ONLY.  Modifications to fields may
*   performed only through the appropriate library function
*   calls.
*
***************************************************************/

struct MidiCluster {
    struct Node         mcl_Node;               /* linked list node */
    WORD                mcl_Participants;
    struct List         mcl_Receivers,          /* list of receivers */
                        mcl_Senders;            /* list of senders */
    WORD                mcl_PublicParticipants; /* if >0, cluster is public */
    UWORD               mcl_Flags;              /* flags for this location */
        /* NOTE: Cluster name follows structure, and is pointed to by ln_Name */
};

/***************************************************************
*
*   MidiLink -- links a cluster and a MidiNode
*
*   All fields are READ ONLY.  Modifications to fields may
*   performed only through the appropriate library function
*   calls.
*
***************************************************************/

struct MidiLink {
    struct Node         ml_Node;                /* node for cluster list */
    WORD                ml_Pad;
    struct MinNode      ml_OwnerNode;           /* node for interface list */
    struct MidiNode     *ml_MidiNode;           /* interface we belong to... */
    struct MidiCluster  *ml_Location;           /* location we are a member of */
    char                *ml_ClusterComment;     /* comment for cluster */
    UBYTE               ml_Flags;               /* general flags */
    UBYTE               ml_PortID;              /* number of this port */
    UWORD               ml_ChannelMask;         /* mask flags for channel */
    ULONG               ml_EventTypeMask;       /* mask flags for events */

    union SysExFilter {
        UBYTE           b[4];                   /* 1 byte mode, 3 bytes for match id(s) */
        ULONG           sxf_Packed;
    } ml_SysExFilter;

    APTR                ml_ParserData;          /* private MIDI parser data */
    APTR                ml_UserData;            /* attached to events... */
};

    /* SysExFilter members */
#define sxf_Mode b[0]           /* SXFM_ mode bits */
#define sxf_ID1  b[1]           /* 3 1-byte id's or 1 3-byte id */
#define sxf_ID2  b[2]
#define sxf_ID3  b[3]

    /* MidiLink types */
enum {
    MLTYPE_Receiver,
    MLTYPE_Sender,
    MLTYPE_NTypes
};

    /* ml_Flags */
#define MLF_Sender      (1<<0)                  /* this link sends from app */
#define MLF_PartChange  (1<<1)                  /* part change pending */
#define MLF_PrivateLink (1<<2)                  /* make this link private */
#define MLF_DeviceLink  (1<<3)                  /* set by devices only! */

    /* MidiLink tags */
#define MLINK_Base          TAG_USER+65

#define MLINK_Location      (MLINK_Base+0)      /* rendezvous point */
#define MLINK_ChannelMask   (MLINK_Base+1)      /* channel mask bits */
#define MLINK_EventMask     (MLINK_Base+2)      /* event type mask bits */
#define MLINK_UserData      (MLINK_Base+3)      /* user data for incoming events */
#define MLINK_Comment       (MLINK_Base+4)      /* comment for cluster */
#define MLINK_PortID        (MLINK_Base+5)      /* port id number for MidiMsg */
#define MLINK_Private       (MLINK_Base+6)      /* not a public link */
#define MLINK_Priority      (MLINK_Base+7)      /* priority of node */
/* REM: Add tags to change Sysex filter stuff...*/
#define MLINK_SysExFilter   (MLINK_Base+8)      /* three 1-byte headers */
#define MLINK_SysExFilterX  (MLINK_Base+9)      /* one 3-byte header */
#define MLINK_Parse         (MLINK_Base+10)     /* want a MIDI parser */
#define MLINK_Reserved      (MLINK_Base+11)     /* reserved */
#define MLINK_ErrorCode     (MLINK_Base+12)     /* (ULONG *) - Error Code buffer for returning CME_ error codes. */
#define MLINK_Name          (MLINK_Base+13)     /* (STRPTR) - Link name */


/***************************************************************
*
*   SysExFilter modes
*
*   Contents of sxf_Mode.
*
*   Bit packed as follows: 00000mcc
*       m  - mode bit
*       cc - count bits 0 - 3 (only used for SXFM_1Byte)
*
***************************************************************/

#define SXF_ModeBits    0x04
#define SXF_CountBits   0x03        /* mask for count bits for SXFM_1Byte */

#define SXFM_Off        0x00        /* don't filter (equal to SXFM_1Byte | 0) */
#define SXFM_1Byte      0x00        /* match upto 3 1-byte id's */
#define SXFM_3Byte      0x04        /* match a single 3-byte id */


/***************************************************************
*
*   MidiNode
*
*   All fields are READ ONLY.  Modifications to fields may
*   performed only through the appropriate library function
*   calls.
*
***************************************************************/

struct MidiNode
{
    struct Node         mi_Node;                /* linked list node */
    UWORD               mi_ClientType;          /* type of application */
    struct Image        *mi_Image;              /* image for patch panel */

    struct MinList      mi_OutLinks,            /* list of output links */
                        mi_InLinks;             /* list of input links */

    struct Task         *mi_SigTask;            /* task to signal */
    struct Hook         *mi_ReceiveHook,        /* hook (and list node) */
                        *mi_ParticipantHook;    /* hook for participant change */
    BYTE                mi_ReceiveSigBit,       /* signalmask for new data */
                        mi_ParticipantSigBit;   /* signalmask for part change */
    UBYTE               mi_ErrFilter;           /* CMEF_ error filter for ErrFlags */
    UBYTE               mi_Alignment[1];        /* for longword alignment */

    ULONG               *mi_TimeStamp;          /* where timestamps come from */

    ULONG               mi_MsgQueueSize,        /* size of buffers */
                        mi_SysExQueueSize;

    /* private stuff below here */
};

        /* client types */
#define CCType_Sequencer        (1<<0)
#define CCType_SampleEditor     (1<<1)
#define CCType_PatchEditor      (1<<2)
#define CCType_Notator          (1<<3)          /* transcription of MIDI notes  */
#define CCType_EventProcessor   (1<<4)          /* any data altering functions  */
#define CCType_EventFilter      (1<<5)
#define CCType_EventRouter      (1<<6)          /* e.g MIDI thru task */
#define CCType_ToneGenerator    (1<<7)          /* i.e.using Amiga to make sound*/
#define CCType_EventGenerator   (1<<8)          /* e.g algorithmic composition */
#define CCType_GraphicAnimator  (1<<9)          /* e.g syncing animation software to MIDI */

        /* Tags for CreateMidi() and SetMidiAttrs() */
#define MIDI_Base               (TAG_USER+65)
#define MIDI_Name               (MIDI_Base+0)   /* name of application */
#define MIDI_SignalTask         (MIDI_Base+1)   /* task to signal if not this one*/
#define MIDI_RecvHook           (MIDI_Base+2)   /* hook for incoming data */
#define MIDI_PartHook           (MIDI_Base+3)   /* hook for participant change */
#define MIDI_RecvSignal         (MIDI_Base+4)   /* signal to use if incoming */
#define MIDI_PartSignal         (MIDI_Base+5)   /* signal to use if incoming */
#define MIDI_MsgQueue           (MIDI_Base+6)   /* size of event buffer */
#define MIDI_SysExSize          (MIDI_Base+7)   /* size of sysex buffer */
#define MIDI_TimeStamp          (MIDI_Base+8)   /* timer to timestamp with */
#define MIDI_ErrFilter          (MIDI_Base+9)   /* error filter bits */
#define MIDI_ClientType         (MIDI_Base+10)  /* client type mask */
#define MIDI_Image              (MIDI_Base+11)  /* application image */
#define MIDI_ErrorCode          (MIDI_Base+12)  /* (ULONG *) - Error Code buffer for returning CME_ error codes. */


/***************************************************************
*
*   CreateMidi() Error Codes
*
*   These are the IoErr() codes that CreateMidi() can return
*   on failure.
*
*   !!! need specific error code set for each function instead!
*
***************************************************************/

#define CME_NoMem     801       /* memory allocation failed */
#define CME_NoSignals 802       /* signal allocation failed */
#define CME_NoTimer   803       /* timer (CIA) allocation failed */
#define CME_BadPrefs  804       /* badly formed midi.prefs file */

#define CME_NoUnit(unit) (820 + (unit)) /* unit open failure */


#if 0       /* !!! old */
/***************************************************************
*
*   MidiNode tag items for use with CreateMidi().
*
***************************************************************/

#define CMA_base        (TAG_USER + 64)

#define CMA_SysEx       (CMA_base + 0)  /* int - allocate a sys/ex buffer,
                                           ti_Data specifies size in bytes.
                                           Default is 0.  Only valid if
                                           RecvSize is non-zero. */

#define CMA_Parse       (CMA_base + 1)  /* bool - enable usage of ParseMidi().
                                           Default is FALSE. */

#define CMA_Alarm       (CMA_base + 2)  /* bool - enable usage of SetMidiAlarm().
                                           Also allocates AlarmSigBit.  Default is FALSE. */

#define CMA_SendPort    (CMA_base + 3)  /* int - initial SendPort.  Default is CMP_Out(0). */

#define CMA_PortFilter  (CMA_base + 4)  /* int - initial PortFilter.  Default is 0. */

#define CMA_TypeFilter  (CMA_base + 5)  /* int - initial TypeFilter.  Default is 0. */

#define CMA_ChanFilter  (CMA_base + 6)  /* int - initial ChanFilter.  Default is 0. */

#define CMA_SysExFilter (CMA_base + 7)  /* packed - initial SysExFilter as returned
                                           by one of the PackSysExFilterN() macros.
                                           Default is no filtering (i.e. recv all). */

#define CMA_ErrFilter   (CMA_base + 8)  /* int - initial ErrFilter.  Default is 0. */
#endif


/***************************************************************
*
*   MIDI Message Type Bits
*
*   Returned by MidiMsgType() and used with SetMidiFilters().
*
***************************************************************/

#define CMB_Note        0
#define CMB_Prog        1
#define CMB_PitchBend   2

#define CMB_CtrlMSB     3
#define CMB_CtrlLSB     4
#define CMB_CtrlSwitch  5
#define CMB_CtrlByte    6
#define CMB_CtrlParam   7
#define CMB_CtrlUndef   8       /* for future ctrl # expansion */

#define CMB_Mode        9
#define CMB_ChanPress   10
#define CMB_PolyPress   11

#define CMB_RealTime    12
#define CMB_SysCom      13
#define CMB_SysEx       14

    /* (these need to be long for SetMidiFilters()) */
#define CMF_Note        (1L << CMB_Note)
#define CMF_Prog        (1L << CMB_Prog)
#define CMF_PitchBend   (1L << CMB_PitchBend)

#define CMF_CtrlMSB     (1L << CMB_CtrlMSB)
#define CMF_CtrlLSB     (1L << CMB_CtrlLSB)
#define CMF_CtrlSwitch  (1L << CMB_CtrlSwitch)
#define CMF_CtrlByte    (1L << CMB_CtrlByte)
#define CMF_CtrlParam   (1L << CMB_CtrlParam)
#define CMF_CtrlUndef   (1L << CMB_CtrlUndef)

#define CMF_Mode        (1L << CMB_Mode)
#define CMF_ChanPress   (1L << CMB_ChanPress)
#define CMF_PolyPress   (1L << CMB_PolyPress)

#define CMF_RealTime    (1L << CMB_RealTime)
#define CMF_SysCom      (1L << CMB_SysCom)
#define CMF_SysEx       (1L << CMB_SysEx)

    /* some handy type macros */

#define CMF_Ctrl        (CMF_CtrlMSB | CMF_CtrlLSB | CMF_CtrlSwitch | CMF_CtrlByte | CMF_CtrlParam | CMF_CtrlUndef)
#define CMF_Channel     (CMF_Note | CMF_Prog | CMF_PitchBend | CMF_Ctrl | CMF_Mode | CMF_ChanPress | CMF_PolyPress)
#define CMF_All         (CMF_Channel | CMF_RealTime | CMF_SysCom | CMF_SysEx)


/***************************************************************
*
*   MIDI Error Flags
*
*   These are error flags that can arrive at a MidiNode.
*   An application may choose to ignore or process any
*   combination of error flags.  See SetMidiErrFilter() and
*   GetMidiErr() for more information.
*
***************************************************************/

#define CMEB_MsgErr         0   /* invalid message was sent */
#define CMEB_BufferFull     1   /* MidiBuffer is full */
#define CMEB_SysExFull      2   /* SysExBuffer is full */
#define CMEB_ParseMem       3   /* sys/ex memory allocation failure during parse */
#define CMEB_RecvErr        4   /* serial receive error */
#define CMEB_RecvOverflow   5   /* serial receive buffer overflow */
#define CMEB_SysExTooBig    6   /* Attempt to send a sys/ex message bigger than SysExBuffer */

#define CMEF_MsgErr         (1L << CMEB_MsgErr)
#define CMEF_BufferFull     (1L << CMEB_BufferFull)
#define CMEF_SysExFull      (1L << CMEB_SysExFull)
#define CMEF_ParseMem       (1L << CMEB_ParseMem)
#define CMEF_RecvErr        (1L << CMEB_RecvErr)
#define CMEF_RecvOverflow   (1L << CMEB_RecvOverflow)
#define CMEF_SysExTooBig    (1L << CMEB_SysExTooBig)

    /* a handy macro for SetMidiErrFilter() */
#define CMEF_All            (CMEF_MsgErr | CMEF_BufferFull | CMEF_SysExFull | CMEF_SysExTooBig | CMEF_ParseMem | CMEF_RecvErr | CMEF_RecvOverflow)


#if 0   /* !!! old */
/***************************************************************
*
*   MidiTickHookMsg
*
*   Message structure passed to Tick Hooks
*
***************************************************************/

struct MidiTickHookMsg
{
    ULONG ID;                   /* msg ID (always MTHM_Tick for now) */
    ULONG Time;                 /* Time at tick */
};

enum
{
    MTHM_Tick
};


/***************************************************************
*
*   MidiByteHookMsg
*
*   Message structure passed to Byte Hooks
*
***************************************************************/

struct MidiByteHookMsg
{
    ULONG ID;                   /* msg ID (always MBHM_Byte for now) */
    UBYTE UnitNum;              /* Midi Unit number where byte was received */
    UBYTE pad0;
    UWORD RecvData;             /* Data as it was received by the serial hardware.
                                   Bits 0-7 are the MIDI byte.  Bit 15, when set,
                                   indicates a hardware receive error. */
};

enum
{
    MBHM_Byte
};
#endif

/***************************************************************
*
*   Hook Message ID's
*
*   Each Hook passes as the "msg" param a pointer to one of these (LONG)
*   Can be extended for some types of messages
*
***************************************************************/

enum {
    CMSG_Recv,                  /* receive MIDI message                 */
    CMSG_Link                   /* a linkage notification               */
};


/***************************************************************
*
*   CMSG_Link structure
*
***************************************************************/

struct cmLink {
    ULONG    cml_MethodID;
    ULONG    cml_Action;
};

enum {
    CACT_Link,
    CACT_Unlink
};

/***************************************************************
*
*   ClusterNotifyNode
*
***************************************************************/

struct ClusterNotifyNode {
    struct MinNode  cnn_Node;		/* the usual node		*/
    struct Task	    *cnn_Task;		/* task to signal		*/
    BYTE	    cnn_SigBit;		/* sigbit to use		*/
    UBYTE	    pad[3];
};

/***************************************************************
*
*   CAMD Macros
*
*   See camd.doc for info.
*
***************************************************************/

#define PackSysExFilter0()              ((ULONG)SXFM_Off << 24)
#define PackSysExFilter1(id1)           ((ULONG)(SXFM_1Byte | 1) << 24 | (ULONG)(id1) << 16)
#define PackSysExFilter2(id1,id2)       ((ULONG)(SXFM_1Byte | 2) << 24 | (ULONG)(id1) << 16 | (id2) << 8)
#define PackSysExFilter3(id1,id2,id3)   ((ULONG)(SXFM_1Byte | 3) << 24 | (ULONG)(id1) << 16 | (id2) << 8 | (id3))

#define PutMidiMsg(ml,msg)              PutMidi((ml),(msg)->l[0])

#if 0       /* !!! old */
    /* ---- MidiNode */
#define PackSysExFilterX(xid)           ((ULONG)SXFM_3Byte << 24 | (xid))

#define ClearSysExFilter(mi)            SetSysExFilter ((mi), PackSysExFilter0())
#define SetSysExFilter1(mi,id1)         SetSysExFilter ((mi), PackSysExFilter1(id1))
#define SetSysExFilter2(mi,id1,id2)     SetSysExFilter ((mi), PackSysExFilter2(id1,id2))
#define SetSysExFilter3(mi,id1,id2,id3) SetSysExFilter ((mi), PackSysExFilter3(id1,id2,id3))
#define SetSysExFilterX(mi,xid)         SetSysExFilter ((mi), PackSysExFilterX(xid))

#define ClearSysExQueue(mi)             SetSysExQueue ((mi), NULL, 0L)

    /* ---- Message */
    /* REM: These macros no longer exist... */
    /* REM: We'll need to define a new macro to put MIDI bytes directly to a port */
/* #define PutMidi(mi,msg)                 PutMidiToPort ((msg)->mm_Msg & ~0xff | (mi)->SendPort) */
/* #define PutSysEx(mi,data)               PutSysExToPort ((long)(mi)->SendPort, data) */

    /* ---- Unit */
#define GetMidiInPort(unit)             SetMidiInPort ((long)(unit), -1L)
#define GetMidiOutMask(unit)            SetMidiOutMask ((long)(unit), 0L, 0L)
#define AndMidiOutMask(unit,mask)       SetMidiOutMask ((long)(unit), 0L, ~(ULONG)mask)
#define OrMidiOutMask(unit,mask)        SetMidiOutMask ((long)(unit), -1L, (ULONG)mask)
#define MidiThru(unit,enable)           SetMidiOutMask ((long)(unit), (enable) ? -1L : 0L, 1L << CMP_Out(unit))

    /* ---- Timer */
#define ClearMidiAlarm(mi)              SetMidiAlarm ((mi),0L)
#endif

#endif /* MIDI_CAMD_H */
