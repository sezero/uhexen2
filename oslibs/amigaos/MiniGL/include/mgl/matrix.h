/*
 * $Id: matrix.h,v 1.1.1.1 2000/04/07 19:44:51 tfrieden Exp $
 *
 * $Date: 2000/04/07 19:44:51 $
 * $Revision: 1.1.1.1 $
 *
 * (C) 1999 by Hyperion
 * All rights reserved
 *
 * This file is part of the MiniGL library project
 * See the file Licence.txt for more details
 *
 */

#ifndef _MATRIX_H
#define _MATRIX_H

typedef struct Matrix_t
{
	float v[16];
	int flags;                  // Matrix flags
	struct Matrix_t *Inverse;   // optional inverse
	int _pad_[2]; //16 byte align
} Matrix;

#define OF_11 0
#define OF_12 4
#define OF_13 8
#define OF_14 12

#define OF_21 1
#define OF_22 5
#define OF_23 9
#define OF_24 13

#define OF_31 2
#define OF_32 6
#define OF_33 10
#define OF_34 14

#define OF_41 3
#define OF_42 7
#define OF_43 11
#define OF_44 15

#define  MGLMAT_IDENTITY		0x01
#define  MGLMAT_ROTATION		0x02
#define  MGLMAT_TRANSLATION		0x04
#define  MGLMAT_UNIFORM_SCALE		0x08
#define  MGLMAT_GENERAL_SCALE		0x10
#define  MGLMAT_PERSPECTIVE		0x20
#define  MGLMAT_GENERAL		0x40
#define  MGLMAT_GENERAL_3D		0x80
#define  MGLMAT_ORTHO			0x100
#define  MGLMAT_0001			0x200
//surgeon: for 0001*persp - common in spherical mapping
#define  MGLMAT_00NEG10		0x400
//common special case rotations
#define  MGLMAT_ROT001		0x800
#define  MGLMAT_ROT010		0x1000
#define  MGLMAT_ROT100		0x2000
//surgeon: unknown layout
#define  MGLMAT_UNKNOWN		0x4000

#define MGLMASK_0001 (MGLMAT_IDENTITY|MGLMAT_ROTATION|MGLMAT_TRANSLATION|MGLMAT_UNIFORM_SCALE|MGLMAT_GENERAL_SCALE|MGLMAT_ORTHO|MGLMAT_0001|MGLMAT_ROT001|MGLMAT_ROT010|MGLMAT_ROT100)
//ortho and special case rotations added by surgeon

//surgeon: mask for !MLMASK_0001
#define MGLMASK_NONE	(MGLMAT_PERSPECTIVE|MGLMAT_UNKNOWN)

#endif
