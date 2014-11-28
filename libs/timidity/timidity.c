/*
 * TiMidity -- Experimental MIDI to WAVE converter
 * Copyright (C) 1995 Tuukka Toivonen <toivonen@clinet.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "timidity.h"
#include "timidity_internal.h"
#include "common.h"
#include "instrum.h"
#include "playmidi.h"
#include "readmidi.h"
#include "output.h"
#include "tables.h"

#include "filenames.h"

static MidToneBank *master_tonebank[128], *master_drumset[128];

static char def_instr_name[256] = "";

#define MAXWORDS 10
#define MAX_RCFCOUNT 50

/* Quick-and-dirty fgets() replacement. */

static char *timi_fgets(char *s, int size, FILE *fp)
{
    int num_read = 0;
    int newline = 0;

    while (num_read < size && !newline)
    {
	if (fread(&s[num_read], 1, 1, fp) != 1)
	    break;

	/* Unlike fgets(), don't store newline. Under Windows/DOS we'll
	 * probably get an extra blank line for every line that's being
	 * read, but that should be ok.
	 */
	if (s[num_read] == '\n' || s[num_read] == '\r')
	{
	    s[num_read] = '\0';
	    newline = 1;
	}

	num_read++;
    }

    s[num_read] = '\0';

    return (num_read != 0) ? s : NULL;
}

static FILE **rcf_fp;

