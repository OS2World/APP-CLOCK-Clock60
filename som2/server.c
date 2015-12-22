/***************************************************************************\
 *
 * PROGRAM NAME: WPDCF77
 * -------------
 *
 * VERSION: 6.0
 * --------
 *
 * MODULE NAME: SERVER
 * ------------
 *
 * DESCRIPTION:
 * ------------
 *   dialog procedure for the settings page of LAN::Time servers
 *
 * NOTES:
 * ------
 *
 * FUNCTIONS:
 * -----------
 *   SetSlider ()
 *   QuerySlider ()
 *   SrvrIntervalDlgProc ()
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
#define INCL_GPITRANSFORMS
#define INCL_WINDIALOGS
#define INCL_WINSTDSLIDER
#define INCL_WINWINDOWMGR
#define INCL_WINBUTTONS
#include <os2.h>

#include <stdlib.h>

#include "dcfioctl.h"

#pragma info(noeff)         // suppress SOM compiler informationals

#include "wpdcf77.ih"
#include "wpdcf.h"

#include "clock.h"
#include "res.h"
#include "clockpan.h"
#include "dcf77.h"
#include "server.h"
#include "clkdata.h"

/*--------------------------------------------------------------*\
 *  Global definitions and variables for this module            *
\*--------------------------------------------------------------*/
#define TICKNUMBER      5

static PCHAR pszScale[] = {"1min", "6h", "12h", "18h", "24h  "};    // spaces at the end for positioning!
static USHORT usTick[TICKNUMBER] = {0, 0, 0, 0};                    // tick position slider 1

/*******************************************************************\
    SetSlider: Sets the slider to position in instance data
    Input: hwnd: parent window
\*******************************************************************/
VOID SetSlider (HWND hwnd)
    {
    USHORT usPosition;
    CHAR   szText[5];

    if (ulSrvrInterval <= 21600)
        usPosition = fRound (usTick[1]*(fRound (ulSrvrInterval/60.) - 1) / 359.);
    else
        usPosition = fRound ((usTick[TICKNUMBER-1] - usTick[1])*
            (fRound (ulSrvrInterval/600.) - 36)/108.) + usTick[1];

    WinSendDlgItemMsg (hwnd, DID_SERVERINTERVAL, SLM_SETSLIDERINFO,
        MPFROM2SHORT (SMA_SLIDERARMPOSITION, SMA_RANGEVALUE),
        MPFROMSHORT (usPosition));

    WinSetDlgItemText (hwnd, DID_TXT_INTERVAL, _itoa (ulSrvrInterval/60, szText, 10));

    return;
    }

/*******************************************************************\
    QuerySlider: Calculates the length of interval in seconds out
                 of the length of the slider
                 Result will be set to ulSrvrInterval
    Input: hwnd: parent window
\*******************************************************************/
VOID QuerySlider (HWND hwnd)
    {
    USHORT usPosition;
    CHAR   szText[5];

    usPosition = (USHORT) WinSendDlgItemMsg (hwnd, DID_SERVERINTERVAL,
        SLM_QUERYSLIDERINFO,
        MPFROM2SHORT (SMA_SLIDERARMPOSITION, SMA_RANGEVALUE),
        MPVOID);

    if (usPosition < usTick[1])
        ulSrvrInterval = 60 * (fRound ((359.*usPosition)/usTick[1]) + 1);
    else
        ulSrvrInterval = 600 *
            (fRound ((108. * (usPosition-usTick[1]))/(usTick[TICKNUMBER-1]-usTick[1])) + 36);

    WinSetDlgItemText (hwnd, DID_TXT_INTERVAL, _itoa (ulSrvrInterval/60, szText, 10));

    return;
    }

