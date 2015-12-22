/***************************************************************************\
 *
 * PROGRAMMNAME: WPDCF77
 * -------------
 *
 * VERSION: 6.0
 * --------
 *
 * MODULNAME: DCF77.H
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
#ifndef DCF77_H
#define DCF77_H

LONG fRound (double x);
VOID FillEntry (HWND hwnd, USHORT usIOAddr);
VOID ReadEntry (HWND hwnd, ULONG ulDID);
USHORT CalcId (BYTE bPort);
BOOL ahtoul (PUSHORT pusPort, PSZ pszString);
VOID SetSlider1 (HWND hwnd);
VOID SetSlider2 (HWND hwnd);
VOID QuerySlider1 (HWND hwnd);
VOID QuerySlider2 (HWND hwnd);
VOID SetSupply (HWND hwnd);
VOID SetSignal (HWND hwnd);
VOID CheckScanMode (BYTE bOldPort, USHORT usOldIOAddr);
VOID SaveSetData (WPDCF77 *somSelf, ULONG flType);
VOID SetVisibility (HWND hwnd);
ULONG GetStatusMask (WPDCF77Data *somThis);
VOID SetStatusCtls (HWND hwnd, ULONG ulMask);
VOID SetOffsCtls (HWND hwnd);
MRESULT EXPENTRY DCF77DlgProc1 (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY IODlgProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY DCF77DlgProc2 (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY DCF77DlgProc3 (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY StatusDlgProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY InfoWndProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY TmOffsDlgProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
VOID determineDrvType (VOID);
BOOL isAnySource (VOID);
BOOL isClient (VOID);
BOOL isDriver (VOID);
BOOL isHopf603x (VOID);
BOOL isHopf6038 (VOID);
BOOL isUSBDevice (VOID);
BOOL isExtModule (VOID);
BOOL isSrvrEqClnt (VOID);
VOID Ins1Value (USHORT flStatus);
VOID InsStatusBuffer (USHORT flRcvrStatus);
BOOL Get_DCFStatus (PDCF77_STATUS pStatDCF);
BOOL Get_GPSCoord (PDCF77_COORD pCoordGPS);
BOOL Query_DCFData (PDCF77_DATA pDataDCF);
BOOL Query_DCFOffset (PDCF77_OFFSET pOffsDCF);
BOOL Query_DCFDateTime (PDCF77_DATETIME pDateTimeDCF);
BOOL Set_DCFData (PDCF77_DATA pDataDCF);
BOOL Set_DCFOffset (PDCF77_OFFSET pOffsDCF);
USHORT GetDCFStatus (VOID);
BOOL GetDCFData (VOID);
BOOL GetDCFOffs (VOID);
BOOL GetDCFDateTime (VOID);
USHORT GetGPSCoord (VOID);

#endif /* DCF77_H */
