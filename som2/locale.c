/***************************************************************************\
 *
 * PROGRAM NAME: WPDCF77
 * -------------
 *
 * VERSION: 6.0
 * --------
 *
 * MODULE NAME: LOCALE
 * ------------
 *
 * DESCRIPTION:
 * ------------
 *   Responsible for the settings pages to set the time zone
 *
 * NOTES:
 * ------
 *
 * FUNCTIONS:
 * -----------
 *   GetWeekday ()
 *   GetDTfromLDT ()
 *   GetSwitchDate ()
 *   IsBeforeDate ()
 *   IsValidInterval ()
 *   Read2Dates ()
 *   ComposeTextLine ()
 *   FillStaticText ()
 *   FillCombobox ()
 *   FillControls ()
 *   InitTZFile ()
 *   InitTZName ()
 *   TZNewFile ()
 *   TZInit ()
 *   TZCommand ()
 *   TZControl ()
 *   TZTimer ()
 *   TZClose ()
 *   TimezoneDlgProc ()
 *   DaysInMonth ()
 *   InitGPSDriver ()
 *
 *  Ver.    Date      Comment
 *  ----    --------  -------
 *  1.00    10-23-94  First release
 *  2.00    06-16-95  Extensions
 *  3.00    10-30-95  Client/Server support; SOM2
 *  4.00    03-15-96  Support for hopf
 *  4.20    10-15-96  Support for hopf GPS
 *  4.30    17-05-97  Support for HR-Timer
 *  4.40    01-31-00  Multimedia support, bug fixing
 *  5.10    02-10-01  Bug fixing position of icon view
 *  5.20    06-09-01  Bug fixing audio data
 *  5.30    01-16-02  Implementation of reminder function
 *  5.40    11-22-03  Bug fixing reminder function
 *  6.00    02-15-04  USB support
 *
 *  Copyright (C) noller & breining software 1995...2007
 *
\******************************************************************************/
#define INCL_DOSDATETIME
#define INCL_DOSDEVICES
#define INCL_DOSDEVIOCTL
#define INCL_DOSFILEMGR
#define INCL_DOSMISC
#define INCL_GPITRANSFORMS
#define INCL_WINDIALOGS
#define INCL_WINENTRYFIELDS
#define INCL_WINLISTBOXES
#define INCL_WINWINDOWMGR
#define INCL_WINSTDFILE
#define INCL_WINCOUNTRY
#include <os2.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "tloc.h"
#include "dcfioctl.h"

#pragma info(noeff)         // suppress SOM compiler informationals

#include "wpdcf77.ih"
#include "wpdcf.h"

#include "cutil.h"
#include "clock.h"
#include "clockpan.h"
#include "clkdata.h"
#include "res.h"
#include "dlg.h"
#include "dcf77.h"
#include "sz_wz.h"
#include "locale.h"
#include "debug.h"

/*--------------------------------------------------------------*\
 *  Globale Definitionen und Variable fr dieses Modul          *
\*--------------------------------------------------------------*/
SHORT DaysPerMonth[13] = {0,31,28,31,30,31,30,31,31,30,31,30,31};

/* Stringl„ngen (einschlieálich Terminierungszeichen '\0') */
#define CCHMAXPARSELINE     80
#define CCHMAXSTATICTEXT    40

/*******************************************************************\
    GetWeekday: Berechnung des Wochentags aus dem Datum.
                Die Berechnung erfolgt unabh„ngig vom Gltigkeits-
                beginn des Gregorianischen Kalenders
    Eingang:    pdt: DATETIME mit day, month, year
    Return:     0=So, 1=Mo, 2=Di, 3=Mi, 4=Do, 5=Fr, 6=Sa
\*******************************************************************/
USHORT GetWeekday (PDATETIME pdt)
    {
    ULONG c_month;
    ULONG c_year;
    ULONG days;

    c_month = (pdt->month + 9) % 12;
    c_year  = pdt->year - ((c_month > 9) ? 1 : 0);

    days  = ((c_month * 26) + 5) / 10;
    days += (c_year * 5) / 4 - c_year/100 + c_year/400;
    days += pdt->day + 2;

    return days % 7;
    }

