/***************************************************************************\
 *
 * PROGRAMMNAME: WPDCF77
 * -------------
 *
 * VERSION: 6.0
 * --------
 *
 * MODULNAME: PLAY.H
 * ----------
 *
 * BESCHREIBUNG:
 * -------------
 *   Prototyp-Definitionen fÅr Audio-Support
 *
 *  Ver.    Date      Comment
 *  ----    --------  -------
 *  4.20    09-12-96  UnterstÅtzung fÅr hopf 6038
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
#ifndef PLAY_H
#define PLAY_H

BOOL ClkMciPlayFile (HWND hwnd, PSZ pszFileName);
BOOL ClkMciLoadFile (HWND hwnd, ULONG ulQwlIndex, PCHAR pszFileName, ULONG ulNumOfPlays);
VOID ClkMciStartPlayBack (HWND hwnd, ULONG ulQwlIndex, USHORT usFinishedMsg);
VOID ClkMciCheckRedo (HWND hwnd, ULONG ulQwlIndex);
VOID ClkMciClose (HWND hwnd, ULONG ulQwlIndex);
VOID ClkMciPassDevice (HWND hwnd, ULONG ulQwlIndex, SHORT sAction);
VOID ClkMciActivate (HWND hwnd, ULONG ulQwlIndex, BOOL isActive);
BOOL ClkMciIsMultimedia (VOID);

#endif /* PLAY_H */
