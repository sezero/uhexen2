/*
	config_file.c
	hexen2 launcher config file handling

	$Id: config_file.c,v 1.46 2007-05-31 21:27:26 sezero Exp $

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to:

		Free Software Foundation, Inc.
		51 Franklin St, Fifth Floor,
		Boston, MA  02110-1301, USA
*/

#include "common.h"
#include "launcher_defs.h"
#include "games.h"
#include "config_file.h"

// Default values for the options
char game_basedir[MAX_OSPATH]	= GAME_DATADIR;
int basedir_nonstd	= 0;
int destiny		= DEST_H2;
int opengl_support	= 1;
int fullscreen		= 1;
int resolution		= RES_640;
int use_con		= 1;
int conwidth		= RES_640;
int fxgamma		= 0;
int is8bit		= 0;
int use_fsaa		= 0;
int aasamples		= 2;
int vsync		= 0;
int use_lm1		= 0;
int gl_nonstd		= 0;
char gllibrary[MAX_OSPATH]	= "libGL.so.1";
int sound		= 1;
int sndrate		= 0;
int sndbits		= 1;
int midi		= 1;
int cdaudio		= 1;
int lan			= 1;
int mouse		= 1;
int debug		= 0;
int debug2		= 0;
int use_heap		= 0;
int use_zone		= 0;
int heapsize		= HEAP_DEFAULT;
int zonesize		= ZONE_DEFAULT;
#ifndef DEMOBUILD
int mp_support		= 0;
int h2game		= 0;
int hwgame		= 0;
#endif


static FILE * open_config_file (char *flags)
{
	FILE	*thefile;
	char	*config_file_name =0;

	config_file_name = (char *)calloc(MAX_OSPATH, sizeof(char));

	snprintf (config_file_name, MAX_OSPATH, "%s/%s", userdir, LAUNCHER_CONFIG_FILE);
	thefile = fopen(config_file_name, flags);
	free (config_file_name);
	// NULL check has to be done later
	return thefile;
}


int write_config_file (void)
{
	FILE	*cfg_file;

	cfg_file = open_config_file("w");
	if (cfg_file == NULL)
	{
		fprintf(stderr, " Error: couldn't open config file for writing\n");
		return 1;
	}
	else
	{
		fprintf(cfg_file, "# Hexen II Launcher Options file\n\n");
		fprintf(cfg_file, "# This file has been automatically generated\n\n");

		fprintf(cfg_file, "game_basedir=\"%s\"\n",game_basedir);
		fprintf(cfg_file, "basedir_nonstd=%d\n",basedir_nonstd);
		fprintf(cfg_file, "destiny=%d\n",destiny);
#ifndef DEMOBUILD
		fprintf(cfg_file, "h2game=%d\n",h2game);
		fprintf(cfg_file, "hwgame=%d\n",hwgame);
		fprintf(cfg_file, "mp_support=%d\n",mp_support);
#endif
		fprintf(cfg_file, "opengl_support=%d\n",opengl_support);
		fprintf(cfg_file, "fullscreen=%d\n",fullscreen);
		fprintf(cfg_file, "resolution=%d\n",resolution);
		fprintf(cfg_file, "use_con=%d\n",use_con);
		fprintf(cfg_file, "conwidth=%d\n",conwidth);
		fprintf(cfg_file, "fxgamma=%d\n",fxgamma);
		fprintf(cfg_file, "is8bit=%d\n",is8bit);
		fprintf(cfg_file, "use_fsaa=%d\n",use_fsaa);
		fprintf(cfg_file, "aasamples=%d\n",aasamples);
		fprintf(cfg_file, "vsync=%d\n",vsync);
		fprintf(cfg_file, "use_lm1=%d\n",use_lm1);
		fprintf(cfg_file, "gl_nonstd=%d\n",gl_nonstd);
		fprintf(cfg_file, "gllibrary=\"%s\"\n",gllibrary);
		fprintf(cfg_file, "sound=%d\n",sound);
		fprintf(cfg_file, "sndrate=%d\n",sndrate);
		fprintf(cfg_file, "sndbits=%d\n",sndbits);
		fprintf(cfg_file, "midi=%d\n",midi);
		fprintf(cfg_file, "cdaudio=%d\n",cdaudio);
		fprintf(cfg_file, "lan=%d\n",lan);
		fprintf(cfg_file, "mouse=%d\n",mouse);
		fprintf(cfg_file, "debug=%d\n",debug);
		fprintf(cfg_file, "debug2=%d\n",debug2);
		fprintf(cfg_file, "use_heap=%d\n",use_heap);
		fprintf(cfg_file, "use_zone=%d\n",use_zone);
		fprintf(cfg_file, "heapsize=%d\n",heapsize);
		fprintf(cfg_file, "zonesize=%d\n",zonesize);
	}
	fclose (cfg_file); 
	printf("Options saved successfully.\n");
	return 0;
}

