/****************************************************************************
*
*                                               MegaGraph Graphics Library
*
*  ========================================================================
*
*    The contents of this file are subject to the SciTech MGL Public
*    License Version 1.0 (the "License"); you may not use this file
*    except in compliance with the License. You may obtain a copy of
*    the License at http://www.scitechsoft.com/mgl-license.txt
*
*    Software distributed under the License is distributed on an
*    "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
*    implied. See the License for the specific language governing
*    rights and limitations under the License.
*
*    The Original Code is Copyright (C) 1991-1998 SciTech Software, Inc.
*
*    The Initial Developer of the Original Code is SciTech Software, Inc.
*    All Rights Reserved.
*
*  ========================================================================
*
*
* Language:             C++ 3.0
* Environment:  any
*
* Description:  Header file for the integer point class.
*
*
****************************************************************************/

#ifndef __MGLPOINT_HPP
#define __MGLPOINT_HPP

#ifndef __MGRAPH_H
#error  Please include MGRAPH.H, not MGLPOINT.HPP directly
#endif

#ifndef __IOSTREAM_H
#include <iostream.h>
#endif

/*--------------------------- Class Definition ----------------------------*/

#ifdef  __INTEL__
// Pa3PyX: done differently in my compiler
#pragma pack(push,1)
//#pragma pack(1)                         /* Pack structures to byte granularity          */
#endif

//---------------------------------------------------------------------------
// The following defines a 2d integer point type, and defines a number of
// useful inline operations on these points. Use the defined methods to
// convert between 2d real points and 2d integer points. The methods provided
// will ensure that the results obtained are consistent.
//---------------------------------------------------------------------------

class MGLPoint {
public:
        int     x,y;                    // Coordinates of the point

                        // Default constructor (does nothing)
        inline  MGLPoint()      {};

                        // Constructor given 2 integers
        inline  MGLPoint(int x1,int y1) { x = x1; y = y1; };

                        // Constructor given a point
        inline  MGLPoint(const MGLPoint& p)     { x = p.x; y = p.y; };

                        // Constructor given an MGL C style point
        inline  MGLPoint(const point_t& p)      { x = p.x; y = p.y; };

                        // Overloaded cast to an MGL C style point
        inline  operator point_t& () const      { return (point_t&)x; };

                        // Forcible cast to an MGL C style point
        inline  point_t& p() const      { return (point_t&)x; };

                        // Assignment operator given a point
        inline  MGLPoint& operator = (const MGLPoint& p)
                        {
                                x = p.x; y = p.y; return *this;
                        };

                        // Standard arithmetic operators for 2d integer points
        friend  MGLPoint operator + (const MGLPoint& v1,const MGLPoint& v2);
        friend  MGLPoint operator - (const MGLPoint& v1,const MGLPoint& v2);
        friend  MGLPoint operator * (const MGLPoint& v1,const MGLPoint& v2);
        friend  MGLPoint operator * (const MGLPoint& v,int s);
        friend  MGLPoint operator * (int s,const MGLPoint& v);
        friend  MGLPoint operator / (const MGLPoint& v,int s);

                        // Faster methods to add and multiply
        inline  MGLPoint& operator += (const MGLPoint& v);
        inline  MGLPoint& operator -= (const MGLPoint& v);
        inline  MGLPoint& operator *= (const MGLPoint& v);
        inline  MGLPoint& operator *= (int s);
        inline  MGLPoint& operator /= (int s);

                        // Methods to negate a vector
        inline  MGLPoint operator - () const;
        inline  MGLPoint& negate();                     // Faster

                        // Method to determine if a point is zero
        inline  ibool isZero() const;

                        // Method to determine if two points are equal
        inline  ibool operator == (const MGLPoint& p) const;

                        // Method to determine if two points are not equal
        inline  ibool operator != (const MGLPoint& p) const;

                        // Friend to display the contents of a 2d point
        friend  ostream& operator << (ostream& s,const MGLPoint& p);
        };

/*------------------------- Inline member functions -----------------------*/

//---------------------------------------------------------------------------
// Standard arithmetic operators for integer points.
//---------------------------------------------------------------------------

inline MGLPoint operator + (const MGLPoint& p1,const MGLPoint& p2)
{
        return MGLPoint(p1.x + p2.x, p1.y + p2.y);
}

inline MGLPoint operator - (const MGLPoint& p1,const MGLPoint& p2)
{
        return MGLPoint(p1.x - p2.x, p1.y - p2.y);
}

inline MGLPoint operator * (const MGLPoint& p1,const MGLPoint& p2)
{
        return MGLPoint(p1.x * p2.x, p1.y * p2.y);
}

inline MGLPoint operator * (const MGLPoint& p1,int s)
{
        return MGLPoint(p1.x * s, p1.y * s);
}

inline MGLPoint operator * (int s,const MGLPoint& p1)
{
        return MGLPoint(p1.x * s, p1.y * s);
}

inline MGLPoint operator / (const MGLPoint& p1,int s)
{
        return MGLPoint(p1.x / s, p1.y / s);
}

//---------------------------------------------------------------------------
// Faster methods to add and multiply integer points.
//---------------------------------------------------------------------------

inline MGLPoint& MGLPoint::operator += (const MGLPoint& p)
{
        x += p.x;
        y += p.y;
        return *this;
}

inline MGLPoint& MGLPoint::operator -= (const MGLPoint& p)
{
        x -= p.x;
        y -= p.y;
        return *this;
}

inline MGLPoint& MGLPoint::operator *= (const MGLPoint& p)
{
        x *= p.x;
        y *= p.y;
        return *this;
}

inline MGLPoint& MGLPoint::operator *= (int s)
{
        x *= s;
        y *= s;
        return *this;
}

inline MGLPoint& MGLPoint::operator /= (int s)
{
        x /= s;
        y /= s;
        return *this;
}

//---------------------------------------------------------------------------
// Methods to negate points.
//---------------------------------------------------------------------------

inline MGLPoint MGLPoint::operator - () const
{
        return MGLPoint(-x,-y);
}

inline MGLPoint& MGLPoint::negate()
{
        x = -x;
        y = -y;
        return *this;
}

//---------------------------------------------------------------------------
// Miscellaneous operations.
//---------------------------------------------------------------------------

inline ibool MGLPoint::isZero() const
{
        return (x == 0 && y == 0);
}

inline ibool MGLPoint::operator == (const MGLPoint& p) const
{
        return (x == p.x && y == p.y);
}

inline ibool MGLPoint::operator != (const MGLPoint& p) const
{
        return (x != p.x || y != p.y);
}

#ifdef  __INTEL__

// Pa3PyX: done differently in my compiler
#pragma pack(pop)
//#pragma pack()
#endif

#endif  // __MGLPOINT_HPP
