/***************************************************************************\
 *
 * PROGRAM NAME: WPDCF77
 * -------------
 *
 * VERSION: 6.0
 * --------
 *
 * MODULE NAME: DIALOGS
 * ------------
 *
 * DESCRIPTION:
 * ------------
 *   Responsible for the settings pages to set up the appearance
 *   of the clock
 *
 * NOTES:
 * ------
 *
 * FUNCTIONS:
 * -----------
 *   ClkTicksDlgProc ()
 *   ClkColorsDlgProc ()
 *   AboutDlgProc ()
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
#define INCL_WININPUT
#define INCL_WINFRAMEMGR
#define INCL_WINTRACKRECT
#define INCL_WINMENUS
#define INCL_WINSYS
#define INCL_WINMESSAGEMGR
#define INCL_WINDIALOGS
#define INCL_WINBUTTONS
#define INCL_WINENTRYFIELDS
#define INCL_WINWINDOWMGR
#define INCL_WINSCROLLBARS
#define INCL_WINSTDFONT
#define INCL_WINSTDFILE
#define INCL_WINWINDOWMGR
#define INCL_WINWORKPLACE
#define INCL_WINMLE
#define INCL_GPITRANSFORMS
#define INCL_GPIPRIMITIVES
#define INCL_GPILOGCOLORTABLE
#define INCL_DOSSEMAPHORES
#define INCL_DOSDATETIME

#include <os2.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dcfioctl.h"

#pragma info(noeff)         // suppress SOM compiler informationals

#include "wpdcf77.ih"
#include "wppalet.h"
#include "wpclrpal.h"
#include "wpfolder.h"
#include "wpdcf.h"

#include "clock.h"
#include "res.h"
#include "clockpan.h"
#include "clkdata.h"
#include "clkmem.h"
#include "bitmap.h"
#include "entryfield.h"
#include "dialogs.h"
#include "wndproc.h"
#include "dcf77.h"
#include "debug.h"

/*--------------------------------------------------------------*\
 *  Globale Definitionen und Variable fÅr dieses Modul          *
\*--------------------------------------------------------------*/
typedef struct _RGBCELL
    {
    CELL    Cell;
    BYTE    bRed;
    BYTE    bGreen;
    BYTE    bBlue;
    } RGBCELL;

#define SZ_FONTSIZE         "20"

/* kann global bleiben, da diese Adresse im System konstant bleibt */
PFNWP pOldParentWndProc;

/*******************************************************************\
    FindColorPaletteObject: WPObject instance pointer zum
                            Farbpalettenobject suchen
    Return:  WPObject * instance pointer
\*******************************************************************/
OBJECT FindColorPaletteObject (VOID)
    {
    // Die Standard OS/2-Installation enthÑlt 2 WPColorPalette Instanzen:
    // Eine im Template-Ordner, eine im Konfigurations-Ordner
    // Es werden beide gesucht, falls eine versehentlich gelîscht wurde.
    static PSZ pszObject[] = {"<WP_HIRESCLRPAL>",
                              "<256Color_Template>",
                              NULL};
    ULONG   ulc;
    HOBJECT hObject;
    OBJECT  pObject;

    for (ulc = 0; pszObject[ulc] != NULL; ulc++)
        {
        hObject = WinQueryObject (pszObject[ulc]);
        if (hObject != NULLHANDLE)
            break;
        }

    pObject = _wpclsObjectFromHandle (_WPColorPalette, hObject);
    DebugULx (D_DLG, "FindObject", "handle is", pObject);

    return pObject;
    }

/*******************************************************************\
    ClkTickDlgProc: Settings-Seite fÅr Uhrdarstellung
    Eingang: hwnd: Window-Handle
             msg:  Message-Typ
             mp1:  1. Parameter
             mp2:  2. Parameter
\*******************************************************************/
MRESULT EXPENTRY ClkTicksDlgProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
    {
    WPDCF77      *somSelf;
    WPDCF77Data  *somThis;

    switch (msg)
        {
        case WM_INITDLG:
            somSelf = (WPDCF77 *)mp2;
            somThis = WPDCF77GetData (somSelf);
            WinSetWindowPtr (hwnd, QWL_USER, (PVOID)somSelf);
            _pIData->bWasInitButtonsP = FALSE;    /* siehe WM_ADJUSTWINDOWPOS */
            _pIData->ulOldPresence    = _ulPresence;
            return (MRESULT)TRUE;

        /* Die WinDefDlgProc setzt vor dem sichtbarmachen der Dialogbox */
        /* den Fokus auf den ersten Button und checked ihn vorher.      */
        /* Dies wird hier rÅckgÑngig gemacht: dieser Aufruf macht die   */
        /* Dialogbox sichtbar.                                          */
        case WM_ADJUSTWINDOWPOS:
            somSelf = (WPDCF77 *)WinQueryWindowPtr (hwnd, QWL_USER);
            somThis = WPDCF77GetData (somSelf);
            if ((((PSWP)mp1)->fl & SWP_SHOW) && !_pIData->bWasInitButtonsP)
                WinSendMsg (hwnd, WM_COMPLETEINITDLG, 0, 0);
            break;

        /* mp1: DID_CANCEL   => alte Parameter laden    */
        /*      DID_STANDARD => Standardparameter laden */
        /*      0            => _ulPresence gilt        */
        case WM_COMPLETEINITDLG:
            somSelf = (WPDCF77 *)WinQueryWindowPtr (hwnd, QWL_USER);
            somThis = WPDCF77GetData (somSelf);
            switch (SHORT1FROMMP (mp1))
                {
                case DID_CANCEL:
                    _ulPresence = _pIData->ulOldPresence;
                    break;

                case DID_STANDARD:
                    _ulPresence = PRESENCE_DEFAULT;
                    break;
                }
            _ulPresence = SetSettings1Buttons (hwnd, _ulPresence);
            _pIData->bWasInitButtonsP = TRUE;
            return 0;

        case WM_COMMAND:
            if (SHORT1FROMMP (mp2) == CMDSRC_PUSHBUTTON)
                {
                switch (SHORT1FROMMP (mp1))
                    {
                    case DID_STANDARD:
                    case DID_CANCEL:
                        WinSendMsg (hwnd, WM_COMPLETEINITDLG,
                            MPFROMSHORT (SHORT1FROMMP (mp1)), 0);
                        break;
                    }
                }
            return 0;

        case WM_CONTROL:
            somSelf = (WPDCF77 *)WinQueryWindowPtr (hwnd, QWL_USER);
            somThis = WPDCF77GetData (somSelf);
            if ((SHORT2FROMMP (mp1) == BN_CLICKED) && (_pIData->bWasInitButtonsP == TRUE))
                {
                switch (SHORT1FROMMP (mp1))
                    {
                    case DID_SHW_TIMEONLY:
                        _ulPresence &= ~PRESENCE_DISP_DATE;
                        _ulPresence |= PRESENCE_DISP_TIME;
                        WinEnableControl (hwnd, DID_SHW_ANALOG, TRUE);
                        break;

                    case DID_SHW_DATEONLY:
                        _ulPresence &= ~(PRESENCE_DISP_TIME | PRESENCE_DISP_ANALOG);
                        _ulPresence |= PRESENCE_DISP_DATE;
                        WinEnableControl (hwnd, DID_SHW_ANALOG, FALSE);
                        break;

                    case DID_SHW_DATETIME:
                        _ulPresence |= PRESENCE_DISP_DATE | PRESENCE_DISP_TIME;
                        WinEnableControl (hwnd, DID_SHW_ANALOG, TRUE);
                        break;

                    case DID_SHW_DIGITAL:
                        _ulPresence &= ~PRESENCE_DISP_ANALOG;
                        _ulPresence |= PRESENCE_DISP_DIGITAL;
                        WinEnableControl (hwnd, DID_SHW_DATEONLY,   TRUE);
                        WinEnableControl (hwnd, DID_SHW_SECHAND,   FALSE);
                        WinEnableControl (hwnd, DID_SHW_HOURTICKS, FALSE);
                        WinEnableControl (hwnd, DID_SHW_MINTICKS,  FALSE);
                        WinEnableControl (hwnd, DID_SHW_BAVARIAN,  FALSE);
                        break;

                    case DID_SHW_ANALOG:
                        _ulPresence &= ~PRESENCE_DISP_DIGITAL;
                        _ulPresence |= PRESENCE_DISP_ANALOG;
                        WinEnableControl (hwnd, DID_SHW_DATEONLY, FALSE);
                        WinEnableControl (hwnd, DID_SHW_SECHAND,   TRUE);
                        WinEnableControl (hwnd, DID_SHW_HOURTICKS, TRUE);
                        WinEnableControl (hwnd, DID_SHW_MINTICKS,  TRUE);
                        WinEnableControl (hwnd, DID_SHW_BAVARIAN,  TRUE);
                        break;

                    case DID_SHW_SECHAND:
                        if (WinQueryButtonCheckstate (hwnd, DID_SHW_SECHAND))
                            _ulPresence |= PRESENCE_DISP_SEC_HAND;
                        else
                            _ulPresence &= ~PRESENCE_DISP_SEC_HAND;
                        break;

                    case DID_SHW_HOURTICKS:
                        if (WinQueryButtonCheckstate (hwnd, DID_SHW_HOURTICKS))
                            _ulPresence |= PRESENCE_MAJORTICKS;
                        else
                            _ulPresence &= ~PRESENCE_MAJORTICKS;
                        break;

                    case DID_SHW_MINTICKS:
                        if (WinQueryButtonCheckstate (hwnd, DID_SHW_MINTICKS))
                            _ulPresence |= PRESENCE_MINORTICKS;
                        else
                            _ulPresence &= ~PRESENCE_MINORTICKS;
                        break;

                    case DID_SHW_TITLEBAR:
                        if (WinQueryButtonCheckstate (hwnd, DID_SHW_TITLEBAR))
                            _ulPresence |= PRESENCE_TITLEBAR;
                        else
                            _ulPresence &= ~PRESENCE_TITLEBAR;
                        ClkShowFrameControls (_hwndFrame,
                            _ulPresence & PRESENCE_TITLEBAR ? TRUE : FALSE);
                        break;

                    case DID_SHW_BAVARIAN:
                        if (WinQueryButtonCheckstate (hwnd, DID_SHW_BAVARIAN))
                            _ulPresence |= PRESENCE_BAVARIAN;
                        else
                            _ulPresence &= ~PRESENCE_BAVARIAN;
                        break;

                    case DID_SAVE_ICONPOS:
                        if (WinQueryButtonCheckstate (hwnd, DID_SAVE_ICONPOS))
                            _ulPresence |= PRESENCE_MINMAXPOS;
                        else
                            _ulPresence &= ~PRESENCE_MINMAXPOS;
                    }

                _wpRefreshClockView (somSelf);
                return 0;
                }
            break;

        case WM_DESTROY:
            WinSendMsg (hwnd, WM_CLOSE, 0, 0);
            return 0;

        case WM_CLOSE:
            somSelf = (WPDCF77 *)WinQueryWindowPtr (hwnd, QWL_USER);
            _wpSaveDeferred (somSelf);
            break;
        }

    return WinDefDlgProc (hwnd, msg, mp1, mp2);
    }

