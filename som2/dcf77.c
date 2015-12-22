/***************************************************************************\
 *
 * PROGRAM NAME: WPDCF77
 * -------------
 *
 * VERSION: 6.0
 * --------
 *
 * MODULE NAME: DCF77
 * ------------
 *
 * DESCRIPTION:
 * ------------
 *   Responsible for communication with the driver and handling of associated settings pages
 *
 * NOTES:
 * ------
 *   The driver has a scan mode to search the signal line connected with the receiver.
 *   In the WPS class this is handled as follows:
 *   - Scan mode will be activated, if
 *     - The port is changed to a game port
 *     - The port is changed to a serial port without SIGNAL_SERIAL transmission,
 *     - The I/O-Adresse is changed
 *     The result will be put into the class data
 *   - The scan mode may also be requested in DCF77DlgProc3
 *   - The structure DCFData is requested from the driver once per second. 
 *     If the settings page is open long enough, the line recognized by the driver will
 *     be saved in the class data.
 *   - If the settings page isn't open long enough, the scan mode will be activated again
 *     during next system boot.
 *
 * FUNCTIONS:
 * -----------
 *   EnableControls ()
 *   fRound ()
 *   FillEntry ()
 *   ReadEntry ()
 *   CalcId ()
 *   ahtoul ()
 *   SetSlider1 ()
 *   SetSlider2Pos ()
 *   QuerySlider1 ()
 *   QuerySlider2 ()
 *   FindMask ()
 *   SetSupply ()
 *   SetSignal ()
 *   CheckScanMode ()
 *   SaveSetData ()
 *   SetVisibility ()
 *   GetStatusMask ()
 *   SetStatusCtls ()
 *   SetOffsCtls ()
 *   DCF77DlgProc1 ()
 *   IODlgProc ()
 *   DCF77DlgProc2 ()
 *   DCF77DlgProc3 ()
 *   StatusDlgProc ()
 *   InfoWndProc ()
 *   TmOffsDlgProc ()
 *   isAnySource ()
 *   isClient ()
 *   isDriver ()
 *   isSrvrEqClnt ()
 *   Get_DCFStatus ()
 *   Query_DCFData ()
 *   Query_DCFOffset ()
 *   Query_DCFDateTime ()
 *   Set_DCFData ()
 *   Set_DCFOffset ()
 *   GetDCFStatus ()
 *   GetDCFData ()
 *   GetDCFOffs ()
 *   GetDCFDateTime ()
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
#define INCL_DOSSEMAPHORES
#define INCL_DOSMISC
#define INCL_WINWINDOWMGR
#define INCL_WINBUTTONS
#define INCL_WINDIALOGS
#define INCL_WINCOUNTRY
#define INCL_WINENTRYFIELDS
#define INCL_WINSTDSLIDER
#define INCL_WINFRAMEMGR
#define INCL_WININPUT
#define INCL_WINLISTBOXES
#define INCL_WINMESSAGEMGR
#define INCL_WINSHELLDATA
#define INCL_WINSYS
#define INCL_WINSTDSPIN
#define INCL_GPIPRIMITIVES
#define INCL_GPITRANSFORMS
#define INCL_GPIPATHS
#include <os2.h>

#include <unidef.h>
#include <uconv.h>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <cutil.h>

#include "dcfioctl.h"

#pragma info(noeff)         // suppress SOM compiler informationals

#include "wpdcf77.ih"
#include "wpdcf.h"

#include "res.h"
#include "clock.h"
#include "clockpan.h"
#include "clkdata.h"
#include "dcf77.h"
#include "debug.h"

/*--------------------------------------------------------------*\
 *  Globale Definitionen und Variable fÅr dieses Modul          *
\*--------------------------------------------------------------*/
#define CONVERT_BEGIN   0               // ahtoul: fÅhrende Leerzeichen unterdrÅcken
#define CONVERT_ACTION  1               // ahtoul: Konvertierung lÑuft
#define CONVERT_END     2               // ahtoul: es dÅrfen nur noch Leerzeichen folgen

#define SZNULL          "0"             // String: 0

#define CCHMAXTEXT      64              // maximale TextlÑnge fÅr div. Zwecke
#define CCHMAXUNICODE   256             // Grî·e von DCF77_STRING

#define MASK_ANT        0x0001          // DID_INFO_ANT aktualisieren
#define MASK_LPH        0x0002          // DID_INFO_LPH aktualisieren
#define MASK_LPS        0x0004          // DID_INFO_LPS aktualisieren
#define MASK_TZ1        0x0008          // DID_INFO_TZ1 aktualisieren
#define MASK_TZ2        0x0010          // DID_INFO_TZ2 aktualisieren
#define MASK_CARRIER    0x0020          // DID_INFO_CARRIER aktualisieren
#define MASK_VALID      0x0040          // DID_INFO_VALID aktualisieren
#define MASK_SCAN       0x0080          // DID_INFO_SCAN aktualisieren
#define MASK_RECOG      0x0100          // DID_INFO_RECOG aktualisieren
#define MASK_WRONG      0x0200          // DID_INFO_WRONG aktualisieren
#define MASK_SET        0x0400          // DID_INFO_SET aktualisieren
#define MASK_TIME       0x0800          // DID_INFO_TIME aktualisieren
#define MASK_DATE       0x1000          // DID_INFO_DATE aktualisieren
#define MASK_VERSION    0x2000          // DID_INFO_VERSION aktualisieren
#define MASK_POLARITY   0x4000          // DID_INFO_POLARITY aktualisieren
#define MASK_GPSCOORD   0x8000          // DID_INFO_POSITION aktualisieren
#define MASK_ALL        0xFFFF          // alle Elemente aktualisieren

#define TICKNUMBER1     4               // Zahl der EintrÑge in pszScale[] fÅr DID_UPDINTERVAL
#define TICKNUMBER2     5               // 3...7 Intervalle
#define TICKNUMBER3     6               // Zahl der Ticks fÅr DID_GOFFSET
#define TICKNUMBER4     25              // Zahl der Ticks fÅr DID_LOFFSET
#define ENTRYNUMBER     9               // Zahl der EintrÑge in pszCombo[]

#define MININTERVAL     1               // Minimum: PrÅfintervalle
#define MAXINTERVAL     10              // Maximum: PrÅfintervalle

/* Definitionen fÅr den Schwellwert-Slider */
#define SL_THRESH_MIN   50              // niedrigster Sliderwert in ms
#define SL_THRESH_MAX   250             // hîchster Sliderwert in ms

static PCHAR  pszScale[] = {"1min", "1h", "12h", "24h  "};  // Leerzeichen am Ende wg. Positionierung!
static USHORT usTick[TICKNUMBER1] = {0, 0, 0, 0};           // Tickposition Slider 1

const PCHAR  pszCombo[] = {"Bit 0", "Bit 1", "Bit 2", "Bit 3",
                           "Bit 4", "Bit 5", "Bit 6", "Bit 7", "Auto"};

typedef struct _CONTROLMASK             // Struktur zur Enable/Disable-Steuerung
    {                                   // von Controls in den Dialogen
    ULONG   id;                         // Dialog-ID
    USHORT  flMask;                     // Maskenbit
    BOOL    bDisplay;                   // Zusatzinformation; bDisplay mu· TRUE sein.
    } CONTROLMASK;
typedef CONTROLMASK *PCONTROLMASK;

CONTROLMASK OffsMask[] = {{DID_ISUTC,           MASK_TO_SYSWIDE,  TRUE},
                          {DID_GOFFSET,         MASK_TO_SYSWIDE,  TRUE},
                          {0,                   0,                FALSE}};

CONTROLMASK IntfMask[] = {{DID_PORT_SER1,       MASK_SS_SER1,     TRUE},
                          {DID_PORT_SER2,       MASK_SS_SER2,     TRUE},
                          {DID_PORT_SER3,       MASK_SS_SER3,     TRUE},
                          {DID_PORT_SER4,       MASK_SS_SER4,     TRUE},
                          {DID_PORT_SERX,       MASK_SS_SERX,     TRUE},
                          {DID_PORT_GAME,       MASK_SS_GAME,     TRUE},
                          {DID_PORT_GAMEX,      MASK_SS_GAMEX,    TRUE},
                          {DID_PORT_HOPF6036,   MASK_SS_HOPF,     TRUE},
                          {DID_PORT_USB,        MASK_SS_USB,      TRUE},
                          {0,                   0,                FALSE}};

CONTROLMASK IntvMask[] = {{DID_UPDINTERVAL,     MASK_IV_UPDATE,   TRUE},
                          {DID_TXT_UPDINTERVAL, MASK_IV_UPDATE,   TRUE},
                          {DID_THRESHOLD,       MASK_IV_THRESH,   TRUE},
                          {DID_TXT_THRESHOLD,   MASK_IV_THRESH,   TRUE},
                          {DID_CHECKINTERVAL,   MASK_IV_CHECK,    TRUE},
                          {DID_HR_TIMER,        MASK_IV_HRTIMER,  TRUE},
                          {0,                   0,                FALSE}};

CONTROLMASK CtrlMask[] = {{DID_POWER_ON,        MASK_LN_POWER,    TRUE},
                          {DID_POWER_1,         MASK_LN_POWER,    TRUE},
                          {DID_POWER_2,         MASK_LN_POWER,    TRUE},
                          {DID_SIGNAL_CTL,      MASK_LN_CONTROL,  TRUE},
                          {DID_SIGNAL_POS,      MASK_LN_CONTROL,  TRUE},
                          {DID_SIGNAL_SER,      MASK_LN_SIGNAL,   TRUE},
                          {0,                   0,                FALSE}};

/*******************************************************************\
    EnableControls: Setzt den Enable/Disable-Status von Controls in
    den Dialogmasken. Der Sollwert stammt Åber StatDCF direkt aus
    dem Treiber.
    Eingang: hwnd:        Window-handle der Maske
             flMask:      Maskenwert von Treiberschnittstelle
             pMask:       Flagbit/ID-Paare
\*******************************************************************/
VOID EnableControls (HWND hwnd, ULONG flMask, PCONTROLMASK pMask)
    {
    BOOL bDisplay;

    while (pMask->id)
        {
        bDisplay = (flMask & pMask->flMask) && pMask->bDisplay;
        if (WinIsControlEnabled (hwnd, pMask->id) != bDisplay)
            WinEnableControl (hwnd, pMask->id, bDisplay);
        pMask++;
        }
    }

/*******************************************************************\
    FindMask: Suche eines CONTROLMASK-Eintrages in einer Tabelle
    in AbhÑngigkeit der Dialog-ID
    Eingang: pMask: Startadresse der Tabelle
    return:  NULL:  nicht gefunden
             sonst: Adresse des CONTROLMASK-Eintrages
\*******************************************************************/
PCONTROLMASK FindMask (PCONTROLMASK pMask, USHORT id)
    {
    while (pMask->id)
        {
        if (pMask->id == id)
            return pMask;
        pMask++;
        }

    return NULL;
    }

/*******************************************************************\
    fRound: Runden einer Flie·kommazahl, Ergebnis ist Integer
    Eingang: x: double-Wert
    return:  Integer-Ergebnis
\*******************************************************************/
LONG fRound (double x)
    {
    return (LONG) (floor (x) + (x - floor (x) >= 0.5 ? 1. : 0.));
    }

/*******************************************************************\
    FillEntry: Unterprogramm fÅr DCF77DlgProc1 zum Setzen des
               Textes im Static-Text Feld.
    Eingang: hwnd:     Window-Handle der Dialogbox
             usIOAddr: Wert fÅr Entry-Feld
\*******************************************************************/
VOID FillEntry (HWND hwnd, USHORT usIOAddr)
    {
    CHAR szText[5];

    WinUpper (hab, 0, 0, _itoa (usIOAddr, szText, 16));
    WinSetDlgItemText (hwnd, DID_PORTENTRY, szText);
    return;
    }

/*******************************************************************\
    ReadEntry: Liest die Zahl aus dem Entryfeld und wandelt
               sie in einen USHORT um.
               Bei korrektem Ergebnis wird DataDCF.ulIOAddr verÑndert,
               sonst erscheint eine Messagebox.
    Eingang: hwnd:  Window-Handle der Dialogbox
             ulDID: Dialog-ID
\*******************************************************************/
VOID ReadEntry (HWND hwnd, ULONG ulDID)
    {
    USHORT usPort;
    CHAR   szText[5];

    WinQueryDlgItemText (hwnd, ulDID, sizeof (szText), szText);
    if (ahtoul (&usPort, szText))
        if (WinQueryWindowUShort (hwnd, QWS_ID) != IDD_COMPORT || usPort <= 9)
            {
            DataDCF.usIOAddr = usPort;
            return;
            }

    MessageBox (hwnd, IDMSG_NODIGIT, 0, MB_ENTER | MB_ERROR | MB_MOVEABLE, WA_ERROR, NULL);
    return;
    }

/*******************************************************************\
    CalcId: Rechnet den PORT_* - Wert in eine DID_PORT_* - Id um.
    Eingang: bPort: PORT_* - Wert
    return:  DID_PORT_* - Wert
\*******************************************************************/
USHORT CalcId (BYTE bPort)
    {
    switch (bPort)
        {
        casePORT_HOPF6036:
            return DID_PORT_HOPF6036;

        case PORT_SERX:
            return DID_PORT_SERX;

        case PORT_GAMEX:
            return DID_PORT_GAMEX;

        case PORT_USB:
            return DID_PORT_USB;
        }

    if (bPort != 0 && !(bPort & PORT_PORTX))
        return bPort + DID_PORT_SER1 - 1;

    return DID_PORT_SER1;
    }

