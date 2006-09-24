#ifndef __HX2_RESOURCE_H
#define __HX2_RESOURCE_H

#define IDS_STRING1	1
#define IDI_ICON2	1
#define IDD_DIALOG1	108
#define IDD_PROGRESS	109
#define IDB_HWBITMAP	112
#define IDC_PROGRESS	1000

// Icon file to use
//
#if defined(H2W)
#define ICO_FILE	"../icons/hexenworld.ico"
//#elif defined(H2MP)
//#define ICO_FILE	"icons/h2mp.ico"
#else
#define ICO_FILE	"icons/hexen2.ico"
#endif

// String
//
#if defined(H2W)
#define H2_STRING	"HexenWorld"
//#elif defined(H2MP)
//#define H2_STRING	"Hexen II+"
#else
#define H2_STRING	"Hexen II"
#endif

// Bitmap
// 
#if defined(H2W)
#define HWBITMAP	"win_stuff/hexenworld.bmp"
#else
#define HWBITMAP	"win_stuff/hexen2.bmp"
#endif

#endif	/* __HX2_RESOURCE_H */

