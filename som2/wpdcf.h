/***************************************************************************\
 *
 * PROGRAMMNAME: WPDCF77
 * -------------
 *
 * VERSION: 6.0
 * --------
 *
 * MODULNAME: WPDCF.H
 * ----------
 *
 * BESCHREIBUNG:
 * -------------
 *   allgemeine Definitionen f�r DCF77-WPS-Uhr
 *
 *  Ver.    Date      Comment
 *  ----    --------  -------
 *  1.00    10-23-94  First release
 *  2.00    06-16-95  Extensions
 *  3.00    10-30-95  Client/Server Support; SOM2
 *  4.00    03-15-96  Unterst�tzung f�r hopf
 *  4.20    10-15-96  Unterst�tzung f�r hopf GPS
 *  4.30    05-17-97  Unterst�tzung HR-Timer; Bitmap-Unterst�tzung
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
#ifndef WPDCF_H
#define WPDCF_H

/* Typ-Definition des Threads f�r _beginthread */
typedef VOID _Optlink FNTHD(PVOID);
typedef FNTHD *PFNTHD;

/* Benutzerdefinierte Ansichten; siehe auch "IDM_*-Werte" in res.h */
#define OPEN_ANALOG     OPEN_USER           // Anzeige mit Ziffernblatt u. Zeiger
#define OPEN_DIGITAL    OPEN_USER+1         // Digitalanzeige
#define OPEN_ABOUT      OPEN_USER+2         // Produktinformation
#define OPEN_STATUS     OPEN_USER+3         // DCF77-Status
#define OPEN_GRAPH      OPEN_USER+4         // DCF77-Grafik f�r Empfangspegelverlauf
#define OPEN_ANTADJ     OPEN_USER+5         // DCF77-Grafik f�r Antenneneinrichtung beim Hopf 6036-Modul
#define OPEN_REMIND     OPEN_USER+6         // Erinnerungsfunktion ein-/ausschalten
#define OPEN_DEBUG      OPEN_USER+7         // DEBUG-Einstellungen

/* Window-Data-Struktur: f�r Uhr (ClkWndProc) */
typedef struct _WINDOWDATA
    {
    USHORT      cb;                     // Gr��e der Struktur
    WPDCF77     *somSelf;               // Zeiger auf die Instanz
    WPDCF77Data *somThis;               // Zeiger auf Instanzdaten
    USEITEM     UseItem;                // global class usage information
    VIEWITEM    ViewItem;               // global class view information
    ULONG       cxRes;                  // horizontale Bildschirmaufl�sung
    ULONG       cyRes;                  // vertikale Bildschirmaufl�sung
    ULONG       cColorPlanes;           // Farbebenen Grafikkarte
    ULONG       cColorBitcount;         // Bits pro Farbebene
    } WINDOWDATA;
typedef WINDOWDATA *PWINDOWDATA;

/* Funktions-Prototypen */
HWND ClkInit (WPDCF77 *somSelf);

#define CLASSNAME       "WPDCF77"           // Als Id-String f�r Instanzvariable

#define DEFAULT_TITLE   "Systemuhr"         // Default-Titel