/*******************************************************************\
    ahtoul: Konvertiert einen String in HEX-Darstellung (4 Ziffern)
            in einen USHORT-Wert.
    Eingang: pusPort:   Zeiger auf Zielvariable
             pszString: Quellstring
\*******************************************************************/
BOOL ahtoul (PUSHORT pusPort, PSZ pszString)
    {
    ULONG ulState = CONVERT_BEGIN;
    LONG i;

    WinUpper (hab, 0, 0, pszString);

    for (*pusPort = i = 0; i < 4; i++, pszString++)
        {
        if (*pszString == '\0')
            break;
        if (isspace (*pszString))
            {
            if (ulState == CONVERT_ACTION)
                ulState = CONVERT_END;
            }
        else
            {
            if (ulState == CONVERT_END)
                return FALSE;
            else
                {
                ulState = CONVERT_ACTION;
                *pszString -= '0';
                if (*pszString > 9)
                    *pszString -= 'A' - '0' - 10;
                if ((ULONG)(BYTE)*pszString > 15)
                    return FALSE;
                *pusPort = 16 * *pusPort + *pszString;
                }
            }
        }

    return TRUE;
    }

/*******************************************************************\
    SetSlider1: Setzt den Slider auf die Position der
                   Instanzdaten
    Eingang: hwnd:    Parent-Window
\*******************************************************************/
VOID SetSlider1 (HWND hwnd)
    {
    USHORT usPosition;
    CHAR   szText[5];

    if (DataDCF.usSetInterval < 60)
        usPosition = fRound (usTick[1]*(DataDCF.usSetInterval-1)/59.);
    else if (DataDCF.usSetInterval >= 60 && DataDCF.usSetInterval < 720)
        usPosition = fRound ((usTick[2] - usTick[1])*(DataDCF.usSetInterval-60)/660.) + usTick[1];
    else
        usPosition = fRound ((usTick[3] - usTick[2])*(DataDCF.usSetInterval-720)/720.) + usTick[2];

    WinSendDlgItemMsg (hwnd, DID_UPDINTERVAL, SLM_SETSLIDERINFO,
        MPFROM2SHORT (SMA_SLIDERARMPOSITION, SMA_RANGEVALUE),
        MPFROMSHORT (usPosition));

    WinSetDlgItemText (hwnd, DID_TXT_UPDINTERVAL, _itoa (DataDCF.usSetInterval, szText, 10));

    return;
    }

/*******************************************************************\
    SetSlider2: Setzt den Tick-Text des Schwellwert-Sliders
    Eingang: hwnd:        Parent-Window
             usTimerMode: Timer-Modus (IMER_MODE_*)
             usTickLen:   Tick-LÑnge in ms
\*******************************************************************/
VOID SetSlider2 (HWND hwnd)
    {
    LONG         l, lVal;
    LONG         lSnaps, lInc, liText;
    LONG         lThresMin, lThresMax;
    SHORT        sLenSnap;
    USHORT       usPosition, usThreshold;
    CHAR         szText[5];
    PCHAR        pszText;
    SLDCDATA     sldCData;
    ULONG        flStyle;
    SWP          swpSlider;
    PCONTROLMASK pCtrlMask;

    WinQueryWindowPos (WinWindowFromID (hwnd, DID_THRESHOLD), &swpSlider);

    /* aktuell existierenden Slider lîschen */
    WinDestroyWindow (WinWindowFromID (hwnd, DID_THRESHOLD));

    /* Style festlegen; fÅr HR-Timer Read-Only Modus setzen */
    flStyle  = WS_VISIBLE | WS_TABSTOP | SLS_BOTTOM | SLS_BUTTONSLEFT | SLS_HORIZONTAL;
    flStyle |= (DataDCF.usTimerMode == TIMER_MODE_HR ? SLS_READONLY : 0);

    /* Voreinstellungen bei deaktiviertem Slider */
    lSnaps = 2;

    pCtrlMask = FindMask (IntvMask, DID_THRESHOLD);
    if ((StatDCF.flMaskIV & pCtrlMask->flMask) && pCtrlMask->bDisplay)
        {
        /* Abstand zwischen 2 Snap-Positionen in ms */
        lInc = DataDCF.usTicklen / (DataDCF.usTimerMode == TIMER_MODE_SYS ? 4 : 1);
        if (lInc == 0)
            lInc++;

        /* Minimum/Maximum des Sliderbereiches festlegen */
        lThresMin = (SL_THRESH_MIN + lInc - 1) / lInc * lInc;
        lThresMax = (SL_THRESH_MAX + lInc - 1) / lInc * lInc;

        /* Zahl der Snap-Positionen */
        lSnaps = 1 + (lThresMax - lThresMin) / lInc;

        /* Abstand der beschrifteten Ticks */
        liText = DataDCF.usTicklen * (DataDCF.usTimerMode == TIMER_MODE_SYS ? 1 : 5);
        }

    /* Neuen Slider erzeugen */
    memset (&sldCData, 0, sizeof (SLDCDATA));
    sldCData.cbSize             = sizeof (SLDCDATA);
    sldCData.usScale1Increments = lSnaps;
    WinCreateWindow (hwnd,                          /*  Parent-window handle. */
                     WC_SLIDER,                     /*  Registered-class name. */
                     "",                            /*  Window text. */
                     flStyle,                       /*  Window style. */
                     swpSlider.x,                   /*  x-coordinate of window position. */
                     swpSlider.y,                   /*  y-coordinate of window position. */
                     swpSlider.cx,                  /*  Width of window, in window coordinates. */
                     swpSlider.cy,                  /*  Height of window, in window coordinates. */
                     hwnd,                          /*  Owner-window handle. */
                     HWND_TOP,                      /*  Sibling-window handle. */
                     DID_THRESHOLD,                 /*  Window identifier. */
                     &sldCData,                     /*  Pointer to control data. */
                     NULL);                         /*  Presentation parameters. */

    if ((StatDCF.flMaskIV & pCtrlMask->flMask) && pCtrlMask->bDisplay)
        {
        /* Tick-LÑngen setzen; Setzen der Tick-Texte */
        if (liText)
            {
            for (l = 0, lVal = lThresMin; l < lSnaps; l++, lVal += lInc)
                {
                if (lVal % liText == 0)
                    {
                    pszText = szText;
                    if (l == 0)
                        *pszText++ = ' ';                   // wegen Darstellungsproblem 1. String einrÅcken
                    _itoa (lVal, pszText, 10);
                    if (l >= lSnaps - 1)
                        strcat (pszText, "  ");             // wegen Darstellungsproblem letzten String ausrÅcken
                    WinSendDlgItemMsg (hwnd, DID_THRESHOLD, SLM_SETSCALETEXT,
                        MPFROMSHORT (l), MPFROMP (szText));
                    sLenSnap = 5;
                    }
                else
                    sLenSnap = 3;
                WinSendDlgItemMsg (hwnd, DID_THRESHOLD, SLM_SETTICKSIZE,
                    MPFROM2SHORT (l, sLenSnap), MPVOID);
                }
            }

        /* Sliderposition setzen, der korrigierte Wert geht an den Treiber, */
        /* da SLM_SETSLIDERINFO eine WM_CONTROL-Message erzeugt             */
        usThreshold = min (DataDCF.usThreshold, lThresMax);
        usThreshold = max (usThreshold, lThresMin);

        usPosition = (usThreshold - lThresMin) / lInc;
        WinSendDlgItemMsg (hwnd, DID_THRESHOLD, SLM_SETSLIDERINFO,
            MPFROM2SHORT (SMA_SLIDERARMPOSITION, SMA_INCREMENTVALUE),
            MPFROMSHORT (usPosition));

        /* Text des Textfeldes setzen */
        WinSetDlgItemText (hwnd, DID_TXT_THRESHOLD, _itoa (usThreshold, szText, 10));
        }

    /* Checkbox-Zustand setzen */
    WinCheckButton (hwnd, DID_HR_TIMER, DataDCF.usTimerMode == TIMER_MODE_HR);

    return;
    }

/*******************************************************************\
    QuerySlider1: Berechnet aus der Sliderposition von Slider 1
                  die LÑnge des Intervalles in Minuten.
                  Das Ergebnis wird in DataDCF.usSetInterval
                  abgelegt.
    Eingang: hwnd:    Parent-Window
\*******************************************************************/
VOID QuerySlider1 (HWND hwnd)
    {
    USHORT usPosition;
    CHAR   szText[5];

    usPosition = (USHORT) WinSendDlgItemMsg (hwnd, DID_UPDINTERVAL,
        SLM_QUERYSLIDERINFO,
        MPFROM2SHORT (SMA_SLIDERARMPOSITION, SMA_RANGEVALUE),
        MPVOID);

    if (usPosition < usTick[1])
        DataDCF.usSetInterval = fRound (59*usPosition/usTick[1]) + 1;
    else if (usPosition >= usTick[1] && usPosition < usTick[2])
        DataDCF.usSetInterval = fRound (660*(usPosition-usTick[1])/(usTick[2]-usTick[1])) + 60;
    else
        DataDCF.usSetInterval = fRound (720*(usPosition-usTick[2])/(usTick[3]-usTick[2])) + 720;

    WinSetDlgItemText (hwnd, DID_TXT_UPDINTERVAL, _itoa (DataDCF.usSetInterval, szText, 10));
    return;
    }

/*******************************************************************\
    QuerySlider2: Berechnet aus der Sliderposition von Slider 2
                  die Zahl der Ticks. Das Ergebnis wird in
                  DataDCF.usThreshold abgelegt.
    Eingang: hwnd:    Parent-Window
\*******************************************************************/
VOID QuerySlider2 (HWND hwnd)
    {
    LONG   lInc;
    LONG   lThresMin;
    CHAR   szText[4];
    USHORT usPosition;

// ??? Vorsicht: usticklen kann 0 sein (Defaultwert?)!

    /* nur aufrufen, wenn der Slider aktiviert ist */
    if (FindMask (IntvMask, DID_THRESHOLD)->bDisplay)
        {
        /* Abstand zwischen 2 Snap-Positionen in ms */
        lInc = DataDCF.usTicklen / (DataDCF.usTimerMode == TIMER_MODE_SYS ? 4 : 1);

        /* Minimum/Maximum des Sliderbereiches festlegen */
        lThresMin = (SL_THRESH_MIN + lInc - 1) / lInc * lInc;

        usPosition = (USHORT) WinSendDlgItemMsg (hwnd, DID_THRESHOLD,
            SLM_QUERYSLIDERINFO,
            MPFROM2SHORT (SMA_SLIDERARMPOSITION, SMA_INCREMENTVALUE),
            MPVOID);

        DataDCF.usThreshold = usPosition * lInc + lThresMin;

        WinSetDlgItemText (hwnd, DID_TXT_THRESHOLD, _itoa (DataDCF.usThreshold, szText, 10));
        }

    return;
    }

/*******************************************************************\
    SetSupply: Setzen der Supply-Buttons
    Eingang: hwnd:    Parent-Window
\*******************************************************************/
VOID SetSupply (HWND hwnd)
    {
    BOOL bSupportsSupply;

    /* UnterstÅtzt die Hardware eine Energieversorgung? */
    switch (DataDCF.bPort)
        {
        case PORT_SER1:
        case PORT_SER2:
        case PORT_SER3:
        case PORT_SER4:
        case PORT_SERX:
        case PORT_SHMEM:
            bSupportsSupply = TRUE;
            break;
        default:
            bSupportsSupply = FALSE;
        }

    if (!bSupportsSupply)
        WinSendDlgItemMsg (hwnd, DID_CANCEL, BM_SETDEFAULT, MPFROMSHORT (TRUE), MPVOID);

    FindMask (CtrlMask, DID_POWER_ON)->bDisplay = bSupportsSupply;
    FindMask (CtrlMask, DID_POWER_1)->bDisplay  = bSupportsSupply && !(DataDCF.bSupply & SUPPLY_OFF);
    FindMask (CtrlMask, DID_POWER_2)->bDisplay  = bSupportsSupply && !(DataDCF.bSupply & SUPPLY_OFF);
    WinCheckButton (hwnd, DID_POWER_ON, bSupportsSupply && !(DataDCF.bSupply & SUPPLY_OFF));

    if (bSupportsSupply && !(DataDCF.bSupply & SUPPLY_OFF))
        {
        WinSetDlgItemText (hwnd, DID_POWER_1, DataDCF.bSupply & SUPPLY_L1_POS ? "+" : "-");
        WinSetDlgItemText (hwnd, DID_POWER_2, DataDCF.bSupply & SUPPLY_L2_POS ? "+" : "-");
        }
    else
        {
        WinSetDlgItemText (hwnd, DID_POWER_1, " ");
        WinSetDlgItemText (hwnd, DID_POWER_2, " ");
        }

    return;
    }

/*******************************************************************\
    SetSignal: Setzen der Signal-Gruppe
    Eingang: hwnd:    Parent-Window
\*******************************************************************/
VOID SetSignal (HWND hwnd)
    {
    BOOL bIsGame;
    BOOL bIsSerial;
    BOOL bIsScan;

    /* Ist Gameport-Uhr selektiert? */
    bIsGame = (DataDCF.bPort & 0x0F) == PORT_GAME;
    if (bIsGame)
        DataDCF.bSignal &= ~SIGNAL_SERIAL;

    /* Ist öbertragung Åber RxD-Leitung selektiert? */
    bIsSerial = (DataDCF.bSignal & SIGNAL_SERIAL) != 0;

    /* Ist der Scan-Modus aktiviert */
    bIsScan = (DataDCF.bSignal & SIGNAL_SCAN) != 0;

    /* Combobox selektieren */
    WinSendDlgItemMsg (hwnd, DID_SIGNAL_POS, LM_SELECTITEM,
        MPFROMSHORT (DataDCF.bSignal & SIGNAL_MASK),
        MPFROMSHORT (TRUE));

    WinSetDlgItemText (hwnd, DID_SIGNAL_POS,
        bIsSerial ? "" : pszCombo[bIsScan ? ENTRYNUMBER-1 : DataDCF.bSignal & SIGNAL_MASK]);

    /* Radio-Button setzen */
    WinCheckButton (hwnd,
        bIsSerial ? DID_SIGNAL_SER : DID_SIGNAL_CTL, TRUE);

    /* ENABLE-Zustand der Controls */
    FindMask (CtrlMask, DID_SIGNAL_POS)->bDisplay = !bIsSerial && StatDCF.flMaskLN & MASK_LN_CONTROL;
    FindMask (CtrlMask, DID_SIGNAL_SER)->bDisplay = !bIsGame   && StatDCF.flMaskLN & MASK_LN_SIGNAL;

    return;
    }

