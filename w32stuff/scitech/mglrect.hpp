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
* Description:  Header file for an integer rectangle class.
*
*
****************************************************************************/

#ifndef __MGLRECT_HPP
#define __MGLRECT_HPP

#ifndef __MGRAPH_H
#error  Please include MGRAPH.H, not MGLRECT.HPP directly
#endif

#ifndef __MGLPOINT_HPP
#include "mglpoint.hpp"
#endif

/*--------------------------- Class Definition ----------------------------*/

#ifdef  __INTEL__

// Pa3PyX: done differently in my compiler
#pragma pack(push,1)
#pragma pack(1)                         /* Pack structures to byte granularity          */
#endif

//---------------------------------------------------------------------------
// The following defines an integer class of rectangles. Note that we do
// not overload the = operator, nor do we provide a copy constructor for
// the class. Rectangles are simple classes, and the compiler does a better
// job of copying them for us if we don't provide these member functions.
// Copy constructors and overloaded = operators should only really be used
// when the class contains pointers.
//
// A point to note about the definition of rectangles used here. The bottom
// right point is _NOT_ included in the rectangle's definition. This means
// that a rectangle (10,10) - (20,20) includes all the points from 10,10
// thru to 19,19 inclusive. This is consistent with the way Quickdraw on
// the Mac defines rectangles and makes processing a little easier in some
// cases.
//---------------------------------------------------------------------------

class MGLRect {
public:
        MGLPoint        topLeft;                // Top left point of rectangle
        MGLPoint        botRight;               // Bottom right point of rectangle

                        // Default constructor (does nothing)
        inline  MGLRect() {};

                        // Constructor given a set of integers
        inline  MGLRect(int left,int top,int right,int bottom) :
                                topLeft(left,top),botRight(right,bottom) {};

                        // Constructor given two points
        inline  MGLRect(const MGLPoint& topLeft,const MGLPoint& botRight) :
                                topLeft(topLeft),botRight(botRight) {};

                        // Constructor given an MGL C style rectangle
        inline  MGLRect(const rect_t& r) :
                                topLeft(r.left,r.top),botRight(r.right,r.bottom) {};

                        // Overloaded cast to an MGL C style rectangle
        inline  operator rect_t& () const       { return (rect_t&)topLeft; };

                        // Forcible cast to an MGL C style rectangle
        inline  rect_t& r() const       { return (rect_t&)topLeft; };

                        // Set the rectangle coordinates
        inline  MGLRect& set(int left,int top,int right,int bottom)
                        { topLeft.x = left; topLeft.y = top;
                          botRight.x = right; botRight.y = bottom; return *this; };
        inline  MGLRect& set(const MGLPoint& tl,const MGLPoint& br)
                        { topLeft = tl; botRight = br; return *this; };

                        // Methods to access the left, top, right and bottom coords.
        inline  int& left(void)                                 { return topLeft.x; };
        inline  int& top(void)                                  { return topLeft.y; };
        inline  int& right(void)                                { return botRight.x; };
        inline  int& bottom(void)                               { return botRight.y; };
        inline  const int& left(void) const             { return topLeft.x; };
        inline  const int& top(void) const              { return topLeft.y; };
        inline  const int& right(void) const    { return botRight.x; };
        inline  const int& bottom(void) const   { return botRight.y; };

                        // Return the height and width
        inline  int width(void) const           { return right() - left(); };
        inline  int height(void) const          { return bottom() - top(); };

                        // Method to move a rectangle to a specified position
        inline  MGLRect& moveTo(int x,int y);
        inline  MGLRect& moveTo(const MGLPoint& p)      { return moveTo(p.x,p.y); };

                        // Method to offset a rectangle by a specified amount
        inline  MGLRect& offset(int dx,int dy);
        inline  MGLRect& offset(const MGLPoint& d)      { return offset(d.x,d.y); };
        inline  MGLRect offsetBy(int dx,int dy) const;
        inline  MGLRect offsetBy(const MGLPoint& d) const { return offsetBy(d.x,d.y); };

                        // Method to inset a rectangle by a specified amount
        inline  MGLRect& inset(int dx,int dy);
        inline  MGLRect& inset(const MGLPoint& d)       { return inset(d.x,d.y); };
        inline  MGLRect insetBy(int dx,int dy) const;
        inline  MGLRect insetBy(const MGLPoint& d) const { return insetBy(d.x,d.y); };

                        // Method to inflate a rectangle by a specified amount
        inline  MGLRect& inflate(int dx,int dy)         { return inset(-dx,-dy); };
        inline  MGLRect& inflate(const MGLPoint& d)     { return inflate(d.x,d.y); };
        inline  MGLRect inflatedBy(int dx,int dy) const { return insetBy(-dx,-dy); };
        inline  MGLRect inflatedBy(const MGLPoint& d) const { return inflatedBy(d.x,d.y); };