/*******************************************************************\
    SrvrIntervalDlgProc: Dialog prozedure for
                         LAN::Time server settings
    Input: hwnd: window handle
           msg:  message type
           mp1:  parameter 1
           mp2:  parameter 2
\*******************************************************************/
MRESULT EXPENTRY SrvrIntervalDlgProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
    {
    SHORT       l;
    WPDCF77     *somSelf;
    WPDCF77Data *somThis;

    switch (msg)
        {
        case WM_INITDLG:
            somSelf = (WPDCF77 *)mp2;
            somThis = WPDCF77GetData (somSelf);
            WinSetWindowPtr (hwnd, QWL_USER, (PVOID) somSelf);

            _pIData->ulOldSrvrInterval = _pIData->ulSrvrInterval = ulSrvrInterval;
            _pIData->bOldSendSrvrTime  = _pIData->bSendSrvrTime  = bSendSrvrTime;

            /* set slider tick length */
            WinSendDlgItemMsg (hwnd, DID_SERVERINTERVAL, SLM_SETTICKSIZE,
                MPFROM2SHORT (SMA_SETALLTICKS, 5), MPVOID);

            /* name slider scales */
            for (l = 0; l < TICKNUMBER; l++)
                WinSendDlgItemMsg (hwnd, DID_SERVERINTERVAL, SLM_SETSCALETEXT,
                    MPFROMSHORT (l), (MPARAM) pszScale[l]);

            /* determine tick positions, if not yet done */
            if (usTick[TICKNUMBER-1] == 0)
                {
                for (l = 0; l < TICKNUMBER; l++)
                    usTick[l] = (USHORT) WinSendDlgItemMsg (hwnd, DID_SERVERINTERVAL,
                        SLM_QUERYTICKPOS, MPFROMSHORT (l), MPVOID);

                for (l = TICKNUMBER-1; l >= 0; l--)
                    usTick[l] -= usTick[0];         // tick 0 begins at position 0!
                }

            /* set slider positions and buttons */
            SetSlider (hwnd);
            WinCheckButton (hwnd, DID_SERVERSENDTIME, bSendSrvrTime);

            /* start timer for updating of display */
            _wpAdd1sTimer (somSelf, hwnd);
            return (MRESULT)TRUE;

        case WM_CONTROL:
            switch (LONGFROMMP (mp1))
                {
                case DID_SERVERINTERVAL + (SLN_SLIDERTRACK<<16):
                case DID_SERVERINTERVAL + (SLN_CHANGE<<16):
                    QuerySlider (hwnd);
                    somSelf = (WPDCF77 *)WinQueryWindowPtr (hwnd, QWL_USER);
                    somThis = WPDCF77GetData (somSelf);
                    _pIData->ulSrvrInterval = ulSrvrInterval;
                    break;

                case DID_SERVERSENDTIME + (BN_CLICKED<<16):
                case DID_SERVERSENDTIME + (BN_DBLCLICKED<<16):
                    somSelf = (WPDCF77 *)WinQueryWindowPtr (hwnd, QWL_USER);
                    somThis = WPDCF77GetData (somSelf);
                    _pIData->bSendSrvrTime = bSendSrvrTime =
                        WinQueryButtonCheckstate (hwnd, DID_SERVERSENDTIME);
                    break;

                default:
                    return 0;
                }
            return 0;

        case WM_COMMAND:
            if (SHORT1FROMMP (mp2) == CMDSRC_PUSHBUTTON)
                {
                ULONG ulTmpInterval    = ulSrvrInterval;
                BOOL  bTmpSendSrvrTime = bSendSrvrTime;

                somSelf   = (WPDCF77 *)WinQueryWindowPtr (hwnd, QWL_USER);
                somThis   = WPDCF77GetData (somSelf);
                switch (SHORT1FROMMP (mp1))
                    {
                    case DID_CANCEL:
                        ulSrvrInterval = _pIData->ulSrvrInterval = _pIData->ulOldSrvrInterval;
                        bSendSrvrTime  = _pIData->bSendSrvrTime  = _pIData->bOldSendSrvrTime;
                        break;

                    case DID_STANDARD:
                        ulSrvrInterval = _pIData->ulSrvrInterval = INTERVAL_DEFAULT;
                        bSendSrvrTime  = _pIData->bSendSrvrTime  = SENDTIME_DEFAULT;
                        break;

                    default:
                        return 0;
                    }

                if (ulTmpInterval != ulSrvrInterval)
                    SetSlider (hwnd);

                if (bTmpSendSrvrTime != bSendSrvrTime)
                    WinCheckButton (hwnd, DID_SERVERSENDTIME, bSendSrvrTime);
                }
            return 0;

        case WM_1STIMER:
            somSelf = (WPDCF77 *)WinQueryWindowPtr (hwnd, QWL_USER);
            somThis = WPDCF77GetData (somSelf);
            if (ulSrvrInterval != _pIData->ulSrvrInterval)
                {
                _pIData->ulSrvrInterval = ulSrvrInterval;
                SetSlider (hwnd);
                }
            if (bSendSrvrTime != _pIData->bSendSrvrTime)
                {
                _pIData->bSendSrvrTime = bSendSrvrTime;
                WinCheckButton (hwnd, DID_SERVERSENDTIME, bSendSrvrTime);
                }
            return 0;

        case WM_DESTROY:
            WinSendMsg (hwnd, WM_CLOSE, 0, 0);
            return 0;

        case WM_CLOSE:
            somSelf = (WPDCF77 *)WinQueryWindowPtr (hwnd, QWL_USER);
            somThis = WPDCF77GetData (somSelf);

            SaveSetData (somSelf, SAVESET_INIINTERVAL | SAVESET_SHMEMINTERVAL);
            _wpRemove1sTimer (somSelf, hwnd);
            break;
        }

    return WinDefDlgProc (hwnd, msg, mp1, mp2);
    }