/* Keys f�r Instanzdaten */
#define IDKEY_PRESENCE       1      // Key f�r _ulPresence
#define IDKEY_BACKGND        2      // Key f�r _clrBackground
#define IDKEY_FACE           3      // Key f�r _clrFace
#define IDKEY_TICK           4      // Key f�r _clrTick
#define IDKEY_HHAND          5      // Key f�r _clrHourHand
#define IDKEY_MHAND          6      // Key f�r _clrMinuteHand
#define IDKEY_DATE           7      // Key f�r _clrDateOnAnalog
#define IDKEY_DATETXT        8      // Key f�r _clrDateText
#define IDKEY_TIMETXT        9      // Key f�r _clrTimeText
#define IDKEY_TRANSPARENT   10      // Key f�r _clrTransparent
#define IDKEY_ALARM         11      // Key f�r _dtAlarm
#define IDKEY_LOFFSET       12      // Key f�r _lLOffset
#define IDKEY_FAMILYD       13      // Key f�r _szFontFamilyD
#define IDKEY_FAMILYT       14      // Key f�r _szFontFamilyT
#define IDKEY_FATTRSD       15      // Key f�r _fAttrD
#define IDKEY_FATTRST       16      // Key f�r _fAttrT
#define IDKEY_CX            17      // Key f�r _lcxClock
#define IDKEY_CY            18      // Key f�r _lcyClock
#define IDKEY_X             19      // Key f�r _lxPos
#define IDKEY_Y             20      // Key f�r _lyPos
#define IDKEY_ALARMTXT      21      // Key f�r _szAlarmMsg
#define IDKEY_ALARMMMFILE   22      // Key f�r _szAlarmSoundFile
#define IDKEY_ALARMPROG     23      // Key f�r _szAlarmProg
#define IDKEY_ALARMPARM     24      // Key f�r _szAlarmParm
#define IDKEY_ALARMDIR      25      // Key f�r _szAlarmDir
#define IDKEY_TIMEZONE      26      // Key f�r _szTimezone
#define IDKEY_GRAPHCX       27      // Key f�r _lcxGraph
#define IDKEY_GRAPHCY       28      // Key f�r _lcyGraph
#define IDKEY_GRAPHX        29      // Key f�r _lxGraphPos
#define IDKEY_GRAPHY        30      // Key f�r _lyGraphPos
#define IDKEY_BMP_HHAND     31      // Key f�r _szPathHHandBmp
#define IDKEY_BMP_MHAND     32      // Key f�r _szPathMHandBmp
#define IDKEY_BMP_FACE      33      // Key f�r _szPathFaceBmp
#define IDKEY_IX            34      // Key f�r _lxIconPos
#define IDKEY_IY            35      // Key f�r _lyIconPos

/* Application name und Key names f�r Klassendaten */
#define APP_DCF         CLASSNAME
#define KEY_SETTINGS    "Settings"          // Key f�r DCF77_DATA
#define KEY_OFFSET      "TOffset"           // Key f�r DCF77_OFFSET
#define KEY_INTERVAL    "SrvrInterval"      // Key f�r ulSrvrInterval
#define KEY_SENDTIME    "SrvrSendTime"      // Key f�r bSendSrvrTime
#define KEY_TIMEZONE    "TimeZone"          // Key f�r Zeitzone
#define KEY_AUDIOFILE   "AudioFile"         // Key f�r AudioFile
#define KEY_AUDIOMODE   "AudioMode"         // Key f�r Mode (Is1PerHour)
#define KEY_AUDIOOBJECT "AudioObject"       // Key f�r AudioObject (RingingObject)

#define SAVESET_INIDATA         0x0001      // DCF77_DATA in INI-Datei schreiben bzw. erfolgreich gelesen
#define SAVESET_INIOFFSET       0x0002      // DCF77_OFFSET in INI-Datei schreiben bzw. erfolgreich gelesen
#define SAVESET_INIINTERVAL     0x0004      // ulSrvInterval in INI-Datei schreiben bzw. erfolgreich gelesen
#define SAVESET_INITIMEZONE     0x0008      // szLocFile+szTZLocale in INI-Datei schreiben bzw. erfolgreich gelesen
#define SAVESET_DRVRDATA        0x0010      // DCF77_DATA an Treiber �bergeben
#define SAVESET_DRVROFFSET      0x0020      // DCF77_OFFSET an Treiber �bergeben
#define SAVESET_SHMEMINTERVAL   0x0040      // ulSrvInterval/bSendSrvrInterval in Shared memory schreiben
#define SAVESET_AUDIODATA       0x0080      // Daten f�r Audio-Support schreiben bzw. erfolgreich gelesen

#define PRESENCE_DEFAULT    PRESENCE_MINORTICKS | PRESENCE_MAJORTICKS | \
                            PRESENCE_TITLEBAR   | PRESENCE_CARRIER    | \
                            PRESENCE_DISP_TIME  | PRESENCE_DISP_DATE  | \
                            PRESENCE_DISP_ANALOG

#define INTERVAL_DEFAULT    600             // Intervall f�r LAN::Time in sec.
#define SENDTIME_DEFAULT    TRUE            // Default f�r bSendSrvrTime

