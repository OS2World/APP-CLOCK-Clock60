/***************************************************************************\
 *
 * PROGRAMMNAME: WPDCF77
 * -------------
 *
 * VERSION: 6.0
 * --------
 *
 * MODULNAME: DLG.H
 * ----------
 *
 * BESCHREIBUNG:
 * -------------
 *   Header-Datei fÅr CLOCK.DLG
 *
 *  Ver.    Date      Comment
 *  ----    --------  -------
 *  1.00    10-23-94  First release
 *  4.00    03-15-96  UnterstÅtzung fÅr hopf
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
#ifndef DLG_H
#define DLG_H

#define CLASS_MARKER                "MarkerClass"       /* Klassenname fÅr Markerfeld */

#define ID_MAIN                     10
#define ID_ICON                     11
#define ID_MENUFRAME                12

#define IDD_DATETIME                100
#define IDD_SETTING1                200
#define IDD_SETTING2                300
#define IDD_DCF77_1                 400
#define IDD_DCF77_2                 500
#define IDD_DCF77_3                 600
#define IDD_DCF77_STATUS            700
#define IDD_TMOFFS                  800
#define IDD_ALARM                   900
#define IDD_ABOUTBOX                1000
#define IDD_ALARM2                  1100
#define IDD_IOADDR                  1200
#define IDD_COMPORT                 1201
#define IDD_ALARM3                  1300
#define IDD_SERVER                  1400
#define IDD_MESSAGEBOX              1500
#define IDD_GRAPH                   1600
#define IDD_TIMEZONE                1700
#define IDD_SETTING3                1800
#define IDD_AUDIO                   1900
#define IDD_DEBUG                   2000

/* gemeinsame IDs */
#define DID_HELP                    20
#define DID_STANDARD                21

/* TimeDlgProc (IDD_DATETIME) */
/* AlarmDlgProc (IDD_ALARM) */
#define DID_SETHOUR                 101
#define DID_SETMINUTE               102
#define DID_SETSEC                  103
#define DID_SETDAY                  104
#define DID_SETMONTH                105
#define DID_SETYEAR                 106
#define DID_ALRM_ON                 901
#define DID_ALRM_REMINDER           902
#define DID_ALRM_OFF                903
#define DID_ALRM_SOUND              904
#define DID_ALRM_POPUP              905
#define DID_ALRM_LAUNCH             906
#define DID_ALRM_SINGLE             907
#define DID_TXT_HOURS               908
#define DID_TXT_SECONDS             909
#define DID_TXT_DAY                 910
#define DID_TXT_MONTH               911
#define DID_TXT_YEAR                912
#define DID_ALRM_MON                913
#define DID_ALRM_TUE                914
#define DID_ALRM_WED                915
#define DID_ALRM_THU                916
#define DID_ALRM_FRI                917
#define DID_ALRM_SAT                918
#define DID_ALRM_SUN                919
#define DID_TXT_MON                 920
#define DID_TXT_TUE                 921
#define DID_TXT_WED                 922
#define DID_TXT_THU                 923
#define DID_TXT_FRI                 924
#define DID_TXT_SAT                 925
#define DID_TXT_SUN                 926
#define DID_SETREMINDER             927
#define DID_TXT_GROUP2              928
#define DID_TXT_GROUP1              929

/* ClkTicksDlgProc (IDD_SETTING1) */
#define DID_SHW_TIMEONLY            201
#define DID_SHW_DATEONLY            202
#define DID_SHW_DATETIME            203
#define DID_SHW_DIGITAL             204
#define DID_SHW_ANALOG              205
#define DID_SHW_SECHAND             206
#define DID_SHW_HOURTICKS           207
#define DID_SHW_MINTICKS            208
#define DID_SHW_BAVARIAN            209
#define DID_SHW_TITLEBAR            210
#define DID_SAVE_ICONPOS            211

/* ClkColorsDlgProc (IDD_SETTING2) */
#define DID_COL_FACE                301
#define DID_COL_TICK                302
#define DID_COL_BACKGND             303
#define DID_COL_DATE                304
#define DID_COL_HHAND               305
#define DID_COL_MHAND               306
#define DID_COL_DATETXT             307
#define DID_COL_TIMETXT             308
#define DID_COL_TRANSPARENT         309
#define DID_CHG_COLOR               310
#define DID_FONT_DATE               311
#define DID_FONT_TIME               312
#define DID_CHG_FONT                313

/* DCF77DlgProc1 (IDD_DCF77_1) */
#define DID_PORT_SER1               401
#define DID_PORT_SER2               402
#define DID_PORT_SER3               403
#define DID_PORT_SER4               404
#define DID_PORT_GAME               405
#define DID_PORT_SERX               406
#define DID_PORT_GAMEX              407
#define DID_PORTENTRY               408
#define DID_PORTTXT                 409
#define DID_PORT_HOPF6036           410
#define DID_PORT_USB                411
#define DID_PORTNUM                 412
#define DID_PORT_DEVDESC            413

