/***************************************************************************\
 *
 * PROGRAMMNAME: WPDCF77
 * -------------
 *
 * VERSION: 6.0
 * --------
 *
 * MODULNAME: CLOCKPAN.RC
 * ----------
 *
 * BESCHREIBUNG:
 * -------------
 *   Header-Datei fÅr Panel-IDs der Online-Hilfe
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
#ifndef DID_HELP_PB
  #define DID_HELP_PB               267
#endif /* DID_HELP_PB */
#ifndef DID_HELP_BUTTON
  #define DID_HELP_BUTTON           310
#endif /* DID_HELP_BUTTON */

#define PANEL_MAIN                  10
#define PANEL_VIEWANALOG            20
#define PANEL_VIEWDIGITAL           30
#define PANEL_DATETIME              1100
#define PANEL_SETTING1              1200
#define PANEL_SETTING2              1300
#define PANEL_DCF77_1               1400
#define PANEL_DCF77_2               1500
#define PANEL_DCF77_3               1600
#define PANEL_DCF77_STATUS          1700
#define PANEL_TMOFFS                1800
#define PANEL_ALARM                 1900
#define PANEL_ALARM2                2100
#define PANEL_IOADDR                2200
#define PANEL_ALARM3                2300
#define PANEL_SERVER                2400
#define PANEL_COMPORT               2500
#define PANEL_GRAPH                 2600
#define PANEL_TIMEZONE              2700
#define PANEL_SETTING3              2800
#define PANEL_AUDIO                 2900

#define PANEL_CANCEL                10100
#define PANEL_STANDARD              10101

/* reserviert fÅr weitere Hilfeseiten: 20000-29999 */

/* zu PANEL_DATETIME */
#define PANEL_HOURS                 PANEL_DATETIME + 1
#define PANEL_MINUTES               PANEL_DATETIME + 2
#define PANEL_SECONDS               PANEL_DATETIME + 3
#define PANEL_MONTH                 PANEL_DATETIME + 4
#define PANEL_DAY                   PANEL_DATETIME + 5
#define PANEL_YEAR                  PANEL_DATETIME + 6

/* zu PANEL_TMOFFS */
#define PANEL_UTC                   PANEL_TMOFFS + 1
#define PANEL_GOFFSET               PANEL_TMOFFS + 2
#define PANEL_LOFFSET               PANEL_TMOFFS + 3
#define PANEL_TZONE                 PANEL_TMOFFS + 4

/* zu PANEL_SETTING1 */
#define PANEL_TIME                  PANEL_SETTING1 + 1
#define PANEL_DATE                  PANEL_SETTING1 + 2
#define PANEL_DATIM                 PANEL_SETTING1 + 3
#define PANEL_DIGITAL               PANEL_SETTING1 + 4
#define PANEL_ANALOG                PANEL_SETTING1 + 5
#define PANEL_SHAND                 PANEL_SETTING1 + 6
#define PANEL_HTICK                 PANEL_SETTING1 + 7
#define PANEL_MTICK                 PANEL_SETTING1 + 8
#define PANEL_BAVARIAN              PANEL_SETTING1 + 9
#define PANEL_TITLEBAR              PANEL_SETTING1 + 10
#define PANEL_ICONPOS               PANEL_SETTING1 + 11

/* zu PANEL_SETTING2 */
#define PANEL_FONT                  DID_HELP_BUTTON
#define PANEL_CFACE                 PANEL_SETTING2 + 1
#define PANEL_CBGND                 PANEL_SETTING2 + 2
#define PANEL_CHTICK                PANEL_SETTING2 + 3
#define PANEL_CDATEBGND             PANEL_SETTING2 + 4
#define PANEL_CHHAND                PANEL_SETTING2 + 5
#define PANEL_CMHAND                PANEL_SETTING2 + 6
#define PANEL_CDATE                 PANEL_SETTING2 + 7
#define PANEL_CTIME                 PANEL_SETTING2 + 8
#define PANEL_CTRANSPARENT          PANEL_SETTING2 + 9
#define PANEL_CHGCOL                PANEL_SETTING2 + 10
#define PANEL_FDATE                 PANEL_SETTING2 + 11
#define PANEL_FTIME                 PANEL_SETTING2 + 12
#define PANEL_CHGFONT               PANEL_SETTING2 + 13