static int read_config_file(const char *name, int rcf_count)
{
  char  tmp[TIM_MAXPATH];
  char *w[MAXWORDS], *cp;
  MidToneBank *bank;
  int i, j, k, line, r, words;

  if (rcf_count >= MAX_RCFCOUNT)
  {
    DEBUG_MSG("Probable source loop in configuration files\n");
    return -1;
  }

  if (!(rcf_fp[rcf_count]=open_file(name)))
    return -1;

  bank = NULL;
  line = 0;
  r = -1; /* start by assuming failure, */

  while (timi_fgets(tmp, sizeof(tmp), rcf_fp[rcf_count]))
  {
    line++;
    words=0;
    w[0]=strtok(tmp, " \t\240");
    if (!w[0]) continue;

    /* Originally the TiMidity++ extensions were prefixed like this */
    if (strcmp(w[0], "#extension") == 0)
    {
      w[0]=strtok(NULL, " \t\240");
      if (!w[0]) continue;
    }

    if (*w[0] == '#')
      continue;

    while (w[words] && *w[words] != '#' && (words < MAXWORDS))
      w[++words]=strtok(0," \t\240");

    /* TiMidity++ adds a number of extensions to the config file format.
     * Many of them are completely irrelevant to SDL_sound, but at least
     * we shouldn't choke on them.
     *
     * Unfortunately the documentation for these extensions is often quite
     * vague, gramatically strange or completely absent.
     */
    if (!strcmp(w[0], "comm")      /* "comm" program second        */ ||
        !strcmp(w[0], "HTTPproxy") /* "HTTPproxy" hostname:port    */ ||
        !strcmp(w[0], "FTPproxy")  /* "FTPproxy" hostname:port     */ ||
        !strcmp(w[0], "mailaddr")  /* "mailaddr" your-mail-address */ ||
        !strcmp(w[0], "opt")       /* "opt" timidity-options       */  )
    {
      /* + "comm" sets some kind of comment -- the documentation is too
       *   vague for me to understand at this time.
       * + "HTTPproxy", "FTPproxy" and "mailaddr" are for reading data
       *   over a network, rather than from the file system.
       * + "opt" specifies default options for TiMidity++.
       *
       * Quite useless for us, so they can safely remain no-ops.
       */
    }
    else if (!strcmp(w[0], "timeout")) /* "timeout" program second */
    {
      /* Specifies a timeout value of the program. A number of seconds
       * before TiMidity kills the note. No urgent need for it.
       */
      DEBUG_MSG("FIXME: Implement \"timeout\" in TiMidity config.\n");
    }
    else if (!strcmp(w[0], "copydrumset")  /* "copydrumset" drumset */ ||
	     !strcmp(w[0], "copybank")) /* "copybank" bank       */
    {
      /* Copies all the settings of the specified drumset or bank to
       * the current drumset or bank. May be useful later, but not a
       * high priority.
       */
       DEBUG_MSG("FIXME: Implement \"%s\" in TiMidity config.\n", w[0]);
    }
    else if (!strcmp(w[0], "undef")) /* "undef" progno */
    {
      /* Undefines the tone "progno" of the current tone bank (or
       * drum set?). Not a high priority.
       */
      DEBUG_MSG("FIXME: Implement \"undef\" in TiMidity config.\n");
    }
    else if (!strcmp(w[0], "altassign")) /* "altassign" prog1 prog2 ... */
    {
      /* Sets the alternate assign for drum set. Whatever that's
       * supposed to mean.
       */
      DEBUG_MSG("FIXME: Implement \"altassign\" in TiMidity config.\n");
    }
    else if (!strcmp(w[0], "soundfont") ||
	     !strcmp(w[0], "font"))
    {
      /* "soundfont" sf_file "remove"
       * "soundfont" sf_file ["order=" order] ["cutoff=" cutoff]
       *                     ["reso=" reso] ["amp=" amp]
       * "font" "exclude" bank preset keynote
       * "font" "order" order bank preset keynote
       */
      DEBUG_MSG("FIXME: Implmement \"%s\" in TiMidity config.\n", w[0]);
    }
    else if (!strcmp(w[0], "progbase"))
    {
      /* The documentation for this makes absolutely no sense to me, but
       * apparently it sets some sort of base offset for tone numbers.
       */
      DEBUG_MSG("FIXME: Implement \"progbase\" in TiMidity config.\n");
    }
    else if (!strcmp(w[0], "map")) /* "map" name set1 elem1 set2 elem2 */
    {
      /* This one is used by the "eawpats". Looks like it's used
       * for remapping one instrument to another somehow.
       */
      DEBUG_MSG("FIXME: Implement \"map\" in TiMidity config.\n");
    }
    /* Standard TiMidity config */
    else if (!strcmp(w[0], "dir"))
    {
      if (words < 2)
      {
	DEBUG_MSG("%s: line %d: No directory given\n", name, line);
	goto fail;
      }
      for (i=1; i<words; i++)
	add_to_pathlist(w[i], strlen(w[i]));
    }
    else if (!strcmp(w[0], "source"))
    {
      if (words < 2)
      {
	DEBUG_MSG("%s: line %d: No file name given\n", name, line);
	goto fail;
      }
      for (i=1; i<words; i++)
      {
	r = read_config_file(w[i], rcf_count + 1);
	if (r != 0)
	  goto fail;
      }
      r = -1; /* not finished yet, */
    }
    else if (!strcmp(w[0], "default"))
    {
      if (words != 2)
      {
	DEBUG_MSG("%s: line %d: Must specify exactly one patch name\n",
		name, line);
	goto fail;
      }
      strncpy(def_instr_name, w[1], 255);
      def_instr_name[255]='\0';
    }
    else if (!strcmp(w[0], "drumset"))
    {
      if (words < 2)
      {
	DEBUG_MSG("%s: line %d: No drum set number given\n", name, line);
	goto fail;
      }
      i=atoi(w[1]);
      if (i<0 || i>127)
      {
	DEBUG_MSG("%s: line %d: Drum set must be between 0 and 127\n",
		name, line);
	goto fail;
      }
      if (!master_drumset[i])
      {
	master_drumset[i] = (MidToneBank *) safe_malloc(sizeof(MidToneBank));
	master_drumset[i]->tone = (MidToneBankElement *) safe_malloc(128 * sizeof(MidToneBankElement));
      }
      bank=master_drumset[i];
    }
    else if (!strcmp(w[0], "bank"))
    {
      if (words < 2)
      {
	DEBUG_MSG("%s: line %d: No bank number given\n", name, line);
	goto fail;
      }
      i=atoi(w[1]);
      if (i<0 || i>127)
      {
	DEBUG_MSG("%s: line %d: Tone bank must be between 0 and 127\n",
		name, line);
	goto fail;
      }
      if (!master_tonebank[i])
      {
	master_tonebank[i] = (MidToneBank *) safe_malloc(sizeof(MidToneBank));
	master_tonebank[i]->tone = (MidToneBankElement *) safe_malloc(128 * sizeof(MidToneBankElement));
      }
      bank=master_tonebank[i];
    }
    else
    {
      if ((words < 2) || (*w[0] < '0' || *w[0] > '9'))
      {
	DEBUG_MSG("%s: line %d: syntax error\n", name, line);
	goto fail;
      }
      i=atoi(w[0]);
      if (i<0 || i>127)
      {
	DEBUG_MSG("%s: line %d: Program must be between 0 and 127\n",
		name, line);
	goto fail;
      }
      if (!bank)
      {
	DEBUG_MSG("%s: line %d: Must specify tone bank or drum set before assignment\n",
		name, line);
	goto fail;
      }
      safe_free(bank->tone[i].name);
      bank->tone[i].name=(char *) safe_malloc(strlen(w[1])+1);
      strcpy(bank->tone[i].name,w[1]);
      bank->tone[i].note=bank->tone[i].amp=bank->tone[i].pan=
      bank->tone[i].strip_loop=bank->tone[i].strip_envelope=
      bank->tone[i].strip_tail=-1;

      for (j=2; j<words; j++)
      {
	if (!(cp=strchr(w[j], '=')))
	{
	  DEBUG_MSG("%s: line %d: bad patch option %s\n", name, line, w[j]);
	  goto fail;
	}
	*cp++=0;
	if (!strcmp(w[j], "amp"))
	{
	  k=atoi(cp);
	  if ((k<0 || k>MAX_AMPLIFICATION) || (*cp < '0' || *cp > '9'))
	  {
	    DEBUG_MSG("%s: line %d: amplification must be between 0 and %d\n",
		    name, line, MAX_AMPLIFICATION);
	    goto fail;
	  }
	  bank->tone[i].amp=k;
	}
	else if (!strcmp(w[j], "note"))
	{
	  k=atoi(cp);
	  if ((k<0 || k>127) || (*cp < '0' || *cp > '9'))
	  {
	    DEBUG_MSG("%s: line %d: note must be between 0 and 127\n",
		    name, line);
	    goto fail;
	  }
	  bank->tone[i].note=k;
	}
	else if (!strcmp(w[j], "pan"))
	{
	  if (!strcmp(cp, "center"))
	    k=64;
	  else if (!strcmp(cp, "left"))
	    k=0;
	  else if (!strcmp(cp, "right"))
	    k=127;
	  else
	    k=((atoi(cp)+100) * 100) / 157;
	  if ((k<0 || k>127) || (k==0 && *cp!='-' && (*cp < '0' || *cp > '9')))
	  {
	    DEBUG_MSG("%s: line %d: panning must be left, right, center, or between -100 and 100\n",
		    name, line);
	    goto fail;
	  }
	  bank->tone[i].pan=k;
	}
	else if (!strcmp(w[j], "keep"))
	{
	  if (!strcmp(cp, "env"))
	    bank->tone[i].strip_envelope=0;
	  else if (!strcmp(cp, "loop"))
	    bank->tone[i].strip_loop=0;
	  else
	  {
	    DEBUG_MSG("%s: line %d: keep must be env or loop\n", name, line);
	    goto fail;
	  }
	}
	else if (!strcmp(w[j], "strip"))
	{
	  if (!strcmp(cp, "env"))
	    bank->tone[i].strip_envelope=1;
	  else if (!strcmp(cp, "loop"))
	    bank->tone[i].strip_loop=1;
	  else if (!strcmp(cp, "tail"))
	    bank->tone[i].strip_tail=1;
	  else
	  {
	    DEBUG_MSG("%s: line %d: strip must be env, loop, or tail\n",
		    name, line);
	    goto fail;
	  }
	}
	else
	{
	  DEBUG_MSG("%s: line %d: bad patch option %s\n", name, line, w[j]);
	  goto fail;
	}
      }
    }
  }

  r = 0; /* we're good. */
fail:
  fclose(rcf_fp[rcf_count]);
  rcf_fp[rcf_count] = NULL;
  return r;
}

