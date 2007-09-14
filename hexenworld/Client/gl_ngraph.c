// gl_ngraph.c

#include "quakedef.h"

static	GLuint	netgraphtexture;	// netgraph texture

#define	NET_GRAPHHEIGHT	32
#define	NET_TIMINGS	256
static	int	packet_latency[NET_TIMINGS];
static	byte	ngraph_texels[NET_GRAPHHEIGHT][NET_TIMINGS];

static void R_LineGraph (int x, int h)
{
	int		i, color;

	if (h == 10000)
		color = 0x6f;	// yellow
	else if (h == 9999)
		color = 0x4f;	// red
	else if (h == 9998)
		color = 0xd0;	// blue
	else
		color = 0xfe;	// white

	if (h > NET_GRAPHHEIGHT)
		h = NET_GRAPHHEIGHT;

	for (i = 0 ; i < h ; i++)
	{
		if (i & 1)
			ngraph_texels[NET_GRAPHHEIGHT - i - 1][x] = 0xff;
		else
			ngraph_texels[NET_GRAPHHEIGHT - i - 1][x] = (byte)color;
	}

	for ( ; i < NET_GRAPHHEIGHT ; i++)
		ngraph_texels[NET_GRAPHHEIGHT - i - 1][x] = (byte)0xff;
}


#if 0	// no users.
extern	byte		*draw_chars;	// 8*8 graphic characters

void Draw_CharToNetGraph (int x, int y, int num)
{
	int		row, col;
	byte	*source;
	int		drawline;
	int		nx;

	row = num >> 4;
	col = num & 15;
	source = draw_chars + (row<<10) + (col<<3);

	for (drawline = 8; drawline; drawline--, y++)
	{
		for (nx = 0 ; nx < 8 ; nx++)
		{
			if (source[nx] != 255)
				ngraph_texels[y][nx+x] = 0x60 + source[nx];
		}
		source += 128;
	}
}
#endif


/*
==============
R_NetGraph
==============
*/
void R_NetGraph (void)
{
	int		a, x, i, y;
	frame_t		*frame;
	int		lost;
	char		st[80];
	unsigned int	ngraph_pixels[NET_GRAPHHEIGHT][NET_TIMINGS];

	for (i = cls.netchan.outgoing_sequence - UPDATE_BACKUP + 1 ;
				i <= cls.netchan.outgoing_sequence ;
				i++)
	{
		frame = &cl.frames[i&UPDATE_MASK];
		if (frame->receivedtime == -1)
			packet_latency[i&255] = 9999;	// dropped
		else if (frame->receivedtime == -2)
			packet_latency[i&255] = 10000;	// choked
		else if (frame->invalid)
			packet_latency[i&255] = 9998;	// invalid delta
		else
			packet_latency[i&255] = (frame->receivedtime - frame->senttime)*20;
	}

	x = 0;
	lost = 0;
	for (a = 0 ; a < NET_TIMINGS ; a++)
	{
		i = (cls.netchan.outgoing_sequence-a) & 255;
		if (packet_latency[i] == 9999)
			lost++;
		R_LineGraph (NET_TIMINGS-1-a, packet_latency[i]);
	}

	// now load the netgraph texture into gl and draw it
	for (y = 0; y < NET_GRAPHHEIGHT; y++)
		for (x = 0; x < NET_TIMINGS; x++)
			ngraph_pixels[y][x] = d_8to24table[ngraph_texels[y][x]];

	x = -((vid.width - 320)>>1);
	y = vid.height - sb_lines - 24 - NET_GRAPHHEIGHT - 1;

	M_DrawTextBox (x, y, NET_TIMINGS/8, NET_GRAPHHEIGHT/8 + 1);
	y += 8;

	sprintf(st, "%3i%% packet loss", lost*100/NET_TIMINGS);
	Draw_String(8, y, st);
	y += 8;

	GL_Bind(netgraphtexture);

	glTexImage2D_fp (GL_TEXTURE_2D, 0, gl_alpha_format,
				NET_TIMINGS, NET_GRAPHHEIGHT, 0, GL_RGBA,
				GL_UNSIGNED_BYTE, ngraph_pixels);

	glTexEnvf_fp(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	x = 8;
	glColor3f_fp (1,1,1);
	glBegin_fp (GL_QUADS);
	glTexCoord2f_fp (0, 0);
	glVertex2f_fp (x, y);
	glTexCoord2f_fp (1, 0);
	glVertex2f_fp (x+NET_TIMINGS, y);
	glTexCoord2f_fp (1, 1);
	glVertex2f_fp (x+NET_TIMINGS, y+NET_GRAPHHEIGHT);
	glTexCoord2f_fp (0, 1);
	glVertex2f_fp (x, y+NET_GRAPHHEIGHT);
	glEnd_fp ();
}


void R_InitNetgraphTexture (void)
{
	netgraphtexture = texture_extension_number;
	texture_extension_number++;
}

