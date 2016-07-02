#ifndef MIDI_MIDIDEFS_H
#define MIDI_MIDIDEFS_H

/************************************************************************
*			     C. A. M. D.				*
*************************************************************************
* CMU Amiga Midi Driver - Carnegie Mellon University			*
* 1988			- Commodore Amiga				*
*									*
* Design & Development	- Roger B. Dannenberg				*
*			- Jean-Christophe Dhellemmes			*
*			- Bill Barton					*
* Copyright 1989 Carnegie Mellon University				*
*************************************************************************
*
* mididefs.h  - MIDI message definitions
*
* This information is derived from MIDI 1.0 Detailed Specification
* v4.0 published by the Internation MIDI Association and is
* current as of June, 1988.  The Sys/Ex ID numbers are hot off the
* IMA's FAX machine as of Jan 8, 1990.
*
*************************************************************************
* Date	      | Change
*------------------------------------------------------------------------
* 22-Jul-1990 : Updated for new bank select controller # (BB)
*  5-Nov-1988 : Created (BB)
************************************************************************/


/***************************************************************
*
*   Status Bytes
*
***************************************************************/

    /* Channel Voice Messages (1sssnnnn) (OR with channel number) */
#define MS_NoteOff	0x80
#define MS_NoteOn	0x90
#define MS_PolyPress	0xA0
#define MS_Ctrl 	0xB0
#define MS_Mode 	0xB0
#define MS_Prog 	0xC0
#define MS_ChanPress	0xD0
#define MS_PitchBend	0xE0

#define MS_StatBits	0xF0
#define MS_ChanBits	0x0F

    /* System Common Messages (11110sss) */
#define MS_System	0xF0	    /* min system status byte */
#define MS_SysEx	0xF0
#define MS_QtrFrame	0xF1
#define MS_SongPos	0xF2
#define MS_SongSelect	0xF3
#define MS_TuneReq	0xF6
#define MS_EOX		0xF7

    /* System Real Time Messages (11111sss) */
#define MS_RealTime	0xF8	    /* min real time status byte */
#define MS_Clock	0xF8
#define MS_Start	0xFA
#define MS_Continue	0xFB
#define MS_Stop 	0xFC
#define MS_ActvSense	0xFE
#define MS_Reset	0xFF


/***************************************************************
*
*   Standard Controllers
*
***************************************************************/

    /* Continuous 14 bit - MSB: 0-1f, LSB: 20-3f */
#define MC_Bank 	0x00
#define MC_ModWheel	0x01
#define MC_Breath	0x02
#define MC_Foot 	0x04
#define MC_PortaTime	0x05
#define MC_DataEntry	0x06
#define MC_Volume	0x07
#define MC_Balance	0x08
#define MC_Pan		0x0a
#define MC_Expression	0x0b
#define MC_General1	0x10
#define MC_General2	0x11
#define MC_General3	0x12
#define MC_General4	0x13

    /* Continuous 7 bit (switches: 0-3f=off, 40-7f=on) */
#define MC_Sustain	0x40
#define MC_Porta	0x41
#define MC_Sustenuto	0x42
#define MC_SoftPedal	0x43
#define MC_Hold2	0x45
#define MC_General5	0x50
#define MC_General6	0x51
#define MC_General7	0x52
#define MC_General8	0x53
#define MC_ExtDepth	0x5b
#define MC_TremoloDepth 0x5c
#define MC_ChorusDepth	0x5d
#define MC_CelesteDepth 0x5e
#define MC_PhaserDepth	0x5f

    /* Parameters */
#define MC_DataIncr	0x60
#define MC_DataDecr	0x61
#define MC_NRPNL	0x62
#define MC_NRPNH	0x63
#define MC_RPNL 	0x64
#define MC_RPNH 	0x65

#define MC_Max		0x78	/* max controller value */


/***************************************************************
*
*   Channel Modes
*
***************************************************************/

#define MM_Min		0x79	/* min mode value */

#define MM_ResetCtrl	0x79
#define MM_Local	0x7a
#define MM_AllOff	0x7b
#define MM_OmniOff	0x7c
#define MM_OmniOn	0x7d
#define MM_Mono 	0x7e
#define MM_Poly 	0x7f


/***************************************************************
*
*   Registered Parameter Numbers
*
*   These are 16 bit values that need to be separated into two
*   bytes for use with the MC_RPNH & MC_RPNL messages using 8
*   bit math (hi = MRP_ >> 8, lo = MRP_ & 0xff) as opposed to 7
*   bit math.  This is done so that the defines match the
*   numbers from the MMA.  See MIDI 1.0 Detailed Spec v4.0 pp
*   12, 23 for more info.
*
***************************************************************/

#define MRP_PBSens	0x0000
#define MRP_FineTune	0x0001
#define MRP_CourseTune	0x0002


/***************************************************************
*
*   MTC Quarter Frame messages
*
*   Qtr Frame message is F1 0nnndddd where
*
*	nnn is a message type defined below
*	dddd is 4 bit data nibble for those message types
*
*   Each pair of nibbles is combined by the receiver into a
*   single byte. There are masks and type values defined for
*   some of these data bytes below.
*
***************************************************************/

    /* Message Types */
#define MTCQ_FrameL	0x00
#define MTCQ_FrameH	0x10
#define MTCQ_SecL	0x20
#define MTCQ_SecH	0x30
#define MTCQ_MinL	0x40
#define MTCQ_MinH	0x50
#define MTCQ_HourL	0x60
#define MTCQ_HourH	0x70	/* also contains time code type */

    /* Message Masks */
