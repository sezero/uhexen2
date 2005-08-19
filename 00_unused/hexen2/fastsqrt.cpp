// fastsqrt.cpp
//
// 
#include <math.h>

extern "C" {
float _cdecl fastlog(float x);
float _cdecl fastsqrt(float x);
float _cdecl fastinvsqrt(float x);
}

inline long int AsInteger(float f) {return *(long int *) &f;};
inline float AsFloat(long int i) {return *(float *) &i;};
const long int OneAsInteger = AsInteger(1.0f);
const long int OneAsIntegerShift = (OneAsInteger>>1);
const long int OnePlusAs = OneAsInteger + OneAsIntegerShift;
const float ScaleUp = float(0x00800000);
const float ScaleDown = 1./ScaleUp;

float _cdecl fastlog(float x)
	{
		return float (AsInteger(x) - OneAsInteger) * ScaleDown; 
	}

#pragma optimize( "g", off )
float _cdecl fastsqrt(float x)
 	{
		float y=AsFloat((AsInteger(x)>>1) + OneAsIntegerShift);
		return (y*y+x)/(2*y);
	}
#pragma optimize( "", on ) 

float _cdecl fastinvsqrt(float x)
	{
		float y = AsFloat(OnePlusAs - (AsInteger(x)>>1));
		return y*(1.47 - .47 * x*y*y);
	}