static int init_alloc_banks (void)
{
  if (setjmp(malloc_env)) {
      DEBUG_MSG("Out of memory\n");
      mid_exit ();
      return -2;
  }

  /* Allocate memory for the standard tonebank and drumset */
  master_tonebank[0] = (MidToneBank *) safe_malloc(sizeof(MidToneBank));
  master_tonebank[0]->tone = (MidToneBankElement *) safe_malloc(128 * sizeof(MidToneBankElement));

  master_drumset[0] = (MidToneBank *) safe_malloc(sizeof(MidToneBank));
  master_drumset[0]->tone = (MidToneBankElement *) safe_malloc(128 * sizeof(MidToneBankElement));

  return 0;
}

static void init_begin_config(const char *cf)
{
  const char *p;

  rcf_fp = (FILE **) safe_malloc(MAX_RCFCOUNT * sizeof(FILE*));
  p = FIND_LAST_DIRSEP(cf);
  if (p != NULL)
      add_to_pathlist(cf, p - cf + 1); /* including DIRSEP */
}

static int init_with_config(const char *cf)
{
  int rc;

  if (setjmp(malloc_env)) {
      DEBUG_MSG("Out of memory\n");
      mid_exit ();
      return -2;
  }

  init_begin_config(cf);
  rc = read_config_file(cf, 0);
  if (rc != 0)
      mid_exit ();
  else
  {
      free(rcf_fp);
      rcf_fp = NULL;
  }
  return rc;
}

