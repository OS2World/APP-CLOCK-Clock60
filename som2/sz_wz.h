/***************************************************************************\
 *
 * PROGRAMMNAME: WPDCF77
 * -------------
 *
 * VERSION: 6.0
 * --------
 *
 * MODULNAME: SZ_WZ.H
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
#ifndef SZ_WZ_H
#define SZ_WZ_H

APIRET OpenTZFile (PSZ pszFileName, PHFILE phFile);
APIRET RewindTZFile (HFILE hFile);
APIRET CloseTZFile (HFILE hFile);
BOOL GetNextLineOfType (HFILE hFile, PSZ pszLine, USHORT usLen, TokType usType);
BOOL MoveToTZNo (HFILE hFile, USHORT us, PSZ pszLine, USHORT usLen);
BOOL GetTZString (HFILE hFile, PSZ pszLine, USHORT usLen, PTLOC_DEF pdt);
USHORT FindEntry (HFILE hFile, PSZ pszLine, USHORT usLen, PSZ pszTZone);
BOOL IsValidHandle (HFILE hFile);

#endif /* SZ_WZ_H */
