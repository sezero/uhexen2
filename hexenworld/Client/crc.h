/*
	crc.h
	crc functions

	$Id: crc.h,v 1.5 2007-03-14 21:03:31 sezero Exp $
*/

#ifndef __HX2_CRC_H
#define __HX2_CRC_H

void CRC_Init(unsigned short *crcvalue);
void CRC_ProcessByte(unsigned short *crcvalue, byte data);
void CRC_ProcessBlock (byte *start, unsigned short *crcvalue, int count);
unsigned short CRC_Value(unsigned short crcvalue);
unsigned short CRC_Block (byte *start, int count);

#endif	/* __HX2_CRC_H */

