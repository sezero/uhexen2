/*
	pathutil.c
	filename handling utilities

	$Id: pathutil.c,v 1.4 2007-12-14 16:41:16 sezero Exp $
*/


// HEADER FILES ------------------------------------------------------------

#include "q_stdinc.h"
#include "compiler.h"
#include "arch_def.h"
#include "pathutil.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

void DefaultExtension (char *path, const char *extension)
{
	char	*src;
//
// if path doesn't have a .EXT, append extension
// (extension should include the .)
//
	src = path + strlen(path) - 1;

	while (*src != PATHSEPERATOR && src != path)
	{
		if (*src == '.')
			return;	// it has an extension
		src--;
	}

	strcat (path, extension);
}

void DefaultPath (char *path, const char *basepath)
{
	char	temp[128];

	if (path[0] == PATHSEPERATOR)
		return;		// absolute path location

	strcpy (temp,path);
	strcpy (path,basepath);
	strcat (path,temp);
}

void StripFilename (char *path)
{
	int		length;

	length = strlen(path)-1;
	while (length > 0 && path[length] != PATHSEPERATOR)
		length--;
	path[length] = 0;
}

void StripExtension (char *path)
{
	int		length;

	length = strlen(path)-1;
	while (length > 0 && path[length] != '.')
	{
		length--;
		if (path[length] == '/' || path[length] == '\\')
			return;		// no extension
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
void ExtractFilePath (const char *path, char *dest)
{
	const char	*src;

	src = path + strlen(path) - 1;

//
// back up until a \ or the start
//
//	while (src != path && src[-1] != PATHSEPERATOR)
	while (src != path && src[-1] != '\\' && src[-1] != '/')
		src--;

	memcpy (dest, path, src-path);
	dest[src-path] = 0;
}

void ExtractFileBase (const char *path, char *dest)
{
	const char	*src;

	src = path + strlen(path) - 1;

//
// back up until a \ or the start
//
	while (src != path && src[-1] != PATHSEPERATOR)
		src--;

	while (*src && *src != '.')
	{
		*dest++ = *src++;
	}
	*dest = 0;
}

void ExtractFileExtension (const char *path, char *dest)
{
	const char	*src;

	src = path + strlen(path) - 1;

//
// back up until a . or the start
//
	while (src != path && src[-1] != '.')
		src--;
	if (src == path)
	{
		*dest = 0;	// no extension
		return;
	}

	strcpy (dest,src);
}