/* Stringl�ngen schlie�en Terminierungszeichen '\0' ein */
//#define CCHMAXPGNAME    32                  // max. Stringl�nge f�r Seitennamen
//#define CCHMAXALARMMSG  128                 // max. Stringl�nge f�r den Alarmtext
//#define CCHMAXTZONE     25                  // max. Stringl�nge f�r Zeitzone
//#define CCHMAXDATETIME  12                  // max. Stringl�nge f�r Zeit-/Datumstring
//#define CCHMAXPARM      256                 // max. Stringl�nge f�r Parameterstring Alarmprogramm
#define CCHMAXMSG       80                  // max. Stringl�nge f�r Messagetexte

/* Zahl der Bit-Shifts f�r SOM_*Level - Variable */
#define SHFT_TRACE              0
#define SHFT_WARN               2
#define SHFT_ASSERT             4

/* Definitionen f�r die Darstellung der Uhr */
#define PRESENCE_MINORTICKS     0x00000001
#define PRESENCE_MAJORTICKS     0x00000002
#define PRESENCE_TITLEBAR       0x00000004
#define PRESENCE_WINTER         0x00000008
#define PRESENCE_CARRIER        0x00000010
#define PRESENCE_BAVARIAN       0x00000020
#define PRESENCE_MINMAXPOS      0x00000040
#define PRESENCE_DISP_TIME      0x00001000
#define PRESENCE_DISP_DATE      0x00002000
#define PRESENCE_DISP_SEC_HAND  0x00100000
#define PRESENCE_DISP_MINIMIZE  0x00200000
#define PRESENCE_DISP_DIGITAL   0x10000000
#define PRESENCE_DISP_ANALOG    0x20000000

/* Default-Farben */
#define CLR_DEFAULT_BACKGND     0x00B000        /* Kotzgr�n */
#define CLR_DEFAULT_FACE        0x00B000        /* Kotzgr�n */
#define CLR_DEFAULT_TICK        0xFF6440        /* Hellrot  */
#define CLR_DEFAULT_HHAND       0xD00000        /* Blutrot  */
#define CLR_DEFAULT_MHAND       0xD00000        /* Blutrot  */
#define CLR_DEFAULT_DATEONANALG 0x00E000        /* Hellgr�n */
#define CLR_DEFAULT_TRANSPARENT 0x008080        /* Cyan     */

/* Name der Help-Datei */
#define HELPFILENAME            "WPDCF77.HLP"

/* Definitionen f�r Fonteinstellung */
#define FONT_DEFAULT_FACE       "Courier"
#define FONT_DEFAULT_FAMILY     "Courier"
#define FONT_DEFAULT_TYPE       0
#define FONT_DEFAULT_USE        FATTR_FONTUSE_OUTLINE

#define LCID_DATE               100             /* Font f�r Datum */
#define LCID_TIME               101             /* Font f�r Uhrzeit */

/* Benutzerspezifische Window-Messages */
#define WM_COMPLETEINITDLG      WM_USER         // ClkTicksDlgProc, ClkColorsDlgProc
#define WM_UPDATESPB            WM_USER+1       // TimeDlgProc
#define WM_MESSAGE              WM_USER+2       // ClkWndProc;      mp1: Message-Text
                                                //                  mp2: Message-Style
#define WM_1STIMER              WM_USER+3       // 1s-Timer
#define WM_SETMARKER            WM_USER+4       // InfoWndProc: Markersymbol zeichnen
                                                //                  mp1: Marker-Symbol (LONG)
#define WM_ERRHWHOPF            WM_USER+5       // GraphWndProc: Problem mit hopf-Hardware
#define WM_REPAINT              WM_USER+6       // GraphWndProc: Daten in Grafiksegment laden und neu zeichnen

#define WM_USER_STARTPLAY       WM_USER+7       // ClkWndProc: Initialisierung des Audio-Teils
#define WM_PLAY_FINISHED        WM_USER+8       // ClkWndProc: Abspielen der Audiodatei beendet
#define WM_PLAY_MMFILE          WM_USER+9       // ClkWndProc: Abspielen einer MM-Datei �ber ClkMciPlayFile

#define WM_SETCELL              WM_USER+780     // NewFrameWndProc; mp1: RGB-Wert
#define WM_SETCELL40            0x601           // NewFrameWndProc; Ersatz f�r WM_SETCELL unter Warp 4.0

#endif /* WPDCF_H */
