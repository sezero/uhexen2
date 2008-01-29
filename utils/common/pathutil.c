/*
	pathutil.c
	filename handling utilities

	$Id: pathutil.c,v 1.5 2008-01-29 12:03:10 sezero Exp $
*/


// HEADER FILES ------------------------------------------------------------

#include "q_stdinc.h"
#include "compiler.h"
#include "arch_def.h"
#include "pathutil.h"
#include "cmdlib.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

void DefaultExtension (char *path, const char *extension, size_t len)
{
	char	*src;
//
// if path doesn't have a .EXT, append extension
// (extension should include the .)
//
	if (!*path)
		return;
	src = path + strlen(path) - 1;

	while (*src != PATHSEPERATOR && src != path)
	{
		if (*src == '.')
			return;		// it has an extension
		src--;
	}

	qerr_strlcat(__thisfunc__, __LINE__, path, extension, len);
}

void DefaultPath (char *path, const char *basepath, size_t len)
{
	char	temp[128];

	if (path[0] == '/' || path[0] == '\\')
		return;		// absolute path location
	if (path[0] != '\0' && path[1] == ':')
		return;		// absolute path location, like c:\dir

	qerr_strlcpy(__thisfunc__, __LINE__, temp, path, sizeof(temp));
	qerr_strlcpy(__thisfunc__, __LINE__, path, basepath, len);
	qerr_strlcat(__thisfunc__, __LINE__, path, temp, len);
}

void StripFilename (char *path)
{
	int		length;

	length = (int)strlen(path) - 1;
	while (length > 0 && path[length] != '/' && path[length] != '\\')
		length--;
	path[length] = 0;
}

void StripExtension (char *path)
{
	int		length;

	length = (int)strlen(path) - 1;
	while (length > 0 && path[length] != '.')
	{
		length--;
		if (path[length] == '/' || path[length] == '\\')
			return;		/* no extension */
	}
	if (length)
		path[length] = 0;
}

/*
====================
Extract file parts
// FIXME: should include the slash, otherwise backing to
// an empty path will be wrong when appending a slash
====================
*/
void ExtractFilePath (const char *in, char *out, size_t outsize)
{
	const char	*src;
	size_t		len;

	src = in + strlen(in) - 1;
	if (src <= in)
	{
		*out = '\0';
		return;
	}

	while (src != in && src[-1] != '\\' && src[-1] != '/')
		src--;

	len = src - in;
	if (len >= outsize)
		len = outsize - 1;
	memcpy (out, in, len);
	out[len] = '\0';
}

void ExtractFileBase (const char *in, char *out, size_t outsize)
{
	const char	*src;

	src = in + strlen(in) - 1;
	if (src <= in)
	{
		*out = '\0';
		return;
	}

	while (src != in && src[-1] != '/' && src[-1] != '\\')
		src--;
	while (*src && *src != '.' && outsize)
	{
		if (--outsize)
			*out++ = *src++;
	}
	*out = '\0';
}

void ExtractFileExtension (const char *in, char *out, size_t outsize)
{
	const char	*src;
	size_t		len;

	len = strlen(in);
	if (len < 2)	/* nothing meaningful */
	{
	_noext:
		*out = '\0';
		return;
	}
	src = in + len - 1;
	while (src != in && src[-1] != '.')
		src--;
	if (src == in || strchr(src, '/') != NULL || strchr(src, '\\') != NULL)
		goto _noext;	/* no extension, at not least in the file itself. */

	q_strlcpy (out, src, outsize);
}