/*******************************************************************\
    GetDTfromLDT: Rechnet pldt fr das aktuelle Jahr in ein
                  Datum um (Umsetzen der Intervalle)
    Eingang: PTLOC_DEF pldt:       Intervall
             USHORT usYear:        aktuelles Jahr
    Ausgang: PDATETIME pdtSwitch:  Umschaltzeit
\*******************************************************************/
VOID GetDTfromLDT (PTLOC_DEF pldt, USHORT usYear, PDATETIME pdtSwitch)
    {
    if (pldt->usLastDay == 0)
        {
        /* kein Tages-Intervall -> Wochentagintervall */
        pdtSwitch->hours      = pldt->usHours;
        pdtSwitch->minutes    = pldt->usMinutes;
        pdtSwitch->seconds    = 0;
        pdtSwitch->hundredths = 0;
        pdtSwitch->day        = pldt->usFirstDay;
        pdtSwitch->month      = pldt->usMonth;
        pdtSwitch->year       = usYear;
        pdtSwitch->timezone   = 0;

        pdtSwitch->weekday    = GetWeekday (pdtSwitch);
        }
    else
        {
        /* Tages-Intervall -> kein Wochentagintervall */
        pdtSwitch->hours      = pldt->usHours;
        pdtSwitch->minutes    = pldt->usMinutes;
        pdtSwitch->seconds    = 0;
        pdtSwitch->hundredths = 0;
        pdtSwitch->day        = pldt->usFirstDay;
        pdtSwitch->month      = pldt->usMonth;
        pdtSwitch->year       = usYear;
        pdtSwitch->timezone   = 0;
        pdtSwitch->weekday    = pldt->usFirstWeekday;
        pdtSwitch->day        = pldt->usFirstDay +
                                (7 + pldt->usFirstWeekday - GetWeekday (pdtSwitch)) % 7;

        }

    return;
    }

/*******************************************************************\
    IsBeforeDate: Datum 1 ist vor Datum 2
    Eingang: PDATETIME pdt1:  Datum 1
             PDATETIME pdt1:  Vergleichsdatum 2
\*******************************************************************/
BOOL IsBeforeDate (PDATETIME pdt1, PDATETIME pdt2)
    {
    if (((ULONG)pdt1->month<<16) + (ULONG)((pdt1->day<<11) + (pdt1->hours<<6) + pdt1->minutes) <
        ((ULONG)pdt2->month<<16) + (ULONG)((pdt2->day<<11) + (pdt2->hours<<6) + pdt2->minutes))
        return TRUE;
    return FALSE;
    }

/*******************************************************************\
    GetSwitchDate: Berechnet aus dem aktuellen Jahr und dem
                   Sommerzeit/Winterzeit-Umschaltintervall die
                   n„chste Umschaltzeit.
    Eingang: PTLOC_DEF pldt:       Intervall
             USHORT usYear:        aktuelles Jahr
    Ausgang: PDATETIME pdtSwitch:  Umschaltzeit
\*******************************************************************/
VOID GetSwitchDate (PTLOC_DEF pldt, PDATETIME pdtActual, PDATETIME pdtSwitch)
    {
    DATETIME dt;

    GetDTfromLDT (pldt, pdtActual->year, &dt);
    if (pldt->usLastYear != 0 && !IsBeforeDate (pdtActual, &dt))
        GetDTfromLDT (pldt, pdtActual->year + 1, &dt);

    *pdtSwitch = dt;
    return;
    }

