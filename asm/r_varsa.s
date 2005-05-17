//
// r_varsa.s
//

#include "asm_i386.h"
#include "quakeasm.h"
#include "asm_draw.h"
#include "d_ifacea.h"

#if id386

	.data

//-------------------------------------------------------
// ASM-only variables
//-------------------------------------------------------
.globl	float_1, float_particle_z_clip, float_point5
.globl	float_minus_1, float_0
float_0:		.single	0.0
float_1:		.single	1.0
float_minus_1:	.single	-1.0
float_particle_z_clip:	.single	PARTICLE_Z_CLIP
float_point5:	.single	0.5

.globl	fp_16, fp_64k, fp_1m, fp_64kx64k
.globl	fp_1m_minus_1
.globl	fp_8 
fp_1m:			.single	1048576.0
fp_1m_minus_1:	.single	1048575.0
fp_64k:			.single	65536.0
fp_8:			.single	8.0
fp_16:			.single	16.0
fp_64kx64k:		.long	0x4f000000	// (float)0x8000*0x10000


.globl	FloatZero, Float2ToThe31nd, FloatMinus2ToThe31nd
FloatZero:				.long	0
Float2ToThe31nd:		.long	0x4f000000
FloatMinus2ToThe31nd:	.long	0xcf000000

.globl	C(r_bmodelactive)
C(r_bmodelactive):	.long	0

#endif	// id386

