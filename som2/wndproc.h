/***************************************************************************\
 *
 * PROGRAMMNAME: WPDCF77
 * -------------
 *
 * VERSION: 6.0
 * --------
 *
 * MODULNAME: WNDPROC.H
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
#ifndef WNDPROC_H
#define WNDPROC_H

MRESULT EXPENTRY ClkWndProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
VOID ClkCreate (HWND hwnd, PWINDOWDATA pWindowData);
VOID ClkDestroy (PWINDOWDATA pWindowData);
VOID ClkSize (HWND hwnd);
VOID ClkTimer (HWND hwnd);
VOID ClkStartAudioPlay (HWND hwnd, PWINDOWDATA pWindowData, ULONG ulHours);
VOID ClkPresparam (HWND hwnd, ULONG ulType);
VOID ClkDrawString (HPS hps, ULONG ulFGColor, PCHAR pszString, PRECTL prcl);
VOID ClkDrawDigitalTime (HWND hwnd, PDATETIME pdt, ULONG ulMode);
VOID ClkRefresh (WPDCF77Data *somThis);
VOID ClkShowFrameControls (HWND hwndFrame, BOOL bShow);
VOID ClkSetFonts (WPDCF77Data *somThis);
VOID ClkSetRGBColors (WPDCF77Data *somThis);
VOID ClkShadeLight (PCOLORS psColors);
BYTE ClkShadeRGBByte (BYTE brgb);
BYTE ClkLightRGBByte (BYTE brgb);

#endif /* WNDPROC_H */