/*******************************************************************\
    CheckScanMode: Bestimmt, ob der Scan-Modus gestartet werden mu·
                   und setzt das SCAN-Bit in bSignal
    Eingang: bOldPort: Inhalt von bPort vor der énderung
             usIOAddr: Inhalt von usIOAddr vor der énderung
\*******************************************************************/
VOID CheckScanMode (BYTE bOldPort, USHORT usOldIOAddr)
    {

    if (bOldPort != DataDCF.bPort || usOldIOAddr != DataDCF.usIOAddr)
        {
        if (DataDCF.bPort != PORT_HOPF6036 &&
            (((DataDCF.bPort & ~PORT_PORTX) == PORT_GAME) ||
            ((DataDCF.bPort & ~PORT_PORTX) != PORT_GAME) && !(DataDCF.bSignal & SIGNAL_SERIAL)))
            {
            DataDCF.bSignal |= SIGNAL_SCAN;
            DebugE (D_IO, "CheckScanMode", "Scan Mode activated");
            }
        }

    return;
    }

/*******************************************************************\
    SaveSetData: Ruft _clsSaveSetData
    Eingang: somSelf: Objekt-Zeiger
\*******************************************************************/
VOID SaveSetData (WPDCF77 *somSelf, ULONG flType)
    {
    SOMClass *Metaclass;

    Metaclass = SOM_GetClass (somSelf);
    M_WPDCF77_clsSaveSetData (Metaclass, flType);

    return;
    }

/*******************************************************************\
    SetVisibility: Setzen des ENABLE bzw. SHOW-Status von Controls
                   in der Statusmaske. Der Sollwert stammt Åber
                   StatDCF direkt aus dem Treiber.
    Eingang: hwnd:        Window-handle der Maske
\*******************************************************************/
VOID SetVisibility (HWND hwnd)
    {
    ULONG id, fl;
    BOOL  b;

    /* PolaritÑt */
    b = StatDCF.flMaskST & MASK_ST_POLARITY ? TRUE : FALSE;
    WinShowWindow (WinWindowFromID (hwnd, DID_TXT_POLARITY),  b);
    WinShowWindow (WinWindowFromID (hwnd, DID_INFO_POLARITY), b);

    /* Check-Marken */
    for (id=DID_TXT_ANT, fl=MASK_ST_ANT; id<=DID_TXT_TZ2; id++, fl<<=1)
        {
        b = StatDCF.flMaskST & fl ? TRUE : FALSE;
        WinEnableControl (hwnd, id, b);
        WinEnableControl (hwnd, id-DID_TXT_ANT+DID_INFO_ANT, b);
        }

    for (id=DID_TXT_CARRIER, fl=MASK_ST_CARRIER; id<=DID_TXT_SCAN; id++, fl<<=1)
        {
        b = StatDCF.flMaskST & fl ? TRUE : FALSE;
        WinEnableControl (hwnd, id, b);
        WinEnableControl (hwnd, id-DID_TXT_CARRIER+DID_INFO_CARRIER, b);
        }

    /* ZÑhler */
    b = StatDCF.flMaskST & MASK_ST_BAD ? TRUE : FALSE;
    WinShowWindow (WinWindowFromID (hwnd, DID_INFO_WRONG), b);
    WinShowWindow (WinWindowFromID (hwnd, DID_TXT_WRONG1), b);
    WinShowWindow (WinWindowFromID (hwnd, DID_TXT_WRONG2), b);
    b = StatDCF.flMaskST & MASK_ST_GOOD ? TRUE : FALSE;
    WinShowWindow (WinWindowFromID (hwnd, DID_INFO_RECOG), b);
    WinShowWindow (WinWindowFromID (hwnd, DID_TXT_RECOG1), b);
    WinShowWindow (WinWindowFromID (hwnd, DID_TXT_RECOG2), b);
    b = StatDCF.flMaskST & MASK_ST_SET ? TRUE : FALSE;
    WinShowWindow (WinWindowFromID (hwnd, DID_INFO_SET), b);
    WinShowWindow (WinWindowFromID (hwnd, DID_TXT_SET1), b);
    WinShowWindow (WinWindowFromID (hwnd, DID_TXT_SET2), b);

    return;
    }

/*******************************************************************\
    GetStatusMask: Bestimmen, welche Controls des Statusfenster
                   sich verÑndert haben
    Eingang: somThis: Zeiger auf Instanzdaten
\*******************************************************************/
ULONG GetStatusMask (WPDCF77Data *somThis)
    {
    ULONG   ulMask = 0;
    USHORT  us;

    GetDCFStatus ();
    GetDCFDateTime ();
    GetGPSCoord ();

    if (StatDCF.flStatus ^ _pIData->OldStatDCF.flStatus)
        {
        /** Mit der GÅltigkeit der Zeit hat sich etwas geÑndert **/
        ulMask = MASK_ALL;
        }
    else if ((StatDCF.flStatus & (STATUS_ISDRIVER | STATUS_TIMEVALID)) == (STATUS_ISDRIVER | STATUS_TIMEVALID))
        {
        /* Zeit korrekt abgeholt */
        if (_pIData->OldDtDCF.seconds != DtDCF.seconds ||
            _pIData->OldDtDCF.minutes != DtDCF.minutes ||
            _pIData->OldDtDCF.hours   != DtDCF.hours)
            ulMask |= MASK_TIME;
        if (_pIData->OldDtDCF.day     != DtDCF.day   ||
            _pIData->OldDtDCF.month   != DtDCF.month ||
            _pIData->OldDtDCF.year    != DtDCF.year)
            ulMask |= MASK_DATE;

        /* GPS-Position */
        if (_pIData->OldCoordGPS.Breite != CoordGPS.Breite ||
            _pIData->OldCoordGPS.Laenge != CoordGPS.Laenge)
            ulMask |= MASK_GPSCOORD;

        /* Treiberversion */
        if (_pIData->OldStatDCF.cVerMajor != StatDCF.cVerMajor ||
            _pIData->OldStatDCF.cVerMinor != StatDCF.cVerMinor)
            ulMask |= MASK_VERSION;

        /* PolaritÑt */
        if (_pIData->OldStatDCF.bPolarity != StatDCF.bPolarity)
            ulMask |= MASK_POLARITY;

        /* DCF77-generierte Statusmeldungen */
        for (us = 0; us <= (DID_INFO_TZ2 - DID_INFO_ANT); us++)
            if ((_pIData->OldStatDCF.flStatus ^ StatDCF.flStatus) & STATUS_ANT<<us)
                ulMask |= MASK_ANT<<us;

        /* Treibergenerierte Statusmeldungen */
        for (us = 0; us <= (DID_INFO_SCAN - DID_INFO_CARRIER); us++)
            if ((_pIData->OldStatDCF.flStatus ^ StatDCF.flStatus) & STATUS_CARRIER<<us)
                ulMask |= MASK_CARRIER<<us;
        }
    else if (_pIData->OldDtDCF.seconds != DtDCF.seconds)
        ulMask |= MASK_DATE | MASK_TIME;

    /* ZÑhlerstÑnde */
    if (_pIData->OldStatDCF.uscGoodSeconds != StatDCF.uscGoodSeconds)
        ulMask |= MASK_RECOG;
    if (_pIData->OldStatDCF.uscBadSeconds  != StatDCF.uscBadSeconds)
        ulMask |= MASK_WRONG;
    if (_pIData->OldStatDCF.uscTimeSet     != StatDCF.uscTimeSet)
        ulMask |= MASK_SET;

    _pIData->OldStatDCF  = StatDCF;
    _pIData->OldDtDCF    = DtDCF;
    _pIData->OldCoordGPS = CoordGPS;

    return ulMask;
    }

/*******************************************************************\
    SetStatusCtls: Setzt die Controls im Statusfenster
    Eingang: hwnd:    Dialog-Window handle
             somThis: Zeiger auf Instanzdaten
             ulMask:  MASK_*-Werte
\*******************************************************************/
VOID SetStatusCtls (HWND hwnd, ULONG ulMask)
    {
    ULONG    ulc1, ulc2;
    CHAR     szText1[CCHMAXTEXT];
    CHAR     szText2[CCHMAXTEXT];
    PCHAR    pszCoord[4];
    DATETIME dt;

    if (ulMask & (MASK_TIME | MASK_DATE))
        {
        if (!(StatDCF.flStatus & STATUS_ISDRIVER))
            {
            szText1[0] = '\0';
            ulc1       = 0;
            WinLoadString (hab, hmod, isSrvrEqClnt () ? IDS_DRIVERFAIL : IDS_SERVERVERSION, CCHMAXTEXT, szText2);
            }
        else if (!(StatDCF.flStatus & STATUS_CARRIER))
            {
            ulc1 = WinLoadString (hab, hmod, IDS_TIMENOCARRIER, CCHMAXTEXT, szText1);
                   WinLoadString (hab, hmod, IDS_DATENOCARRIER, CCHMAXTEXT, szText2);
            }
        else if (!(StatDCF.flStatus & STATUS_TIMEVALID))
            {
            ulc1 = WinLoadString (hab, hmod, IDS_TIMEFAIL, CCHMAXTEXT, szText1);
                   WinLoadString (hab, hmod, IDS_DATEFAIL, CCHMAXTEXT, szText2);

            szText1[ulc1++] = '0' + DtDCF.seconds / 10;
            szText1[ulc1++] = '0' + DtDCF.seconds % 10;
            }
        else
            {
            ulc1 = WinLoadString (hab,
                                  hmod,
                                  isHopf6038 () ? IDS_TIMEOK_GPS : IDS_TIMEOK_DCF,
                                  CCHMAXTEXT,
                                  szText1);
            ulc2 = WinLoadString (hab,
                                  hmod,
                                  isHopf6038 () ? IDS_DATEOK_GPS : IDS_DATEOK_DCF,
                                  CCHMAXTEXT,
                                  szText2);

            dt.hours   = DtDCF.hours;
            dt.minutes = DtDCF.minutes;
            dt.seconds = DtDCF.seconds;
            dt.day     = DtDCF.day;
            dt.month   = DtDCF.month;
            dt.year    = DtDCF.year;
            GetDateTime (&dt, MODE_YEAR_2D | MODE_PM, szText1 + ulc1, szText2 + ulc2);

            ulc1 = strlen (szText1);
            }

        /* Leerzeichen anhÑngen, um Springen der Anzeige zu vermeiden */
        szText1[ulc1++] = ' ';
        szText1[ulc1]   = '\0';

        /* Text ausgeben */
        if (ulMask & MASK_TIME)
            WinSetDlgItemText (hwnd, DID_INFO_TIME, szText1);
        if (ulMask & MASK_DATE)
            WinSetDlgItemText (hwnd, DID_INFO_DATE, szText2);
        }

    /* Ausgabe: PolaritÑt */
    if (ulMask & MASK_POLARITY)
        {
        WinLoadString (hab, hmod, StatDCF.bPolarity ? IDS_POSITIVE : IDS_NEGATIVE, CCHMAXTEXT, szText1);
        WinSetDlgItemText (hwnd, DID_INFO_POLARITY, szText1);
        }

    /* Ausgabe: Version */
    if (ulMask & MASK_VERSION)
        {
        _itoa (StatDCF.cVerMajor, szText1, 10);
        strcat (szText1, ".");
        _itoa (StatDCF.cVerMinor, szText1 + strlen (szText1), 10);
        WinSetDlgItemText (hwnd, DID_INFO_VERSION, szText1);
        }

    /* DCF77-generierte Statusmeldungen */
    for (ulc1 = 0; ulc1 <= (DID_INFO_TZ2 - DID_INFO_ANT); ulc1++)
        if (ulMask & MASK_ANT<<ulc1)
            WinSendDlgItemMsg (hwnd, DID_INFO_ANT + ulc1, WM_SETMARKER,
                MPFROMLONG (StatDCF.flStatus & (STATUS_ANT<<ulc1) ? MARKSYM_SOLIDDIAMOND : MARKSYM_DIAMOND),
                MPVOID);

    /* Treibergenerierte Statusmeldungen */
    for (ulc1 = 0; ulc1 <= (DID_INFO_SCAN - DID_INFO_CARRIER); ulc1++)
        if (ulMask & MASK_CARRIER<<ulc1)
            WinSendDlgItemMsg (hwnd, DID_INFO_CARRIER + ulc1, WM_SETMARKER,
                MPFROMLONG (StatDCF.flStatus & (STATUS_CARRIER<<ulc1) ? MARKSYM_SOLIDDIAMOND : MARKSYM_DIAMOND),
                MPVOID);

    if (ulMask & MASK_RECOG)
        WinSetDlgItemText (hwnd, DID_INFO_RECOG, _itoa (StatDCF.uscGoodSeconds, szText1, 10));
    if (ulMask & MASK_WRONG)
        WinSetDlgItemText (hwnd, DID_INFO_WRONG, _itoa (StatDCF.uscBadSeconds,  szText1, 10));
    if (ulMask & MASK_SET)
        WinSetDlgItemText (hwnd, DID_INFO_SET,   _itoa (StatDCF.uscTimeSet,     szText1, 10));

    /* Ausgabe: GPS-Koordinaten */
    if (isHopf6038 () && ulMask & IDS_GPSPOSITION)
        {
        if (CoordGPS.Breite != -1)
            {
            DosAllocMem ((PPVOID)pszCoord, 4*5, PAG_COMMIT | PAG_READ | PAG_WRITE);
            pszCoord[1] = pszCoord[0] + 1*5;
            pszCoord[2] = pszCoord[0] + 2*5;
            pszCoord[3] = pszCoord[0] + 3*5;
            _itoa (CoordGPS.Breite/3600000, pszCoord[0], 10);
            _itoa ((CoordGPS.Breite%3600000) / 60000, pszCoord[1], 10);
            _itoa (CoordGPS.Laenge/3600000, pszCoord[2], 10);
            _itoa ((CoordGPS.Laenge%3600000) / 60000, pszCoord[3], 10);
            ulc1 = WinLoadString (hab, hmod, IDS_GPSPOSITION, CCHMAXTEXT, szText1);
            DosInsertMessage (pszCoord, 4, szText1, ulc1, szText2, CCHMAXTEXT, &ulc2);
            DosFreeMem (pszCoord[0]);
            }
        else
            WinLoadString (hab, hmod, IDS_GPSINVPOSITION, CCHMAXTEXT, szText2);
        WinSetDlgItemText (hwnd, DID_INFO_POSITION, szText2);
        }

    return;
    }