/*******************************************************************\
    IsValidInterval: Prft, ob das *pldt-Intervall noch fr *pdt
                     gltig ist.
    Eingang: PTLOC_DEF pldt: Intervall
             PDATETIME pdt:  aktuelle Zeit
\*******************************************************************/
BOOL IsValidInterval (PTLOC_DEF pldt, PDATETIME pdt)
    {
    DATETIME dt;

    if (pldt->usLastYear == 0 && pdt->year == pldt->usFirstYear)
        {
        GetDTfromLDT (pldt, pdt->year, &dt);
        return IsBeforeDate (pdt, &dt);
        }
    else if ((pdt->year >= pldt->usFirstYear) && (pdt->year <= pldt->usLastYear))
        {
        GetDTfromLDT (pldt, pdt->year, &dt);
        if (IsBeforeDate (pdt, &dt))
            return TRUE;            // Umschaltzeitpunkt in diesem Jahr noch nicht erreicht
        if (pdt->year+1 <= pldt->usLastYear)
            return TRUE;            // n„chstes Jahr geh”rt auch noch zum gltigen Intervall
        }

    return FALSE;
    }

/*******************************************************************\
    Read2Dates: Liest aus der TZ-Datei 2 Zeitintervalle, die fr
                das aktuelle Datum gltig sind. Der Dateizeiger muá
                dazu hinter der heading-Zeile des gewnschten
                Eintrages stehen.
                Im Fehlerfall werden mit '0' gefllte Strukturen
                zurckgegeben
    Eingang: HFILE     hFile:        Dateihandle
             PSZ       pszBuffer:    Puffer fr Dateioperationen
             USHORT    usLen:        L„nge des Puffers
             PDATETIME pdt:          aktuelles Datum/Uhrzeit
    Ausgang: PTLOC_DEF pldt1, pldt2: Ergebnis
\*******************************************************************/
VOID Read2Dates (HFILE hFile, PSZ pszBuffer, USHORT usLen, PTLOC_DEF pldt1, PTLOC_DEF pldt2, PDATETIME pdt)
    {
    TLOC_DEF ldt;
    BOOL     bRC;

    memset (pldt1, 0, sizeof (TLOC_DEF));
    memset (pldt2, 0, sizeof (TLOC_DEF));

    /* Erstes gltiges Intervall suchen */
    while ((bRC = GetTZString (hFile, pszBuffer, usLen, &ldt)) == TRUE)
        if (IsValidInterval (&ldt, pdt))
            {
            *pldt1 = ldt;
            break;
            }

    /* Zweites gltiges Intervall suchen */
    if (bRC)
        {
        while (GetTZString (hFile, pszBuffer, usLen, &ldt))
            if (IsValidInterval (&ldt, pdt))
                {
                *pldt2 = ldt;
                break;
                }
        }

    return;
    }

/*******************************************************************\
    ComposeTextLine: Konvertiert eine TLOC_DEF-Struktur in ein
                     lesbares ASCII-Format
    Eingang: PTLOC_DEF pldt:    Eingangsstruktur
             PDATETIME pdt:     aktuelles Datum
    Ausgang: PSZ       pszLine: Ergebnispuffer
             ULONG     usLen:   L„nge des Puffers
\*******************************************************************/
PCHAR ComposeTextLine (PTLOC_DEF pldt, PDATETIME pdt, PSZ pszLine, ULONG usLen)
    {
    CHAR      szTemp[CCHMAXSTATICTEXT];
    DATETIME  dtSwitch;
    CHAR      szWeekday[3];
    CHAR      szDate[11];
    CHAR      szTime[12];
    CHAR      szUTCDiff[7];
    PCHAR     pszTable[] = {szWeekday, szDate, szTime, szUTCDiff};

    if (pldt->usMonth == 0)
        WinLoadString (hab, hmod, IDS_INVALIDDATE, usLen, pszLine);
    else
        {
        GetSwitchDate (pldt, pdt, &dtSwitch);
        WinLoadString (hab, hmod, IDS_WEEKDAY_SUN + dtSwitch.weekday, 3, szWeekday);
        GetDateTime (&dtSwitch, MODE_PM, szTime, szDate);
        szUTCDiff[0] = '+';
        _itoa (pldt->sTimeDiff, &szUTCDiff[pldt->sTimeDiff >= 0 ? 1 : 0], 10);

        DosInsertMessage (pszTable, 4,
                          szTemp, WinLoadString (hab, hmod, IDS_SWITCHDATETIME, CCHMAXSTATICTEXT, szTemp),
                          pszLine, usLen, &usLen);
        pszLine[usLen] = '\0';
        }

    return pszLine;
    }

