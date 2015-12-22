/***************************************************************************\
 *
 * PROGRAMMNAME: WPDCF77
 * -------------
 *
 * VERSION: 6.0
 * --------
 *
 * MODULNAME: ALARMTHD.H
 * ----------
 *
 * BESCHREIBUNG:
 * -------------
 *   allgemeine Definitionen f〉 DCF77-WPS-Uhr
 *
 *  Ver.    Date      Comment
 *  ----    --------  -------
 *  1.00    10-23-94  First release
 *  2.00    06-16-95  Extensions
 *  3.00    10-30-95  Client/Server Support; SOM2
 *  4.00    03-15-96  Unterst》zung f〉 hopf
 *  4.20    10-15-96  Unterst》zung f〉 hopf GPS
 *  4.30    05-17-97  Unterst》zung HR-Timer; Bitmap-Unterst》zung
 *  4.40    01-31-00  Multimedia-Unterst》zung, Bugfixing
 *  5.10    02-10-01  Fehlerkorrektur Positionsspeicherung Iconview
 *  5.20    06-09-01  Fehlerkorrektur Audiodaten
 *  5.30    01-16-02  Implementierung der Erinnerungsfunktion
 *  5.40    11-22-03  Fehlerkorrektur Erinnerungsfunktion
 *  6.00    02-15-04  USB Unterst》zung
 *
 *  Copyright (C) noller & breining software 1995...2004
 *
\******************************************************************************/
#ifndef ALARMTHD_H
#define ALARMTHD_H

typedef enum
    {
    activate,       // Alarm starten, wenn Mode = AM_ACTIVE
    start,          // Alarm starten, wenn Mode = AM_ACTIVE oder AM_REMINDER
    stop,           // Alarm beenden, AlarmThread beenden
    toggle          // Alarmmodus umschalten
    } AlarmStartAction;

VOID _Optlink AlarmThread (WPDCF77 *somSelf);
BOOL IsAlarmActive (WPDCF77Data *somThis);
BOOL AlarmStart (WPDCF77Data *somThis, AlarmStartAction action);
BOOL AlarmSetTimer (WPDCF77Data *somThis, PDATETIME pdtActual);
VOID _Optlink th1sTimer (void *pArg);

#endif /* ALARMTHD_H */