/*******************************************************************\
    LoadBitmapFiles: LÑdt die Bitmap-Dateien fÅr Zeiger und
                     Ziffernblatt und erzeugt jeweils ein HBITMAP
    Eingang: somThis:       Zeiger auf Instanzdaten
             bDisplayError: TRUE: Fehlermeldung anzeigen
                            bei BMP-Fehler
                            FALSE: Fehlermeldung unterdrÅcken
\*******************************************************************/
VOID LoadBitmapFiles (WPDCF77Data *somThis, BOOL bDisplayError)
    {
    ULONG ulc, ul[3];
    PSZ   psz[3];

    DebugE (D_WND, "LoadBitmapFiles, file minute hand", _szPathMHandBmp);
    DebugE (D_WND, "LoadBitmapFiles, file hour   hand", _szPathHHandBmp);
    DebugE (D_WND, "LoadBitmapFiles, file face       ", _szPathFaceBmp);
    if (_pIData->hBmpMHand)
        GpiDeleteBitmap (_pIData->hBmpMHand);
    if (_pIData->hBmpHHand)
        GpiDeleteBitmap (_pIData->hBmpHHand);
    if (_pIData->hBmpFace)
        GpiDeleteBitmap (_pIData->hBmpFace);

    psz[0] = _szPathMHandBmp;
    psz[1] = _szPathHHandBmp;
    psz[2] = _szPathFaceBmp;
    ul[0] = createBitmap (_hpsBuffer, _szPathMHandBmp, &_pIData->hBmpMHand, &_pIData->ptlMHandHotspot);
    ul[1] = createBitmap (_hpsBuffer, _szPathHHandBmp, &_pIData->hBmpHHand, &_pIData->ptlHHandHotspot);
    ul[2] = createBitmap (_hpsBuffer, _szPathFaceBmp,  &_pIData->hBmpFace,  NULL);

    /* Fehlerbehandlung: Messagebox */
    if (bDisplayError && (ul[0] || ul[1] || ul[2]))
        {
        PSZ pMessage;

        pMessage = allocMem (2 * CCHMAXMSG + 3 * CCHMAXPATH);
        if (pMessage)
            {
            DebugS (D_DLG, "Fehler beim ôffnen von BMP-Files");
            pMessage[0] = '\0';
            if ((ul[0] == BMPERR_OPENERR) || (ul[1] == BMPERR_OPENERR) || ul[2] == (BMPERR_OPENERR))
                {
                WinLoadMessage (hab, hmod, IDMSG_ERRFILENOTFOUND, CCHMAXMSG, pMessage);
                for (ulc = 0; ulc < 3; ulc++)
                    if (ul[ulc] == BMPERR_OPENERR)
                    {
                    strcat (pMessage, psz[ulc]);
                    strcat (pMessage, "\n");
                    }
                strcat (pMessage, "\n");
                DebugE (D_DLG, "Fehlermeldung: ", pMessage);
                }
            if (ul[0] == BMPERR_INVBMP || ul[1] == BMPERR_INVBMP || ul[2] == BMPERR_INVBMP)
                {
                WinLoadMessage (hab, hmod, IDMSG_ERRBMPFILE, CCHMAXMSG, pMessage + strlen (pMessage));
                for (ulc = 0; ulc < 3; ulc++)
                    if (ul[ulc] == BMPERR_INVBMP)
                    {
                    strcat (pMessage, psz[ulc]);
                    strcat (pMessage, "\n");
                    }
                DebugE (D_DLG, "Fehlermeldung: ", pMessage);
                }
            MessageBox (_hwndClient, 0,  MBTITLE_ERROR, MB_OK | MB_ERROR, WA_ERROR, pMessage);
            freeMem ((PPVOID)&pMessage);
            }
        }

    DebugULx (D_WND, "LoadBitmapFiles, exit", "HBITMAP minute hand", (ULONG)_pIData->hBmpMHand);
    DebugULx (D_WND, "LoadBitmapFiles, exit", "HBITMAP hour   hand", (ULONG)_pIData->hBmpHHand);
    DebugULx (D_WND, "LoadBitmapFiles, exit", "HBITMAP face       ", (ULONG)_pIData->hBmpFace);

    return;
    }

/*******************************************************************\
    BrowseFile: ôffnen des File-Dialogs.
    Eingang: hwnd:       Window-Handle der Dialog-Prozedur
             pszText:    Start-Pfad und -Datei
    Ausgang: pszText:    neue Datei mit absolutem Pfad
\*******************************************************************/
LONG BrowseFile (HWND hwnd, PCHAR pszText)
    {
    FILEDLG FileDlg;
    CHAR    szTitle[CCHMAXPGNAME];
    ULONG ul;

    DebugE (D_DRAG, "BrowseFile", "begin");
    WinLoadString (hab, hmod, IDS_BITMAP_SEARCH_FILE, CCHMAXPGNAME, szTitle);
    FileDlg.cbSize          = sizeof (FILEDLG);
    FileDlg.fl              = FDS_OPEN_DIALOG | FDS_CENTER | FDS_HELPBUTTON;
    FileDlg.ulUser          = 0;
    FileDlg.pszTitle        = szTitle;
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
    strcpy (FileDlg.szFullFile, pszText);

    DebugE (D_DRAG, "BrowseFile", "end");
    ul = WinFileDlg (HWND_DESKTOP, hwnd, &FileDlg);
    DebugULx (D_DRAG, "BrowseFile", "return", ul);
    if (FileDlg.lReturn == DID_OK)
        strcpy (pszText, FileDlg.szFullFile);

    return FileDlg.lReturn;
    }

