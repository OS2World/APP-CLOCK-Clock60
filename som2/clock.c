/***************************************************************************\
 *
 * PROGRAM NAME: WPDCF77
 * -------------
 *
 * VERSION: 6.0
 * --------
 *
 * MODULE NAME: CLOCK
 * ------------
 *
 * DESCRIPTION:
 * ------------
 *   Creation of the frame and client window for the clock, frame window procedure
 *
 * NOTES:
 * ------
 *
 * FUNCTIONS:
 * -----------
 *   ClkInit ()
 *   ClkFrameWndProc ()
 *   MessageBox ()
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
#define INCL_PM
#define INCL_GPITRANSFORMS
#define INCL_WINHELP
#define INCL_WINWINDOWMGR
#define INCL_WINMESSAGEMGR
#define INCL_WINDIALOGS
#define INCL_DOSSEMAPHORES
#define INCL_DOSDATETIME
#define INCL_DOSDEVIOCTL
#define INCL_DOSPROCESS

#include <os2.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "dcfioctl.h"

#pragma info(noeff)         // suppress SOM compiler informationals

#include "wpdcf77.ih"
#include "wpdcf.h"

#include "dialogs.h"
#include "clock.h"
#include "res.h"
#include "clockpan.h"
#include "clkdata.h"
#include "clkmem.h"
#include "alarmthd.h"
#include "wndproc.h"
#include "debug.h"

/*--------------------------------------------------------------*\
 *  Global definitions and variables for this module            *
\*--------------------------------------------------------------*/
/* may be global, as this address is constant within system */
static PFNWP  pfnwpFrameWndProc;    // old FrameWndProc for WinSubclassWindow

/*******************************************************************\
    ClkInit: Initialize the clock: Create frame and client
    Input:   somSelf: Pointer to instance
    return:  frame window handle
\*******************************************************************/
HWND ClkInit (WPDCF77 *somSelf)
    {
    CHAR        szClkName[CCHMAXMSG];
    ULONG       ulcTime;
    WPDCF77Data *somThis;
    PWINDOWDATA pWindowData;
    FRAMECDATA  flFrameCtlData;             /* Frame Ctl Data */

    somThis = WPDCF77GetData (somSelf);

    WinLoadString (hab, hmod, IDS_APPNAME, CCHMAXMSG, szClkName);

    _timerResources.ulTimerValue = SEM_TIMER_VALUE;

    /* On registration of the class space is cleared for 2 WindowPtr.           */
    /* The first will contain PWINDOWDATA, the second is used for audio support */
    /* (in the functions within play.h 1 has to specified for ulQwlIndex)       */
    if (!WinRegisterClass (hab, szClkName, ClkWndProc,
                           CS_SIZEREDRAW | CS_MOVENOTIFY, 2*sizeof (PVOID)))
        return (NULLHANDLE);

    pWindowData = (PWINDOWDATA) _wpAllocMem (somSelf, sizeof (WINDOWDATA), NULL);

    if (!pWindowData)
        {
        DebugE (D_DIAG, "ClockInit", "Error on allocation of pWindowData");
        return NULLHANDLE;
        }

    memset ((PVOID) pWindowData, 0, sizeof (WINDOWDATA));
    pWindowData->cb      = sizeof (WINDOWDATA);
    pWindowData->somSelf = somSelf;
    pWindowData->somThis = somThis;

    /* Create frame window */
    flFrameCtlData.cb            = sizeof (flFrameCtlData);
    flFrameCtlData.flCreateFlags = FCF_SIZEBORDER | FCF_NOBYTEALIGN | FCF_SYSMENU |
                                   FCF_MINMAX     | FCF_TITLEBAR;
    flFrameCtlData.hmodResources = hmod;
    flFrameCtlData.idResources   = ID_MAIN;

    _hwndFrame = WinCreateWindow (
        HWND_DESKTOP,              /* parent */
        WC_FRAME,
        _wpQueryTitle (somSelf),   /* pointer to window text */
        0,                         /* window style */
        0, 0, 0, 0,
        NULLHANDLE,
        HWND_TOP,
        (USHORT) ID_MAIN,          /* window identifier                       */
        (PVOID) &flFrameCtlData,   /* pointer to buffer                       */
        NULL);                     /* pointer to structure with pres. params. */

    if (!_hwndFrame)
        return NULLHANDLE;

    /* Needed for overloading of frame window procedure */
    WinSetWindowPtr (_hwndFrame, QWL_USER, (PVOID)somSelf);

    /* Create client window */
    _hwndClient = WinCreateWindow (    /* use of WinCreateWindow to be able to specify CtlData */
        _hwndFrame,                    /* parent */
        szClkName,
        NULL,                      /* pointer to window text */
        0,                         /* window style */
        0, 0, 0, 0,
        _hwndFrame,
        HWND_TOP,
        (USHORT) FID_CLIENT,       /* window identifier */
        pWindowData,               /* CtlData */
        NULL);                     /* PresParms */

    if (!_hwndClient)
        {
        WinDestroyWindow (_hwndFrame);
        return NULLHANDLE;
        }

    pfnwpFrameWndProc = WinSubclassWindow (_hwndFrame, ClkFrameWndProc);

    /* load bitmap files for hand and clock face */
    LoadBitmapFiles (somThis, FALSE);

    /* wait for termination of old timer thread */
    for (ulcTime = 0; ulcTime < 50; ulcTime++)
        {
        if (_timerResources.hSemTimer == 0)
            break;
        DosSleep (100L);
        }

    /* start alarm thread */
    memset (&_timerResources, '\0', sizeof (TIMER_RESOURCES));
    if ((_timerResources.tidAlarmThread = _beginthread ((PFNTHD)AlarmThread,
                                                        NULL,
                                                        STACKLEN,
                                                        somSelf)) == -1)
        {
        MessageBox (_hwndFrame,
                    IDMSG_ERRALARMTHREAD,
                    MBTITLE_ERROR,
                    MB_OK | MB_ERROR | MB_MOVEABLE,
                    0,
                    NULL);
        WinDestroyWindow (_hwndFrame);
        _hwndFrame = NULLHANDLE;
        return NULLHANDLE;
        }

    if (!AlarmStart (somThis, activate))
        _wpSaveDeferred (somSelf);

    return _hwndFrame;
    }

