/***************************************************************************\
 *
 * PROGRAMMNAME: WPDCF77
 * -------------
 *
 * VERSION: 6.0
 * --------
 *
 * MODULNAME: UDTIME.H
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
#ifndef UDTIME_H
#define UDTIME_H

MRESULT EXPENTRY TimeDlgProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY AlarmDlgProc (HWND hwnd, ULONG usMsg, MPARAM mp1, MPARAM mp2);
VOID GetDateTimeOffs (LONG lOffset, PDATETIME pdt);
LONG GetTimeLowOffs (LONG lOffset);

#endif /* UDTIME_H */
