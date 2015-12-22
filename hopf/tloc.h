/***************************************************************************\
 *
 * PROGRAMMNAME: WPDCF77
 * -------------
 *
 * VERSION: 6.0
 * --------
 *
 * MODULNAME: TLOC.H
 * ----------
 *
 * BESCHREIBUNG:
 * -------------
 *   Header-Datei locale-Definitionen
 *
 *  Ver.    Date      Comment
 *  ----    --------  -------
 *  4.20    09-12-96  UnterstÅtzung fÅr hopf 6038
 *  5.20    06-09-01  Fehlerkorrektur Audiodaten
 *
 *  Copyright (C) noller & breining software 1995...2004
 *
\******************************************************************************/
#ifndef TLOC_H
#define TLOC_H

#define _TLOC_

typedef enum {invalid, comment, heading, entryline} TokType;

#ifdef __32BIT__
typedef ULONG  IOTYPE;
#else
typedef USHORT IOTYPE;
typedef USHORT APIRET;
#endif

#pragma pack(1)

typedef struct _TLOC_DEF
    {
    USHORT usHours;                             // Umschalt-Uhrzeit: Stunden
    USHORT usMinutes;                           // Umschalt-Uhrzeit: Minuten
    USHORT usFirstDay, usLastDay;               // Umschalt-Datum:   (Intervall) Tage
    USHORT usMonth;                             // Umschalt-Datum:   Monat
    USHORT usFirstYear, usLastYear;             // Umschalt-Datum:   Intervall Jahre
    USHORT usFirstWeekday, usLastWeekday;       // Umschalt-Datum:   (Intervall) Wochentag
    SHORT  sTimeDiff;                           // Zeitzone: Differenz zu UTC
    } TLOC_DEF, *PTLOC_DEF;

#pragma pack()

/***** Funktionsprototypen *****/
/* locale.c */
APIRET OpenTZFile (PSZ pszFileName, PHFILE phFile);
APIRET RewindTZFile (HFILE hFile);
APIRET CloseTZFile (HFILE hFile);
USHORT FindEntry  (HFILE hFile, PSZ pszLine, USHORT usLen, PSZ pszTZone);
BOOL GetNextLineOfType (HFILE hFile, PSZ pszLine, USHORT usLen, TokType usType);
BOOL MoveToTZNo (HFILE hFile, USHORT us, PSZ pszLine, USHORT usLen);
BOOL GetTZString (HFILE hFile, PSZ pszLine, USHORT usLen, PTLOC_DEF pdt);
BOOL IsValidHandle (HFILE hFile);

#endif /* TLOC_H */
