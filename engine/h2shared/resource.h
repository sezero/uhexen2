// resource.h -- Windows resource header
// $Id: resource.h,v 1.7 2007-10-31 18:56:37 sezero Exp $

#ifndef __HX2_RESOURCE_H
#define __HX2_RESOURCE_H

#define IDS_STRING1	1
#define IDI_ICON2	1
#define IDD_DIALOG1	108
#define IDD_PROGRESS	109
#define IDB_HXBITMAP	112
#define IDC_PROGRESS	1000

// Icon file to use
//
#if defined(H2W)
#define RES_ICONFILE	"../../resource/hexenworld.ico"
#elif defined(H2MP)
#define RES_ICONFILE	"../resource/h2mp.ico"
#else
#define RES_ICONFILE	"../resource/hexen2.ico"
#endif

// String
//
#if defined(H2W)
#define SPLASH_STR	"Starting HexenWorld..."
#define RES_STRING	"HexenWorld"
//#elif defined(H2MP)
//#define SPLASH_STR	"Starting Hexen II+..."
//#define RES_STRING	"HexenII"
#else
#define SPLASH_STR	"Starting Hexen II..."
#define RES_STRING	"HexenII"
#endif

// Bitmap
//
#if defined(H2W)
#define SPLASH_BMP	"../../resource/hexenworld.bmp"
#else
#define SPLASH_BMP	"../resource/hexen2.bmp"
#endif

#endif	/* __HX2_RESOURCE_H */