int cfg_read_basedir (void)
{
	FILE	*cfg_file;
	char	buff[1024], *tmp;

	game_basedir[0] = '\0';
	cfg_file = open_config_file("r");
	if (cfg_file == NULL)
	{
		printf("Creating default configuration file...\n");
		return write_config_file();
	}
	else
	{
		int	cnt = 0;

		do {
			memset(buff, 0, sizeof(buff));
			fgets(buff, sizeof(buff), cfg_file);
			if (!feof(cfg_file))
			{
				if (buff[0] == '#')
					continue;
				// remove end-of-line characters
				tmp = buff;
				while (*tmp)
				{
					if (*tmp == '\r' || *tmp == '\n')
						*tmp = '\0';
					tmp++;
				}
				// parse: whitespace isn't tolerated.
				if (strstr(buff, "game_basedir=") == buff)
				{
					size_t		len;
					tmp = buff+13;
					len = strlen(tmp);
					// first and last chars must be quotes
					if (tmp[0] != '\"' || tmp[len-1] != '\"' || len-2 >= sizeof(game_basedir))
						continue;
					memset (game_basedir, 0, sizeof(game_basedir));
					memcpy (game_basedir, tmp+1, len-2);
					++cnt;
				}
				else if (strstr(buff, "basedir_nonstd=") == buff)
				{
					basedir_nonstd = atoi(buff + 15);
					if (basedir_nonstd != 0 && basedir_nonstd != 1)
						basedir_nonstd = 0;
					++cnt;
				}

				if (cnt >= 2)
					break;
			}

		} while (!feof(cfg_file));

		fclose (cfg_file);
	}

	return 0;
}