/*******************************************************************\
    FillStaticText: Fllt 2 TLOC_DEF-Strukturen in die Textfelder
                    der Dialogbox
    Eingang: HWND      hwnd:         Dateihandle
             PTLOC_DEF pldt1, pldt2: Eingangsstrukturen
             PDATETIME pdt:          aktuelles Datum
\*******************************************************************/
VOID FillStaticText (HWND hwnd, PTLOC_DEF pldt1, PTLOC_DEF pldt2, PDATETIME pdt)
    {
    CHAR      szTemp[CCHMAXSTATICTEXT];
    PTLOC_DEF pldtWinter, pldtSummer;

    if (pldt1->usMonth < pldt2->usMonth)
        {
        pldtWinter = pldt2;
        pldtSummer = pldt1;
        }
    else
        {
        pldtWinter = pldt1;
        pldtSummer = pldt2;
        }

    /* IDs DID_TXT_TZSUMMER und DID_TXT_TZWINTER fllen */
    ComposeTextLine (pldtWinter, pdt, szTemp, CCHMAXSTATICTEXT);
    WinSetDlgItemText (hwnd, DID_TXT_TZWINTER, szTemp);

    ComposeTextLine (pldtSummer, pdt, szTemp, CCHMAXSTATICTEXT);
    WinSetDlgItemText (hwnd, DID_TXT_TZSUMMER, szTemp);

    return;
    }

/*******************************************************************\
    FillCombobox: Fllt die Combobox mit den Zeitzonenstrings
    Eingang: HWND         hwnd:      Window-Handle der Dialogbox
             WPDCF77Data *somThis:   Zeiger auf Instanzdaten
             HFILE        hFile:     Dateihandle
             PSZ          pszBuffer: Puffer fr Dateioperationen
             USHORT       usLen:     L„nge des Puffers
\*******************************************************************/
VOID FillCombobox (HWND hwnd, WPDCF77Data *somThis, HFILE hFile, PSZ pszBuffer, USHORT usLen)
    {
    SHORT i;

    RewindTZFile (hFile);

    /* Combobox auffllen */
    for (i=0; GetNextLineOfType (hFile, pszBuffer, usLen, heading); i++)
        WinSendDlgItemMsg (hwnd, DID_TIMEZONE, LM_INSERTITEM, MPFROMSHORT (LIT_END), (MPARAM)pszBuffer);

    /* aktuell eingestellte Zeitzone selektieren */
    if (_pIData->sTZIndex)
        WinSendDlgItemMsg (hwnd, DID_TIMEZONE, LM_SELECTITEM, MPFROMSHORT (_pIData->sTZIndex-1), MPFROMLONG (TRUE));

    return;
    }

/*******************************************************************\
    FillControls: Fllt die Ausgabecontrols der Dialogbox
    Eingang: HWND         hwnd:       Window-Handle der Dialogbox
             WPDCF77Data *somThis:    Zeiger auf Instanzdaten
             HFILE        hFile:      Dateihandle
             PSZ          pszBuffer:  Puffer fr Dateioperationen
             USHORT       usLen:      L„nge des Puffers
             BOOL         bFillCombo: Combobox aktualisieren
\*******************************************************************/
VOID FillControls (HWND hwnd, WPDCF77Data *somThis, HFILE hFile, PSZ pszBuffer, USHORT usLen, BOOL bFillCombo)
    {
    TLOC_DEF ldt1, ldt2;
    DATETIME dt;

    /* Bearbeiten der Locale-Datei */
    DosGetDateTime (&dt);

    if (_pIData->sTZIndex)
        {
        Read2Dates (hFile, pszBuffer, usLen, &ldt1, &ldt2, &dt);
        FillStaticText (hwnd, &ldt1, &ldt2, &dt);
        }
    else
        {
        WinSetDlgItemText (hwnd, DID_TXT_TZWINTER, "");
        WinSetDlgItemText (hwnd, DID_TXT_TZSUMMER, "");
        if (_pIData->sTZIndex)
            MessageBox (hwnd, IDMSG_ERRLOCENTRY, MBTITLE_ERROR, MB_OK | MB_ERROR, WA_ERROR, NULL);
        }

    if (bFillCombo)
        FillCombobox (hwnd, somThis, hFile, pszBuffer, usLen);

    return;
    }