/*******************************************************************\
    ClkFrameWndProc: Frame window procedure of clock
    Eingang: hwnd: window handle
             msg:  message type
             mp1:  parameter 1
             mp2:  parameter 2
\*******************************************************************/
MRESULT EXPENTRY ClkFrameWndProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
    {
    WPDCF77     *somSelf;
    WPDCF77Data *somThis;
    register USHORT cmd;
    HWND   hwndT;
    SWP    swp;

    switch (msg)
        {
        case WM_DESTROY:
            somSelf = (WPDCF77 *)WinQueryWindowPtr (hwnd, QWL_USER);
            somThis = WPDCF77GetData (somSelf);
            if (WinIsChild (_hwndTitleBar, HWND_OBJECT))
                ClkShowFrameControls (hwnd, TRUE);
            break;

        case WM_HELP:
            if (SHORT1FROMMP (mp2) == CMDSRC_PUSHBUTTON && SHORT1FROMMP (mp1) == DID_HELP ||
                SHORT1FROMMP (mp2) == CMDSRC_ACCELERATOR)
                {
                somSelf = (WPDCF77 *)WinQueryWindowPtr (hwnd, QWL_USER);
                somThis = WPDCF77GetData (somSelf);
                _wpDisplayHelp (somSelf,
                    _ulPresence & PRESENCE_DISP_DIGITAL ? PANEL_VIEWDIGITAL : PANEL_VIEWANALOG,
                    HELPFILENAME);
                return 0;
                }
            break;

        case WM_SYSCOMMAND:
            somSelf = (WPDCF77 *)WinQueryWindowPtr (hwnd, QWL_USER);
            somThis = WPDCF77GetData (somSelf);
            if (WinIsChild (_hwndTitleBar, HWND_OBJECT))
                {
                cmd = SHORT1FROMMP (mp1);

                /* ignore during MouseCapture */
                if (WinQueryCapture (HWND_DESKTOP) != NULLHANDLE)
                    return 0;

                switch (cmd)
                    {
                    case SC_RESTORE:
                    case SC_MAXIMIZE:
                        /* get restore position */
                        WinQueryWindowPos (hwnd, &swp);
                        swp.fl = cmd == SC_RESTORE ? SWP_RESTORE : SWP_MAXIMIZE;

                        /* activate/deactivate, if Ctl key is pressed */
                        /* don't set flags, except origin of key code */
                        /* is accelerator                             */
                        if (SHORT1FROMMP (mp2) != CMDSRC_ACCELERATOR)
                            {
                            if (WinGetKeyState (HWND_DESKTOP, VK_CTRL) & 0x8000)
                                swp.fl &= ~(SWP_DEACTIVATE | SWP_ACTIVATE);
                            }

                        WinSetMultWindowPos(NULLHANDLE, (PSWP)&swp, 1);
                        return 0;

                    case SC_SIZE:
                        /* start keyboard sizing interface by sending of */
                        /* WM_TRACKFRAME to itself (frame window)        */
                        WinSendMsg (hwnd, WM_TRACKFRAME,
                            MPFROMSHORT (TF_SETPOINTERPOS), MPVOID );
                        return 0;

                    case SC_MOVE:
                        /* start keyboard moving interface by sendign of */
                        /* WM_TRACKFRAME to itself (frame window)        */
                        WinSendMsg (hwnd, WM_TRACKFRAME,
                            MPFROMSHORT (TF_SETPOINTERPOS | TF_MOVE), MPVOID );
                        return 0;

                    case SC_CLOSE:
                        if ((hwndT = WinWindowFromID (hwnd, FID_CLIENT)) == NULLHANDLE)
                            hwndT = hwnd;
                        WinSetParent (_hwndTitleBar, hwnd, FALSE);
                        WinSetParent (_hwndSysMenu,  hwnd, FALSE);
                        WinSetParent (_hwndMinMax,   hwnd, FALSE);

                        WinPostMsg (hwndT, WM_CLOSE, MPVOID, MPVOID );
                        return 0;
                    }
                }
            break;
        }

    return (*pfnwpFrameWndProc)(hwnd, msg, mp1, mp2);
    }

