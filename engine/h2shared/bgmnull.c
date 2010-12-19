/* BGM code for when we are configured for no sound :  */
/* _NO_SOUND should have been defined by the Makefile. */

#ifdef _NO_MIDIDRV
/* no sound driver && codecs, no midi driver, nada...  */
#include "bgmnull_none.c"

#else /* perversely, we have no sound, but have midi.. */
#include "bgmnull_midi.c"

#endif	/* _NO_MIDIDRV */

