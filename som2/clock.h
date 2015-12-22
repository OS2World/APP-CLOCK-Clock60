/***************************************************************************\
 *
 * PROGRAMMNAME: WPDCF77
 * -------------
 *
 * VERSION: 6.0
 * --------
 *
 * MODULNAME: CLOCK.H
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
#ifndef CLOCK_H
#define CLOCK_H

/* Was mu· DrawClock alles zeichnen? */
#define DCLK_ALL        1       /* gesamte Uhr zeichnen */
#define DCLK_HANDS      2       /* nur die Zeiger */
#define DCLK_STATUS     3       /* DCF-77 Statusanzeigen; Vorsicht bei Sommerzeitumschaltung! */

/* Anzahl der LONG-Werte in einer Transformationsmatrix */
#define MATLF_SIZE (sizeof (MATRIXLF) / sizeof (LONG))

/* Welcher Zeiger soll von DrawHand gezeichnet werden? */
#define HT_HOUR           1
#define HT_MINUTE         2
#define HT_HOUR_SHADE     3
#define HT_MINUTE_SHADE   4
#define HT_SECOND         5
#define HT_ALARM          6

#define SHADE               0
#define LIGHT               1
#define SURFACE             2
#define BACKGROUND          3

#define TIMER_ASYNC         1
#define TIMER_CONT          2

#define STACKLEN            0x8000                  // Stackgrî·e fÅr Threads

#define CCHGRPHBUF          4000                    // Grî·e des Puffers fÅr Grafik in Byte
#define CCHMINPERPIXEL      15                      // Minuten/Pixel bei Darstellung fÅr Standard-Module
#define CCHGRPHMAX          1200                    // grî·ter Wert in pGraphData
#define CCHLVLBUF           300                     // Puffer fÅr DCF77_GETLVLDATA

#define SEM_TIMER_VALUE     ( (ULONG) (1000L * 60 * 60) )

#define SZ_KEYNAME  "Preferences"

typedef struct _INISAVERESTORE
{
    SWP    swp;
    USHORT yRestore;
    USHORT xRestore;
    USHORT yMinRestore;
    USHORT xMinRestore;
} INISAVERESTORE;

typedef struct _DATETIME FAR *LPDT;                 // For DosGet/SetDateTime

/* Title fÅr MessageBox-Funktion */
#define MBTITLE_NONE    0
#define MBTITLE_ERROR   -1

/* Werte fÅr die Steuervariable ulGetDCFData */
#define GET_OK          0                           // GetDCFData () lÑuft
#define GET_PAUSE       1                           // GetDCFData () angehalten

/*--------------------------------------------------------------*\
 *  Standard Dialog box ids
\*--------------------------------------------------------------*/
#define FILEOPEN        2000
#define FILESAVE        2001

#define MSGBOXID        1001
#define OBJECTID        1002

/*--------------------------------------------------------------*\
 *  Definitionen fÅr DCF-77 Empfang                             *
\*--------------------------------------------------------------*/
#define RCVR_IS_DRIVER      0x0001  /* Treiber vorhanden */
#define RCVR_IS_CARRIER     0x0002  /* TrÑger vorhanden  */
#define RCVR_IS_VALID       0x0004  /* Zeit ist gÅltig   */
#define RCVR_IS_WINTER      0x0008  /* Winterzeit        */

HWND ClkInit (WPDCF77 *somSelf);
MRESULT EXPENTRY ClkFrameWndProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
ULONG MessageBox (HWND hwndOwner, ULONG idText, ULONG idTitle, ULONG flStyle, ULONG ulBeep, PSZ pszText);

#endif /* CLOCK_H */
