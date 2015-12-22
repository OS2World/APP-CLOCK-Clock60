/***************************************************************************\
 *
 * PROGRAMMNAME: WPDCF77
 * -------------
 *
 * VERSION: 6.0
 * --------
 *
 * MODULNAME: CLKDATA.H
 * ----------
 *
 * BESCHREIBUNG:
 * -------------
 *   Header-Datei fÅr CLKDATA.C
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
#ifndef CLKDATA_H
#define CLKDATA_H

#include "wpdcf77.ih"

extern HAB      hab;                // Anchor block wird von clsInitData gefÅllt
extern HMODULE  hmod;               // Module handle

#define MAXTIMERS           32      // Maximalzahl von Timern
extern ULONG    ulcTimers;          // Zahl der registrierten Timer
extern HWND     hwndTimers[MAXTIMERS];

extern HFILE    hDrvr;              // Handle fÅr DCF77-IOCtl's; -1: Treiber nicht gefunden
extern PSHMEM   pShMem;             // Zeiger auf Shared memory
extern UCHAR    cDrvType;           // Treibertyp

extern PUSHORT  pusAdjBuffer;       // Zeiger auf Puffer fÅr Antennen-Einrichtung
extern ULONG    ulAdjBufferSize;    // Grî·e von pusAdjBuffer in Byte
extern ULONG    ulMaxPixel;         // max. Zahl der Me·punkte in pusAdjBuffer
extern TID      tidGraphThread;     // Thread-Handle fÅr Antennen-Einrichtung

extern ULONG    ulDCFDelta;         // TrÑgerabsenkung des DCF77-Senders
extern PUSHORT  pGraphData;         // Zeiger auf Puffer fÅr EmpfangsqualitÑt
extern ULONG    ulIndexGrphData;    // Index des aktuell bearbeiteten Platzes in pGraphData
extern ULONG    ulcGraphInData;     // Zahl der aufaddierten Daten in pGraphData

extern ULONG    ulGraphView;        // View-Typ fÅr das Grafik-Fenster
extern WPDCF77 *somSelfGraphView;   // Objektzeiger der Instanz, die das Grafikfenster geîffnet hat

extern ULONG    ulStatusTStamp;     // Time-Stamp fÅr letzte Statusabfrage
extern ULONG    ulDataTStamp;       // Time-Stamp fÅr letzte Datenabfrage
extern ULONG    ulOffsTStamp;       // Time-Stamp fÅr letzte Offsetabfrage
extern ULONG    ulTimeTStamp;       // Time-Stamp fÅr letzte Zeitabfrage
extern ULONG    ulCoordTStamp;      // Time-Stamp fÅr letzte Koordinatenabfrage
extern ULONG    ulTimerCntr;        // SekundenzÑhler fÅr Time-Stamp-Vergleich
extern ULONG    ulGetDCFData;       // Steuervariable fÅr GetDCFData ()
extern DCF77_STATUS    StatDCF;     // Puffer fÅr DCF-Status
extern DCF77_DATETIME  DtDCF;       // Puffer fÅr Datum/Uhrzeit aus Treiber
extern DCF77_COORD     CoordGPS;    // Datenstruktur fÅr GPS-Position aus Treiber

extern BOOL     bIsAudio;                   // TRUE: Multimedia-Support im System installiert

// Persistente Instanzdaten der Metaklasse
extern DCF77_DATA      DataDCF;     // Datenstruktur fÅr Treibereinstellungen
extern DCF77_OFFSET    OffsDCF;     // Datenstruktur fÅr Treiber-Zeitoffseteinstellung

extern CHAR     szLocName[CCHMAXTZONE];     // Puffer fÅr Zeitzonennamen in der locale-Datei
extern CHAR     szLocFile[CCHMAXPATH];      // Pfad+Dateiname der locale-Datei

extern ULONG    ulSrvrInterval;             // Intervall fÅr LAN::Time
extern BOOL     bSendSrvrTime;              // True: bei öbertragungsfehler Serverzeit senden

extern CHAR     szClsGongFile[CCHMAXPATH];  // WAV-Datei fÅr den Stundengong
extern BOOL     bClsIs1PerHour;             // TRUE: WAV-Datei nur 1x jede Stunde spielen
extern HOBJECT  hClsRingingObject;          // Instanz-Handle der Uhr mit Sound

#endif /* CLKDATA_H */
