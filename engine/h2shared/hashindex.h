/*
 * Fast hash table for indexes and arrays
 * from the Doom 3 GPL Source Code source adapted for uhexen2.
 *
 * Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __HASHINDEX_H__
#define __HASHINDEX_H__

typedef struct hashindex_s
{
	int hashSize;
	int *hash;
	int *indexChain;
	int hashMask;
} hashindex_t;

void Hash_Allocate(hashindex_t *hi, int hashSize);
void Hash_Free(hashindex_t *hi);
void Hash_Add(hashindex_t *hi, int key, int index);
void Hash_Remove(hashindex_t *hi, int key, int index);
void Hash_Clear(hashindex_t *hi);

/*
================
Hash_First

get the first index from the hash, returns -1 if empty hash entry
================
*/
static inline int Hash_First(hashindex_t *hi, int key)
{
	return hi->hash[key & hi->hashMask];
}

/*
================
Hash_Next

get the next index from the hash, returns -1 if at the end of the hash chain
================
*/
static inline int Hash_Next(hashindex_t *hi, int index)
{
	//assert(index >= 0 && index < hi->indexSize);
	return hi->indexChain[index];
}

/*
================
Hash_GenerateKeyString
================
*/
static inline int Hash_GenerateKeyString(hashindex_t *hi, const char *string, qboolean caseSensitive)
{
	int i, hash = 0;
	if (caseSensitive) {
		for (i = 0; *string != '\0'; i++) {
			hash += (*string++) * (i + 119);
		}
	} else {
		for (i = 0; *string != '\0'; i++) {
			hash += q_tolower(*string++) * (i + 119);
		}
	}
	return hash & hi->hashMask;
}

/*
================
Hash_GenerateKeyInt
================
*/
static inline int Hash_GenerateKeyInt(hashindex_t *hi, int n)
{
	return n & hi->hashMask;
}

#endif /* !__HASHINDEX_H__ */