/*******************************************************************\
    SetOffsCtls: Setzt die Controls der globalen
                 Zeitoffseteinstellungen
    Eingang: hwnd:    Dialog-Window handle
\*******************************************************************/
VOID SetOffsCtls (HWND hwnd)
    {
    BOOL   bBtnState;
    USHORT usSliderPos;

    bBtnState   = OffsDCF.flUTC ? TRUE : FALSE;
    usSliderPos = OffsDCF.sOffset+2;

    if (WinQueryButtonCheckstate (hwnd, DID_ISUTC) != bBtnState)
        WinCheckButton (hwnd, DID_ISUTC, bBtnState);

    if ((USHORT)WinSendDlgItemMsg (hwnd, DID_GOFFSET, SLM_QUERYSLIDERINFO,
        MPFROM2SHORT (SMA_SLIDERARMPOSITION, SMA_INCREMENTVALUE), MPVOID) != usSliderPos)
        {
        WinSendDlgItemMsg (hwnd, DID_GOFFSET, SLM_SETSLIDERINFO,
            MPFROM2SHORT (SMA_SLIDERARMPOSITION, SMA_INCREMENTVALUE),
            MPFROMSHORT (usSliderPos));
        }

    return;
    }

/*******************************************************************\
    getUSBDeviceDesc: kopiert den USB device descriptor in den
                      Puffer.
    Eingang: pszText:   Puffer
             ulcLength: Puffer-LÑnge
\*******************************************************************/
VOID getUSBDeviceDesc (PSZ pszText, ULONG ulcLength)
    {
    BYTE        bBuffer[CCHMAXUNICODE];
    CHAR        szText[CCHMAXTEXT];
    USHORT      usUSBDesc[] = {DESC_STR_MANU, DESC_STR_PRODUCT, DESC_STR_SERIAL, 0};
    ULONG       ulcParm;
    ULONG       ulcData;
    ULONG       ul;
    BOOL        bRC;
    int         rc;
    UconvObject uconvObj;
    UniChar     uncEmpty = 0;

    pszText[0] = '\0';

    rc = UniCreateUconvObject (&uncEmpty, &uconvObj);
    if (rc == ULS_SUCCESS)
        {
        for (ul = 0; usUSBDesc[ul] != 0; ul++)
            {
            ulcParm = sizeof (USHORT);
            ulcData = CCHMAXUNICODE;
            ((DCF77_STRING *)bBuffer)->usSize    = CCHMAXUNICODE - sizeof (DCF77_STRING) + 1;
            ((DCF77_STRING *)bBuffer)->uscStrlen = 0;
            bRC = DosDevIOCtl (hDrvr, IOCTL_DCF77, DCF77_GETDEVICEDESCR,
                               &usUSBDesc[ul], ulcParm, &ulcParm,
                               bBuffer,        ulcData, &ulcData) ? FALSE : TRUE;
            if (bRC)
                {
                rc = UniStrFromUcs (uconvObj,
                                    szText,
                                    (UniChar *)(((DCF77_STRING *)bBuffer)->cDesc),
                                    CCHMAXTEXT);

                DebugE (D_DLG, "getUSBDeviceDesc => received string", szText);

                if (rc == ULS_SUCCESS)
                    {
                    strncat (pszText, szText, ulcLength - strlen (pszText) - 1);

                    switch (usUSBDesc[ul])
                        {
                        case DESC_STR_MANU:
                            strncat (pszText, "\n",   ulcLength - strlen (pszText) - 1);
                            break;

                        case DESC_STR_PRODUCT:
                            strncat (pszText, "\nSerial: ",   ulcLength - strlen (pszText) - 1);
                            break;
                        }
                    }
                }
            }

        UniFreeUconvObject (uconvObj);
        }

    return;
    }

