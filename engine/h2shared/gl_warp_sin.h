/* gl_warp_sin.h
 *
 * Copyright (C) 1996-1997  Id Software, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA.
 */

	 0,		 0.19633,	 0.392541,	 0.588517,
	 0.784137,	 0.979285,	 1.17384,	 1.3677,
	 1.56072,	 1.75281,	 1.94384,	 2.1337,
	 2.32228,	 2.50945,	 2.69512,	 2.87916,
	 3.06147,	 3.24193,	 3.42044,	 3.59689,
	 3.77117,	 3.94319,	 4.11282,	 4.27998,
	 4.44456,	 4.60647,	 4.76559,	 4.92185,
	 5.07515,	 5.22538,	 5.37247,	 5.51632,
	 5.65685,	 5.79398,	 5.92761,	 6.05767,
	 6.18408,	 6.30677,	 6.42566,	 6.54068,
	 6.65176,	 6.75883,	 6.86183,	 6.9607,
	 7.05537,	 7.14579,	 7.23191,	 7.31368,
	 7.39104,	 7.46394,	 7.53235,	 7.59623,
	 7.65552,	 7.71021,	 7.76025,	 7.80562,
	 7.84628,	 7.88222,	 7.91341,	 7.93984,
	 7.96148,	 7.97832,	 7.99036,	 7.99759,
	 8,		 7.99759,	 7.99036,	 7.97832,
	 7.96148,	 7.93984,	 7.91341,	 7.88222,
	 7.84628,	 7.80562,	 7.76025,	 7.71021,
	 7.65552,	 7.59623,	 7.53235,	 7.46394,
	 7.39104,	 7.31368,	 7.23191,	 7.14579,
	 7.05537,	 6.9607,	 6.86183,	 6.75883,
	 6.65176,	 6.54068,	 6.42566,	 6.30677,
	 6.18408,	 6.05767,	 5.92761,	 5.79398,
	 5.65685,	 5.51632,	 5.37247,	 5.22538,
	 5.07515,	 4.92185,	 4.76559,	 4.60647,
	 4.44456,	 4.27998,	 4.11282,	 3.94319,
	 3.77117,	 3.59689,	 3.42044,	 3.24193,
	 3.06147,	 2.87916,	 2.69512,	 2.50945,
	 2.32228,	 2.1337,	 1.94384,	 1.75281,
	 1.56072,	 1.3677,	 1.17384,	 0.979285,
	 0.784137,	 0.588517,	 0.392541,	 0.19633,
	 9.79717e-16,	-0.19633,	-0.392541,	-0.588517,
	-0.784137,	-0.979285,	-1.17384,	-1.3677,
	-1.56072,	-1.75281,	-1.94384,	-2.1337,
	-2.32228,	-2.50945,	-2.69512,	-2.87916,
	-3.06147,	-3.24193,	-3.42044,	-3.59689,
	-3.77117,	-3.94319,	-4.11282,	-4.27998,
	-4.44456,	-4.60647,	-4.76559,	-4.92185,
	-5.07515,	-5.22538,	-5.37247,	-5.51632,
	-5.65685,	-5.79398,	-5.92761,	-6.05767,
	-6.18408,	-6.30677,	-6.42566,	-6.54068,
	-6.65176,	-6.75883,	-6.86183,	-6.9607,
	-7.05537,	-7.14579,	-7.23191,	-7.31368,
	-7.39104,	-7.46394,	-7.53235,	-7.59623,
	-7.65552,	-7.71021,	-7.76025,	-7.80562,
	-7.84628,	-7.88222,	-7.91341,	-7.93984,
	-7.96148,	-7.97832,	-7.99036,	-7.99759,
	-8,		-7.99759,	-7.99036,	-7.97832,
	-7.96148,	-7.93984,	-7.91341,	-7.88222,
	-7.84628,	-7.80562,	-7.76025,	-7.71021,
	-7.65552,	-7.59623,	-7.53235,	-7.46394,
	-7.39104,	-7.31368,	-7.23191,	-7.14579,
	-7.05537,	-6.9607,	-6.86183,	-6.75883,
	-6.65176,	-6.54068,	-6.42566,	-6.30677,
	-6.18408,	-6.05767,	-5.92761,	-5.79398,
	-5.65685,	-5.51632,	-5.37247,	-5.22538,
	-5.07515,	-4.92185,	-4.76559,	-4.60647,
	-4.44456,	-4.27998,	-4.11282,	-3.94319,
	-3.77117,	-3.59689,	-3.42044,	-3.24193,
	-3.06147,	-2.87916,	-2.69512,	-2.50945,
	-2.32228,	-2.1337,	-1.94384,	-1.75281,
	-1.56072,	-1.3677,	-1.17384,	-0.979285,
	-0.784137,	-0.588517,	-0.392541,	-0.19633,

