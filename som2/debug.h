/***************************************************************************\
 *
 * PROGRAMMNAME: WPDCF77
 * -------------
 *
 * VERSION: 6.0
 * --------
 *
 * MODULNAME: DEBUG.H
 * ----------
 *
 * BESCHREIBUNG:
 * -------------
 *   Debug-Funktionen
 *
 *  Ver.    Date      Comment
 *  ----    --------  -------
 *  1.00    10-23-94  First release
 *  2.00    06-16-95  Extensions
 *  3.00    10-30-95  Client/Server Support; SOM2
 *  4.00    03-15-96  Untersttzung fr hopf
 *  4.20    10-15-96  Untersttzung fr hopf GPS
 *  4.30    05-17-97  Untersttzung HR-Timer; Bitmap-Untersttzung
 *  4.40    01-31-00  Multimedia-Untersttzung, Bugfixing
 *  5.10    02-10-01  Fehlerkorrektur Positionsspeicherung Iconview
 *  5.20    06-09-01  Fehlerkorrektur Audiodaten
 *  5.30    01-16-02  Implementierung der Erinnerungsfunktion
 *  5.40    11-22-03  Fehlerkorrektur Erinnerungsfunktion
 *  6.00    02-15-04  USB Untersttzung
 *
 *  Copyright (C) noller & breining software 1995...2004
 *
\******************************************************************************/
#ifndef DEBUG_H
#define DEBUG_H

#ifndef CLASSNAME
#define CLASSNAME   "WPDCF77"       // Als Id-String fr Instanzvariable
#endif

int SOMLINK myReplacementForSOMOutChar (char c);

/* Private Debug-Hilfroutinen fr PMPRINTF */
#ifdef DEBUG
  MRESULT EXPENTRY DbgDlgProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

  int   pmprintf(char *f, ...);
  extern ULONG ulDebugMask;
  extern CHAR  szDebug[128];

  /* DEBUG-Maske */
  #define D_IO      0x00000001      // Kommunikation mit Treiber
  #define D_ALARM   0x00000002      // Alarm-Thread
  #define D_TIMER   0x00000004      // Timer-Thread
  #define D_INI     0x00000008      // Instanz- und Klassendaten
  #define D_MENU    0x00000010      // Men-Operationen
  #define D_WND     0x00000020      // Window-Control
  #define D_DRAG    0x00000040      // Direct manipulation
  #define D_DIAG    0x00000080      // Diagnostik: interne Fehler
  #define D_DLG     0x00000100      // Diagnose in Dialog-Panels
  #define D_MINMAX  0x00000200      // Diagnose Min/Max-Verhalten
  #define D_STAT    0x00000400      // Diagnose Statistik-Grafik

  /* Hilfe-Text fr DEBUG-Maske */
  #define D_TEXT_MASK   "\
D_IO      Kommunikation mit Treiber    0x00000001\n\
D_ALARM   Alarm-Thread                 0x00000002\n\
D_TIMER   Timer-Thread                 0x00000004\n\
D_INI     Instanz- und Klassendaten    0x00000008\n\
D_MENU    Men-Operationen             0x00000010\n\
D_WND     Window-Control               0x00000020\n\
D_DRAG    Direct manipulation          0x00000040\n\
D_DIAG    Diagnostik: interne Fehler   0x00000080\n\
D_DLG     Diagnose in Dialog-Panels    0x00000100\n\
D_MINMAX  Diagnose Min/Max-Verhalten   0x00000200\n\
D_STAT    Diagnose Statistik-Grafik    0x00000400\n"

  #define DebugS(m,title)         if (m&ulDebugMask) pmprintf ("%s| %s\n", CLASSNAME, title);
  #define DebugE(m,title,text)    if (m&ulDebugMask) pmprintf ("%s| %s => %s\n", CLASSNAME, title, text)
  #define DebugULd(m,title,s,ul)  if (m&ulDebugMask) pmprintf ("%s| %s => %s: %d\n", CLASSNAME, title, s, ul)
  #define DebugULx(m,title,s,ul)  if (m&ulDebugMask) pmprintf ("%s| %s => %s: 0x%X\n", CLASSNAME, title, s, ul)
#else
  #define DebugS(m,title)         {}
  #define DebugE(m,title,text)    {}
  #define DebugULd(m,title,s,ul)  {}
  #define DebugULx(m,title,s,ul)  {}
#endif /* DEBUG */

#endif /*  DEBUG_H */
