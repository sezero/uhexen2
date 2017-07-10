#ifndef MIDI_MIDIPREFS_H
#define MIDI_MIDIPREFS_H

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifndef LIBRARIES_IFFPARSE_H
#include <libraries/iffparse.h>
#endif

#define MaxMidiDevName		32
#define MaxMidiInOutName	32
#define MaxMidiComment		34

struct MidiUnitDef 
{
	UBYTE MidiDeviceName[32];
	UBYTE MidiClusterInName[32];
	UBYTE MidiClusterOutName[32];
	UBYTE MidiDeviceComment[34];
	UBYTE MidiDevicePort;
	UBYTE Flags;
	ULONG XmitQueueSize;
	ULONG RecvQueueSize;
	ULONG Reserved[4];
};

#define MUDF_Internal (1 << 0)
#define MUDF_Ignore	 (1 << 1)

#define MinXmitQueueSize 512
#define MinRecvQueueSize 2048

#define ID_MIDI MAKE_ID('M','I','D','I')

struct MidiPrefs 
{
	UBYTE NUnits;
	UBYTE pad0[3];
	struct MidiUnitDef UnitDef[1];
};

#define MidiPrefsName "midi.prefs"

#endif
