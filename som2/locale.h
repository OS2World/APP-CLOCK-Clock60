/***************************************************************************\
 *
 * PROGRAMMNAME: WPDCF77
 * -------------
 *
 * VERSION: 6.0
 * --------
 *
 * MODULNAME: LOCALE.H
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
#ifndef LOCALE_H
#define LOCALE_H

MRESULT EXPENTRY TimezoneDlgProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
USHORT DaysInMonth (PDATETIME pDT);
VOID InitGPSDriver (VOID);

#endif /* LOCALE_H */