/*******************************************************************\
    InitTZFile: ™ffnet die TZ-Datei und fllt den Namen in das
                Textfeld ein
    Eingang: HWND         hwnd:      Window-Handle der Dialogbox
             WPDCF77Data *somThis:   Zeiger auf Instanzdaten
\*******************************************************************/
VOID InitTZFile (HWND hwnd, WPDCF77Data *somThis)
    {
    CloseTZFile (_pIData->hLocFile);

    DebugE (D_WND, "InitTZFile, Datei", _pIData->szLocFile);
    if (OpenTZFile (_pIData->szLocFile, &(_pIData->hLocFile)) == 0)
        WinSetDlgItemText (hwnd, DID_TZ_FILE, _pIData->szLocFile);
    else
        _pIData->hLocFile = -1;

    return;
    }

/*******************************************************************\
    InitTZName: Fllt die Controls der Dialogbox
    Eingang: HWND         hwnd:       Window-Handle der Dialogbox
             WPDCF77Data *somThis:    Zeiger auf Instanzdaten
             BOOL         bFillCombo: Combobox aktualisieren
\*******************************************************************/
VOID InitTZName (HWND hwnd, WPDCF77Data *somThis, BOOL bFillCombo)
    {
    CHAR         szParseBuffer[CCHMAXPARSELINE];

    RewindTZFile (_pIData->hLocFile);
    DebugE (D_WND, "InitTZName, _pIData->szLocName", _pIData->szLocName);
    _pIData->sTZIndex = FindEntry (_pIData->hLocFile, szParseBuffer, CCHMAXPARSELINE, _pIData->szLocName);
    DebugULd (D_WND, "InitTZName", "_pIData->sTZIndex", _pIData->sTZIndex);
    FillControls (hwnd, somThis, _pIData->hLocFile, szParseBuffer, CCHMAXPARSELINE, bFillCombo);

    return;
    }

/*******************************************************************\
    TZNewFile: ™ffnet die TZ-Datei und fllt die Controls der
               Dialogbox
    Eingang: HWND         hwnd:      Window-Handle der Dialogbox
             WPDCF77Data *somThis:   Zeiger auf Instanzdaten
\*******************************************************************/
VOID TZNewFile (HWND hwnd, WPDCF77Data *somThis)
    {
    strcpy (_pIData->szLocFile,    szLocFile);
    strcpy (_pIData->szOldLocFile, szLocFile);
    strcpy (_pIData->szLocName,    szLocName);
    strcpy (_pIData->szOldLocName, szLocName);

    _pIData->bNoControl = TRUE;
    InitTZFile (hwnd, somThis);
    InitTZName (hwnd, somThis, TRUE);
    _pIData->bNoControl = FALSE;

    return;
    }

/*******************************************************************\
    TZInit: Callback-Routine fr WM_INITDLG-Message
    Eingang: HWND     hwnd:    Window-Handle der Dialogbox
             WPDCF77 *somSelf: Objekt-Zeiger
\*******************************************************************/
VOID TZInit (HWND hwnd, WPDCF77 *somSelf)
    {
    WPDCF77Data *somThis = WPDCF77GetData (somSelf);

    WinSetWindowPtr (hwnd, QWL_USER, (PVOID)somSelf);

    _pIData->hLocFile = -1;
    TZNewFile (hwnd, somThis);

    /* Timer zur Aktualisierung der Anzeige starten */
    _wpAdd1sTimer (somSelf, hwnd);
    return;
    }