/*******************************************************************\
    SetSettings1Buttons: Einstellung der Buttons der ersten
                         Settings-Page
    Eingang: hwnd:       Window-Handle der Dialog-Prozedur
             ulPresence: Darstellungs-Flags (siehe _ulPresence)
\*******************************************************************/
ULONG SetSettings1Buttons (HWND hwnd, ULONG ulPresence)
    {
    ULONG ulId;

    if (ulPresence & PRESENCE_DISP_SEC_HAND)
        WinCheckButton (hwnd, DID_SHW_SECHAND, TRUE);
    if (ulPresence & PRESENCE_MINORTICKS)
        WinCheckButton (hwnd, DID_SHW_MINTICKS, TRUE);
    if (ulPresence & PRESENCE_MAJORTICKS)
        WinCheckButton (hwnd, DID_SHW_HOURTICKS, TRUE);
    if (ulPresence & PRESENCE_BAVARIAN)
        WinCheckButton (hwnd, DID_SHW_BAVARIAN, TRUE);
    if (ulPresence & PRESENCE_MINMAXPOS)
        WinCheckButton (hwnd, DID_SAVE_ICONPOS, TRUE);
    if (ulPresence & PRESENCE_DISP_ANALOG)
        {
        /* Analoganzeige */
        switch (ulPresence & (PRESENCE_DISP_TIME | PRESENCE_DISP_DATE))
            {
            /* Nur Uhrzeit zeigen */
            case  PRESENCE_DISP_TIME:
                ulId = DID_SHW_TIMEONLY;
                break;

            /* Bei Analoguhr verboten */
            case  PRESENCE_DISP_DATE:
                ulPresence |= PRESENCE_DISP_TIME;
                ulId = DID_SHW_DATETIME;
                break;

            /* Uhrzeit/Datum zeigen */
            case  PRESENCE_DISP_TIME | PRESENCE_DISP_DATE:
                ulId = DID_SHW_DATETIME;
                break;
            }
        WinEnableControl (hwnd, DID_SHW_DATEONLY, FALSE);
        WinCheckButton (hwnd, ulId, TRUE);
        WinCheckButton (hwnd, DID_SHW_ANALOG, TRUE);
        }
    else
        {
        /* Digitalanzeige */
        switch (ulPresence & (PRESENCE_DISP_TIME | PRESENCE_DISP_DATE))
            {
            /* Nur Uhrzeit zeigen */
            case  PRESENCE_DISP_TIME:
                ulId = DID_SHW_TIMEONLY;
                break;

            /* Nur Datum zeigen */
            case  PRESENCE_DISP_DATE:
                WinEnableControl (hwnd, DID_SHW_ANALOG, FALSE);
                ulId = DID_SHW_DATEONLY;
                break;

            /* Uhrzeit/Datum zeigen */
            case  PRESENCE_DISP_TIME | PRESENCE_DISP_DATE:
                ulId = DID_SHW_DATETIME;
                break;
            }
        WinCheckButton (hwnd, ulId, TRUE);
        WinCheckButton (hwnd, DID_SHW_DIGITAL, TRUE);
        WinEnableControl (hwnd, DID_SHW_SECHAND,   FALSE);
        WinEnableControl (hwnd, DID_SHW_HOURTICKS, FALSE);
        WinEnableControl (hwnd, DID_SHW_MINTICKS,  FALSE);
        WinEnableControl (hwnd, DID_SHW_BAVARIAN,  FALSE);
        }

    if (ulPresence & PRESENCE_TITLEBAR)
        WinCheckButton (hwnd, DID_SHW_TITLEBAR, TRUE);

    return ulPresence;
    }

/*******************************************************************\
    IsOtherClockActive: Untersucht, ob bereits eine andere Uhr fÅr
                        die Ausgabe des Stundengongs aktiviert ist
    Eingang: hObject:   SOM-Object-Handle des aktivierten Objekts
    return:  TRUE:      anderes Objekt ist aktiviert
             FALSE:     Stundengong im System nicht aktiviert
\*******************************************************************/
BOOL IsOtherClockActive (HOBJECT hObject)
    {
    if (hClsRingingObject == hObject || hClsRingingObject == NULLHANDLE)
        return FALSE;
    else
        return TRUE;
    }

/*******************************************************************\
    IsClockWndChanged:  Der Zustand fÅr die Stundengong-Ausgabe des
                        aktuellen Fensters hat sich geÑndert
    Eingang: hwnd:      Window-Handle der Dialog-Prozedur
             hObject:   SOM-Object-Handle des aktivierten Objekts
    return:  TRUE:      Zustand hat sich geÑndert
             FALSE:     keine énderung
\*******************************************************************/
BOOL IsClockWndChanged (HWND hwnd, HOBJECT hObject)
    {
    if (!IsOtherClockActive (hObject) && !WinIsControlEnabled (hwnd, DID_GONG_PROG))
        return TRUE;

    if (IsOtherClockActive (hObject) && WinIsControlEnabled (hwnd, DID_GONG_PROG))
        return TRUE;

    return FALSE;
    }

/*******************************************************************\
    CheckAudioObjectDeleted: PrÅft, ob das Objekt, fÅr das die
                             Stundengong-Ausgabe aktiviert war,
                             gelîscht wurde; falls ja, werden
                             die Klassendaten fÅr Audio gelîscht.
    Eingang: somSelf:      Self-Zeiger auf aktuelles Objekt
             hObject:      SOM-Object-Handle des aktivierten Objekts
\*******************************************************************/
VOID CheckAudioObjectDeleted (WPDCF77 *somSelf, HOBJECT hObject)
    {
    M_WPDCF77 *somClass;
    WPDCF77   *somObject;

    if (IsOtherClockActive (hObject))
        {
        somClass = SOM_GetClass (somSelf);
        somObject = _wpclsObjectFromHandle (somClass, hClsRingingObject);
        if (somObject == NULL)
            {
            hClsRingingObject = NULLHANDLE;
            szClsGongFile[0] = '\0';
            bClsIs1PerHour   = FALSE;
            }
        }

    return;
    }

