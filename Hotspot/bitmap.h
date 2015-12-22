/***************************************************************************\
 *
 * PROGRAMMNAME: WPDCF77
 * -------------
 *
 * VERSION: 5.4
 * --------
 *
 * MODULNAME: BITMAP.H
 * ----------
 *
 * BESCHREIBUNG:
 * -------------
 *   Header-Datei mit den Definitionen fÅr Bitmap.c
 *
 *  Ver.    Date      Comment
 *  ----    --------  -------
 *  1.00    10-23-94  First release
 *  2.00    06-16-95  Extensions
 *  3.00    10-30-95  Client/Server Support; SOM2
 *  4.00    03-15-96  UnterstÅtzung fÅr hopf
 *  4.20    10-15-96  UnterstÅtzung fÅr hopf GPS
 *  4.30    17-05-97  UnterstÅtzung HR-Timer; Bitmap-UnterstÅtzung
 *  4.40    01-31-00  Multimedia-UnterstÅtzung, Bugfixing
 *  5.20    06-09-01  Fehlerkorrektur Audiodaten
 *  5.30    01-16-02  Implementierung der Erinnerungsfunktion
 *  5.40    11-22-03  Fehlerkorrektur Erinnerungsfunktion
 *
 *  Copyright (C) noller & breining software 1995...2003
 *
\******************************************************************************/
#ifndef BITMAP_H
#define BITMAP_H

ULONG createBitmap (HPS hps, PSZ pszFilename, PHBITMAP phBM, PPOINTL pHotspot);

#define BMPERR_OK           0
#define BMPERR_OPENERR      1
#define BMPERR_INVBMP       2

/* BITMAPINFOHEADER fÅr Windows-Bitmaps */
typedef struct _WINBITMAPINFOHEADER
{
   ULONG  cbFix;            /* Length of structure                    */
   ULONG  cx;               /* Bit-map width in pels                  */
   ULONG  cy;               /* Bit-map height in pels                 */
   USHORT cPlanes;          /* Number of bit planes                   */
   USHORT cBitCount;        /* Number of bits per pel within a plane  */
   ULONG  ulCompression;    /* Compression scheme used to store the bitmap */
   ULONG  cbImage;          /* Length of bit-map storage data in bytes*/
   ULONG  cxResolution;     /* x resolution of target device          */
   ULONG  cyResolution;     /* y resolution of target device          */
   ULONG  cclrUsed;         /* Number of color indices used           */
   ULONG  cclrImportant;    /* Number of important color indices      */
} WINBITMAPINFOHEADER;


#endif /* BITMAP_H */