/*******************************************************************\
    TZCommand: Callback-Routine fr WM_COMMAND-Message
    Eingang: HWND   hwnd:   Window-Handle der Dialogbox
             USHORT dlgid:  ID des ausl”senden Controls
             USHORT source: Ausl”sende Aktion CMDSRC_*
\*******************************************************************/
BOOL TZCommand (HWND hwnd, USHORT dlgid, USHORT source)
    {
    CHAR         szBuffer[CCHMAXPGNAME];
    FILEDLG      FileDlg;
    WPDCF77     *somSelf = (WPDCF77 *)WinQueryWindowPtr (hwnd, QWL_USER);
    WPDCF77Data *somThis = WPDCF77GetData (somSelf);

    if (source == CMDSRC_PUSHBUTTON)
        {
        switch (dlgid)
            {
            case DID_CANCEL:
                TZNewFile (hwnd, somThis);
                return TRUE;

            case DID_TZ_SELECT:
                WinLoadString (hab, hmod, IDS_SEARCH_TZTAB, CCHMAXPGNAME, szBuffer);
                WinQueryDlgItemText (hwnd, DID_TZ_FILE, CCHMAXPATH, FileDlg.szFullFile);
                FileDlg.cbSize          = sizeof (FILEDLG);
                FileDlg.fl              = FDS_OPEN_DIALOG | FDS_CENTER | FDS_HELPBUTTON;
                FileDlg.ulUser          = 0;
                FileDlg.pszTitle        = szBuffer;
                FileDlg.pszOKButton     = NULL;
                FileDlg.pfnDlgProc      = NULL;
                FileDlg.pszIType        = NULL;
                FileDlg.papszITypeList  = NULL;
                FileDlg.pszIDrive       = NULL;
                FileDlg.papszIDriveList = NULL;
                FileDlg.hMod            = NULLHANDLE;
                FileDlg.usDlgId         = 0;
                FileDlg.x               = 0;
                FileDlg.y               = 0;

                WinFileDlg (HWND_DESKTOP, hwnd, &FileDlg);
                DebugE (D_WND, "TZCommand, szFullFile", FileDlg.szFullFile);
                if (FileDlg.lReturn == DID_OK)
                    {
                    _pIData->bNoControl = TRUE;
                    CloseTZFile (_pIData->hLocFile);
                    strcpy (_pIData->szLocFile, FileDlg.szFullFile);
                    WinUpper (hab, 0, 0, _pIData->szLocFile);
                    InitTZFile (hwnd, somThis);
                    InitTZName (hwnd, somThis, TRUE);
                    _pIData->bNoControl = FALSE;
                    }
                return TRUE;
            }
        }

    return FALSE;
    }

/*******************************************************************\
    TZControl: Callback-Routine fr WM_CONTROL-Message
    Eingang: HWND   hwnd:   Window-Handle der Dialogbox
             USHORT dlgid:  ID des ausl”senden Controls
             USHORT notify: Ausl”sende Aktion CBN_*
\*******************************************************************/
BOOL TZControl (HWND hwnd, USHORT dlgid, USHORT notify)
    {
    SHORT        index;
    WPDCF77     *somSelf = (WPDCF77 *)WinQueryWindowPtr (hwnd, QWL_USER);
    WPDCF77Data *somThis = WPDCF77GetData (somSelf);

    switch (dlgid)
        {
        case DID_TIMEZONE:
            if (notify == CBN_LBSELECT && !_pIData->bNoControl && IsValidHandle (_pIData->hLocFile))
                {
                _pIData->bNoControl = TRUE;
                index = (SHORT)WinSendDlgItemMsg (hwnd, DID_TIMEZONE,
                                                  LM_QUERYSELECTION,
                                                  MPFROMSHORT (LIT_FIRST),
                                                  MPVOID);
                WinSendDlgItemMsg (hwnd, DID_TIMEZONE,
                                   LM_QUERYITEMTEXT,
                                   MPFROM2SHORT (index, CCHMAXTZONE),
                                   MPFROMP (_pIData->szLocName));
                DebugE (D_WND, "TZControl, Text", _pIData->szLocName);
                InitTZName (hwnd, somThis, FALSE);
                _pIData->bNoControl = FALSE;
                }
            return TRUE;
        }

    return FALSE;
    }