/*******************************************************************\
    SetAudioControls: Stellt die Controls im Dialog "ClkAudioDlgProc"
                      ein. Das globale HOBJECT hClsRingingObject
                      wird eingestellt, wenn das aktuelle oder kein
                      Objekt fÅr Stundengong-Ausgabe aktiviert ist.
                      (Freigabe des HOBJECT, falls Eingabefeld
                      ohne Inhalt ist).
    Eingang: hwnd:         Window-Handle der Dialog-Prozedur
             somThis:      Zeiger auf Instanzdaten
             hObject:      SOM-Object-Handle des aktivierten Objekts
             bForceUpdate: Einstellungen an den Controls werden
                           gemacht ohne PrÅfung, ob notwendig
\*******************************************************************/
VOID SetAudioControls (HWND hwnd, WPDCF77Data *somThis, HOBJECT hObject, BOOL bForceUpdate)
    {
    CHAR szMsg[CCHMAXMSG];

    DebugULx (D_WND, "SetAudioControls, WM_INITDLG", " hObject", (ULONG)hObject);

    if (bForceUpdate || IsClockWndChanged (hwnd, hObject))
        {
        if (IsOtherClockActive (hObject))
            {
            WinLoadString (hab, hmod, IDS_GONG_TEXT, CCHMAXMSG, szMsg);
            WinSetDlgItemText (hwnd, DID_GONG_TEXT, szMsg);

            /* Text nur lîschen, wenn Eingabefeld Text enthÑlt, um eine */
            /* Rekursion durch EN_CHANGED-Messages zu verhindern        */
            if (WinQueryDlgItemTextLength (hwnd, DID_GONG_PROG))
                WinSetDlgItemText (hwnd, DID_GONG_PROG, "");

            WinEnableControl (hwnd, DID_GONG_PROG, FALSE);
            WinEnableControl (hwnd, DID_GONG_SRCH, FALSE);
            WinEnableControl (hwnd, DID_GONG_MD1,  FALSE);
            WinEnableControl (hwnd, DID_GONG_MDN,  FALSE);
            WinEnableControl (hwnd, DID_STANDARD,  FALSE);
            WinEnableControl (hwnd, DID_CANCEL,    FALSE);

            _pIData->szClsGongFile[0] = '\0';
            _pIData->bClsIs1PerHour   = FALSE;
            }
        else
            {
            hClsRingingObject = hObject;
            WinSetDlgItemText (hwnd, DID_GONG_TEXT, "");

            WinEnableControl (hwnd, DID_GONG_PROG, TRUE);
            WinEnableControl (hwnd, DID_GONG_SRCH, TRUE);
            WinEnableControl (hwnd, DID_GONG_MD1,  TRUE);
            WinEnableControl (hwnd, DID_GONG_MDN,  TRUE);
            WinEnableControl (hwnd, DID_STANDARD,  TRUE);
            WinEnableControl (hwnd, DID_CANCEL,    TRUE);

            WinSetDlgItemText (hwnd, DID_GONG_PROG, szClsGongFile);
            WinCheckButton (hwnd, bClsIs1PerHour ? DID_GONG_MD1 : DID_GONG_MDN, TRUE);

            strcpy (_pIData->szClsGongFile, szClsGongFile);
            _pIData->bClsIs1PerHour = bClsIs1PerHour;
            }
        }

    if (!IsOtherClockActive (hObject))
        if (szClsGongFile[0] == '\0')
            hClsRingingObject = NULLHANDLE;
        else
            hClsRingingObject = hObject;

    return;
    }

/*******************************************************************\
    ClkColorsDlgProc: Settings-Page fÅr die Farb-/Fonteinstellung
    Eingang: hwnd: Window-Handle
             msg:  Message-Typ
             mp1:  1. Parameter
             mp2:  2. Parameter
\*******************************************************************/
MRESULT EXPENTRY ClkColorsDlgProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
    {
    WPDCF77       *somSelf;
    WPDCF77Data   *somThis;
    RGBCELL       Cell;
    FONTDLG       Font;
    ULONG         ulTmpColor;
    USHORT        usActualBtnFont;

    switch (msg)
        {
        case WM_INITDLG :
            somSelf = (WPDCF77 *)mp2;
            somThis = WPDCF77GetData (somSelf);
            WinSetWindowPtr (hwnd, QWL_USER, (PVOID)somSelf);
            _pIData->bWasInitButtonsC = FALSE;          // siehe WM_ADJUSTWINDOWPOS
            return (MRESULT)TRUE;

        /* Die WinDefDlgProc setzt vor dem sichtbarmachen der Dialogbox */
        /* den Fokus auf den ersten Button und checked ihn vorher.      */
        /* Dies wird hier rÅckgÑngig gemacht: dieser Aufruf macht die   */
        /* Dialogbox sichtbar.                                          */
        case WM_ADJUSTWINDOWPOS:
            somSelf = (WPDCF77 *)WinQueryWindowPtr (hwnd, QWL_USER);
            somThis = WPDCF77GetData (somSelf);
            if ((((PSWP)mp1)->fl & SWP_SHOW) && !_pIData->bWasInitButtonsC)
                WinSendMsg (hwnd, WM_COMPLETEINITDLG, MPVOID, MPVOID);
            break;

        case WM_COMPLETEINITDLG:
            somSelf = (WPDCF77 *)WinQueryWindowPtr (hwnd, QWL_USER);
            somThis = WPDCF77GetData (somSelf);
            if (_pIData->bWasInitButtonsC == FALSE)
                {
                pOldParentWndProc = WinSubclassWindow (WinQueryWindow (hwnd, QW_PARENT), NewParentWndProc);

                usActualBtnFont = DID_FONT_DATE;
                WinCheckButton (hwnd, usActualBtnFont, TRUE);

                _pIData->clrBackgnd      = _clrBackgnd;          // Originalzustand sichern
                _pIData->clrFace         = _clrFace;
                _pIData->clrTick         = _clrTick;
                _pIData->clrDateOnAnalog = _clrDateOnAnalog;
                _pIData->clrHHand        = _clrHHand;
                _pIData->clrMHand        = _clrMHand;
                _pIData->clrTransparent  = _clrTransparent;
                _pIData->clrDateText     = _clrDateText;
                _pIData->clrTimeText     = _clrTimeText;
                memcpy (&_pIData->fAttrD, &_fAttrD, sizeof (FATTRS));
                memcpy (&_pIData->fAttrT, &_fAttrT, sizeof (FATTRS));
                strcpy (_pIData->szFontFamilyD, _szFontFamilyD);
                strcpy (_pIData->szFontFamilyT, _szFontFamilyT);
                _pIData->bWasInitButtonsC = TRUE;
                }
            return 0;

        case WM_COMMAND:
            if (SHORT1FROMMP (mp2) == CMDSRC_PUSHBUTTON)
                {
                WPObject *pWPColorPalette;

                somSelf = (WPDCF77 *)WinQueryWindowPtr (hwnd, QWL_USER);
                somThis = WPDCF77GetData (somSelf);
                switch (SHORT1FROMMP (mp1))
                    {
                    case DID_CHG_COLOR:
                        if (WinQueryButtonCheckstate (hwnd, DID_COL_FACE))
                            ulTmpColor = _clrFace;
                        else if (WinQueryButtonCheckstate (hwnd, DID_COL_TICK))
                            ulTmpColor = _clrTick;
                        else if (WinQueryButtonCheckstate (hwnd, DID_COL_BACKGND))
                            ulTmpColor = _clrBackgnd;
                        else if (WinQueryButtonCheckstate (hwnd, DID_COL_DATE))
                            ulTmpColor = _clrDateOnAnalog;
                        else if (WinQueryButtonCheckstate (hwnd, DID_COL_HHAND))
                            ulTmpColor = _clrHHand;
                        else if (WinQueryButtonCheckstate (hwnd, DID_COL_MHAND))
                            ulTmpColor = _clrMHand;
                        else if (WinQueryButtonCheckstate (hwnd, DID_COL_TRANSPARENT))
                            ulTmpColor = _clrTransparent;
                        else if (WinQueryButtonCheckstate (hwnd, DID_COL_DATETXT))
                            ulTmpColor = _clrDateText;
                        else if (WinQueryButtonCheckstate (hwnd, DID_COL_TIMETXT))
                            ulTmpColor = _clrTimeText;
                        else return 0;

                        Cell.Cell.cbData = sizeof (RGBCELL);
                        Cell.bRed        = (BYTE) ulTmpColor;
                        Cell.bGreen      = (BYTE)(ulTmpColor>>8);
                        Cell.bBlue       = (BYTE)(ulTmpColor>>16);

                        pWPColorPalette = FindColorPaletteObject ();
                        if (pWPColorPalette)
                            _wpEditCell (pWPColorPalette, (PCELL)(&Cell), WinQueryWindow (hwnd, QW_PARENT));

                        _wpRefreshClockView (somSelf);
                        return 0;

                    case DID_CHG_FONT:
                        memset (&Font, 0, sizeof (FONTDLG));
                        Font.cbSize         = sizeof (FONTDLG);
                        Font.hpsScreen      = WinGetPS (hwnd);
                        Font.pfnDlgProc     = NewFontDlgProc;
                        Font.pszPtSizeList  = SZ_FONTSIZE;
                        Font.usFamilyBufLen = FACESIZE;
                        Font.fl             = FNTS_CENTER | FNTS_VECTORONLY | FNTS_INITFROMFATTRS | FNTS_HELPBUTTON;
                        Font.clrFore        = CLR_BLACK;
                        Font.clrBack        = CLR_WHITE;
                        if (WinQueryButtonCheckstate (hwnd, DID_FONT_DATE))
                            {
                            Font.pszFamilyname  = _szFontFamilyD;
                            memcpy (&Font.fAttrs, &_fAttrD, sizeof (FATTRS));
                            }
                        else
                            {
                            Font.pszFamilyname  = _szFontFamilyT;
                            memcpy (&Font.fAttrs, &_fAttrT, sizeof (FATTRS));
                            }

                        WinFontDlg (HWND_DESKTOP, hwnd, &Font);
                        if (Font.lReturn == DID_OK)
                            {
                            if (WinQueryButtonCheckstate (hwnd, DID_FONT_DATE))
                                {
                                memcpy (&_fAttrD, &Font.fAttrs, sizeof (FATTRS));
                                strcpy (_szFontFamilyD, Font.pszFamilyname);
                                }
                            else
                                {
                                memcpy (&_fAttrT, &Font.fAttrs, sizeof (FATTRS));
                                strcpy (_szFontFamilyT, Font.pszFamilyname);
                                }

                            _wpRefreshClockView (somSelf);
                            }

                        WinReleasePS (Font.hpsScreen);
                        break;

                    case DID_STANDARD:
                        _clrBackgnd      = CLR_DEFAULT_BACKGND; // Defaultfarben setzen
                        _clrFace         = CLR_DEFAULT_FACE;
                        _clrTick         = CLR_DEFAULT_TICK;
                        _clrDateOnAnalog = CLR_DEFAULT_DATEONANALG;
                        _clrHHand        = CLR_DEFAULT_HHAND;
                        _clrMHand        = CLR_DEFAULT_MHAND;
                        _clrTransparent  = CLR_DEFAULT_TRANSPARENT;
                        _clrDateText     = CLR_BLACK;
                        _clrTimeText     = CLR_BLACK;
                        memset (&_fAttrD, 0, sizeof (FATTRS));
                        memset (&_fAttrT, 0, sizeof (FATTRS));
                        _fAttrD.usRecordLength = _fAttrT.usRecordLength = sizeof (FATTRS);
                        _fAttrD.fsFontUse      = _fAttrT.fsFontUse      = FONT_DEFAULT_USE;
                        _fAttrD.fsType         = _fAttrT.fsType         = FONT_DEFAULT_TYPE;
                        strcpy (_fAttrD.szFacename, FONT_DEFAULT_FACE);
                        strcpy (_fAttrT.szFacename, FONT_DEFAULT_FACE);
                        strcpy (_szFontFamilyD, FONT_DEFAULT_FAMILY);
                        strcpy (_szFontFamilyT, FONT_DEFAULT_FAMILY);
                        _wpRefreshClockView (somSelf);
                        return 0;

                    case DID_CANCEL:
                        _clrBackgnd      = _pIData->clrBackgnd;     // Originalzustand restaurieren
                        _clrFace         = _pIData->clrFace;
                        _clrTick         = _pIData->clrTick;
                        _clrDateOnAnalog = _pIData->clrDateOnAnalog;
                        _clrHHand        = _pIData->clrHHand;
                        _clrMHand        = _pIData->clrMHand;
                        _clrTransparent  = _pIData->clrTransparent;
                        _clrDateText     = _pIData->clrDateText;
                        _clrTimeText     = _pIData->clrTimeText;
                        memcpy (&_fAttrD, &_pIData->fAttrD, sizeof (_fAttrD));
                        memcpy (&_fAttrT, &_pIData->fAttrT, sizeof (_fAttrT));
                        strcpy (_szFontFamilyD, _pIData->szFontFamilyD);
                        strcpy (_szFontFamilyT, _pIData->szFontFamilyT);
                        _wpRefreshClockView (somSelf);
                        return 0;
                    }
                }
            return 0;

        /* mp1: RGB-Wert */
        case WM_SETCELL:
            DebugULx (D_DLG, "Client", "WM_SETCELL", (ULONG)mp1);
            somSelf = (WPDCF77 *)WinQueryWindowPtr (hwnd, QWL_USER);
            somThis = WPDCF77GetData (somSelf);
            if (WinQueryButtonCheckstate (hwnd, DID_COL_FACE))
                _clrFace = LONGFROMMP (mp1);
            if (WinQueryButtonCheckstate (hwnd, DID_COL_TICK))
                _clrTick = LONGFROMMP (mp1);
            if (WinQueryButtonCheckstate (hwnd, DID_COL_BACKGND))
                _clrBackgnd = LONGFROMMP (mp1);
            if (WinQueryButtonCheckstate (hwnd, DID_COL_DATE))
                _clrDateOnAnalog = LONGFROMMP (mp1);
            if (WinQueryButtonCheckstate (hwnd, DID_COL_HHAND))
                _clrHHand = LONGFROMMP (mp1);
            if (WinQueryButtonCheckstate (hwnd, DID_COL_MHAND))
                _clrMHand = LONGFROMMP (mp1);
            if (WinQueryButtonCheckstate (hwnd, DID_COL_TRANSPARENT))
                _clrTransparent = LONGFROMMP (mp1);
            if (WinQueryButtonCheckstate (hwnd, DID_COL_DATETXT))
                _clrDateText = LONGFROMMP (mp1);
            if (WinQueryButtonCheckstate (hwnd, DID_COL_TIMETXT))
                _clrTimeText = LONGFROMMP (mp1);
            _wpRefreshClockView (somSelf);
            return 0;

        case WM_DESTROY:
            WinSendMsg (hwnd, WM_CLOSE, 0, 0);
            return 0;

        case WM_CLOSE:
            WinSubclassWindow (WinQueryWindow (hwnd, QW_PARENT), pOldParentWndProc);
            somSelf = (WPDCF77 *)WinQueryWindowPtr (hwnd, QWL_USER);
            _wpSaveDeferred (somSelf);
            break;
        }

    return WinDefDlgProc (hwnd, msg, mp1, mp2);
    }

