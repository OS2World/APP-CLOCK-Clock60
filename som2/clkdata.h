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
 *   Header-Datei f�r CLKDATA.C
 *
 *  Ver.    Date      Comment
 *  ----    --------  -------
 *  1.00    10-23-94  First release
 *  4.40    01-31-00  Multimedia-Unterst�tzung, Bugfixing
 *  5.10    02-10-01  Fehlerkorrektur Positionsspeicherung Iconview
 *  5.20    06-09-01  Fehlerkorrektur Audiodaten
 *  5.30    01-16-02  Implementierung der Erinnerungsfunktion
 *  5.40    11-22-03  Fehlerkorrektur Erinnerungsfunktion
 *  6.00    02-15-04  USB Unterst�tzung
 *
 *  Copyright (C) noller & breining software 1995...2004
 *
\******************************************************************************/
#ifndef CLKDATA_H
#define CLKDATA_H

#include "wpdcf77.ih"

extern HAB      hab;                // Anchor block wird von clsInitData gef�llt
extern HMODULE  hmod;               // Module handle

#define MAXTIMERS           32      // Maximalzahl von Timern
extern ULONG    ulcTimers;          // Zahl der registrierten Timer
extern HWND     hwndTimers[MAXTIMERS];

extern HFILE    hDrvr;              // Handle f�r DCF77-IOCtl's; -1: Treiber nicht gefunden
extern PSHMEM   pShMem;             // Zeiger auf Shared memory
extern UCHAR    cDrvType;           // Treibertyp

extern PUSHORT  pusAdjBuffer;       // Zeiger auf Puffer f�r Antennen-Einrichtung
extern ULONG    ulAdjBufferSize;    // Gr��e von pusAdjBuffer in Byte
extern ULONG    ulMaxPixel;         // max. Zahl der Me�punkte in pusAdjBuffer
extern TID      tidGraphThread;     // Thread-Handle f�r Antennen-Einrichtung

extern ULONG    ulDCFDelta;         // Tr�gerabsenkung des DCF77-Senders
extern PUSHORT  pGraphData;         // Zeiger auf Puffer f�r Empfangsqualit�t
extern ULONG    ulIndexGrphData;    // Index des aktuell bearbeiteten Platzes in pGraphData
extern ULONG    ulcGraphInData;     // Zahl der aufaddierten Daten in pGraphData

extern ULONG    ulGraphView;        // View-Typ f�r das Grafik-Fenster
extern WPDCF77 *somSelfGraphView;   // Objektzeiger der Instanz, die das Grafikfenster ge�ffnet hat

extern ULONG    ulStatusTStamp;     // Time-Stamp f�r letzte Statusabfrage
extern ULONG    ulDataTStamp;       // Time-Stamp f�r letzte Datenabfrage
extern ULONG    ulOffsTStamp;       // Time-Stamp f�r letzte Offsetabfrage
extern ULONG    ulTimeTStamp;       // Time-Stamp f�r letzte Zeitabfrage
extern ULONG    ulCoordTStamp;      // Time-Stamp f�r letzte Koordinatenabfrage
extern ULONG    ulTimerCntr;        // Sekundenz�hler f�r Time-Stamp-Vergleich
extern ULONG    ulGetDCFData;       // Steuervariable f�r GetDCFData ()
extern DCF77_STATUS    StatDCF;     // Puffer f�r DCF-Status
extern DCF77_DATETIME  DtDCF;       // Puffer f�r Datum/Uhrzeit aus Treiber
extern DCF77_COORD     CoordGPS;    // Datenstruktur f�r GPS-Position aus Treiber

extern BOOL     bIsAudio;                   // TRUE: Multimedia-Support im System installiert

// Persistente Instanzdaten der Metaklasse
extern DCF77_DATA      DataDCF;     // Datenstruktur f�r Treibereinstellungen
extern DCF77_OFFSET    OffsDCF;     // Datenstruktur f�r Treiber-Zeitoffseteinstellung

extern CHAR     szLocName[CCHMAXTZONE];     // Puffer f�r Zeitzonennamen in der locale-Datei
extern CHAR     szLocFile[CCHMAXPATH];      // Pfad+Dateiname der locale-Datei

extern ULONG    ulSrvrInterval;             // Intervall f�r LAN::Time
extern BOOL     bSendSrvrTime;              // True: bei �bertragungsfehler Serverzeit senden

extern CHAR     szClsGongFile[CCHMAXPATH];  // WAV-Datei f�r den Stundengong
extern BOOL     bClsIs1PerHour;             // TRUE: WAV-Datei nur 1x jede Stunde spielen
extern HOBJECT  hClsRingingObject;          // Instanz-Handle der Uhr mit Sound

#endif /* CLKDATA_H */
