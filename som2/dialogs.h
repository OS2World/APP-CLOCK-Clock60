/***************************************************************************\
 *
 * PROGRAMMNAME: WPDCF77
 * -------------
 *
 * VERSION: 6.0
 * --------
 *
 * MODULNAME: DIALOGS.H
 * ----------
 *
 * BESCHREIBUNG:
 * -------------
 *   allgemeine Definitionen fÅr DCF77-WPS-Uhr
 *
 *  Ver.    Date      Comment
 *  ----    --------  -------
 *  1.00    10-23-94  First release
 *  2.00    06-16-95  Extensions
 *  3.00    10-30-95  Client/Server Support; SOM2
 *  4.00    03-15-96  UnterstÅtzung fÅr hopf
 *  4.20    10-15-96  UnterstÅtzung fÅr hopf GPS
 *  4.30    05-17-97  UnterstÅtzung HR-Timer; Bitmap-UnterstÅtzung
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
#ifndef DIALOGS_H
#define DIALOGS_H

MRESULT EXPENTRY ClkTicksDlgProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
VOID LoadBitmapFiles (WPDCF77Data *somThis, BOOL bDisplayError);
LONG BrowseFile (HWND hwnd, PCHAR pszText);
ULONG SetSettings1Buttons (HWND hwnd, ULONG ulPresence);
BOOL IsOtherClockActive (HOBJECT hObject);
BOOL IsClockWndChanged (HWND hwnd, HOBJECT hObject);
VOID CheckAudioObjectDeleted (WPDCF77 *somSelf, HOBJECT hObject);
VOID SetAudioControls (HWND hwnd, WPDCF77Data *somThis, HOBJECT hObject, BOOL bForceUpdate);
MRESULT EXPENTRY ClkColorsDlgProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY ClkBmpsDlgProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY Alarm2DlgProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY Alarm3DlgProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY ClkAudioDlgProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY AboutDlgProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY NewParentWndProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY NewFontDlgProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

#endif /* DIALOGS_H */