/*******************************************************************\
    ClkBmpsDlgProc: Settings-Page fÅr die Bitmap-Einstellungen
    Eingang: hwnd: Window-Handle
             msg:  Message-Typ
             mp1:  1. Parameter
             mp2:  2. Parameter
\*******************************************************************/
MRESULT EXPENTRY ClkBmpsDlgProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
    {
    WPDCF77      *somSelf;
    WPDCF77Data  *somThis;

    switch (msg)
        {
        case WM_INITDLG:
            somSelf = (WPDCF77 *)mp2;
            somThis = WPDCF77GetData (somSelf);
            WinSetWindowPtr (hwnd, QWL_USER, (PVOID)somSelf);

            SubclassEntryField (WinWindowFromID (hwnd, DID_SET3_HOUR));
            SubclassEntryField (WinWindowFromID (hwnd, DID_SET3_MINUTE));
            SubclassEntryField (WinWindowFromID (hwnd, DID_SET3_FACE));

            WinSetWindowULong (WinWindowFromID (hwnd, DID_SET3_HOUR),   QWL_USER, DRTYPE_BITMAP);
            WinSetWindowULong (WinWindowFromID (hwnd, DID_SET3_MINUTE), QWL_USER, DRTYPE_BITMAP);
            WinSetWindowULong (WinWindowFromID (hwnd, DID_SET3_FACE),   QWL_USER, DRTYPE_BITMAP);

            /* Kopien der Instanzdaten fÅr Dateipfade erzeugen */
            strcpy (_pIData->szPathMHandBmp, _szPathMHandBmp);
            strcpy (_pIData->szPathHHandBmp, _szPathHHandBmp);
            strcpy (_pIData->szPathFaceBmp,  _szPathFaceBmp );

            WinSendDlgItemMsg (hwnd, DID_SET3_HOUR,   EM_SETTEXTLIMIT, MPFROMSHORT (CCHMAXPATH), MPVOID);
            WinSendDlgItemMsg (hwnd, DID_SET3_MINUTE, EM_SETTEXTLIMIT, MPFROMSHORT (CCHMAXPATH), MPVOID);
            WinSendDlgItemMsg (hwnd, DID_SET3_FACE,   EM_SETTEXTLIMIT, MPFROMSHORT (CCHMAXPATH), MPVOID);

            /* Eingabefelder fÅllen */
            WinSetDlgItemText (hwnd, DID_SET3_HOUR,   _szPathHHandBmp);
            WinSetDlgItemText (hwnd, DID_SET3_MINUTE, _szPathMHandBmp);
            WinSetDlgItemText (hwnd, DID_SET3_FACE,   _szPathFaceBmp);

            return (MRESULT)TRUE;

        case WM_COMMAND:
            somSelf = (WPDCF77 *)WinQueryWindowPtr (hwnd, QWL_USER);
            somThis = WPDCF77GetData (somSelf);
            if (SHORT1FROMMP (mp2) == CMDSRC_PUSHBUTTON)
                {
                PCHAR       pszText;

                switch (SHORT1FROMMP (mp1))
                    {
                    case DID_STANDARD:
                        _szPathHHandBmp[0] =
                        _szPathMHandBmp[0] =
                        _szPathFaceBmp[0]  = '\0';

                        WinSetDlgItemText (hwnd, DID_SET3_HOUR,   _szPathHHandBmp);
                        WinSetDlgItemText (hwnd, DID_SET3_MINUTE, _szPathMHandBmp);
                        WinSetDlgItemText (hwnd, DID_SET3_FACE,   _szPathFaceBmp);
                        break;

                    case DID_CANCEL:
                        strcpy (_szPathHHandBmp, _pIData->szPathHHandBmp);
                        strcpy (_szPathMHandBmp, _pIData->szPathMHandBmp);
                        strcpy (_szPathFaceBmp,  _pIData->szPathFaceBmp );

                        WinSetDlgItemText (hwnd, DID_SET3_HOUR,   _szPathHHandBmp);
                        WinSetDlgItemText (hwnd, DID_SET3_MINUTE, _szPathMHandBmp);
                        WinSetDlgItemText (hwnd, DID_SET3_FACE,   _szPathFaceBmp);
                        break;

                    case DID_SET3_QHOUR:
                        pszText = _szPathHHandBmp;
                        if (BrowseFile (hwnd, pszText) == DID_OK)
                            WinSetDlgItemText (hwnd, DID_SET3_HOUR, _szPathHHandBmp);
                        break;

                    case DID_SET3_QMINUTE:
                        pszText = _szPathMHandBmp;
                        if (BrowseFile (hwnd, pszText) == DID_OK)
                            WinSetDlgItemText (hwnd, DID_SET3_MINUTE, _szPathMHandBmp);
                        break;

                    case DID_SET3_QFACE:
                        pszText = _szPathFaceBmp;
                        if (BrowseFile (hwnd, pszText) == DID_OK)
                            WinSetDlgItemText (hwnd, DID_SET3_FACE, _szPathFaceBmp);
                        break;
                    }
                }
            return 0;

        case WM_DESTROY:
            WinSendMsg (hwnd, WM_CLOSE, 0, 0);
            return 0;

        case WM_CLOSE:
            somSelf = (WPDCF77 *)WinQueryWindowPtr (hwnd, QWL_USER);
            somThis = WPDCF77GetData (somSelf);

            WinQueryDlgItemText (hwnd, DID_SET3_HOUR,   CCHMAXPATH, _szPathHHandBmp);
            WinQueryDlgItemText (hwnd, DID_SET3_MINUTE, CCHMAXPATH, _szPathMHandBmp);
            WinQueryDlgItemText (hwnd, DID_SET3_FACE,   CCHMAXPATH, _szPathFaceBmp);

            LoadBitmapFiles (somThis, TRUE);

            _wpRefreshClockView (somSelf);
            _wpSaveDeferred (somSelf);
            break;
        }
    return WinDefDlgProc (hwnd, msg, mp1, mp2);
    }