/* DCF77DlgProc2 (IDD_DCF77_2) */
#define DID_UPDINTERVAL             501
#define DID_TXT_UPDINTERVAL         502
#define DID_THRESHOLD               503
#define DID_TXT_THRESHOLD           504
#define DID_HR_TIMER                505
#define DID_CHECKINTERVAL           506

/* DCF77DlgProc3 (IDD_DCF77_3) */
#define DID_POWER_ON                601
#define DID_POWER_1                 602
#define DID_POWER_2                 603
#define DID_SIGNAL_CTL              604
#define DID_SIGNAL_POS              605
#define DID_SIGNAL_SER              606

/* StatusDlgProc (IDD_DCF77_STATUS) */
#define DID_INFO_TIME               701
#define DID_INFO_DATE               702
#define DID_TXT_POLARITY            703
#define DID_INFO_POLARITY           704
#define DID_TXT_VERSION             705
#define DID_INFO_VERSION            706
#define DID_TXT_ANT                 707
#define DID_TXT_LPH                 708
#define DID_TXT_LPS                 709
#define DID_TXT_TZ1                 710
#define DID_TXT_TZ2                 711
#define DID_TXT_CARRIER             712
#define DID_TXT_VALID               713
#define DID_TXT_SCAN                714
#define DID_INFO_ANT                715         // DCF77-generierte Statusmeldungen
#define DID_INFO_LPH                716         //              "
#define DID_INFO_LPS                717         //              "
#define DID_INFO_TZ1                718         //              "
#define DID_INFO_TZ2                719         //              "
#define DID_INFO_CARRIER            720         // Treibergenerierte Statusmeldungen
#define DID_INFO_VALID              721         //              "
#define DID_INFO_SCAN               722         //              "
#define DID_INFO_RECOG              723
#define DID_INFO_WRONG              724
#define DID_INFO_SET                725
#define DID_TXT_RECOG1              726
#define DID_TXT_RECOG2              727
#define DID_TXT_WRONG1              728
#define DID_TXT_WRONG2              729
#define DID_TXT_SET1                730
#define DID_TXT_SET2                731
#define DID_INFO_POSITION           732

/* TmOffsDlgProc (IDD_TMOFFS) */
#define DID_ISUTC                   801
#define DID_GOFFSET                 802
#define DID_LOFFSET                 803
#define DID_TZTXT                   804

/* Alarm2DlgProc (IDD_ALARM2) */
#define DID_ALARMTEXT               1101
#define DID_WAVE_PROG               1102
#define DID_WAVE_SRCH               1103

/* IODlgProc (IDD_IOADDR) */
#define DID_IOADDR                  1202

/* Alarm3DlgProc (IDD_ALARM3) */
#define DID_ALRM_PROG               1301
#define DID_ALRM_PARM               1302
#define DID_ALRM_DIR                1303
#define DID_ALRM_SRCH               1304

/* SrvrIntervalDlgProc (IDD_SERVER) */
#define DID_SERVERINTERVAL          1401
#define DID_TXT_INTERVAL            1402
#define DID_SERVERSENDTIME          1403

/* TimezoneDlgProc (IDD_TIMEZONE) */
#define DID_TXT_TZSUMMER            1701
#define DID_TXT_TZWINTER            1702
#define DID_TZ_FILE                 1703
#define DID_TIMEZONE                1704
#define DID_TZ_SELECT               1705

/* ClkBmpsDlgProc (IDD_SETTING3) */
#define DID_SET3_HOUR               1801
#define DID_SET3_QHOUR              1802
#define DID_SET3_MINUTE             1803
#define DID_SET3_QMINUTE            1804
#define DID_SET3_FACE               1805
#define DID_SET3_QFACE              1806

/* ClkAudioDlgProc (IDD_AUDIO) */
#define DID_GONG_PROG               1901
#define DID_GONG_SRCH               1902
#define DID_GONG_MD1                1903
#define DID_GONG_MDN                1904
#define DID_GONG_TEXT               1905

/* DbgDlgProc (IDD_DEBUG) */
#define DID_TL_0                    2001
#define DID_TL_1                    2002
#define DID_TL_2                    2003
#define DID_WL_0                    2004
#define DID_WL_1                    2005
#define DID_WL_2                    2006
#define DID_AL_0                    2007
#define DID_AL_1                    2008
#define DID_AL_2                    2009
#define DID_DM                      2010
#define DID_DT                      2011

#endif /* DLG_H */