/*******************************************************************\
    MessageBox: Display a text in a message box.
                The text may be passed as string or message id.
                If the text contains the variables %d, %t and %z,
                they will be substituted by %0, %1 and %2 and then
                WinSubstituteStrings is called.
                The variables are:
                    %t, %0: current time
                    %d, %1: current date
                    %z, %2: time zone
    Input:   hwndOwner: owner window handle
             idText:    message id (RC-MESSAGETABLE)
                        only valid, if pszText == NULL
             idTitle:   mssage id of title:
                        idTitle = 0             => no title
                        idTitle = MBTITLE_ERROR => title "error"
                        idTitle   other         => text id
             flStyle:   WinMessageBox styles
             ulBeep:    WinAlarm beep value; 0 => no beep
             pszText:   message text; NULL => text defined by idText
    return:  return value of WinMessageBox
\*******************************************************************/
ULONG MessageBox (HWND hwndOwner, ULONG idText, ULONG idTitle, ULONG flStyle, ULONG ulBeep, PSZ pszText)
    {
    CHAR  szText[CCHMAXMSG];
    CHAR  szTitle[CCHMAXMSG];
    CHAR  c;
    ULONG ulcText, ulc, ulcSubst;
    ULONG ulRet;
    PSZ   pszTitle, pszSource, pszDest;

    /* Load text, if pszText == NULL */
    if (pszText == NULL)
        {
        if (!WinLoadMessage (WinQueryAnchorBlock (HWND_DESKTOP), hmod,
                             idText, CCHMAXMSG, szText))
            {
            WinAlarm (HWND_DESKTOP, WA_ERROR);
            return MBID_ERROR;
            }
        else
            pszText = szText;
        }

    ulcText = strlen (pszText) + 1;

    DosAllocMem ((PPVOID)&pszSource, ulcText, PAG_COMMIT | PAG_READ | PAG_WRITE);
    memcpy (pszSource, pszText, ulcText);

    for (ulcSubst = ulc = 0; ulc < ulcText-1; ulc++)
        {
        c = WinUpperChar (hab, 0, 0, pszSource[ulc+1]);
        if (pszSource[ulc] == '%' &&  (c == 'T' || c == 'D' || c == 'Z'))
            {
            switch (c)
                {
                case 'T':
                    c = '0';
                    break;

                case 'D':
                    c = '1';
                    break;

                case 'Z':
                    c = '2';
                    break;
                }

            pszSource[++ulc] = c;
            ulcSubst++;
            }
        }

    ulcText += ulcSubst*CCHMAXDATETIME;
    DosAllocMem ((PPVOID)&pszDest, ulcText, PAG_COMMIT | PAG_READ | PAG_WRITE);
    WinSubstituteStrings (hwndOwner, pszSource, ulcText, pszDest);

    /* beep, if ulBeep != 0 */
    if (ulBeep)
        WinAlarm (HWND_DESKTOP, ulBeep);

    /* Load title */
    switch (idTitle)
        {
        case MBTITLE_ERROR:
            pszTitle = NULL;
            break;
        case MBTITLE_NONE:
            pszTitle = "";
            break;
        default:
            if (!WinLoadMessage (WinQueryAnchorBlock (HWND_DESKTOP), hmod,
                                 idTitle, CCHMAXMSG, szTitle))
                {
                WinAlarm (HWND_DESKTOP, WA_ERROR);
                }
            pszTitle = szTitle;
        }

    /* Open message box */
    ulRet = WinMessageBox (HWND_DESKTOP, hwndOwner, pszDest,
                           pszTitle, IDD_MESSAGEBOX, flStyle);

    DosFreeMem (pszSource);
    DosFreeMem (pszDest);

    return ulRet;
    }