/*******************************************************************\
 *  Alarm2DlgProc                                                  *
\*******************************************************************/
VOID Alarm2InitDlg (HWND hwnd, WPDCF77 *somSelf)
    {
    WPDCF77Data  *somThis;
    IPT           ipt;

    somThis = WPDCF77GetData (somSelf);
    WinSetWindowPtr (hwnd, QWL_USER, (PVOID)somSelf);
    _pIDataAlarm2 = allocZeroMem (sizeof (ALARMINSTDATA2));

    WinSendDlgItemMsg (hwnd, DID_ALARMTEXT, MLM_SETTEXTLIMIT,    MPFROMLONG (CCHMAXALARMMSG), MPVOID);
    WinSendDlgItemMsg (hwnd, DID_ALARMTEXT, MLM_FORMAT,          MPFROMSHORT (MLFIE_CFTEXT),  MPVOID);
    WinSendDlgItemMsg (hwnd, DID_ALARMTEXT, MLM_SETIMPORTEXPORT, MPFROMP (_szAlarmMsg),       MPFROMLONG (CCHMAXALARMMSG));
    ipt = 0;
    WinSendDlgItemMsg (hwnd, DID_ALARMTEXT, MLM_IMPORT,          MPFROMP (&ipt),              MPFROMLONG (strlen (_szAlarmMsg)));

    SubclassEntryField (WinWindowFromID (hwnd, DID_WAVE_PROG));
    WinSetWindowULong (WinWindowFromID (hwnd, DID_WAVE_PROG), QWL_USER, DRTYPE_WAVE);
    WinSendDlgItemMsg (hwnd, DID_WAVE_PROG, EM_SETTEXTLIMIT,     MPFROMSHORT (CCHMAXPATH),    MPVOID);
    WinSetDlgItemText (hwnd, DID_WAVE_PROG, _szAlarmSoundFile);

    strcpy (_pIDataAlarm2->szOldAlarmSoundFile, _szAlarmSoundFile);

    return;
    }

VOID Alarm2Command (HWND hwnd, USHORT usCmd, USHORT usSource)
    {
    WPDCF77      *somSelf;
    WPDCF77Data  *somThis;
    IPT           ipt;
    FILEDLG       FileDlg;
    CHAR          szTitle[CCHMAXPGNAME];

    if (usSource == CMDSRC_PUSHBUTTON)
        {
        somSelf = (WPDCF77 *)WinQueryWindowPtr (hwnd, QWL_USER);
        somThis = WPDCF77GetData (somSelf);
        switch (usCmd)
            {
            case DID_CANCEL:
                ipt = 0;
                WinSendDlgItemMsg (hwnd, DID_ALARMTEXT, MLM_DELETE, MPFROMLONG (ipt), MPFROMLONG (CCHMAXALARMMSG));
                WinSendDlgItemMsg (hwnd, DID_ALARMTEXT, MLM_IMPORT, MPFROMP (&ipt), MPFROMLONG (strlen (_szAlarmMsg)));

                strcpy (_szAlarmSoundFile, _pIDataAlarm2->szOldAlarmSoundFile);
                WinSetDlgItemText (hwnd, DID_WAVE_PROG, _szAlarmSoundFile);

                break;

            case DID_STANDARD:
                WinSendDlgItemMsg (hwnd, DID_ALARMTEXT, MLM_SETSEL, MPFROMLONG (0), MPFROMLONG (CCHMAXALARMMSG));
                WinSendDlgItemMsg (hwnd, DID_ALARMTEXT, MLM_CLEAR, MPVOID, MPVOID);

                _szAlarmSoundFile[0] = '\0';
                WinSetDlgItemText (hwnd, DID_WAVE_PROG, _szAlarmSoundFile);

                break;

            case DID_WAVE_SRCH:
                WinLoadString (hab, hmod, IDS_ALRM_SEARCH_SOUNDFILE, CCHMAXPGNAME, szTitle);
                WinQueryDlgItemText (hwnd, DID_WAVE_PROG, CCHMAXPATH, FileDlg.szFullFile);
                FileDlg.cbSize          = sizeof (FILEDLG);
                FileDlg.fl              = FDS_OPEN_DIALOG | FDS_CENTER | FDS_HELPBUTTON;
                FileDlg.ulUser          = 0;
                FileDlg.pszTitle        = szTitle;
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
                if (FileDlg.lReturn == DID_OK)
                    WinSetDlgItemText (hwnd, DID_WAVE_PROG, FileDlg.szFullFile);
                break;
            }
        }

    return;
    }

