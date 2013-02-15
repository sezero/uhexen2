/*
 * xbm_icon.h -- wrapper header file including the correct xbm
 * icon file for different Hexen II flavors
 * $Id: xbm_icon.h,v 1.4 2007-09-07 20:06:03 sezero Exp $
 *
 * Copyright (C) 2005-2012  O.Sezer <sezero@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#if defined(H2W)
#include "../../resource/hexenworld.xbm"
#elif defined(H2MP)
#include "../resource/h2mp.xbm"
#else
#include "../resource/hexen2.xbm"
#endif