int read_config_file (void)
{
	FILE	*cfg_file;
	char	buff[1024], *tmp;

	cfg_file = open_config_file("r");
	if (cfg_file == NULL)
	{
		printf("Creating default configuration file...\n");
		return write_config_file();
	}
	else
	{
		printf("Reading configuration file.... ");
		do {
			memset(buff, 0, sizeof(buff));
			fgets(buff, sizeof(buff), cfg_file);
			if (!feof(cfg_file))
			{
				if (buff[0] == '#')
					continue;
				// remove end-of-line characters
				tmp = buff;
				while (*tmp)
				{
					if (*tmp == '\r' || *tmp == '\n')
						*tmp = '\0';
					tmp++;
				}
				// parse: whitespace isn't tolerated.
				if (strstr(buff, "destiny=") == buff)
				{
					destiny = atoi(buff + 8);
					if (destiny != DEST_H2 && destiny != DEST_HW)
						destiny = DEST_H2;
					if (!(gameflags & GAME_HEXENWORLD))
						destiny = DEST_H2;
				}
#ifndef DEMOBUILD
				else if (strstr(buff, "h2game=") == buff)
				{
					h2game = atoi(buff + 7);
					if (h2game < 0 || h2game >= MAX_H2GAMES)
						h2game = 0;
					if (!h2game_names[h2game].available)
						h2game = 0;
					if (!(gameflags & (GAME_REGISTERED|GAME_REGISTERED_OLD)))
						h2game = 0;
				}
				else if (strstr(buff, "hwgame=") == buff)
				{
					hwgame = atoi(buff + 7);
					if (hwgame < 0 || hwgame >= MAX_HWGAMES)
						hwgame = 0;
					if (!hwgame_names[hwgame].available)
						hwgame = 0;
					if (!(gameflags & (GAME_REGISTERED|GAME_REGISTERED_OLD)))
						hwgame = 0;
				}
				else if (strstr(buff, "mp_support=") == buff)
				{
					mp_support = atoi(buff + 11);
					if (mp_support != 0 && mp_support != 1)
						mp_support = 0;
					if (!(gameflags & GAME_PORTALS && gameflags & (GAME_REGISTERED|GAME_REGISTERED_OLD)))
						mp_support = 0;
				}
#endif
				else if (strstr(buff, "opengl_support=") == buff)
				{
					opengl_support = atoi(buff + 15);
					if (opengl_support != 0 && opengl_support != 1)
						opengl_support = 1;
				}
				else if (strstr(buff, "fullscreen=") == buff)
				{
					fullscreen = atoi(buff + 11);
					if (fullscreen != 0 && fullscreen != 1)
						fullscreen = 1;
				}
				else if (strstr(buff, "resolution=") == buff)
				{
					resolution = atoi(buff + 11);
					if (resolution < 0 || resolution >= RES_MAX)
						resolution = RES_640;
				}
				else if (strstr(buff, "conwidth=") == buff)
				{
					conwidth = atoi(buff + 9);
					if (conwidth < 0 || conwidth >= RES_MAX)
						conwidth = RES_640;
					if (opengl_support && conwidth > resolution)
						conwidth = resolution;
				}
				else if (strstr(buff, "use_con=") == buff)
				{
					use_con = atoi(buff + 8);
					if (use_con != 0 && use_con != 1)
						use_con = 1;
				}
				else if (strstr(buff, "fxgamma=") == buff)
				{
					fxgamma = atoi(buff + 8);
					if (fxgamma != 0 && fxgamma != 1)
						fxgamma = 0;
				}
				else if (strstr(buff, "is8bit=") == buff)
				{
					is8bit = atoi(buff + 7);
					if (is8bit != 0 && is8bit != 1)
						is8bit = 0;
				}
				else if (strstr(buff, "use_fsaa=") == buff)
				{
					use_fsaa = atoi(buff + 9);
					if (use_fsaa != 0 && use_fsaa != 1)
						use_fsaa = 0;
				}
				else if (strstr(buff, "aasamples=") == buff)
				{
					aasamples = atoi(buff + 10);
					if (aasamples < 0 || aasamples > 4 || (aasamples % 2))
						aasamples = 0;
				}
				else if (strstr(buff, "vsync=") == buff)
				{
					vsync = atoi(buff + 6);
					if (vsync != 0 && vsync != 1)
						vsync = 0;
				}
				else if (strstr(buff, "use_lm1=") == buff)
				{
					use_lm1 = atoi(buff + 8);
					if (use_lm1 != 0 && use_lm1 != 1)
						use_lm1 = 0;
				}
				else if (strstr(buff, "gl_nonstd=") == buff)
				{
					gl_nonstd = atoi(buff + 10);
					if (gl_nonstd != 0 && gl_nonstd != 1)
						gl_nonstd = 0;
				}
				else if (strstr(buff, "gllibrary=") == buff)
				{
					size_t		len;
					tmp = buff+10;
					len = strlen(tmp);
					// first and last chars must be quotes
					if (tmp[0] != '\"' || tmp[len-1] != '\"' || len-2 >= sizeof(gllibrary))
						continue;
					memset (gllibrary, 0, sizeof(gllibrary));
					memcpy (gllibrary, tmp+1, len-2);
				}
				else if (strstr(buff, "sound=") == buff)
				{
					sound = atoi(buff + 6);
					if (sound < 0 || sound >= MAX_SOUND)
						sound = 1;
				}
				else if (strstr(buff, "sndrate=") == buff)
				{
					sndrate = atoi(buff + 8);
					if (sndrate < 0 || sndrate >= MAX_RATES)
						sndrate = 0;
				}
				else if (strstr(buff, "sndbits=") == buff)
				{
					sndbits = atoi(buff + 8);
					if (sndbits != 0 && sndbits != 1)
						sndbits = 1;
				}
				else if (strstr(buff, "midi=") == buff)
				{
					midi = atoi(buff + 5);
					if (midi != 0 && midi != 1)
						midi = 1;
				}
				else if (strstr(buff, "cdaudio=") == buff)
				{
					cdaudio = atoi(buff + 8);
					if (cdaudio != 0 && cdaudio != 1)
						cdaudio = 1;
				}
				else if (strstr(buff, "lan=") == buff)
				{
					lan = atoi(buff + 4);
					if (lan != 0 && lan != 1)
						lan = 1;
				}
				else if (strstr(buff, "mouse=") == buff)
				{
					mouse = atoi(buff + 6);
					if (mouse != 0 && mouse != 1)
						mouse = 1;
				}
				else if (strstr(buff, "debug=") == buff)
				{
					debug = atoi(buff + 6);
					if (debug != 0 && debug != 1)
						debug = 0;
				}
				else if (strstr(buff, "debug2=") == buff)
				{
					debug2 = atoi(buff + 7);
					if (debug2 != 0 && debug2 != 1)
						debug2 = 0;
				}
				else if (strstr(buff, "use_heap=") == buff)
				{
					use_heap = atoi(buff + 9);
					if (use_heap != 0 && use_heap != 1)
						use_heap = 0;
				}
				else if (strstr(buff, "use_zone=") == buff)
				{
					use_zone = atoi(buff + 9);
					if (use_zone != 0 && use_zone != 1)
						use_zone = 0;
				}
				else if (strstr(buff, "heapsize=") == buff)
				{
					heapsize = atoi(buff + 9);
					if (heapsize <  HEAP_MINSIZE || heapsize > HEAP_MAXSIZE)
						heapsize = HEAP_DEFAULT;
				}
				else if (strstr(buff, "zonesize=") == buff)
				{
					zonesize = atoi(buff + 9);
					if (zonesize < ZONE_MINSIZE || zonesize > ZONE_MAXSIZE)
						zonesize = ZONE_DEFAULT;
				}
			}

		} while (!feof(cfg_file));

		printf ("OK.\n");
		fclose (cfg_file);
	}

	return 0;
}

