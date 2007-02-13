/* crc.h */

#ifndef __HX2_CRC_H
#define __HX2_CRC_H

void	CRC_Init (unsigned short *crcvalue);
void	CRC_ProcessByte (unsigned short *crcvalue, byte data);
unsigned short	CRC_Value (unsigned short crcvalue);

#define	HASH_TABLE_SIZE		9973
int	COM_Hash (const char *string);

#endif	/* __HX2_CRC_H */

