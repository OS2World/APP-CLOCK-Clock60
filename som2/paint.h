/***************************************************************************\
 *
 * PROGRAMMNAME: WPDCF77
 * -------------
 *
 * VERSION: 6.0
 * --------
 *
 * MODULNAME: DCF77.H
 * ----------
 *
 * BESCHREIBUNG:
 * -------------
 *   Header-Datei mit allgemeinen Definitionen fÅr die Uhr
 *
 *  Ver.    Date      Comment
 *  ----    --------  -------
 *  1.00    10-23-94  First release
 *  4.40    01-31-00  Multimedia-UnterstÅtzung, Bugfixing
 *  5.10    02-10-01  Fehlerkorrektur Positionsspeicherung Iconview
 *  5.20    06-09-01  Fehlerkorrektur Audiodaten
 *  5.30    01-16-02  Implementierung der Erinnerungsfunktion
 *  5.40    11-22-03  Fehlerkorrektur Erinnerungsfunktion
 *  6.00    02-15-04  USB UnterstÅtzung
 *
 *  Copyright (C) noller & breining software 1995...2004
 *
\******************************************************************************/
#ifndef PAINT_H
#define PAINT_H

VOID ClkDrawClock (HPS hps, ULONG ulMode, WPDCF77Data *somThis, PDATETIME pdt, USHORT flRcvrStatus);
VOID ClkUpdateScreen (HPS hps, WPDCF77Data *somThis, RECTL *prclUpdate);
VOID ClkPaint (HWND hwnd, PWINDOWDATA pWindowData);
VOID ClkDrawHand (HPS hps, WPDCF77Data *somThis, SHORT sHandType, SHORT sAngle);
VOID ClkDrawFace (HPS hps, WPDCF77Data *somThis);
VOID ClkDrawRing (HPS hps, WPDCF77Data *somThis);
VOID ClkDrawWS (HPS hps, USHORT flRcvrStatus);
VOID ClkDrawBatt (HPS hps, USHORT flRcvrStatus);
VOID ClkDrawLED (HPS hps, WPDCF77Data *somThis, USHORT flRcvrStatus);
VOID ClkDrawDate (HPS hps, WPDCF77Data *somThis, PDATETIME pdt);
VOID ClkDrawTicks (HPS hps, WPDCF77Data *somThis, USHORT usTicks);
VOID ClkDrawTrapez (HPS hps, POINTL *aptl, LONG color);
VOID ClkDrawFullRing (HPS hps, PPOINTL pptlCenter, FIXED fxRadIn, FIXED fxRadOut, LONG lColor);

#endif /* PAINT_H */
