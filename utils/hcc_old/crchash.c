/*
	crchash.c
	hash functions for HCC

	$Id$
*/

#include "crchash.h"
#include "crc.c"

/*
==============
COM_Hash

==============
*/
int COM_Hash (const char *key)
{
	int		i;
	int		length;
	const char	*keyBack;
	unsigned short	hash;

	length = strlen (key);
	keyBack = key + length - 1;
	hash = CRC_INIT_VALUE;

	if (length > 20)
	{
		length = 20;
	}

	for (i = 0; i < length; i++)
	{
		hash = (hash<<8)^crctable[(hash>>8)^*key++];
		if (++i >= length)
		{
			break;
		}
		hash = (hash<<8)^crctable[(hash>>8)^*keyBack--];
	}

	return hash % HASH_TABLE_SIZE;
}