int mid_init_no_config(void)
{
  master_tonebank[0] = NULL;
  master_drumset[0] = NULL;
  rcf_fp = NULL;

  return init_alloc_banks();
}

int mid_init(const char *config_file)
{
  int rc;

  rc = mid_init_no_config();
  if (rc != 0)
      return rc;

  if (config_file == NULL || *config_file == '\0')
      return init_with_config(CONFIG_FILE);

  return init_with_config(config_file);
}

static void do_song_load(MidIStream *stream, MidDLSPatches *dlspatches, MidSongOptions *options, MidSong **out)
{
  MidSong *song;
  int i;

  *out = NULL;
  if (stream == NULL)
    return;

  if (setjmp(malloc_env)) {
    DEBUG_MSG("Out of memory\n");
 fail:
    mid_song_free (*out);
    *out = NULL;
    return;
  }

  /* Allocate memory for the song */
  *out = (MidSong *)safe_malloc(sizeof(MidSong));
  song = *out;
  song->dlspatches = dlspatches;

  for (i = 0; i < 128; i++)
  {
    if (master_tonebank[i])
    {
      song->tonebank[i] = (MidToneBank *) safe_malloc(sizeof(MidToneBank));
      song->tonebank[i]->tone = master_tonebank[i]->tone;
    }
    if (master_drumset[i])
    {
      song->drumset[i] = (MidToneBank *) safe_malloc(sizeof(MidToneBank));
      song->drumset[i]->tone = master_drumset[i]->tone;
    }
  }

  song->amplification = DEFAULT_AMPLIFICATION;
  song->voices = DEFAULT_VOICES;
  song->drumchannels = DEFAULT_DRUMCHANNELS;

  song->rate = options->rate;
  song->encoding = 0;
  if ((options->format & 0xFF) == 16)
      song->encoding |= PE_16BIT;
  if (options->format & 0x8000)
      song->encoding |= PE_SIGNED;
  if (options->channels == 1)
      song->encoding |= PE_MONO;
  switch (options->format) {
      case MID_AUDIO_S8:
	  song->write = s32tos8;
	  break;
      case MID_AUDIO_U8:
	  song->write = s32tou8;
	  break;
      case MID_AUDIO_S16LSB:
	  song->write = s32tos16l;
	  break;
      case MID_AUDIO_S16MSB:
	  song->write = s32tos16b;
	  break;
      case MID_AUDIO_U16LSB:
	  song->write = s32tou16l;
	  break;
      default:
	  DEBUG_MSG("Unsupported audio format\n");
	  song->write = s32tou16l;
	  break;
  }

  song->buffer_size = options->buffer_size;
  song->resample_buffer = (sample_t *) safe_malloc(options->buffer_size * sizeof(sample_t));
  song->common_buffer = (sint32 *) safe_malloc(options->buffer_size * 2 * sizeof(sint32));

  song->bytes_per_sample =
	((song->encoding & PE_MONO) ? 1 : 2) *
	((song->encoding & PE_16BIT) ? 2 : 1);

  song->control_ratio = options->rate / CONTROLS_PER_SECOND;
  if (song->control_ratio < 1)
      song->control_ratio = 1;
  else if (song->control_ratio > MAX_CONTROL_RATIO)
      song->control_ratio = MAX_CONTROL_RATIO;

  song->lost_notes = 0;
  song->cut_notes = 0;

  song->events = read_midi_file(stream, song, &(song->groomed_event_count),
				&song->samples);

  /* Make sure everything is okay */
  if (!song->events)
    goto fail;

  song->default_instrument = NULL;
  song->default_program = DEFAULT_PROGRAM;

  if (*def_instr_name)
    set_default_instrument(song, def_instr_name);

  load_missing_instruments(song);
}