                        // Method's to determine if a point is within the rectangle
        inline  ibool includes(int x,int y) const;
        inline  ibool includes(const MGLPoint& p) const { return includes(p.x,p.y); };

                        // Methods to find the union of two rectangles
                        MGLRect& makeUnion(int left,int top,int right,int bottom);
        inline  MGLRect operator + (const MGLRect& p) const;
        inline  MGLRect& operator += (const MGLRect& p);

                        // Methods to expand a rectangle to include a specified point
                        MGLRect& expand(int x,int y);
        inline  MGLRect operator + (const MGLPoint& p) const;
        inline  MGLRect& operator += (const MGLPoint& p);

                        // Method to find the intersection of two rectangles
                        MGLRect& makeIntersect(int left,int top,int right,int bottom);
        inline  MGLRect operator & (const MGLRect& p) const;
        inline  MGLRect& operator &= (const MGLRect& p);

                        // Method to determine if two rectangles intersect
                        ibool intersect(int left,int top,int right,int bottom) const;
        inline  ibool intersect(const MGLRect& r) const;

                        // Method to create an empty rectangle
        inline  MGLRect& empty(void);

                        // Method to determine if a rectangle is empty
        inline  ibool isEmpty(void) const;

                        // Method to determine if two rectangles are equal
        inline  ibool operator == (const MGLRect& r) const;

                        // Method to determine if two rectangles are not equal
        inline  ibool operator != (const MGLRect& r) const;

                        // Friend to display the rectangle on a stream
        friend  ostream& operator << (ostream& o,const MGLRect& r);
        };

/*------------------------ Inline member functions ------------------------*/

inline MGLRect& MGLRect::empty(void)
{
        topLeft.x = topLeft.y = botRight.x = botRight.y = 0;
        return *this;
}

inline ibool MGLRect::isEmpty(void) const
{
        return (botRight.y <= topLeft.y || botRight.x <= topLeft.x);
}

inline MGLRect& MGLRect::moveTo(int x,int y)
{
        return offset(x-left(),y-top());
}

inline MGLRect MGLRect::offsetBy(int dx,int dy) const
{
        return MGLRect(topLeft.x + dx,topLeft.y + dy,
                botRight.x + dx,botRight.y + dy);
}

inline MGLRect& MGLRect::offset(int dx,int dy)
{
        topLeft.x += dx;        botRight.x += dx;
        topLeft.y += dy;        botRight.y += dy;
        return *this;
}

inline MGLRect& MGLRect::inset(int dx,int dy)
{
        topLeft.x += dx;        botRight.x -= dx;
        topLeft.y += dy;        botRight.y -= dy;
        return *this;
}

inline MGLRect MGLRect::insetBy(int dx,int dy) const
{
        return MGLRect(topLeft.x + dx,topLeft.y + dy,
                botRight.x - dx,botRight.y - dy);
}

inline ibool MGLRect::includes(int x,int y) const
{
        return (x >= topLeft.x && x < botRight.x &&
                        y >= topLeft.y && y < botRight.y);
}

inline MGLRect MGLRect::operator + (const MGLRect& r) const
{
        return MGLRect(r).makeUnion(left(),top(),right(),bottom());
}

inline MGLRect MGLRect::operator + (const MGLPoint& p) const
{
        return MGLRect(*this).expand(p.x,p.y);
}

inline MGLRect MGLRect::operator & (const MGLRect& r) const
{
        return MGLRect(r).makeIntersect(left(),top(),right(),bottom());
}

inline MGLRect& MGLRect::operator += (const MGLRect& r)
{
        return makeUnion(r.left(),r.top(),r.right(),r.bottom());
}

inline MGLRect& MGLRect::operator += (const MGLPoint& p)
{
        return expand(p.x,p.y);
}

inline MGLRect& MGLRect::operator &= (const MGLRect& r)
{
        return makeIntersect(r.left(),r.top(),r.right(),r.bottom());
}

inline ibool MGLRect::intersect(const MGLRect& r) const
{
        return intersect(r.left(),r.top(),r.right(),r.bottom());
}

inline ibool MGLRect::operator == (const MGLRect& r) const
{
        return (topLeft == r.topLeft && botRight == r.botRight);
}

inline ibool MGLRect::operator != (const MGLRect& r) const
{
        return (topLeft != r.topLeft || botRight != r.botRight);
}

#ifdef  __INTEL__

// Pa3PyX: done differently in my compiler
#pragma pack(pop)
//#pragma pack()
#endif

#endif  // __MGLRECT_HPP