VOID Alarm2Close (HWND hwnd)
    {
    WPDCF77      *somSelf;
    WPDCF77Data  *somThis;
    IPT           ipt;
    ULONG         ulCopy, ulc;
    PCHAR         psz;

    somSelf = (WPDCF77 *)WinQueryWindowPtr (hwnd, QWL_USER);
    somThis = WPDCF77GetData (somSelf);
    ipt     = 0;
    ulCopy  = CCHMAXALARMMSG;
    ulc = (ULONG)WinSendDlgItemMsg (hwnd, DID_ALARMTEXT, MLM_EXPORT, MPFROMP (&ipt), MPFROMP (&ulCopy));
    _szAlarmMsg[ulc] = '\0';

    /* "Newline"-Zeichen entfernen */
    psz = _szAlarmMsg;
    for (ulCopy = 0; ulCopy < ulc; ulCopy++)
        if (_szAlarmMsg[ulCopy] != '\n')
            *psz++ = _szAlarmMsg[ulCopy];
    *psz = '\0';

    WinQueryDlgItemText (hwnd, DID_WAVE_PROG, CCHMAXPATH, _szAlarmSoundFile);

    _wpSaveDeferred (somSelf);

    freeMem ((PPVOID)&_pIDataAlarm2);

    return;
    }

/*******************************************************************\
    Alarm2DlgProc: Dialog-Prozedur fÅr die Einstellung des Alarmtextes
    Eingang: hwnd: Window-Handle
             msg:  Message-Typ
             mp1:  1. Parameter
             mp2:  2. Parameter
\*******************************************************************/
MRESULT EXPENTRY Alarm2DlgProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
    {
    switch (msg)
        {
        case WM_INITDLG:
            Alarm2InitDlg (hwnd, (WPDCF77 *)mp2);
            return (MRESULT)TRUE;

        case WM_COMMAND:
            Alarm2Command (hwnd, SHORT1FROMMP (mp1), SHORT1FROMMP (mp2));
            return 0;

        case WM_DESTROY:
            WinSendMsg (hwnd, WM_CLOSE, MPVOID, MPVOID);
            return 0;

        case WM_CLOSE:
            Alarm2Close (hwnd);
            break;
        }

    return WinDefDlgProc (hwnd, msg, mp1, mp2);
    }

/*******************************************************************\
 *  Alarm3DlgProc                                                  *
\*******************************************************************/
VOID Alarm3InitDlg (HWND hwnd, WPDCF77 *somSelf)
    {
    WPDCF77Data  *somThis;

    somThis = WPDCF77GetData (somSelf);
    WinSetWindowPtr (hwnd, QWL_USER, (PVOID)somSelf);

    WinSendDlgItemMsg (hwnd, DID_ALRM_PROG, EM_SETTEXTLIMIT, MPFROMSHORT (CCHMAXPATH),     MPVOID);
    WinSendDlgItemMsg (hwnd, DID_ALRM_PARM, EM_SETTEXTLIMIT, MPFROMSHORT (CCHMAXPARM),     MPVOID);
    WinSendDlgItemMsg (hwnd, DID_ALRM_DIR,  EM_SETTEXTLIMIT, MPFROMSHORT (CCHMAXPATHCOMP), MPVOID);
    WinSetDlgItemText (hwnd, DID_ALRM_PROG, _szAlarmProg);
    WinSetDlgItemText (hwnd, DID_ALRM_PARM, _szAlarmParm);
    WinSetDlgItemText (hwnd, DID_ALRM_DIR,  _szAlarmDir);

    return;
    }

VOID Alarm3Command (HWND hwnd, USHORT usCmd, USHORT usSource)
    {
    WPDCF77      *somSelf;
    WPDCF77Data  *somThis;
    FILEDLG       FileDlg;
    CHAR          szTitle[CCHMAXPGNAME];

    if (usSource == CMDSRC_PUSHBUTTON)
        {
        switch (usCmd)
            {
            case DID_CANCEL:
                somSelf = (WPDCF77 *)WinQueryWindowPtr (hwnd, QWL_USER);
                somThis = WPDCF77GetData (somSelf);

                WinSetDlgItemText (hwnd, DID_ALRM_PROG, _szAlarmProg);
                WinSetDlgItemText (hwnd, DID_ALRM_PARM, _szAlarmParm);
                WinSetDlgItemText (hwnd, DID_ALRM_DIR,  _szAlarmDir);
                break;

            case DID_STANDARD:
                WinSetDlgItemText (hwnd, DID_ALRM_PROG, "");
                WinSetDlgItemText (hwnd, DID_ALRM_PARM, "");
                WinSetDlgItemText (hwnd, DID_ALRM_DIR,  "");
                break;

            case DID_ALRM_SRCH:
                WinLoadString (hab, hmod, IDS_ALRM_SEARCH_PROG, CCHMAXPGNAME, szTitle);
                WinQueryDlgItemText (hwnd, DID_ALRM_PROG, CCHMAXPATH, FileDlg.szFullFile);
                FileDlg.cbSize          = sizeof (FILEDLG);
                FileDlg.fl              = FDS_OPEN_DIALOG | FDS_CENTER | FDS_HELPBUTTON;
                FileDlg.ulUser          = 0;
                FileDlg.pszTitle        = szTitle;
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
                if (FileDlg.lReturn == DID_OK)
                    WinSetDlgItemText (hwnd, DID_ALRM_PROG, FileDlg.szFullFile);
                break;
            }
        }

    return;
    }

VOID Alarm3Close (HWND hwnd)
    {
    WPDCF77      *somSelf;
    WPDCF77Data  *somThis;

    somSelf = (WPDCF77 *)WinQueryWindowPtr (hwnd, QWL_USER);
    somThis = WPDCF77GetData (somSelf);

    WinQueryDlgItemText (hwnd, DID_ALRM_PROG, CCHMAXPATH, _szAlarmProg);
    WinQueryDlgItemText (hwnd, DID_ALRM_PARM, CCHMAXPATH, _szAlarmParm);
    WinQueryDlgItemText (hwnd, DID_ALRM_DIR,  CCHMAXPATH, _szAlarmDir);

    _wpSaveDeferred (somSelf);

    return;
    }

/*******************************************************************\
    Alarm3DlgProc: Dialog-Prozedur fÅr die Einstellung des Alarmprogrammes
    Eingang: hwnd: Window-Handle
             msg:  Message-Typ
             mp1:  1. Parameter
             mp2:  2. Parameter
\*******************************************************************/
MRESULT EXPENTRY Alarm3DlgProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
    {
    switch (msg)
        {
        case WM_INITDLG:
            Alarm3InitDlg (hwnd, (WPDCF77 *)mp2);
            return (MRESULT)TRUE;

        case WM_COMMAND:
            Alarm3Command (hwnd, SHORT1FROMMP (mp1), SHORT1FROMMP (mp2));
            return 0;

        case WM_DESTROY:
            WinSendMsg (hwnd, WM_CLOSE, MPVOID, MPVOID);
            return 0;

        case WM_CLOSE:
            Alarm3Close (hwnd);
            break;
        }

    return WinDefDlgProc (hwnd, msg, mp1, mp2);
    }

/*******************************************************************\
 *  ClkAudioDlgProc                                                *
\*******************************************************************/
VOID ClkAudioInitDlg (HWND hwnd, WPDCF77 *somSelf)
    {
    WPDCF77Data  *somThis;
    HOBJECT       hObject;

    somThis = WPDCF77GetData (somSelf);
    WinSetWindowPtr (hwnd, QWL_USER, (PVOID)somSelf);

    SubclassEntryField (WinWindowFromID (hwnd, DID_GONG_PROG));
    WinSetWindowULong (WinWindowFromID (hwnd, DID_GONG_PROG), QWL_USER, DRTYPE_WAVE);
    WinSendDlgItemMsg (hwnd, DID_GONG_PROG, EM_SETTEXTLIMIT, MPFROMSHORT (CCHMAXPATH), MPVOID);

    hObject = _wpQueryHandle (somSelf);
    CheckAudioObjectDeleted (somSelf, hObject);
    SetAudioControls (hwnd, somThis, hObject, TRUE);

    /* Timer zur Aktualisierung der Anzeige starten */
    _wpAdd1sTimer (somSelf, hwnd);

    return;
    }