MidSong *mid_song_load_dls(MidIStream *stream, MidDLSPatches *dlspatches, MidSongOptions *options)
{
  MidSong *song;
  do_song_load(stream, dlspatches, options, &song);
  return song;
}

MidSong *mid_song_load(MidIStream *stream, MidSongOptions *options)
{
  MidSong *song;
  do_song_load(stream, NULL, options, &song);
  return song;
}

void mid_song_free(MidSong *song)
{
  int i;

  if (!song) return;

  free_instruments(song);
  if (song->ifp)
    fclose(song->ifp);

  for (i = 0; i < 128; i++) {
    safe_free(song->tonebank[i]);
    safe_free(song->drumset[i]);
  }

  safe_free(song->common_buffer);
  safe_free(song->resample_buffer);
  safe_free(song->events);

  free(song);
}

void mid_exit(void)
{
  int i, j;

  if (rcf_fp)
  {
    for (i = 0; i < MAX_RCFCOUNT; i++)
    {
      if (rcf_fp[i])
	fclose(rcf_fp[i]);
    }
    free(rcf_fp);
    rcf_fp = NULL;
  }

  for (i = 0; i < 128; i++)
  {
    if (master_tonebank[i])
    {
      MidToneBankElement *e = master_tonebank[i]->tone;
      if (e != NULL)
      {
	for (j = 0; j < 128; j++)
	{
	  safe_free(e[j].name);
	}
	free(e);
      }
      free(master_tonebank[i]);
      master_tonebank[i] = NULL;
    }
    if (master_drumset[i])
    {
      MidToneBankElement *e = master_drumset[i]->tone;
      if (e != NULL)
      {
	for (j = 0; j < 128; j++)
	{
	  safe_free(e[j].name);
	}
	free(e);
      }
      free(master_drumset[i]);
      master_drumset[i] = NULL;
    }
  }

  free_pathlist();
}

long mid_get_version (void)
{
  return LIBTIMIDITY_VERSION;
}