#define MTCQ_TypeMask	0x70	/* mask for type bits in message */
#define MTCQ_DataMask	0x0f	/* mask for data bits in message */

    /* Hour Byte */
#define MTCH_TypeMask	0x60	/* mask for time code type */
#define MTCH_HourMask	0x1f	/* hours mask (range 0-23) */

    /* Time code type values for hour byte */
#define MTCT_24FPS	0x00
#define MTCT_25FPS	0x20
#define MTCT_30FPS_Drop 0x40
#define MTCT_30FPS_NonDrop  0x60


/***************************************************************
*
*   Sys/Ex ID numbers
*
*   Now includes 3 byte extension for the American Group.  This
*   new format uses a 0x00 as the sys/ex id followed by two
*   additional bytes that actually identify the manufacturer.
*   These new extended id constants are 32 bit values with 24
*   significant bits.
*
***************************************************************/

    /* North American Group (1 byte) */
#define MID_Sequential	    0x01
#define MID_IDP 	    0x02
#define MID_OctavePlateau   0x03
#define MID_Moog	    0x04
#define MID_Passport	    0x05
#define MID_Lexicon	    0x06
#define MID_Kurzweil	    0x07
#define MID_Fender	    0x08
#define MID_Gulbransen	    0x09
#define MID_AKG 	    0x0a
#define MID_Voyce	    0x0b
#define MID_Waveframe	    0x0c
#define MID_ADA 	    0x0d
#define MID_Garfield	    0x0e
#define MID_Ensoniq	    0x0f
#define MID_Oberheim	    0x10
#define MID_Apple	    0x11
#define MID_GreyMatter	    0x12
#define MID_PalmTree	    0x14
#define MID_JLCooper	    0x15
#define MID_Lowrey	    0x16
#define MID_AdamsSmith	    0x17
#define MID_Emu 	    0x18
#define MID_Harmony	    0x19
#define MID_ART 	    0x1a
#define MID_Baldwin	    0x1b
#define MID_Eventide	    0x1c
#define MID_Inventronics    0x1d
#define MID_Clarity	    0x1f

    /* North American Group (3 byte) */
#define MID_XAmerica	    0x00    /* North American 3 byte prefix */

#define MIDX_DigitalMusic   0x000007L
#define MIDX_Iota	    0x000008L
#define MIDX_Artisyn	    0x00000aL
#define MIDX_IVL	    0x00000bL
#define MIDX_SouthernMusic  0x00000cL
#define MIDX_LakeButler     0x00000dL
#define MIDX_DOD	    0x000010L
#define MIDX_PerfectFret    0x000014L
#define MIDX_KAT	    0x000015L
#define MIDX_Opcode	    0x000016L
#define MIDX_Rane	    0x000017L
#define MIDX_SpatialSound   0x000018L
#define MIDX_KMX	    0x000019L
#define MIDX_Brenell	    0x00001aL
#define MIDX_Peavey	    0x00001bL
#define MIDX_360	    0x00001cL
#define MIDX_Axxes	    0x000020L
#define MIDX_CAE	    0x000026L
#define MIDX_Cannon	    0x00002bL
#define MIDX_BlueSkyLogic   0x00002eL
#define MIDX_Voce	    0x000031L

    /* European Group */
#define MID_SIEL	    0x21
#define MID_Synthaxe	    0x22
#define MID_Hohner	    0x24
#define MID_Twister	    0x25
#define MID_Solton	    0x26
#define MID_Jellinghaus     0x27
#define MID_Southworth	    0x28
#define MID_PPG 	    0x29
#define MID_JEN 	    0x2a
#define MID_SSL 	    0x2b
#define MID_AudioVeritrieb  0x2c
#define MID_Elka	    0x2f
#define MID_Dynacord	    0x30
#define MID_Clavia	    0x33
#define MID_Soundcraft	    0x39

    /* Japanese Group */
#define MID_Kawai	    0x40
#define MID_Roland	    0x41
#define MID_Korg	    0x42
#define MID_Yamaha	    0x43
#define MID_Casio	    0x44
#define MID_Kamiya	    0x46
#define MID_Akai	    0x47
#define MID_JapanVictor     0x48
#define MID_Mesosha	    0x49

    /* Universal ID Numbers */
#define MID_UNC 	0x7d
#define MID_UNRT	0x7e
#define MID_URT 	0x7f


/***************************************************************
*
*   Miscellaneous
*
***************************************************************/

#define MiddleC 	60	/* Middle C note value */
#define DefaultVelocity 64	/* Default Note On or Off velocity */
#define PitchBendCenter 0x2000	/* Pitch Bend center position as a 14 bit word */
#define MClksPerQtr	24	/* MIDI clocks per qtr-note */
#define MClksPerSP	6	/* MIDI clocks per song position index */
#define MCCenter	64	/* Center value for controllers like Pan and Balance */


/***************************************************************
*
*   Macros
*
***************************************************************/

    /* pack high/low bytes of a word into midi format (7/14 bit math) */
#define MIDIHiByte(word) ( (word) >> 7 & 0x7f )
#define MIDILoByte(word) ( (word) & 0x7f )

    /* unpack 2 midi bytes into a word (7/14 bit math) */
#define MIDIWord(hi,lo) ( ((hi) & 0x7f) << 7 | ((lo) & 0x7f) )

    /* make a 3 byte sys/ex id from single bytes (e.g. MakeMIDX(msg[1],msg[2],msg[3]) */
#define MakeMIDX(id0,id1,id2) ((ULONG)(id0)<<16 | (id1)<<8 | (id2))


#endif /* MIDI_MIDIDEFS_H */