/*******************************************************************\
    TZTimer: Callback-Routine fr WM_1STIMER-Message
    Eingang: HWND   hwnd:   Window-Handle der Dialogbox
\*******************************************************************/
VOID TZTimer (HWND hwnd)
    {
    WPDCF77     *somSelf = (WPDCF77 *)WinQueryWindowPtr (hwnd, QWL_USER);
    WPDCF77Data *somThis = WPDCF77GetData (somSelf);

    if (strcmp (_pIData->szOldLocName, szLocName))
        {
        if (strcmp (_pIData->szOldLocFile, szLocFile))
            {
            strcpy (_pIData->szLocFile,    szLocFile);
            strcpy (_pIData->szOldLocFile, szLocFile);
            InitTZFile (hwnd, somThis);
            }
        strcpy (_pIData->szLocName,    szLocName);
        strcpy (_pIData->szOldLocName, szLocName);
        _pIData->bNoControl = TRUE;
        InitTZName (hwnd, somThis, TRUE);
        _pIData->bNoControl = FALSE;
        }

    return;
    }

/*******************************************************************\
    TZClose: Callback-Routine fr WM_CLOSE-Message
    Eingang: HWND   hwnd:   Window-Handle der Dialogbox
\*******************************************************************/
VOID TZClose (HWND hwnd)
    {
    CHAR       szBuffer[CCHMAXPARSELINE];
    DATETIME   dt;
    DCF77_WZSZ DataWzSz;
    ULONG      ulcData, ulcParm;
    BYTE       bParmPacket;

    WPDCF77     *somSelf = (WPDCF77 *)WinQueryWindowPtr (hwnd, QWL_USER);
    WPDCF77Data *somThis = WPDCF77GetData (somSelf);

    /* Bearbeiten der Locale-Datei */
    DosGetDateTime (&dt);

    /* Aktuell eingestellte Zeitzone laden */
    RewindTZFile (_pIData->hLocFile);
    if (_pIData->sTZIndex && MoveToTZNo (_pIData->hLocFile, _pIData->sTZIndex, szBuffer, CCHMAXPARSELINE))
        {
        Read2Dates (_pIData->hLocFile, szBuffer, CCHMAXPARSELINE,
                    &DataWzSz.sDtSwitch1, &DataWzSz.sDtSwitch2, &dt);
        }

    if (hDrvr != -1 &&
        DataWzSz.sDtSwitch1.usFirstDay != 0 && DataWzSz.sDtSwitch1.usFirstYear != 0 &&
        DataWzSz.sDtSwitch2.usFirstDay != 0 && DataWzSz.sDtSwitch2.usFirstYear != 0)
        {
        /* Zugriff auf Treiber, falls vorhanden */
        ulcData = sizeof (DCF77_WZSZ);
        ulcParm = bParmPacket = 0;
        DosDevIOCtl (hDrvr, IOCTL_DCF77, DCF77_SETWZSZ,
            &bParmPacket, sizeof (bParmPacket), &ulcParm,
            &DataWzSz,    sizeof (DCF77_WZSZ),  &ulcData);
        }

    if (_pIData->hLocFile != -1 && IsValidHandle (_pIData->hLocFile))
        {
        CloseTZFile (_pIData->hLocFile);
        _pIData->hLocFile = -1;
        }

    DebugE (D_WND, "TZClose, _pIData->szLocFile", _pIData->szLocFile);
    DebugE (D_WND, "TZClose, _pIData->szLocName", _pIData->szLocName);
    strcpy (szLocFile, _pIData->szLocFile);
    strcpy (szLocName, _pIData->szLocName);

    SaveSetData (somSelf, SAVESET_INITIMEZONE);

    _wpRemove1sTimer (somSelf, hwnd);
    return;
    }

