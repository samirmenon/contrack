// Wild Magic Source Code
// David Eberly
// http://www.geometrictools.com
// Copyright (c) 1998-2008
//
// This library is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2.1 of the License, or (at
// your option) any later version.  The license is available for reading at
// either of the locations:
//     http://www.gnu.org/copyleft/lgpl.html
//     http://www.geometrictools.com/License/WildMagicLicense.pdf
//
// Version: 4.0.2 (2008/09/02)

#ifndef WM4UTILS_H
#define WM4UTILS_H

namespace Wm4
{

/* class WM4_FOUNDATION_ITEM System */
/* { */
/* public: */
/* }; */
// Allocation and deallocation of 2D arrays.  On deallocation, the array
// pointer is set to null.
template <class T> void Allocate (int iCols, int iRows, T**& raatArray);
template <class T> static void Deallocate (T**& raatArray);
// Allocation and deallocation of 3D arrays.  On deallocation, the array
// pointer is set to null.
template <class T> void Allocate (int iCols, int iRows, int iSlices,
    T***& raaatArray);
template <class T> void Deallocate (T***& raaatArray);

#include "Wm4/Wm4Utils.inl"
}

#endif