VOID ClkAudio1sTimer (HWND hwnd)
    {
    WPDCF77      *somSelf;
    WPDCF77Data  *somThis;
    HOBJECT       hObject;

    somSelf = (WPDCF77 *)WinQueryWindowPtr (hwnd, QWL_USER);
    somThis = WPDCF77GetData (somSelf);

    hObject = _wpQueryHandle (somSelf);

    CheckAudioObjectDeleted (somSelf, hObject);

    if (IsClockWndChanged (hwnd, hObject))
        SetAudioControls (hwnd, somThis, hObject, FALSE);

    return;
    }

VOID ClkAudioCommand (HWND hwnd, USHORT usCmd, USHORT usSource)
    {
    WPDCF77      *somSelf;
    WPDCF77Data  *somThis;

    somSelf = (WPDCF77 *)WinQueryWindowPtr (hwnd, QWL_USER);
    somThis = WPDCF77GetData (somSelf);
    if (usSource == CMDSRC_PUSHBUTTON)
        {
        switch (usCmd)
            {
            case DID_STANDARD:
                hClsRingingObject = NULLHANDLE;
                szClsGongFile[0]  = '\0';
                bClsIs1PerHour    = FALSE;
                WinSetDlgItemText (hwnd, DID_GONG_PROG, "");
                WinCheckButton (hwnd, DID_GONG_MDN, TRUE);
                break;

            case DID_CANCEL:
                WinSetDlgItemText (hwnd, DID_GONG_PROG, _pIData->szClsGongFile);
                WinCheckButton (hwnd, _pIData->bClsIs1PerHour ? DID_GONG_MD1 : DID_GONG_MDN, TRUE);
                break;

            case DID_GONG_SRCH:
                BrowseFile (hwnd, szClsGongFile);
                SetAudioControls (hwnd, somThis, _wpQueryHandle (somSelf), TRUE);
                break;
            }
        }

    return;
    }

VOID ClkAudioControl (HWND hwnd, MPARAM mp1)
    {
    WPDCF77      *somSelf;
    WPDCF77Data  *somThis;

    somSelf = (WPDCF77 *)WinQueryWindowPtr (hwnd, QWL_USER);
    somThis = WPDCF77GetData (somSelf);
    switch (SHORT1FROMMP (mp1))
        {
        case DID_GONG_PROG:
            if (SHORT2FROMMP (mp1) == EN_CHANGE)
                {
                WinQueryDlgItemText (hwnd, DID_GONG_PROG, CCHMAXPATH, szClsGongFile);
                SetAudioControls (hwnd, somThis, _wpQueryHandle (somSelf), FALSE);
                }
            break;

        case DID_GONG_MD1:
            if (SHORT2FROMMP (mp1) == BN_CLICKED)
                {
                bClsIs1PerHour = TRUE;
                SetAudioControls (hwnd, somThis, _wpQueryHandle (somSelf), FALSE);
                }
            break;

        case DID_GONG_MDN:
            if (SHORT2FROMMP (mp1) == BN_CLICKED)
                {
                bClsIs1PerHour = FALSE;
                SetAudioControls (hwnd, somThis, _wpQueryHandle (somSelf), FALSE);
                }
            break;
        }

    return;
    }

VOID ClkAudioClose (HWND hwnd)
    {
    WPDCF77      *somSelf;

    somSelf = (WPDCF77 *)WinQueryWindowPtr (hwnd, QWL_USER);

    DebugULx (D_WND, "ClkAudioDlgProc, WM_CLOSE", " hObject", (ULONG)_wpQueryHandle (somSelf));

    SaveSetData (somSelf, SAVESET_AUDIODATA);
    _wpRemove1sTimer (somSelf, hwnd);

    return;
    }

/*******************************************************************\
    ClkAudioDlgProc: Dialog-Prozedur fÅr Gong-Einstellungen
    Eingang: hwnd: Window-Handle
             msg:  Message-Typ
             mp1:  1. Parameter
             mp2:  2. Parameter
\*******************************************************************/
MRESULT EXPENTRY ClkAudioDlgProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
    {
    switch (msg)
        {
        case WM_INITDLG:
            ClkAudioInitDlg (hwnd, (WPDCF77 *)mp2);
            return (MRESULT)TRUE;

        case WM_1STIMER:
            ClkAudio1sTimer (hwnd);
            return 0;

        case WM_COMMAND:
            ClkAudioCommand (hwnd, SHORT1FROMMP (mp1), SHORT1FROMMP (mp2));
            return 0;

        case WM_CONTROL:
            ClkAudioControl (hwnd, mp1);
            return 0;

        case WM_DESTROY:
            WinSendMsg (hwnd, WM_CLOSE, MPVOID, MPVOID);
            return 0;

        case WM_CLOSE:
            ClkAudioClose (hwnd);
            break;
        }

    return WinDefDlgProc (hwnd, msg, mp1, mp2);
    }

/*******************************************************************\
    AboutDlgProc: Dialog-Prozedur fÅr About-Box
    Eingang: hwnd: Window-Handle
             msg:  Message-Typ
             mp1:  1. Parameter
             mp2:  2. Parameter
\*******************************************************************/
MRESULT EXPENTRY AboutDlgProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
    {
    WPDCF77      *somSelf;
    WPDCF77Data  *somThis;
    CHAR         szText[CCHMAXPGNAME];

    switch (msg)
        {
        case WM_INITDLG:
            somSelf = (WPDCF77 *)mp2;
            somThis = WPDCF77GetData (somSelf);
            WinSetWindowPtr (hwnd, QWL_USER, (PVOID)somSelf);

            /* Ansicht registrieren */
            memset (&_pIData->AboutViewItem, '\0', sizeof (VIEWITEM));
            _pIData->AboutUseItem.type    = USAGE_OPENVIEW;
            _pIData->AboutViewItem.view   = OPEN_ABOUT;
            _pIData->AboutViewItem.handle = hwnd;
            _wpAddToObjUseList(somSelf,&_pIData->AboutUseItem);

            WinLoadString (hab, hmod, IDS_ABOUT, CCHMAXPGNAME, szText);
            _wpRegisterView (somSelf, hwnd, szText);
            return FALSE;

        case WM_COMMAND:
            if (SHORT1FROMMP (mp2) == CMDSRC_PUSHBUTTON)
                if (SHORT1FROMMP (mp1) == DID_CANCEL || SHORT1FROMMP (mp1) == DID_OK)
                    WinSendMsg (hwnd, WM_CLOSE, MPVOID, MPVOID);
            return 0;

        case WM_CLOSE:
            somSelf = (WPDCF77 *)WinQueryWindowPtr (hwnd, QWL_USER);
            somThis = WPDCF77GetData (somSelf);
            _wpDeleteFromObjUseList (somSelf, &_pIData->AboutUseItem);
            WinDestroyWindow (hwnd);
            break;
        }

    return WinDefDlgProc (hwnd, msg, mp1, mp2);
    } /* AboutDlgProc () */

/*******************************************************************\
    NewParentWndProc: ErgÑnzt die Standard-Dialog-Prozedur fÅr
                      Frame-Windows um 1 neue Methode, die ihre
                      Clients Åber einen Farbwechsel informiert
    Eingang: hwnd: Window-Handle
             msg:  Message-Typ
             mp1:  1. Parameter
             mp2:  2. Parameter
\*******************************************************************/
MRESULT EXPENTRY NewParentWndProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
    {

    switch (msg)
        {
        case WM_SETCELL40:
        case WM_SETCELL:
            WinBroadcastMsg (hwnd, WM_SETCELL, mp1, mp2, BMSG_SEND | BMSG_DESCENDANTS);
            return 0;
        }

    return (pOldParentWndProc)(hwnd, msg, mp1, mp2);
    }

/*******************************************************************\
    NewFontDlgProc: énderung der WM_INITDLG-Methode der FontDlgProc
    Eingang: hwnd: Window-Handle
             msg:  Message-Typ
             mp1:  1. Parameter
             mp2:  2. Parameter
\*******************************************************************/
MRESULT EXPENTRY NewFontDlgProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
    {

    switch (msg)
        {
        case WM_INITDLG:
            WinShowWindow (WinWindowFromID (hwnd, DID_SIZE_PREFIX), FALSE);
            WinShowWindow (WinWindowFromID (hwnd, DID_SIZE),        FALSE);
            break;
        }

    return WinDefFontDlgProc (hwnd, msg, mp1, mp2);
    }
