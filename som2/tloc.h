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
 *  5.10    02-10-01  Fehlerkorrektur Positionsspeicherung Iconview
 *  5.20    06-09-01  Fehlerkorrektur Audiodaten
 *  5.30    01-16-02  Implementierung der Erinnerungsfunktion
 *  5.40    11-22-03  Fehlerkorrektur Erinnerungsfunktion
 *  6.00    02-15-04  USB UnterstÅtzung
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

#endif /* TLOC_H */