/*******************************************************************\
 *                       Exportierte Funktionen                    *
\*******************************************************************/
/*******************************************************************\
    TimezoneDlgProc: Settings-Page fr die Zeitzone
    Eingang: hwnd: Window-Handle
             msg:  Message-Typ
             mp1:  1. Parameter
             mp2:  2. Parameter
\*******************************************************************/
MRESULT EXPENTRY TimezoneDlgProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
    {
    switch (msg)
        {
        case WM_INITDLG :
            TZInit (hwnd, (WPDCF77 *)mp2);
            return (MRESULT)TRUE;

        case WM_COMMAND:
            if (TZCommand (hwnd, SHORT1FROMMP (mp1), SHORT1FROMMP (mp2)))
                return 0;
            break;

        case WM_CONTROL:
            if (TZControl (hwnd, SHORT1FROMMP (mp1), SHORT2FROMMP (mp1)))
                return 0;
            break;

        case WM_1STIMER:
            TZTimer (hwnd);
            return 0;

        case WM_DESTROY:
            WinSendMsg (hwnd, WM_CLOSE, 0, 0);
            return 0;

        case WM_CLOSE:
            TZClose (hwnd);
            break;
        }

    return WinDefDlgProc (hwnd, msg, mp1, mp2);
    }

VOID InitGPSDriver (VOID)
    {
    ULONG      ulcData, ulcParm;
    BYTE       bParmPacket;
    DATETIME   dt;
    DCF77_WZSZ DataWzSz;
    HFILE      hFile;
    CHAR       szBuffer[CCHMAXPARSELINE];

    GetDCFDateTime ();

    if (DtDCF.timezone == TZ_GPS)
        {
        hFile = -1;
        DebugE (D_WND, "InitGSPDriver", "isGPS");
        if (OpenTZFile (szLocFile, &hFile) == 0)
            {
            /* Bearbeiten der Locale-Datei */
            DosGetDateTime (&dt);

            /* Aktuell eingestellte Zeitzone laden */
            DebugE (D_WND, "InitGPSDriver", "RewindTZFile");
            RewindTZFile (hFile);
            if (FindEntry (hFile, szBuffer, CCHMAXPARSELINE, szLocName))
                {
                DebugE (D_WND, "InitGPSDriver", "Eintrag in TZ-Datei gefunden");
                Read2Dates (hFile, szBuffer, CCHMAXPARSELINE,
                            &DataWzSz.sDtSwitch1, &DataWzSz.sDtSwitch2, &dt);
                }

            if (hDrvr != -1 &&
                DataWzSz.sDtSwitch1.usFirstDay != 0 && DataWzSz.sDtSwitch1.usFirstYear != 0 &&
                DataWzSz.sDtSwitch2.usFirstDay != 0 && DataWzSz.sDtSwitch2.usFirstYear != 0)
                {
                /* Zugriff auf Treiber, falls vorhanden */
                ulcData = sizeof (DCF77_WZSZ);
                ulcParm = bParmPacket = 0;
                DosDevIOCtl (hDrvr, IOCTL_DCF77, DCF77_SETWZSZ,
                             &bParmPacket, sizeof (bParmPacket), &ulcParm,
                             &DataWzSz,    sizeof (DCF77_WZSZ),  &ulcData);
                }

            CloseTZFile (hFile);
            }
        }

    return;
    }

/*******************************************************************\
    DaysInMonth: Berechnet die Zahl der Tage im angegebenen Monat
    Eingang: pDT: Zeiger auf DATETIME-Struktur
    return:  Zahl der Tage im Monat pDT->month
\*******************************************************************/
USHORT DaysInMonth (PDATETIME pDT)
    {
    static SHORT DaysPerMonth[13] = {0,31,28,31,30,31,30,31,31,30,31,30,31};

    if (pDT->month == 2)
        {
        if ((pDT->year % 4 == 0 && pDT->year % 100 != 0) ||
                pDT->year % 400 == 0)
            return 29;
        else
            return 28;
        }
    else
        return DaysPerMonth[pDT->month];
    }