/*******************************************************************\
    DCF77DlgProc1: Window-Prozedur fÅr die erste DCF77-Settings-
                   Seite
    Eingang: hwnd: Window-Handle
             msg:  Message-Typ
             mp1:  1. Parameter
             mp2:  2. Parameter
\*******************************************************************/
MRESULT EXPENTRY DCF77DlgProc1 (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
    {
    WPDCF77     *somSelf;
    WPDCF77Data *somThis;
    CHAR         szText[CCHMAXTEXT];

    switch (msg)
        {
        case WM_INITDLG:
            somSelf = (WPDCF77 *)mp2;
            somThis = WPDCF77GetData (somSelf);
            WinSetWindowPtr (hwnd, QWL_USER, (PVOID) somSelf);

            WinSendDlgItemMsg (hwnd, DID_PORTENTRY, EM_SETTEXTLIMIT, MPFROMSHORT (4), MPVOID);

            _pIData->bPort    = DataDCF.bPort;
            _pIData->usIOAddr = DataDCF.usIOAddr;

            WinCheckButton (hwnd, CalcId (_pIData->bPort), TRUE);
            FillEntry (hwnd, DataDCF.usIOAddr);

            DebugE (D_DLG, "DCF77DlgProc1", "check driver type");
            if (hDrvr == -1)
                {
                /* Treiber fÅr externes hopf-Modul: Dialog umbauen */
                WinShowWindow (WinWindowFromID (hwnd, DID_PORTTXT), FALSE);
                WinLoadString (hab, hmod, IDS_PORT_INDEX, CCHMAXTEXT, szText);
                WinSetDlgItemText (hwnd, DID_PORTNUM, szText);
                }
            else if (isUSBDevice ())
                {
                /* Treiber fÅr USB device: Dialog umbauen */
                DebugE (D_DLG, "DCF77DlgProc1", "USB driver found");
                WinLoadString (hab, hmod, IDS_PORT_USB, CCHMAXTEXT, szText);
                WinSetDlgItemText (hwnd, DID_PORTNUM, szText);
                getUSBDeviceDesc (szText, CCHMAXTEXT);
                WinSetDlgItemText (hwnd, DID_PORT_DEVDESC, szText);
                }

            /* Deaktivieren nicht benîtigter Felder */
            EnableControls (hwnd, StatDCF.flMaskSS, IntfMask);

            _pIData->OldDataDCF1 = DataDCF;

            /* Timer zur Aktualisierung der Anzeige starten */
            _wpAdd1sTimer (somSelf, hwnd);
            return (MRESULT)TRUE;

        case WM_CONTROL:
            switch (LONGFROMMP (mp1))
                {
                case DID_PORT_SER1 + (BN_CLICKED<<16):
                case DID_PORT_SER2 + (BN_CLICKED<<16):
                case DID_PORT_SER3 + (BN_CLICKED<<16):
                case DID_PORT_SER4 + (BN_CLICKED<<16):
                case DID_PORT_GAME + (BN_CLICKED<<16):
                    ulGetDCFData = GET_PAUSE;
                    somSelf = (WPDCF77 *)WinQueryWindowPtr (hwnd, QWL_USER);
                    DataDCF.bPort = SHORT1FROMMP (mp1) - DID_PORT_SER1 + 1;
                    break;

                case DID_PORT_SERX  + (BN_CLICKED<<16):
                case DID_PORT_GAMEX + (BN_CLICKED<<16):
                case DID_PORT_HOPF6036 + (BN_CLICKED<<16):
                    ulGetDCFData = GET_PAUSE;
                    somSelf = (WPDCF77 *)WinQueryWindowPtr (hwnd, QWL_USER);

                    /* I/O-Adresse Åber Dialogbox abfragen und in Dialogbox eintragen */
                    WinDlgBox (HWND_DESKTOP, hwnd, (PFNWP)IODlgProc, hmod,
                        hDrvr == -1 ? IDD_COMPORT : IDD_IOADDR, (PVOID)somSelf);
                    switch (SHORT1FROMMP (mp1))
                        {
                        case DID_PORT_SERX:
                            DataDCF.bPort = PORT_SERX;
                            break;

                        case DID_PORT_GAMEX:
                            DataDCF.bPort = PORT_GAMEX;
                            break;

                        case DID_PORT_HOPF6036:
                            DataDCF.bPort = PORT_HOPF6036;
                            break;
                        }
                    break;

                default:
                    return 0;
                }

            somThis = WPDCF77GetData (somSelf);

            /* Umschaltung auf Gameport oder Seriell-Port mit Ctl-öbertragung: Auto-Scan aktivieren */
            CheckScanMode (_pIData->OldDataDCF1.bPort, _pIData->OldDataDCF1.usIOAddr);

            if (_pIData->OldDataDCF1.bPort != DataDCF.bPort || _pIData->OldDataDCF1.usIOAddr != DataDCF.usIOAddr)
                {
                FillEntry (hwnd, DataDCF.usIOAddr);
                SaveSetData (somSelf, SAVESET_DRVRDATA);
                _pIData->OldDataDCF1 = DataDCF;
                }

            ulGetDCFData = GET_OK;
            return 0;

        case WM_COMMAND:
            if (SHORT1FROMMP (mp2) == CMDSRC_PUSHBUTTON)
                {
                somSelf = (WPDCF77 *)WinQueryWindowPtr (hwnd, QWL_USER);
                somThis = WPDCF77GetData (somSelf);
                switch (SHORT1FROMMP (mp1))
                    {
                    case DID_CANCEL:
                        ulGetDCFData     = GET_PAUSE;
                        DataDCF.bPort    = _pIData->bPort;
                        DataDCF.usIOAddr = _pIData->usIOAddr;
                        break;

                    case DID_STANDARD:
                        // ???
                        return 0;

                    default:
                        return 0;
                    }

                if (_pIData->OldDataDCF1.bPort != DataDCF.bPort || _pIData->OldDataDCF1.usIOAddr != DataDCF.usIOAddr)
                    {
                    /* Umschaltung auf Gameport oder Seriell-Port mit Ctl-öbertragung: Auto-Scan aktivieren */
                    CheckScanMode (_pIData->OldDataDCF1.bPort, _pIData->OldDataDCF1.usIOAddr);

                    WinCheckButton (hwnd, CalcId (_pIData->bPort), TRUE);
                    FillEntry (hwnd, DataDCF.usIOAddr);
                    SaveSetData (somSelf, SAVESET_DRVRDATA);

                    _pIData->OldDataDCF1 = DataDCF;
                    }
                ulGetDCFData = GET_OK;
                }
            return 0;

        case WM_1STIMER:
            somSelf = (WPDCF77 *)WinQueryWindowPtr (hwnd, QWL_USER);
            somThis = WPDCF77GetData (somSelf);
            GetDCFData ();
            GetDCFStatus ();
            if (_pIData->OldDataDCF1.bPort != DataDCF.bPort || _pIData->OldDataDCF1.usIOAddr != DataDCF.usIOAddr)
                {
                WinCheckButton (hwnd, CalcId (DataDCF.bPort), TRUE);
                FillEntry (hwnd, DataDCF.usIOAddr);
                _pIData->OldDataDCF1 = DataDCF;
                }

            /* Deaktivieren nicht benîtigter Felder */
            EnableControls (hwnd, StatDCF.flMaskSS, IntfMask);

            return 0;

        case WM_DESTROY:
            WinSendMsg (hwnd, WM_CLOSE, 0, 0);
            return 0;

        case WM_CLOSE:
            somSelf = (WPDCF77 *)WinQueryWindowPtr (hwnd, QWL_USER);

            SaveSetData (somSelf, SAVESET_INIDATA);
            _wpRemove1sTimer (somSelf, hwnd);
            break;
        }

    return WinDefDlgProc (hwnd, msg, mp1, mp2);
    }

/*******************************************************************\
    IODlgProc: Window-Prozedur fÅr die Eingabe der IO-Adresse
    Eingang: hwnd: Window-Handle
             msg:  Message-Typ
             mp1:  1. Parameter
             mp2:  2. Parameter
\*******************************************************************/
MRESULT EXPENTRY IODlgProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
    {
    WPDCF77 *somSelf;
    CHAR    szText[5];

    switch (msg)
        {
        case WM_INITDLG:
            somSelf = (WPDCF77 *)mp2;
            WinSetWindowPtr (hwnd, QWL_USER, (PVOID) somSelf);

            WinSendDlgItemMsg (hwnd, DID_IOADDR, EM_SETTEXTLIMIT,
                MPFROMSHORT (WinQueryWindowUShort (hwnd, QWS_ID) == IDD_IOADDR ? 4 : 1), MPVOID);
            WinUpper (hab, 0, 0, _itoa (DataDCF.usIOAddr, szText, 16));
            WinSetDlgItemText (hwnd, DID_IOADDR, szText);
            return (MRESULT)TRUE;

        case WM_COMMAND:
            if (SHORT1FROMMP (mp2) == CMDSRC_PUSHBUTTON)
                {
                switch (SHORT1FROMMP (mp1))
                    {
                    case DID_OK:
                        ReadEntry (hwnd, DID_IOADDR);
                        WinDismissDlg (hwnd, DID_OK);
                        return 0;

                    case DID_CANCEL:
                        break;

                    default:
                        return 0;
                    }
                }
            break;

        case WM_HELP:
            if (SHORT1FROMMP (mp2) == CMDSRC_PUSHBUTTON && SHORT1FROMMP (mp1) == DID_HELP ||
                SHORT1FROMMP (mp2) == CMDSRC_ACCELERATOR)
                {
                somSelf = (WPDCF77 *)WinQueryWindowPtr (hwnd, QWL_USER);
                _wpDisplayHelp (somSelf,
                                WinQueryWindowUShort (hwnd, QWS_ID) == IDD_IOADDR ? PANEL_IOADDR : PANEL_COMPORT,
                                HELPFILENAME);
                return 0;
                }
            break;
        }

    return WinDefDlgProc (hwnd, msg, mp1, mp2);
    }

/*******************************************************************\
    DCF77DlgProc2: Window-Prozedur fÅr die zweite DCF77-Settings-
                   Seite
    Eingang: hwnd: Window-Handle
             msg:  Message-Typ
             mp1:  1. Parameter
             mp2:  2. Parameter
\*******************************************************************/
MRESULT EXPENTRY DCF77DlgProc2 (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
    {
    LONG        l;
    WPDCF77     *somSelf;
    WPDCF77Data *somThis;

    switch (msg)
        {
        case WM_INITDLG:
            somSelf = (WPDCF77 *)mp2;
            somThis = WPDCF77GetData (somSelf);
            WinSetWindowPtr (hwnd, QWL_USER, (PVOID) somSelf);

            _pIData->usSetInterval = DataDCF.usSetInterval;
            _pIData->usThreshold   = DataDCF.usThreshold;
            _pIData->bcRepeat      = DataDCF.bcRepeat;
            _pIData->usTicklen     = DataDCF.usTicklen;
            _pIData->usTimerMode   = DataDCF.usTimerMode;

            /* Spin-Button initialisieren */
            WinSendDlgItemMsg (hwnd, DID_CHECKINTERVAL, SPBM_SETLIMITS,
                MPFROMLONG (MAXINTERVAL), MPFROMLONG (MININTERVAL));
            WinSendDlgItemMsg (hwnd, DID_CHECKINTERVAL, SPBM_SETCURRENTVALUE,
                MPFROMLONG (DataDCF.bcRepeat), MPVOID);

            /* Sliderticks setzen, Sliderskalen beschriften */
            WinSendDlgItemMsg (hwnd, DID_UPDINTERVAL, SLM_SETTICKSIZE,
                MPFROM2SHORT (SMA_SETALLTICKS, 5), MPVOID);

            for (l = 0; l < TICKNUMBER1; l++)
                WinSendDlgItemMsg (hwnd, DID_UPDINTERVAL, SLM_SETSCALETEXT,
                    MPFROMSHORT (l), (MPARAM) pszScale[l]);

            /* Checkbox-Enable-Zustand setzen (fÅr EnableControls) */
            FindMask (IntvMask, DID_HR_TIMER)->bDisplay = StatDCF.bIsHRTimerAvail && (StatDCF.flMaskIV & MASK_IV_HRTIMER);

            /* Sliderposition (Slider 2) setzen */
            SetSlider2 (hwnd);

            /* Tickpositionen bestimmen, wenn noch nicht geschehen */
            if (usTick[TICKNUMBER1-1] == 0)
                {
                for (l = 0; l < TICKNUMBER1; l++)
                    usTick[l] = (USHORT) WinSendDlgItemMsg (hwnd, DID_UPDINTERVAL,
                        SLM_QUERYTICKPOS, MPFROMSHORT (l), MPVOID);

                for (l = TICKNUMBER1-1; l >= 0; l--)
                    usTick[l] -= usTick[0];         // Tick 0 beginnt bei Position 0!
                }

            /* Sliderposition (Slider 1) setzen */
            SetSlider1 (hwnd);

            /* Deaktivieren nicht benîtigter Felder */
            EnableControls (hwnd, StatDCF.flMaskIV, IntvMask);

            _pIData->OldDataDCF2 = DataDCF;

            /* Timer zur Aktualisierung der Anzeige starten */
            _wpAdd1sTimer (somSelf, hwnd);
            return (MRESULT)TRUE;

        case WM_CONTROL:
            switch (LONGFROMMP (mp1))
                {
                /* Slider */
                case DID_UPDINTERVAL + (SLN_SLIDERTRACK<<16):
                case DID_UPDINTERVAL + (SLN_CHANGE<<16):
                    ulGetDCFData = GET_PAUSE;
                    QuerySlider1 (hwnd);
                    break;

                case DID_THRESHOLD + (SLN_SLIDERTRACK<<16):
                case DID_THRESHOLD + (SLN_CHANGE<<16):
                    ulGetDCFData = GET_PAUSE;
                    QuerySlider2 (hwnd);
                    break;

                /* Spinbutton */
                case DID_CHECKINTERVAL + (SPBN_ENDSPIN<<16):
                    ulGetDCFData = GET_PAUSE;
                    WinSendMsg ((HWND)mp2, SPBM_QUERYVALUE, &l, MPFROM2SHORT (0, SPBQ_DONOTUPDATE));
                    DataDCF.bcRepeat = (BYTE)l;
                    break;

                /* Checkbox */
                case DID_HR_TIMER + (BN_CLICKED<<16):
                    {
                    somSelf = (WPDCF77 *)WinQueryWindowPtr (hwnd, QWL_USER);
                    somThis = WPDCF77GetData (somSelf);
//???                    if (_hwndFrameGraph)
//???                        WinPostMsg (_hwndFrameGraph, WM_CLOSE, 0, 0);
                    ulGetDCFData = GET_PAUSE;
                    DataDCF.usTimerMode =
                        WinQueryButtonCheckstate (hwnd, DID_HR_TIMER) ? TIMER_MODE_HR : TIMER_MODE_SYS;
                    DataDCF.usTicklen   = 0;
                    break;
                    }

                default:
                    return 0;
                }

            somSelf = (WPDCF77 *)WinQueryWindowPtr (hwnd, QWL_USER);
            somThis = WPDCF77GetData (somSelf);
            if (_pIData->OldDataDCF2.usThreshold   != DataDCF.usThreshold   ||
                _pIData->OldDataDCF2.usSetInterval != DataDCF.usSetInterval ||
                _pIData->OldDataDCF2.bcRepeat      != DataDCF.bcRepeat      ||
                _pIData->OldDataDCF2.usTimerMode   != DataDCF.usTimerMode   ||
                _pIData->OldDataDCF2.usTicklen     != DataDCF.usTicklen)
                {
                SaveSetData (somSelf, SAVESET_DRVRDATA);
                _pIData->OldDataDCF2 = DataDCF;
                }
            ulGetDCFData = GET_OK;
            return 0;

        case WM_COMMAND:
            if (SHORT1FROMMP (mp2) == CMDSRC_PUSHBUTTON)
                {
                somSelf = (WPDCF77 *)WinQueryWindowPtr (hwnd, QWL_USER);
                somThis = WPDCF77GetData (somSelf);
                switch (SHORT1FROMMP (mp1))
                    {
                    case DID_CANCEL:
                        ulGetDCFData          = GET_PAUSE;
                        DataDCF.usSetInterval = _pIData->usSetInterval;
                        DataDCF.usThreshold   = _pIData->usThreshold;
                        DataDCF.bcRepeat      = _pIData->bcRepeat;
                        DataDCF.usTimerMode   = _pIData->usTimerMode;
                        DataDCF.usTicklen     = _pIData->usTicklen;
                        break;

                    case DID_STANDARD:
                        // ???
                        return 0;

                    default:
                        return 0;
                    }

                if (_pIData->OldDataDCF2.usThreshold   != DataDCF.usThreshold   ||
                    _pIData->OldDataDCF2.usSetInterval != DataDCF.usSetInterval ||
                    _pIData->OldDataDCF2.bcRepeat      != DataDCF.bcRepeat      ||
                    _pIData->OldDataDCF2.usTimerMode   != DataDCF.usTimerMode   ||
                    _pIData->OldDataDCF2.usTicklen     != DataDCF.usTicklen)
                    {
                    SetSlider1 (hwnd);
                    SetSlider2 (hwnd);
                    WinSendDlgItemMsg (hwnd, DID_CHECKINTERVAL, SPBM_SETCURRENTVALUE,
                        MPFROMLONG (DataDCF.bcRepeat), MPVOID);
                    SaveSetData (somSelf, SAVESET_DRVRDATA);
                    _pIData->OldDataDCF2 = DataDCF;
                    }
                ulGetDCFData = GET_OK;
                }
            return 0;

        case WM_1STIMER:
            somSelf = (WPDCF77 *)WinQueryWindowPtr (hwnd, QWL_USER);
            somThis = WPDCF77GetData (somSelf);
            GetDCFData ();
            GetDCFStatus ();
            if (_pIData->OldDataDCF2.usThreshold   != DataDCF.usThreshold   ||
                _pIData->OldDataDCF2.usSetInterval != DataDCF.usSetInterval ||
                _pIData->OldDataDCF2.bcRepeat      != DataDCF.bcRepeat      ||
                _pIData->OldDataDCF2.usTimerMode   != DataDCF.usTimerMode   ||
                _pIData->OldDataDCF2.usTicklen     != DataDCF.usTicklen)
                {
                SetSlider1 (hwnd);
                SetSlider2 (hwnd);
                WinSendDlgItemMsg (hwnd, DID_CHECKINTERVAL, SPBM_SETCURRENTVALUE,
                    MPFROMLONG (DataDCF.bcRepeat), MPVOID);
                SaveSetData (somSelf, SAVESET_DRVRDATA);
                _pIData->OldDataDCF2 = DataDCF;
                }

            /* Deaktivieren nicht benîtigter Felder */
            EnableControls (hwnd, StatDCF.flMaskIV, IntvMask);

            return 0;

        case WM_DESTROY:
            WinSendMsg (hwnd, WM_CLOSE, 0, 0);
            return 0;

        case WM_CLOSE:
            somSelf = (WPDCF77 *)WinQueryWindowPtr (hwnd, QWL_USER);
            SaveSetData (somSelf, SAVESET_INIDATA);
            _wpRemove1sTimer (somSelf, hwnd);
            break;
        }

    return WinDefDlgProc (hwnd, msg, mp1, mp2);
    }

/*******************************************************************\
    DCF77DlgProc3: Window-Prozedur fÅr die dritte DCF77-Settings-
                   Seite
    Eingang: hwnd: Window-Handle
             msg:  Message-Typ
             mp1:  1. Parameter
             mp2:  2. Parameter
\*******************************************************************/
MRESULT EXPENTRY DCF77DlgProc3 (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
    {
    LONG        l;
    WPDCF77     *somSelf;
    WPDCF77Data *somThis;

    switch (msg)
        {
        case WM_INITDLG:
            somSelf = (WPDCF77 *)mp2;
            somThis = WPDCF77GetData (somSelf);
            WinSetWindowPtr (hwnd, QWL_USER, (PVOID) somSelf);

            _pIData->bSupply = DataDCF.bSupply;
            _pIData->bSignal = DataDCF.bSignal;

            /* Buttons fÅr Versorgung initialisieren */
            SetSupply  (hwnd);

            /* Gruppe fÅr Signaleingang initialisieren */
            for (l = 0; l < ENTRYNUMBER; l++)
                WinSendDlgItemMsg (hwnd, DID_SIGNAL_POS, LM_INSERTITEM,
                    MPFROMSHORT (LIT_END), MPFROMP (pszCombo[l]));
            SetSignal (hwnd);

            /* Deaktivieren nicht benîtigter Felder */
            EnableControls (hwnd, StatDCF.flMaskLN, CtrlMask);

            _pIData->OldDataDCF3 = DataDCF;

            /* Timer zur Aktualisierung der Anzeige starten */
            _wpAdd1sTimer (somSelf, hwnd);
            return (MRESULT)TRUE;

        case WM_CONTROL:
            switch (LONGFROMMP (mp1))
                {
                /* Radio-Buttons */
                case DID_SIGNAL_SER + (BN_CLICKED<<16):
                    ulGetDCFData = GET_PAUSE;
                    DataDCF.bSignal |=  SIGNAL_SERIAL;
                    DataDCF.bSignal &= ~SIGNAL_SCAN;
                    SetSignal (hwnd);
                    break;

                case DID_SIGNAL_CTL + (BN_CLICKED<<16):
                    ulGetDCFData = GET_PAUSE;
                    DataDCF.bSignal &= ~SIGNAL_SERIAL;
                    SetSignal (hwnd);
                    break;

                /* Combo-Box */
                case DID_SIGNAL_POS + (CBN_ENTER<<16):
                    ulGetDCFData = GET_PAUSE;
                    l = (LONG)(SHORT)WinSendMsg ((HWND)mp2, LM_QUERYSELECTION,
                        MPFROMSHORT (LIT_FIRST), MPVOID);
                    if ((SHORT)l == ENTRYNUMBER-1)
                        DataDCF.bSignal = SIGNAL_SCAN;
                    else if ((SHORT)l != LIT_NONE)
                        DataDCF.bSignal = (BYTE)l;
                    break;

                /* Checkbox */
                case DID_POWER_ON + (BN_CLICKED<<16):
                    ulGetDCFData = GET_PAUSE;
                    DataDCF.bSupply &= ~SUPPLY_OFF;
                    DataDCF.bSupply |= WinQueryButtonCheckstate (hwnd, DID_POWER_ON) ? 0 : SUPPLY_OFF;
                    SetSupply (hwnd);
                    break;

                default:
                    return 0;
                }

            somSelf = (WPDCF77 *)WinQueryWindowPtr (hwnd, QWL_USER);
            somThis = WPDCF77GetData (somSelf);
            if (_pIData->OldDataDCF3.bSignal != DataDCF.bSignal || _pIData->OldDataDCF3.bSupply != DataDCF.bSupply)
                {
                SaveSetData (somSelf, SAVESET_DRVRDATA);
                _pIData->OldDataDCF3 = DataDCF;
                }
            ulGetDCFData = GET_OK;
            return 0;

        case WM_COMMAND:
            if (SHORT1FROMMP (mp2) == CMDSRC_PUSHBUTTON)
                {
                ulGetDCFData = GET_PAUSE;
                somSelf = (WPDCF77 *)WinQueryWindowPtr (hwnd, QWL_USER);
                somThis = WPDCF77GetData (somSelf);
                switch (SHORT1FROMMP (mp1))
                    {
                    case DID_CANCEL:
                        ulGetDCFData    = GET_PAUSE;
                        DataDCF.bSupply = _pIData->bSupply;
                        DataDCF.bSignal = _pIData->bSignal;
                        break;

                    case DID_STANDARD:
                        // ???
                        return 0;

                    case DID_POWER_1:
                        ulGetDCFData     = GET_PAUSE;
                        DataDCF.bSupply ^= SUPPLY_L1_POS;
                        break;

                    case DID_POWER_2:
                        ulGetDCFData     = GET_PAUSE;
                        DataDCF.bSupply ^= SUPPLY_L2_POS;
                        break;

                    default:
                        return 0;
                    }

                if (_pIData->OldDataDCF3.bSignal != DataDCF.bSignal || _pIData->OldDataDCF3.bSupply != DataDCF.bSupply)
                    {
                    SaveSetData (somSelf, SAVESET_DRVRDATA);
                    SetSupply (hwnd);
                    SetSignal (hwnd);
                    _pIData->OldDataDCF3 = DataDCF;
                    }
                ulGetDCFData = GET_OK;
                }
            return 0;

        case WM_WINDOWPOSCHANGED:
            if ((ULONG) mp2 == 0 && (((PSWP)mp1)->fl & SWP_SHOW))
                {
                SetSupply (hwnd);
                SetSignal (hwnd);
                }
            break;

        case WM_1STIMER:
            somSelf = (WPDCF77 *)WinQueryWindowPtr (hwnd, QWL_USER);
            somThis = WPDCF77GetData (somSelf);
            GetDCFData ();
            GetDCFStatus ();
            if (_pIData->OldDataDCF3.bSignal != DataDCF.bSignal || _pIData->OldDataDCF3.bSupply != DataDCF.bSupply)
                {
                SetSupply (hwnd);
                SetSignal (hwnd);
                _pIData->OldDataDCF3 = DataDCF;
                }

            /* Deaktivieren nicht benîtigter Felder */
            EnableControls (hwnd, StatDCF.flMaskLN, CtrlMask);

            return 0;

        case WM_DESTROY:
            WinSendMsg (hwnd, WM_CLOSE, 0, 0);
            return 0;

        case WM_CLOSE:
            somSelf = (WPDCF77 *)WinQueryWindowPtr (hwnd, QWL_USER);
            SaveSetData (somSelf, SAVESET_INIDATA);
            _wpRemove1sTimer (somSelf, hwnd);
            break;
        }

    return WinDefDlgProc (hwnd, msg, mp1, mp2);
    }

/*******************************************************************\
    StatusDlgProc: Window-Prozedur fÅr die Status-Seite
    Eingang: hwnd: Window-Handle
             msg:  Message-Typ
             mp1:  1. Parameter
             mp2:  2. Parameter
\*******************************************************************/
MRESULT EXPENTRY StatusDlgProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
    {
    WPDCF77      *somSelf;
    WPDCF77Data  *somThis;
    CHAR         szText[CCHMAXPGNAME];         // Text fÅr Versionsnummer

    switch (msg)
        {
        case WM_INITDLG:
            somSelf = (WPDCF77 *)mp2;
            somThis = WPDCF77GetData (somSelf);
            WinSetWindowPtr (hwnd, QWL_USER, (PVOID)somSelf);

            /* Ansicht registrieren */
            memset (&_pIData->StatViewItem, '\0', sizeof (VIEWITEM));
            _pIData->StatUseItem.type    = USAGE_OPENVIEW;
            _pIData->StatViewItem.view   = OPEN_STATUS;
            _pIData->StatViewItem.handle = hwnd;
            _wpAddToObjUseList(somSelf,&_pIData->StatUseItem);

            WinLoadString (hab, hmod, IDS_STATUSVIEW, CCHMAXPGNAME, szText);
            _wpRegisterView (somSelf, hwnd, szText);

            /* DCF-Status und -Zeit holen */
            GetDCFStatus ();
            GetDCFDateTime ();
            GetGPSCoord ();

            SetStatusCtls (hwnd, MASK_ALL);
            SetVisibility (hwnd);

            _pIData->OldStatDCF = StatDCF;

            _wpAdd1sTimer (somSelf, hwnd);
            return (MRESULT)TRUE;

        case WM_1STIMER:
            somSelf = (WPDCF77 *)WinQueryWindowPtr (hwnd, QWL_USER);
            somThis = WPDCF77GetData (somSelf);

            GetDCFDateTime ();
            SetStatusCtls (hwnd, GetStatusMask (somThis));
            SetVisibility (hwnd);

            return 0;

        case WM_HELP:
            if (SHORT1FROMMP (mp2) == CMDSRC_PUSHBUTTON && SHORT1FROMMP (mp1) == DID_HELP ||
                SHORT1FROMMP (mp2) == CMDSRC_ACCELERATOR)
                {
                somSelf = (WPDCF77 *)WinQueryWindowPtr (hwnd, QWL_USER);
                _wpDisplayHelp (somSelf, PANEL_DCF77_STATUS, HELPFILENAME);
                return 0;
                }
            break;

        case WM_COMMAND:
            if (SHORT1FROMMP (mp1) == DID_CANCEL && SHORT1FROMMP (mp2) == CMDSRC_PUSHBUTTON)
                WinSendMsg (hwnd, WM_CLOSE, MPVOID, MPVOID);
            return 0;

        case WM_CLOSE:
            somSelf = (WPDCF77 *)WinQueryWindowPtr (hwnd, QWL_USER);
            somThis = WPDCF77GetData (somSelf);
            _wpRemove1sTimer (somSelf, hwnd);
            _wpDeleteFromObjUseList (somSelf, &_pIData->StatUseItem);
            WinDestroyWindow (hwnd);
            break;
        }

    return WinDefDlgProc (hwnd, msg, mp1, mp2);
    }

/*******************************************************************\
    InfoWndProc: Window-Prozedur zur Anzeige des Marker-Symbols
                 in StatusDlgProc.
    Eingang: hwnd: Window-Handle
             msg:  Message-Typ
             mp1:  1. Parameter
             mp2:  2. Parameter
\*******************************************************************/
MRESULT EXPENTRY InfoWndProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
    {
    HPS    hps;
    RECTL  rectl;
    POINTL ptl;
    POINTL aptl[4];
    LONG   lx, ly;

    switch (msg)
        {
        case WM_CREATE:
            WinSetWindowULong (hwnd, 0, MARKSYM_DIAMOND);
            return FALSE;

        case WM_SETMARKER:
            WinSetWindowULong (hwnd, 0, LONGFROMMP (mp1));
            WinInvalidateRect (hwnd, NULL, TRUE);
            return 0;

        case WM_ENABLE:
            WinInvalidateRect (hwnd, NULL, TRUE);
            break;

        case WM_PAINT:
            hps = WinBeginPaint (hwnd, NULLHANDLE, NULL);
            WinQueryWindowRect (hwnd, &rectl);
            WinFillRect (hps, &rectl, SYSCLR_DIALOGBACKGROUND);
            GpiSetColor (hps, WinQueryWindowULong (hwnd, QWL_STYLE) & WS_DISABLED ? CLR_DARKGRAY : CLR_BLACK);

            ptl.x = rectl.xRight / 2;
            ptl.y = rectl.yTop / 2;

            /* Alternative zu Markern (Markerproblem auf ATI-Karten) */
            lx = ptl.x / 2;
            ly = ptl.y / 2;
            aptl[0].x = lx;
            aptl[0].y = ptl.y;
            aptl[1].x = ptl.x;
            aptl[1].y = 2*ptl.y - ly;
            aptl[2].x = 2*ptl.x - lx;
            aptl[2].y = ptl.y;
            aptl[3].x = ptl.x;
            aptl[3].y = ly;
            GpiBeginPath (hps, 1);
            GpiMove (hps, &aptl[3]);
            GpiPolyLine (hps, 4, aptl);
            GpiEndPath (hps);
            if ((LONG)WinQueryWindowULong (hwnd, 0) == MARKSYM_SOLIDDIAMOND)
                GpiFillPath (hps, 1, FPATH_ALTERNATE);
            else
                GpiOutlinePath (hps, 1, 0);

            WinEndPaint (hps);
            break;
        }

    return WinDefWindowProc (hwnd, msg, mp1, mp2);
    }

/*******************************************************************\
    TmOffsDlgProc: Dialog-Prozedur fÅr Zeitoffset-Einstellungen
    Eingang: hwnd: Window-Handle
             msg:  Message-Typ
             mp1:  1. Parameter
             mp2:  2. Parameter
\*******************************************************************/
MRESULT EXPENTRY TmOffsDlgProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
    {
    LONG        l;
    CHAR        szText[4];
    WPDCF77     *somSelf;
    WPDCF77Data *somThis;

    switch (msg)
        {
        case WM_INITDLG:
            somSelf = (WPDCF77 *)mp2;
            somThis = WPDCF77GetData (somSelf);
            WinSetWindowPtr (hwnd, QWL_USER, (PVOID) somSelf);

            _pIData->flUTC    = OffsDCF.flUTC;
            _pIData->sGOffset = OffsDCF.sOffset;
            _pIData->sLOffset = (SHORT)_lLOffset;

            /* Check-Button einstellen */
            WinCheckButton (hwnd, DID_ISUTC, _pIData->flUTC ? TRUE : FALSE);

            /* Slider-TicklÑnge setzen */
            WinSendDlgItemMsg (hwnd, DID_GOFFSET, SLM_SETTICKSIZE,
                MPFROM2SHORT (SMA_SETALLTICKS, 5), MPVOID);
            WinSendDlgItemMsg (hwnd, DID_LOFFSET, SLM_SETTICKSIZE,
                MPFROM2SHORT (SMA_SETALLTICKS, 5), MPVOID);

            /* Sliderskalen beschriften */
            for (l = 0; l < TICKNUMBER3; l++)
                WinSendDlgItemMsg (hwnd, DID_GOFFSET, SLM_SETSCALETEXT,
                    MPFROMSHORT (l), MPFROMP (_itoa (l-2, szText, 10)));

            for (l = 0; l < TICKNUMBER4; l+=4)
                {
                WinSendDlgItemMsg (hwnd, DID_LOFFSET, SLM_SETTICKSIZE,
                    MPFROM2SHORT ((SHORT)l, 7), MPVOID);
                WinSendDlgItemMsg (hwnd, DID_LOFFSET, SLM_SETSCALETEXT,
                    MPFROMSHORT (l), MPFROMP (_itoa (l-12, szText, 10)));
                }

            /* Slider-Positionen einstellen */
            WinSendDlgItemMsg (hwnd, DID_GOFFSET, SLM_SETSLIDERINFO,
                MPFROM2SHORT (SMA_SLIDERARMPOSITION, SMA_INCREMENTVALUE),
                MPFROMSHORT (_pIData->sGOffset+2));

            WinSendDlgItemMsg (hwnd, DID_LOFFSET, SLM_SETSLIDERINFO,
                MPFROM2SHORT (SMA_SLIDERARMPOSITION, SMA_INCREMENTVALUE),
                MPFROMSHORT ((SHORT)(_lLOffset+12)));

            /* Eingabefeld initialisieren */
            WinSetDlgItemText (hwnd, DID_TZTXT, _szTimezone);

            /* Deaktivieren nicht benîtigter Felder */
            EnableControls (hwnd, StatDCF.flMaskTO, OffsMask);

            _pIData->OldOffsDCF = OffsDCF;

            /* Timer zur Aktualisierung der Anzeige starten */
            _wpAdd1sTimer (somSelf, hwnd);
            return (MRESULT)TRUE;

        case WM_CONTROL:
            switch (LONGFROMMP (mp1))
                {
                case DID_ISUTC + (BN_CLICKED<<16):
                    OffsDCF.flUTC = (BYTE)WinQueryButtonCheckstate (hwnd, DID_ISUTC);
                    break;

                case DID_GOFFSET + (SLN_SLIDERTRACK<<16):
                case DID_GOFFSET + (SLN_CHANGE<<16):
                    OffsDCF.sOffset = (SHORT)WinSendDlgItemMsg (hwnd, DID_GOFFSET,
                        SLM_QUERYSLIDERINFO,
                        MPFROM2SHORT (SMA_SLIDERARMPOSITION, SMA_INCREMENTVALUE),
                        MPVOID) - 2;
                    break;

                case DID_LOFFSET + (SLN_SLIDERTRACK<<16):
                case DID_LOFFSET + (SLN_CHANGE<<16):
                    somSelf   = (WPDCF77 *)WinQueryWindowPtr (hwnd, QWL_USER);
                    somThis   = WPDCF77GetData (somSelf);
                    _lLOffset = (LONG)WinSendDlgItemMsg (hwnd, DID_LOFFSET,
                        SLM_QUERYSLIDERINFO,
                        MPFROM2SHORT (SMA_SLIDERARMPOSITION, SMA_INCREMENTVALUE),
                        MPVOID) - 12;
                    return 0;

                default:
                    return 0;
                }

            somSelf = (WPDCF77 *)WinQueryWindowPtr (hwnd, QWL_USER);
            somThis = WPDCF77GetData (somSelf);
            SaveSetData (somSelf, SAVESET_DRVROFFSET);
            _pIData->OldOffsDCF = OffsDCF;
            break;

        case WM_COMMAND:
            if (SHORT1FROMMP (mp2) == CMDSRC_PUSHBUTTON)
                {
                somSelf   = (WPDCF77 *)WinQueryWindowPtr (hwnd, QWL_USER);
                somThis   = WPDCF77GetData (somSelf);
                switch (SHORT1FROMMP (mp1))
                    {
                    case DID_CANCEL:
                        OffsDCF.flUTC   = _pIData->flUTC;
                        OffsDCF.sOffset = _pIData->sGOffset;
                        _lLOffset       = _pIData->sLOffset;
                        WinSetDlgItemText (hwnd, DID_TZTXT, _szTimezone);
                        break;

                    case DID_STANDARD:
                        OffsDCF.flUTC   = 0;
                        OffsDCF.sOffset = 0;
                        _lLOffset       = 0;
                        WinSetDlgItemText (hwnd, DID_TZTXT, "");
                        break;

                    default:
                        return 0;
                    }

                if (_pIData->OldOffsDCF.flUTC != OffsDCF.flUTC || _pIData->OldOffsDCF.sOffset != OffsDCF.sOffset)
                    {
                    SetOffsCtls (hwnd);
                    SaveSetData (somSelf, SAVESET_DRVROFFSET);
                    _pIData->OldOffsDCF = OffsDCF;
                    }

                WinSendDlgItemMsg (hwnd, DID_LOFFSET, SLM_SETSLIDERINFO,
                    MPFROM2SHORT (SMA_SLIDERARMPOSITION, SMA_INCREMENTVALUE),
                    MPFROMSHORT ((SHORT)(_lLOffset+12)));
                }
            return 0;

        case WM_1STIMER:
            somSelf = (WPDCF77 *)WinQueryWindowPtr (hwnd, QWL_USER);
            somThis = WPDCF77GetData (somSelf);
            GetDCFOffs ();
            GetDCFStatus ();
            if (_pIData->OldOffsDCF.flUTC != OffsDCF.flUTC || _pIData->OldOffsDCF.sOffset != OffsDCF.sOffset)
                {
                SetOffsCtls (hwnd);
                _pIData->OldOffsDCF = OffsDCF;
                }

            /* Deaktivieren nicht benîtigter Felder */
            EnableControls (hwnd, StatDCF.flMaskTO, OffsMask);

            return 0;

        case WM_DESTROY:
            WinSendMsg (hwnd, WM_CLOSE, 0, 0);
            return 0;

        case WM_CLOSE:
            somSelf = (WPDCF77 *)WinQueryWindowPtr (hwnd, QWL_USER);
            somThis = WPDCF77GetData (somSelf);
            WinQueryDlgItemText (hwnd, DID_TZTXT, CCHMAXTZONE, _szTimezone);

            SaveSetData (somSelf, SAVESET_INIOFFSET);
            _wpRemove1sTimer (somSelf, hwnd);
            _wpSaveDeferred (somSelf);
            _wpRefreshClockView (somSelf);
            break;
        }

    return WinDefDlgProc (hwnd, msg, mp1, mp2);
    }

/*******************************************************************\
    determineDrvType: interne Funktion, die den Treibertyp bestimmt,
    falls da noch nicht geschehen ist.
\*******************************************************************/
VOID determineDrvType (VOID)
    {
    if (cDrvType == 0)
        {
        GetDCFStatus ();
        cDrvType = StatDCF.cDrvType;
        }

    return;
    }

/*******************************************************************\
    isAnySource: Bestimmt, ob ein Treiber verfÅgbar ist oder nicht.
    Vorrang hat ein vorhandener "*.sys"-Treiber;
    ist dieser nicht vorhanden, wird nach einem Shared memory
    gesucht.
    Diese Funktion ist aus PerformancegrÅnden keine som-Methode
    return: TRUE: Treiber vorhanden
\*******************************************************************/
BOOL isAnySource (VOID)
    {
    if (hDrvr != -1)
        return TRUE;
    if (pShMem)
        {
        if (pShMem->flProgs & (PROG_DRIVER | PROG_CLIENT))
            return TRUE;
        }
    return FALSE;
    }

/*******************************************************************\
    isClient: Bestimmt, ob die Uhr vom Client versorgt wird.
    Diese Funktion ist aus PerformancegrÅnden keine som-Methode
    return: TRUE: Client vorhanden
\*******************************************************************/
BOOL isClient (VOID)
    {
    if (pShMem)
        if (pShMem->flProgs & PROG_CLIENT)
            return TRUE;

    return FALSE;
    }

/*******************************************************************\
    isDriver: Bestimmt, ob die Uhr aus einem Detached-Programm
    versorgt wird (z. B. Treiber fÅr ext. hopf-Modul).
    Diese Funktion ist aus PerformancegrÅnden keine som-Methode
    return: TRUE: Treiber vorhanden
\*******************************************************************/
BOOL isDriver (VOID)
    {
    if (pShMem)
        if (pShMem->flProgs & PROG_DRIVER)
            return TRUE;

    return FALSE;
    }

/*******************************************************************\
    isHopf603x: Bestimmt, ob ein Hopf 603x-Modul installiert ist
    Diese Funktion ist aus PerformancegrÅnden keine som-Methode
    return: TRUE: Hardware vorhanden
\*******************************************************************/
BOOL isHopf603x (VOID)
    {
    determineDrvType ();
    return cDrvType == DRVTYPE_HOPF6036 || cDrvType == DRVTYPE_HOPF6038;
    }

/*******************************************************************\
    isHopf6038: Bestimmt, ob ein Hopf 6038-Modul installiert ist
    Diese Funktion ist aus PerformancegrÅnden keine som-Methode.
    GetDCFDateTime mu· vorher aufgerufen werden.
    return: TRUE: Hardware vorhanden
\*******************************************************************/
BOOL isHopf6038 (VOID)
    {
    determineDrvType ();
    return cDrvType == DRVTYPE_HOPF6038;
    }

/*******************************************************************\
    isUSBDevice: Bestimmt, of ein USB-Treiber installiert ist.
    Diese Funktion ist aus PerformancegrÅnden keine som-Methode.
    GetDCFDateTime mu· vorher aufgerufen werden.
    return: TRUE: Hardware vorhanden
\*******************************************************************/
BOOL isUSBDevice (VOID)
    {
    determineDrvType ();
    return cDrvType == DRVTYPE_USB;
    }

/*******************************************************************\
    isExtModule: Bestimmt, ob ein externes DCF77-Modul installiert
    ist.
    GetDCFDateTime mu· vorher aufgerufen werden.
    return: TRUE: Hardware vorhanden
\*******************************************************************/
BOOL isExtModule (VOID)
    {
    determineDrvType ();
    return cDrvType == DRVTYPE_EXTMOD;
    }

/*******************************************************************\
    isSrvrEqClnt: Bestimmt, ob der Client und der Server dieselbe
    Version besitzen.
    Diese Funktion ist aus PerformancegrÅnden keine som-Methode
    return: FALSE: Eindeutig falsche Serverversion; Client hat auf
                   dieser Maschine deshalb terminiert
\*******************************************************************/
BOOL isSrvrEqClnt (VOID)
    {
    if (pShMem)
        if (!(pShMem->flProgs & PROG_CLIENT) && pShMem->ulTermReason == TERM_VERSION)
            return FALSE;

    return TRUE;
    }

/*******************************************************************\
    Ins1Value: FÅgt einen neuen Statuswert in den Statuspuffer ein.
\*******************************************************************/
VOID Ins1Value (USHORT flStatus)
    {
    if (pGraphData == NULL)
        {
        DosAllocMem ((PPVOID)&pGraphData, CCHGRPHBUF, PAG_COMMIT | PAG_READ | PAG_WRITE);
        ulIndexGrphData = ulcGraphInData = 0;
        pGraphData[0] = 0;
        DebugE (D_STAT, "Ins1Value", "Puffer allokiert");
        }

    pGraphData[ulIndexGrphData] += flStatus;
    DebugULd (D_STAT, "Ins1Value", "aktueller Wert", pGraphData[ulIndexGrphData]);
    if (++ulcGraphInData >= CCHMINPERPIXEL)
        {
        pGraphData[ulIndexGrphData++] *= CCHGRPHMAX / CCHMINPERPIXEL;
        DebugULd (D_STAT, "Ins1Value", "neuer Index", ulIndexGrphData);
        ulcGraphInData = 0;
        if (ulIndexGrphData == CCHGRPHBUF/sizeof (USHORT))
            {
            ulIndexGrphData--;
            memcpy (pGraphData, pGraphData+1, CCHGRPHBUF - sizeof (USHORT));
            }
        pGraphData[ulIndexGrphData] = 0;
        }

    return;
    }

/*******************************************************************\
    InsStatusBuffer: FÅgt einen neuen Statuswert in den Statuspuffer
    ein. Bei Unterbrechung (durch hohe CPU-Last) werden 0-Werte
    eingefÅgt.
\*******************************************************************/
VOID InsStatusBuffer (USHORT flRcvrStatus)
    {
    static ULONG ulOldStatusTCntr = 0;          // zur Auswertung: Unterbrechung der Aufzeichnung

    ULONG ulNum;

    ulNum = (ulTimerCntr - ulOldStatusTCntr) / 60;
    DebugULd (D_IO, "InsStatusBuffer", "Unterbrechung in Minuten", ulNum);

    for (;ulNum > 0; ulNum--)
        Ins1Value (0);

    if (ulTimerCntr % 60 == 0)                  // nur 1x pro Minute auswerten
        Ins1Value (flRcvrStatus & STATUS_TIMEVALID ? 1 : 0);

    ulOldStatusTCntr = ulTimerCntr;
    return;
    }

/*******************************************************************\
    Get_DCFStatus: Low level Routine zum Auslesen des Treiberstatus.
    Ist ein SYS-Treiber verfÅgbar, wird dieser ausgelesen.
    Ist ein CLIENT oder DRIVER verfÅgbar, wird dieser ausgelesen.
    Sonst wird FALSE zurÅckgeliefert.
    return: TRUE: Daten vorhanden
\*******************************************************************/
BOOL Get_DCFStatus (PDCF77_STATUS pStatDCF)
    {
    ULONG ulcData, ulcParm;
    BYTE  bParmPacket;
    BOOL  bRC;

    if (hDrvr != -1)
        {
        /* Zugriff auf Treiber, falls vorhanden */
        ulcParm = ulcData = bParmPacket = 0;
        bRC = DosDevIOCtl (hDrvr, IOCTL_DCF77, DCF77_GETSTATUS,
            &bParmPacket, sizeof (bParmPacket),  &ulcParm,
            pStatDCF,     sizeof (DCF77_STATUS), &ulcData) ? FALSE : TRUE;
        pStatDCF->flStatus |= STATUS_ISDRIVER;
        }
    else if (pShMem && (pShMem->flProgs & (PROG_CLIENT | PROG_DRIVER)))
        {
        /* Zugriff auf LAN::Time Client oder Detached-Programm */
        *pStatDCF = pShMem->dcf77Data.dcf77Status;
        pStatDCF->flStatus |= STATUS_ISDRIVER;
        bRC = TRUE;
        }
    else
        {
        /* keine Datenquelle gefunden */
        bRC = FALSE;
        }

    if (bRC)
        InsStatusBuffer (StatDCF.flStatus);

    return bRC;
    }

/*******************************************************************\
    Get_GPSCoord: Low level Routine zum Auslesen der Koordinaten.
    Ist ein SYS-Treiber verfÅgbar, wird dieser ausgelesen.
    Ein CLIENT oder DRIVER unterstÅtzt diese FunktionalitÑt nicht.
    return: TRUE: Daten vorhanden
            FALSE: keine Daten vorhanden (Fehler / nicht unterstÅtzt)
\*******************************************************************/
BOOL Get_GPSCoord (PDCF77_COORD pCoordGPS)
    {
    ULONG ulcData, ulcParm;
    BYTE  bParmPacket;
    BOOL  bRC;

    if (hDrvr != -1)
        {
        /* Zugriff auf Treiber, falls vorhanden */
        ulcParm = ulcData = bParmPacket = 0;
        bRC = DosDevIOCtl (hDrvr, IOCTL_DCF77, DCF77_QUERYCOORD,
            &bParmPacket, sizeof (bParmPacket),  &ulcParm,
            pCoordGPS,    sizeof (DCF77_COORD),  &ulcData) ? FALSE : TRUE;
        }
    else
        {
        /* keine Datenquelle gefunden */
        bRC = FALSE;
        }

    return bRC;
    }

/*******************************************************************\
    Query_DCFData: Low level Routine zum Auslesen
    der Treibereinstellungen.
    Ist ein SYS-Treiber verfÅgbar, wird dieser ausgelesen.
    Ist ein CLIENT oder DRIVER verfÅgbar, wird dieser ausgelesen.
    Sonst wird FALSE zurÅckgeliefert.
    return: TRUE: Daten vorhanden
\*******************************************************************/
BOOL Query_DCFData (PDCF77_DATA pDataDCF)
    {
    ULONG ulcData, ulcParm;
    BYTE  bParmPacket;
    BOOL  bRC;

    if (hDrvr != -1)
        {
        /* Zugriff auf Treiber, falls vorhanden */
        ulcData     = 0;
        ulcParm     = sizeof (bParmPacket);
        bParmPacket = CMD_DATA_ACTUAL;
        bRC = DosDevIOCtl (hDrvr, IOCTL_DCF77, DCF77_QUERYDATA,
            &bParmPacket, sizeof (bParmPacket), &ulcParm,
            pDataDCF,     sizeof (DCF77_DATA),  &ulcData) ? FALSE : TRUE;
        }
    else if (isDriver ())
        {
        /* Zugriff auf LAN::Time Detached-Programm */
        memcpy (pDataDCF, &(pShMem->dcf77Data.dcf77Data), sizeof (DCF77_DATA));
        bRC = TRUE;
        }
    else if (isClient ())
        {
        /* Zugriff auf LAN::Time Client */
        memset (pDataDCF, '\0', sizeof (DCF77_DATA));
        pDataDCF->bPort = PORT_SHMEM;
        bRC = TRUE;
        }
    else
        /* keine Datenquelle gefunden */
        bRC = FALSE;

    return bRC;
    }

/*******************************************************************\
    Query_DCFOffset: Low level Routine zum Auslesen
    des Zeitoffset
    Ist ein SYS-Treiber verfÅgbar, wird dieser ausgelesen.
    Ist ein CLIENT oder DRIVER verfÅgbar, wird dieser ausgelesen.
    Sonst wird FALSE zurÅckgeliefert.
    return: TRUE: Daten vorhanden
\*******************************************************************/
BOOL Query_DCFOffset (PDCF77_OFFSET pOffsDCF)
    {
    ULONG ulcData, ulcParm;
    BYTE  bParmPacket;
    BOOL  bRC;

    if (hDrvr != -1)
        {
        /* Zugriff auf Treiber, falls vorhanden */
        ulcData = ulcParm = bParmPacket = 0;
        bRC = DosDevIOCtl (hDrvr, IOCTL_DCF77, DCF77_QUERYOFFSET,
            &bParmPacket, sizeof (bParmPacket),  &ulcParm,
            pOffsDCF,     sizeof (DCF77_OFFSET), &ulcData) ? FALSE : TRUE;
        }
    else if (pShMem && (pShMem->flProgs & (PROG_CLIENT | PROG_DRIVER)))
        {
        /* Zugriff auf LAN::Time Client oder Detached-Programm */
        memcpy (pOffsDCF, &(pShMem->dcf77Data.dcf77Offset), sizeof (DCF77_OFFSET));
        bRC = TRUE;
        }
    else
        /* keine Datenquelle gefunden */
        bRC = FALSE;

    return bRC;
    }

/*******************************************************************\
    Query_DCFDateTime: Low level Routine zum Auslesen
    von Datum und Uhrzeit.
    Ist ein SYS-Treiber verfÅgbar, wird dieser ausgelesen.
    Ist ein CLIENT oder DRIVER verfÅgbar, wird dieser ausgelesen.
    Sonst wird FALSE zurÅckgeliefert.
    return: TRUE: Daten vorhanden
\*******************************************************************/
BOOL Query_DCFDateTime (PDCF77_DATETIME pDateTimeDCF)
    {
    ULONG ulcData, ulcParm;
    BYTE  bParmPacket;
    BOOL  bRC;

    if (hDrvr != -1)
        {
        /* Zugriff auf Treiber, falls vorhanden */
        ulcData = ulcParm = bParmPacket = 0;
        bRC = DosDevIOCtl (hDrvr, IOCTL_DCF77, DCF77_GETDATETIME,
            &bParmPacket, sizeof (bParmPacket),    &ulcParm,
            pDateTimeDCF, sizeof (DCF77_DATETIME), &ulcData) ? FALSE : TRUE;
        }
    else if (pShMem && (pShMem->flProgs & (PROG_CLIENT | PROG_DRIVER)))
        {
        /* Zugriff auf LAN::Time Client oder Detached-Programm */
        memcpy (pDateTimeDCF, &(pShMem->dcf77Data.dcf77DateTime), sizeof (DCF77_DATETIME));
        bRC = TRUE;
        }
    else
        /* keine Datenquelle gefunden */
        bRC = FALSE;

    return bRC;
    }

/*******************************************************************\
    Set_DCFData: Low level Routine zum Setzen der Treiberdaten.
    Ist ein SYS-Treiber verfÅgbar, wird dieser geschrieben.
    Ist ein CLIENT oder DRIVER verfÅgbar, wird dieser geschrieben.
    Sonst wird FALSE zurÅckgeliefert.
    return: TRUE: Daten geschrieben
\*******************************************************************/
BOOL Set_DCFData (PDCF77_DATA pDataDCF)
    {
    ULONG ulcData, ulcParm;
    BYTE  bParmPacket;
    HEV   hevServer;
    BOOL  bRC;

    if (hDrvr != -1)
        {
        /* Zugriff auf Treiber, falls vorhanden */
        ulcData = sizeof (DCF77_DATA);
        ulcParm = bParmPacket = 0;
        bRC = DosDevIOCtl (hDrvr, IOCTL_DCF77, DCF77_SETDATA,
            &bParmPacket, sizeof (bParmPacket), &ulcParm,
            pDataDCF,     sizeof (DCF77_DATA),  &ulcData) ? FALSE : TRUE;
        }
    else if (isDriver ())
        {
        /* Zugriff auf Detached Programm */
        memcpy (&(pShMem->dcf77Data.dcf77Data), pDataDCF, sizeof (DCF77_DATA));

        /* énderung an Treiber Åber Semaphore mitteilen */
        if (DosOpenEventSem (SEMNAME, &hevServer) == 0)
            {
            DosPostEventSem (hevServer);
            DosCloseEventSem (hevServer);
            }

        bRC = TRUE;
        }
    else
        /* keine Datensenke gefunden */
        bRC = FALSE;

    return bRC;
    }

/*******************************************************************\
    Set_DCFOffset: Low level Routine zum Setzen des Zeitoffset
    Ist ein SYS-Treiber verfÅgbar, wird dieser geschrieben.
    Ist ein CLIENT oder DRIVER verfÅgbar, wird dieser geschrieben.
    Sonst wird FALSE zurÅckgeliefert.
    return: TRUE: Daten geschrieben
\*******************************************************************/
BOOL Set_DCFOffset (PDCF77_OFFSET pOffsDCF)
    {
    ULONG ulcData, ulcParm;
    BYTE  bParmPacket;
    HEV   hevServer;
    BOOL  bRC;

    if (hDrvr != -1)
        {
        /* Zugriff auf Treiber, falls vorhanden */
        ulcData = sizeof (DCF77_OFFSET);
        ulcParm = bParmPacket = 0;
        bRC = DosDevIOCtl (hDrvr, IOCTL_DCF77, DCF77_SETOFFSET,
            &bParmPacket, sizeof (bParmPacket),  &ulcParm,
            pOffsDCF,     sizeof (DCF77_OFFSET), &ulcData) ? FALSE : TRUE;
        }
    else if (isAnySource ())
        {
        /* Zugriff auf Detached Programm */
        memcpy (&(pShMem->dcf77Data.dcf77Offset), pOffsDCF, sizeof (DCF77_OFFSET));

        /* énderung an Treiber Åber Semaphore mitteilen */
        if (DosOpenEventSem (SEMNAME, &hevServer) == 0)
            {
            DosPostEventSem (hevServer);
            DosCloseEventSem (hevServer);
            }

        bRC = TRUE;
        }
    else
        /* keine Datensenke gefunden */
        bRC = FALSE;

    return bRC;
    }

/*******************************************************************\
    GetDCFStatus: Bestimmen des Zustandes des DCF-Empfanges.
    Es wird eine statische Struktur StatDCF gefÅllt,
    der flStatus-Wert als return-Wert Åbergeben.
    Der Treiber wird im Unterprogramm Get_DCFStatus ÅberprÅft,
    um énderungen von isDriver () zu erfassen.
    return: flStatus-Wert
\*******************************************************************/
USHORT GetDCFStatus (VOID)
    {
    if (ulStatusTStamp != ulTimerCntr || (StatDCF.cVerMajor | StatDCF.cVerMinor) == 0)
        {
        ulStatusTStamp = ulTimerCntr;

        if (Get_DCFStatus (&StatDCF))
            {
            DebugULd (D_IO, "GetDCFStatus", "DCF77_GETSTATUS, cVerMajor",  StatDCF.cVerMajor);
            DebugULd (D_IO, "GetDCFStatus", "DCF77_GETSTATUS, cVerMinor",  StatDCF.cVerMinor);
            DebugULd (D_IO, "GetDCFStatus", "DCF77_GETSTATUS, bPolarity",  StatDCF.bPolarity);
            DebugULx (D_IO, "GetDCFStatus", "DCF77_GETSTATUS, flStatus",   StatDCF.flStatus);
            DebugULx (D_IO, "GetDCFStatus", "DCF77_GETSTATUS, uscBadSec",  StatDCF.uscBadSeconds);
            DebugULx (D_IO, "GetDCFStatus", "DCF77_GETSTATUS, uscGoodSec", StatDCF.uscGoodSeconds);
            DebugULx (D_IO, "GetDCFStatus", "DCF77_GETSTATUS, uscTimeSet", StatDCF.uscTimeSet);
            }
        else
            memset (&StatDCF, '\0', sizeof (DCF77_STATUS));
        }

    return StatDCF.flStatus;
    }

/*******************************************************************\
    GetDCFData: Lesen der DCF77_DATA-Struktur
    Es wird eine statische Struktur DataDCF gefÅllt,
    return: TRUE:  Daten gelesen
            FALSE: Fehler aufgetreten
\*******************************************************************/
BOOL GetDCFData (VOID)
    {
    BOOL bRC = TRUE;

    if (ulDataTStamp != ulTimerCntr)        // Anlauf???; evtl. die Steuervariable ulGetDCFData verwenden
        {
        ulDataTStamp = ulTimerCntr;

        if (ulGetDCFData == GET_OK)
            {
            bRC = Query_DCFData (&DataDCF);
            if (bRC)
                {
                DebugULx (D_IO, "GetDCFData", "DCF77_GETDATA, bSupply",       DataDCF.bSupply);
                DebugULx (D_IO, "GetDCFData", "DCF77_GETDATA, bSignal",       DataDCF.bSignal);
                DebugULx (D_IO, "GetDCFData", "DCF77_GETDATA, bPort",         DataDCF.bPort);
                DebugULd (D_IO, "GetDCFData", "DCF77_GETDATA, bcRepeat",      DataDCF.bcRepeat);
                DebugULx (D_IO, "GetDCFData", "DCF77_GETDATA, usIOAddr",      DataDCF.usIOAddr);
                DebugULd (D_IO, "GetDCFData", "DCF77_GETDATA, usSetInterval", DataDCF.usSetInterval);
                DebugULd (D_IO, "GetDCFData", "DCF77_GETDATA, usThreshold",   DataDCF.usThreshold);
                DebugULd (D_IO, "GetDCFData", "DCF77_GETDATA, usTicklen",     DataDCF.usTicklen);
                DebugULd (D_IO, "GetDCFData", "DCF77_GETDATA, usTimerMode",   DataDCF.usTimerMode);
                }
            else
                memset (&DataDCF, '\0', sizeof (DCF77_DATA));
            }
        }

    return bRC;
    }

/*******************************************************************\
    GetDCFOffs: Lesen der DCF77_OFFSET-Struktur
    Es wird eine statische Struktur OffsDCF gefÅllt,
    return: TRUE:  Daten gelesen
            FALSE: Fehler aufgetreten
\*******************************************************************/
BOOL GetDCFOffs (VOID)
    {
    BOOL bRC = TRUE;

    if (ulOffsTStamp != ulTimerCntr)        // Anlauf???
        {
        ulOffsTStamp = ulTimerCntr;

        bRC = Query_DCFOffset (&OffsDCF);
        if (!bRC)
            memset (&OffsDCF, '\0', sizeof (DCF77_OFFSET));
        }

    return bRC;
    }

/*******************************************************************\
    GetDCFDateTime: Treiberinterne Zeit abholen
    Eingang: somThis: Zeiger auf Instanzdaten
\*******************************************************************/
BOOL GetDCFDateTime (VOID)
    {
    BOOL bRC = TRUE;

    if (ulTimeTStamp != ulTimerCntr)        // Anlauf???
        {
        ulOffsTStamp = ulTimerCntr;

        DebugE (D_WND, "GetDCFDateTime", "Beginn");
        bRC = Query_DCFDateTime (&DtDCF);
        if (!bRC)
            {
            memset (&DtDCF, '\0', sizeof (DCF77_DATETIME));
            StatDCF.flStatus &= ~STATUS_TIMEVALID;
            }
        }

    return bRC;
    }

/*******************************************************************\
    GetGPSCoord: Bestimmen der Koordinaten bei GPS-EmpfÑngern vom
    Typ hopf 6038.
    Es wird eine statische Struktur CoordGPS gefÅllt,
    Der Treiber wird im Unterprogramm Get_DCFStatus ÅberprÅft,
    um énderungen von isDriver () zu erfassen.
    return: flStatus-Wert
\*******************************************************************/
USHORT GetGPSCoord (VOID)
    {
    if (ulCoordTStamp != ulTimerCntr)
        {
        ulCoordTStamp = ulTimerCntr;

        if (Get_GPSCoord (&CoordGPS))
            {
            DebugULd (D_IO, "GetGPSCoord", "DCF77_COORD, Breite",  CoordGPS.Breite);
            DebugULd (D_IO, "GetGPSCoord", "DCF77_COORD, Laenge",  CoordGPS.Laenge);
            }
        else
            memset (&CoordGPS, 0xFF, sizeof (DCF77_COORD));
        }

    return StatDCF.flStatus;
    }