/* zu PANEL_ALARM */
#define PANEL_ALARMACT              PANEL_ALARM + 1
#define PANEL_REMIND                PANEL_ALARM + 2
#define PANEL_ALARMINACT            PANEL_ALARM + 3
#define PANEL_AACOUSTIC             PANEL_ALARM + 4
#define PANEL_MESSAGE               PANEL_ALARM + 5
#define PANEL_LAUNCH                PANEL_ALARM + 6
#define PANEL_AHOURS                PANEL_ALARM + 7
#define PANEL_ASECONDS              PANEL_ALARM + 8
#define PANEL_AMINUTES              PANEL_ALARM + 9
#define PANEL_AMONTH                PANEL_ALARM + 10
#define PANEL_ADAY                  PANEL_ALARM + 11
#define PANEL_AYEAR                 PANEL_ALARM + 12
#define PANEL_SINGLE                PANEL_ALARM + 13
#define PANEL_WEEKDAY               PANEL_ALARM + 14

/* zu PANEL_DCF77_1 */
#define PANEL_SERIAL                PANEL_DCF77_1 + 1
#define PANEL_GAME                  PANEL_DCF77_1 + 2
#define PANEL_SERIALX               PANEL_DCF77_1 + 3
#define PANEL_GAMEX                 PANEL_DCF77_1 + 4

/* zu PANEL_DCF77_2 */
#define PANEL_INTERVAL              PANEL_DCF77_2 + 1
#define PANEL_IMPLEN                PANEL_DCF77_2 + 2
#define PANEL_CHECKREP              PANEL_DCF77_2 + 3
#define PANEL_HRTIMER               PANEL_DCF77_2 + 4

/* zu PANEL_DCF77_3 */
#define PANEL_POWER                 PANEL_DCF77_3 + 1
#define PANEL_CSIGNAL               PANEL_DCF77_3 + 2
#define PANEL_SSIGNAL               PANEL_DCF77_3 + 3

/* zu PANEL_DCF77_STATUS */
#define PANEL_INFPOL                PANEL_DCF77_STATUS + 1
#define PANEL_INFVER                PANEL_DCF77_STATUS + 2
#define PANEL_INFCARRIER            PANEL_DCF77_STATUS + 3
#define PANEL_INFANT                PANEL_DCF77_STATUS + 4
#define PANEL_INFHLEAP              PANEL_DCF77_STATUS + 5
#define PANEL_INFSLEAP              PANEL_DCF77_STATUS + 6
#define PANEL_INFTZ1                PANEL_DCF77_STATUS + 7
#define PANEL_INFTZ2                PANEL_DCF77_STATUS + 8
#define PANEL_INFOK                 PANEL_DCF77_STATUS + 9
#define PANEL_INFNOK                PANEL_DCF77_STATUS + 10
#define PANEL_INFSET                PANEL_DCF77_STATUS + 11
#define PANEL_INFVALID              PANEL_DCF77_STATUS + 12
#define PANEL_INFSCAN               PANEL_DCF77_STATUS + 13

/* zu PANEL_ALARM3 */
#define PANEL_SEARCH                DID_HELP_PB
#define PANEL_ALARM_PATH            PANEL_ALARM3 + 1
#define PANEL_ALARM_PARM            PANEL_ALARM3 + 2
#define PANEL_ALARM_DIR             PANEL_ALARM3 + 3

/* zu PANEL_SERVER */
#define PANEL_SERVERINTERVAL        PANEL_SERVER + 1
#define PANEL_SERVERSENDTIME        PANEL_SERVER + 2

/* zu PANEL_TIMEZONE */
#define PANEL_TZFILENAME            PANEL_TIMEZONE + 1
#define PANEL_TZSELECT              PANEL_TIMEZONE + 2
#define PANEL_TZTIMEZONE            PANEL_TIMEZONE + 3
#define PANEL_TZSUMMER              PANEL_TIMEZONE + 4
#define PANEL_TZWINTER              PANEL_TIMEZONE + 5


/* zu PANEL_SETTING3 */
#define PANEL_MINHAND               PANEL_SETTING3 + 1
#define PANEL_HOURHAND              PANEL_SETTING3 + 2
#define PANEL_BACKGND               PANEL_SETTING3 + 3
#define PANEL_SEARCHBMP             PANEL_SETTING3 + 4

/* zu PANEL_AUDIO */
#define PANEL_WAVFILE               PANEL_AUDIO + 1
#define PANEL_WAVSELECT             PANEL_AUDIO + 2
#define PANEL_PLAY1                 PANEL_AUDIO + 3
#define PANEL_PLAYN                 PANEL_AUDIO + 4

