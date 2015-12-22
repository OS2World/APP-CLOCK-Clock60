/***************************************************************************\
 *
 * PROGRAM NAME: WPDCF77
 * -------------
 *
 * VERSION: 6.0
 * --------
 *
 * MODULE NAME: WNDPROC
 * ------------
 *
 * DESCRIPTION:
 * ------------
 *   window procedure for displaying the clock
 *
 * NOTES:
 * ------
 *
 * FUNCTIONS:
 * -----------
 *   ClkWndProc ()
 *   ClkDrawString ()
 *   ClkDrawDigitalTime ()
 *   ClkRefresh ()
 *   ClkShowFrameControls ()
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


// flRcvrStatus still necessary???
#define INCL_DEV
#define INCL_OS2MM
#define INCL_WININPUT
#define INCL_WINFRAMEMGR
#define INCL_WINTRACKRECT
#define INCL_WINMENUS
#define INCL_WINSYS
#define INCL_WINMESSAGEMGR
#define INCL_WINSHELLDATA
#define INCL_WINWINDOWMGR
#define INCL_WINHELP
#define INCL_WINPOINTERS
#define INCL_GPICONTROL
#define INCL_GPIPRIMITIVES
#define INCL_GPILOGCOLORTABLE
#define INCL_GPIBITMAPS
#define INCL_GPITRANSFORMS
#define INCL_GPIREGIONS
#define INCL_DOSFILEMGR
#define INCL_DOSSEMAPHORES
#define INCL_DOSDATETIME
#define INCL_GPILCIDS

#include <os2.h>
#include <os2me.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cutil.h>

#include "dcfioctl.h"

#pragma info(noeff)         // suppress SOM compiler informationals

#include "wpdcf77.ih"
#include "wpdcf.h"

#include "play.h"
#include "clock.h"
#include "res.h"
#include "clkdata.h"
#include "dcf77.h"
#include "paint.h"
#include "alarmthd.h"
#include "udtime.h"
#include "wndproc.h"
#include "debug.h"

/*--------------------------------------------------------------*\
 *  Globale definitions and variables for this module           *
\*--------------------------------------------------------------*/
#define CCHMAXFONTSIZENAME      32

#define TESTSIZE                12  // test size for font

#define IDX_WINDOWPTR           4   // index of WindowPtrs (QWL_USER + 4)

const SIZEL sizl = {200, 200};      // presentation space size for analog clock

const MATRIXLF vmatlfTrans =
         {
         MAKEFIXED (1, 0), MAKEFIXED (0, 0), 0L,
         MAKEFIXED (0, 0), MAKEFIXED (1, 0), 0L,
         0L,               0L,               1L
         };
const MATRIXLF vmatlfScale =
         {
         MAKEFIXED (1, 0), MAKEFIXED (0, 0), 0L,
         MAKEFIXED (0, 0), MAKEFIXED (1, 0), 0L,
         0L ,              0L,               1L
         };

/*******************************************************************\
    ClkWndProc: window procedure for the clock
    Input:   hwnd: window handle
             msg:  message type
             mp1:  parameter 1
             mp2:  parameter 2
\*******************************************************************/
MRESULT EXPENTRY ClkWndProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
    {
    WPDCF77     *somSelf;
    WPDCF77Data *somThis;
    PWINDOWDATA pWindowData;
    POINTL      ptl;
    SWP         swp;
    CHAR        szSubst[CCHMAXDATETIME];

    switch (msg)
        {
        case WM_CREATE:
            pWindowData = (PWINDOWDATA) mp1;
            ClkCreate (hwnd, pWindowData);
            return FALSE;

        case WM_1STIMER:
            ClkTimer (hwnd);
            return 0;

        case WM_BUTTON2CLICK:
            pWindowData = (PWINDOWDATA)WinQueryWindowPtr (hwnd, QWL_USER);
            somThis = pWindowData->somThis;
            somSelf = pWindowData->somSelf;
            WinQueryPointerPos (HWND_DESKTOP, &ptl);
            WinMapWindowPoints (HWND_DESKTOP, hwnd, &ptl, 1);
            _wpDisplayMenu (somSelf, _hwndFrame, _hwndFrame, &ptl, MENU_OPENVIEWPOPUP, 0);
            break;

        case WM_PAINT:
            ClkPaint (hwnd, (PWINDOWDATA)WinQueryWindowPtr (hwnd, QWL_USER));
            return 0;

        /* WM_MINMAXFRAME is used for controlling the icon position.                  */
        /* On default OS/2 controls the icon positions separately from window         */
        /* positions.                                                                 */
        /* The following messages will be handled: WM_MINMAXFRAME - WM_MOVE - WM_SIZE */
        /* (some receive multiple times)                                              */
        /* If in _ulPresence DISPLAY_ICONPOS is set, the following happens:           */
        /* - WM_MINMAXFRAME: On change Icon <-> Normal in ulMinMaxStat                */
        /*                   is set to MINMAX_CHANGED                                 */
        /* - WM_MOVE:        ulMinMaxStat == MINMAX_NORMAL: save position             */
        /*                   otherwise: ulMinMaxStat = MINMAX_WAIT4SIZE               */
        /* - WM_SIZE:        ulMinMaxStat == MINMAX_WAIT4SIZE: re-set position,       */
        /*                   ulMinMaxStat = MINMAX_NORMAL                             */
        case WM_MINMAXFRAME:
            pWindowData = (PWINDOWDATA)WinQueryWindowPtr (hwnd, QWL_USER);
            somThis = pWindowData->somThis;
            if (_ulPresence & PRESENCE_MINMAXPOS)
                {
                _ulMinMaxStat = MINMAX_CHANGED;
                DebugE (D_MINMAX, "ClkWndProc", "WM_MINMAXFRAME: MINMAX_CHANGED");
                }
            break;

        case WM_PLAY_MMFILE:
            pWindowData = (PWINDOWDATA)WinQueryWindowPtr (hwnd, QWL_USER);
            somThis = pWindowData->somThis;
            DebugE (D_ALARM, "ClkWndProc", "play multimedia file");

            if (_szAlarmSoundFile[0] != '\0' && bIsAudio)
                {
                BOOL bRC;
                bRC = ClkMciPlayFile (_hwndClient, _szAlarmSoundFile);
                DebugULx (D_ALARM, "ClkWndProc", "return for mciPlayFile: ", (bRC ? "TRUE" : "FALSE"));
                }
            return 0;

        case WM_SIZE:
            ClkSize (hwnd);
            return 0;

        case WM_MOVE:
            pWindowData = (PWINDOWDATA)WinQueryWindowPtr (hwnd, QWL_USER);
            somThis = pWindowData->somThis;

            WinQueryWindowPos (_hwndFrame, &swp);
            if (!(swp.fl & SWP_MINIMIZE))
                {
                _lcxClock = swp.cx;
                _lcyClock = swp.cy;
                }

            /* Change Icon<->Normal? -> special handling; see WM_MINMAXFRAME */
            if (_ulMinMaxStat == MINMAX_NORMAL)
                {
                if (!(swp.fl & SWP_MINIMIZE))
                    {
                    _lxPos = swp.x;
                    _lyPos = swp.y;
                    DebugE (D_MINMAX, "ClkWndProc", "WM_MOVE: Position saved");
                    }
                else if (_ulPresence & PRESENCE_MINMAXPOS)
                    {
                    _lxIconPos = swp.x;
                    _lyIconPos = swp.y;
                    DebugE (D_MINMAX, "ClkWndProc", "WM_MOVE: Icon-Position saved");
                    }
                }
            else
                _ulMinMaxStat = MINMAX_WAIT4SIZE;

            _wpSaveDeferred (pWindowData->somSelf);
            return 0;

        case WM_BUTTON1DOWN:
            pWindowData = (PWINDOWDATA)WinQueryWindowPtr (hwnd, QWL_USER);
            somThis = pWindowData->somThis;
            if (WinIsChild (_hwndTitleBar, HWND_OBJECT))
                return WinSendMsg (_hwndFrame, WM_TRACKFRAME,
                                   MPFROMSHORT((USHORT)mp2 | TF_MOVE), MPVOID);
            else
                break;

        case WM_MESSAGE:
            pWindowData = (PWINDOWDATA)WinQueryWindowPtr (hwnd, QWL_USER);
            somThis = pWindowData->somThis;
            MessageBox (hwnd, (ULONG)mp1, IDMSG_ALARMTITLE, (ULONG)mp2, 0, *_szAlarmMsg == '\0' ? NULL : _szAlarmMsg);
            return 0;

        case WM_SUBSTITUTESTRING:
            pWindowData = (PWINDOWDATA)WinQueryWindowPtr (hwnd, QWL_USER);
            somThis = pWindowData->somThis;
            switch (SHORT1FROMMP (mp1))
                {
                case 0:                     // parameter %t: time
                    GetDateTime (&_dt, MODE_PM | MODE_YEAR_2D, szSubst, NULL);
                    return szSubst;

                case 1:                     // parameter %d: date
                    GetDateTime (&_dt, MODE_PM | MODE_YEAR_2D, NULL, szSubst);
                    return szSubst;

                case 2:                     // parameter %z: time zone
                    return _szTimezone;
                }
            return 0;

        case WM_PRESPARAMCHANGED:
            ClkPresparam (hwnd, (ULONG)LONGFROMMP (mp1));
            return 0;

        /* WM_USER_STARTPLAY is used for initialization of audio output      */
        /* mp1: SHORT1: number of play actions                               */
        /*      SHORT2: reserved                                             */
        /* mp2: reserved                                                     */
        case WM_USER_STARTPLAY:
            pWindowData = (PWINDOWDATA)WinQueryWindowPtr (hwnd, QWL_USER);
            ClkStartAudioPlay (hwnd, pWindowData, LONGFROMMP (mp1));
            return 0;

        /* MM_MCIPASSDEVICE informs the system about a change in ownership   */
        /* of a commonly used multimedia device                              */
        /* mp1: USHORT1: usDeviceID                                          */
        /*      USHORT2: reserved                                            */
        /* mp2: USHORT1: usEvent                                             */
        /*      USHORT2: reserved                                            */
        case MM_MCIPASSDEVICE:
            ClkMciPassDevice (hwnd, IDX_WINDOWPTR, SHORT1FROMMP (mp2));
            break;

        /* WM_ACTIVATE informs the system about a change of the window focus */
        /* This is used to acquire the ownership of the multimedia system    */
        /* via MM_ACQUIREDEVICE .                                            */
        /* mp1: USHORT1: usactive                                            */
        /*      USHORT2: reserved                                            */
        /* mp2: hwnd                                                         */
        case WM_ACTIVATE:
            ClkMciActivate (hwnd, IDX_WINDOWPTR, (BOOL)mp1);
            break;

        /* WM_PLAY_FINISHED informs the system about a finished play of an   */
        /* audio file.                                                       */
        case WM_PLAY_FINISHED:
            ClkMciCheckRedo (hwnd, IDX_WINDOWPTR);
            return 0;

        case WM_DESTROY:
            ClkDestroy ((PWINDOWDATA)WinQueryWindowPtr (hwnd, QWL_USER));
            return 0;

        case WM_CLOSE:
            pWindowData = (PWINDOWDATA)WinQueryWindowPtr (hwnd, QWL_USER);
            somThis = pWindowData->somThis;

            if (pWindowData == NULL)
                {
                DebugE (D_WND | D_DIAG, "ClkWndProc", "WM_CLOSE: couldn't get window words");
                return FALSE;
                }

            /* Alarm anhalten */
            DebugE (D_ALARM, "ClkWndProc", "Alarm STOPPED");
            AlarmStart (somThis, stop);
            _wpDeleteFromObjUseList (pWindowData->somSelf, &pWindowData->UseItem);
            _wpRemove1sTimer (pWindowData->somSelf, hwnd);

            ClkMciClose (hwnd, IDX_WINDOWPTR);

            DebugULx (D_WND, "WM_CLOSE", "hwndFrame", _hwndFrame);
            WinDestroyWindow (_hwndFrame);
            _wpFreeMem (pWindowData->somSelf, (PBYTE)pWindowData);
            return 0;
        }

    return WinDefWindowProc (hwnd, msg, mp1, mp2);
    }

/*******************************************************************\
    ClkCreate: Handle WM_CREATE message: create presentation spaces,
               initialize variable 
    Input:   hwnd:        window handle
             pWindowData: pointer to window data
\*******************************************************************/
VOID ClkCreate (HWND hwnd, PWINDOWDATA pWindowData)
    {
    LONG lxTemp, lyTemp;
    WPDCF77Data *somThis;

    /* initialize access to instance data */
    somThis = pWindowData->somThis;

    /* Class view/usage details and window specific data */
    pWindowData->UseItem.type    = USAGE_OPENVIEW;
    pWindowData->ViewItem.view   = (_ulPresence & PRESENCE_DISP_DIGITAL) ? OPEN_DIGITAL : OPEN_ANALOG;
    pWindowData->ViewItem.handle = _hwndFrame;

    /* fill Window-Ptr with pWindowData and register view */
    WinSetWindowPtr (hwnd, QWL_USER, pWindowData);
    _wpAddToObjUseList (pWindowData->somSelf, &pWindowData->UseItem);
    _wpRegisterView (pWindowData->somSelf, _hwndFrame,
                     _ulPresence & PRESENCE_DISP_DIGITAL ? "Digital" : "Analog");
    WinSetFocus (HWND_DESKTOP, _hwndFrame);

    /* initialize audio */
    WinSetWindowPtr (hwnd, QWL_USER + IDX_WINDOWPTR, NULL);
    bIsAudio = ClkMciIsMultimedia () ? TRUE : FALSE;
    DebugE (D_WND, "ClkCreate => bIsAudio", (bIsAudio ? "TRUE" : "FALSE"));

    /* delete icon to paint a "living" icon */
    WinSendMsg (_hwndFrame, WM_SETICON, MPVOID, MPVOID);

    _vmatlfDateTrans = vmatlfTrans;
    _vmatlfDateScale = vmatlfScale;

    _hwndTitleBar = WinWindowFromID (_hwndFrame, FID_TITLEBAR);
    _hwndSysMenu  = WinWindowFromID (_hwndFrame, FID_SYSMENU);
    _hwndMinMax   = WinWindowFromID (_hwndFrame, FID_MINMAX);

    if (pWindowData->ViewItem.view == OPEN_ANALOG)
        {
        /* determine screen size; open device context */
        /* create presentation space                  */
        _hdc = WinOpenWindowDC (hwnd);
        _hps = GpiCreatePS (hab, _hdc, (PSIZEL)&sizl,
                            PU_ARBITRARY | GPIT_MICRO | GPIA_ASSOC);

        /* create own off-screen 'buffer' */
        _hdcBuffer = DevOpenDC ((HAB)0L, OD_MEMORY, "*", 0L, NULL, _hdc);
        _hpsBuffer = GpiCreatePS (hab, _hdcBuffer, (PSIZEL)&sizl,
                                  PU_ARBITRARY | GPIT_MICRO | GPIA_ASSOC);

        /* switch system to RGB mode */
        GpiCreateLogColorTable (_hpsBuffer, 0, LCOLF_RGB, 0, 0, NULL);

        /* determine resolution, to draw a round clock face */
        DevQueryCaps (_hdc, CAPS_HORIZONTAL_RESOLUTION, 1, (PLONG)&pWindowData->cxRes);
        DevQueryCaps (_hdc, CAPS_VERTICAL_RESOLUTION,   1, (PLONG)&pWindowData->cyRes);
        DevQueryCaps (_hdc, CAPS_COLOR_PLANES,          1, (PLONG)&pWindowData->cColorPlanes);
        DevQueryCaps (_hdc, CAPS_COLOR_BITCOUNT,        1, (PLONG)&pWindowData->cColorBitcount);
        }
    else
        {
        _hdc = WinOpenWindowDC (hwnd);
        _hps = GpiCreatePS (hab, _hdc, (PSIZEL)&sizl,
                            PU_PELS | GPIT_MICRO | GPIA_ASSOC);
        }

    ClkSetRGBColors (somThis);
    ClkSetFonts (somThis);

    /* get date and time */
    GetDateTimeOffs (_lLOffset, &_dt);
    _flRcvrStatus     = GetDCFStatus ();
    _flRcvrStatusMask = StatDCF.flMaskST & (MASK_ST_TZONE1 | MASK_ST_BATT);

    /* position window and make visible */
    if (_ulPresence & PRESENCE_DISP_MINIMIZE)
        {
        lxTemp = _lxIconPos;
        lyTemp = _lyIconPos;
        }
    else
        {
        lxTemp = _lxPos;
        lyTemp = _lyPos;
        }

    WinSetWindowPos (_hwndFrame, NULLHANDLE,
        lxTemp, lyTemp, _lcxClock, _lcyClock,
        SWP_ACTIVATE | SWP_SIZE | SWP_MOVE | (_ulPresence & PRESENCE_DISP_MINIMIZE ? SWP_MINIMIZE : 0));

    if (_ulPresence & (PRESENCE_MINMAXPOS | PRESENCE_DISP_MINIMIZE))
        WinSetWindowPos (_hwndFrame, NULLHANDLE,
                         lxTemp, lyTemp, _lcxClock, _lcyClock,
                         SWP_MOVE);

    /* show title bar? */
    if (!(_ulPresence & PRESENCE_TITLEBAR))
        ClkShowFrameControls (_hwndFrame, FALSE);

    WinShowWindow (_hwndFrame, TRUE);

    /* start timer */
    _wpAdd1sTimer (pWindowData->somSelf, hwnd);

    return;
    }

/*******************************************************************\
    ClkDestroy: Handle WM_DESTROY message:
                free presentation spaces
    Input: pWindowData: pointer to window data
\*******************************************************************/
VOID ClkDestroy (PWINDOWDATA pWindowData)
    {
    WPDCF77Data *somThis;
    HBITMAP     hbm;

    /* initialize access to instance data */
    somThis = pWindowData->somThis;

    if (pWindowData->ViewItem.view == OPEN_ANALOG)
        {
        hbm = GpiSetBitmap (_hpsBuffer, NULLHANDLE);

        if (hbm != NULLHANDLE)
            GpiDeleteBitmap (hbm);
        }

    GpiDestroyPS (_hpsBuffer);
    DevCloseDC   (_hdcBuffer);
    _hpsBuffer = NULLHANDLE;
    _hdcBuffer = NULLHANDLE;

    GpiDestroyPS (_hps);
    DevCloseDC   (_hdc);
    _hps = NULLHANDLE;
    _hdc = NULLHANDLE;

    return;
    }

/*******************************************************************\
    ClkSize: Handle WM_SIZE message:
             Calculate page viewports, determine position of 
             date field
    Input: hwnd: window handle
\*******************************************************************/
VOID ClkSize (HWND hwnd)
    {
    LONG             cxSquare, cySquare, cxEdge, cyEdge, cxWindow, cyWindow;
    RECTL            rclWindow;
    RECTL            rclPage;               // size of page viewports
    SWP              swp;
    HBITMAP          hbm;
    BITMAPINFOHEADER bmp;
    PWINDOWDATA      pWindowData;
    WPDCF77Data      *somThis;

    /* initialize access to instance data */
    pWindowData = (PWINDOWDATA)WinQueryWindowPtr (hwnd, QWL_USER);
    somThis = pWindowData->somThis;

    /* is clock iconized? */
    WinQueryWindowPos (_hwndFrame, &swp);
    if (swp.fl & SWP_MINIMIZE)
        _ulPresence |= PRESENCE_DISP_MINIMIZE;
    else
        {
        _ulPresence &= ~PRESENCE_DISP_MINIMIZE;
        _lcxClock = swp.cx;
        _lcyClock = swp.cy;
        }

    /* change Icon<->Normal? -> special handling; see WM_MINMAXFRAME */
    if (_ulMinMaxStat == MINMAX_WAIT4SIZE)
        {
        if (_ulPresence & PRESENCE_DISP_MINIMIZE)
            {
            swp.x = _lxIconPos;
            swp.y = _lyIconPos;
            }
        else
            {
            swp.x = _lxPos;
            swp.y = _lyPos;
            }
        WinSetWindowPos (_hwndFrame, NULLHANDLE,
            swp.x, swp.y, _lcxClock, _lcyClock,
            SWP_MOVE);
        _ulMinMaxStat = MINMAX_NORMAL;
        DebugE (D_MINMAX, "ClkWndProc", "WM_SIZE: Position set");
        }
    else
        {
        if (swp.fl & SWP_MINIMIZE)
            {
            _lxIconPos = swp.x;
            _lyIconPos = swp.y;
            DebugE (D_MINMAX, "ClkWndProc", "WM_SIZE: Icon-Position saved");
            }
        else
            {
            _lxPos    = swp.x;
            _lyPos    = swp.y;
            DebugE (D_MINMAX, "ClkWndProc", "WM_SIZE: Position saved");
            }
        }

    if (pWindowData->ViewItem.view == OPEN_ANALOG)
        {
        /* delete present bitmaps in buffer, if any */
        hbm = GpiSetBitmap (_hpsBuffer, NULLHANDLE);
        if (hbm != NULLHANDLE)
            GpiDeleteBitmap (hbm);

        /* determine size of client window */
        WinQueryWindowRect (hwnd, &rclWindow);
        cxWindow = rclWindow.xRight - rclWindow.xLeft;
        cyWindow = rclWindow.yTop - rclWindow.yBottom;

        /* create bitmap in window size */
        bmp.cbFix     = sizeof(BITMAPINFOHEADER);
        bmp.cx        = (SHORT)cxWindow;
        bmp.cy        = (SHORT)cyWindow;
        bmp.cPlanes   = (SHORT)pWindowData->cColorPlanes;
        bmp.cBitCount = (SHORT)pWindowData->cColorBitcount;
        hbm = GpiCreateBitmap(_hpsBuffer, (PBITMAPINFOHEADER2)&bmp, 0, NULL, NULL);
        GpiSetBitmap (_hpsBuffer, hbm);

        /* Assumption: Rectangle is limited by y-direction */
        /* -> calculate cx to achieve square rectangle     */
        /* if wrong assumption: converse action            */
        cySquare = cyWindow - 4;
        cxSquare = (cyWindow * pWindowData->cxRes) / pWindowData->cyRes;
        if (cxWindow < cxSquare)
            {
            cxSquare = cxWindow - 4;
            cySquare = (cxWindow * pWindowData->cyRes) / pWindowData->cxRes;
            }

        /* determine remaining area and set page viewport */
        cxEdge = (cxWindow - cxSquare) / 2;
        cyEdge = (cyWindow - cySquare) / 2;
        rclPage.xLeft   = cxEdge;
        rclPage.xRight  = cxWindow - cxEdge;
        rclPage.yBottom = cyEdge;
        rclPage.yTop    = cyWindow - cyEdge;

        /* determine position of date; priority sequence: */
        /* 1. Below clock                                 */
        /* 2. Left of clock                               */
        /* 3. Inside clock                                */
        if (cyWindow > (cySquare*6/5))          /* Below clock */
            {
            /* Matrix:  2.5    0      0  */
            /*          0      2.5    0  */
            /*         15    -17      1  */
            _vmatlfDateTrans.lM31  = 15;        /* horizontal */
            _vmatlfDateTrans.lM32  = -17;       /* vertical   */
            _vmatlfDateScale.fxM11 = MAKEFIXED (2, 0x8000);
            _vmatlfDateScale.fxM22 = MAKEFIXED (2, 0x8000);
            rclPage.yTop    += cyEdge;
            rclPage.yBottom += cyEdge;
            }
        else if (cxWindow > (cxSquare * 31/10))
            {
            /* Matrix:  7.875  0      0  */
            /*          0      7.875  0  */
            /*        -53      4      1  */
            _vmatlfDateTrans.lM31  = -53 ;      /* horizontal */
            _vmatlfDateTrans.lM32  = 4;         /* vertical   */
            _vmatlfDateScale.fxM11 = MAKEFIXED (7, 0xE000);
            _vmatlfDateScale.fxM22 = MAKEFIXED (7, 0xE000);
            rclPage.xRight += cxEdge;
            rclPage.xLeft  += cxEdge;
            }
        else if (cxWindow > (cxSquare * 2))     /* Left */
            {
            /* Matrix:  3.8125 0      0  */
            /*          0      7.8725 0  */
            /*        -52      2      1  */
            _vmatlfDateTrans.lM31  = -52;       /* horizontal */
            _vmatlfDateTrans.lM32  = 2;         /* vertical   */
            _vmatlfDateScale.fxM11 = MAKEFIXED (3, 0xD000);
            _vmatlfDateScale.fxM22 = MAKEFIXED (3, 0xD000);
            rclPage.xRight += cxEdge;
            rclPage.xLeft  += cxEdge;
            }
        else                                    /* Inside */
            {
            _vmatlfDateTrans.lM31  = 24;        /* horizontal */
            _vmatlfDateTrans.lM32  = 23;        /* vertical   */
            _vmatlfDateScale.fxM11 = MAKEFIXED (2, 0);
            _vmatlfDateScale.fxM22 = MAKEFIXED (2, 0);
            }

        GpiSetPageViewport (_hps,       &rclPage);
        GpiSetPageViewport (_hpsBuffer, &rclPage);
        }

    _bBufferDirty = TRUE;

    _wpSaveDeferred (pWindowData->somSelf);

    return;
    }

/*******************************************************************\
    ClkTimer: Handle WM_1STIMER message:
    Input: hwnd: window handle
\*******************************************************************/
VOID ClkTimer (HWND hwnd)
    {
    PWINDOWDATA  pWindowData;
    WPDCF77      *somSelf;
    WPDCF77Data  *somThis;
    DATETIME     dtNew;
    USHORT       flRcvrStatusNew;
    USHORT       flRcvrStatusMaskNew;
    ULONG        ulTime;
    BOOL         bUpdate = FALSE;

    /* initialize access to instance data */
    pWindowData = (PWINDOWDATA)WinQueryWindowPtr (hwnd, QWL_USER);
    somSelf     = pWindowData->somSelf;
    somThis     = pWindowData->somThis;

    /* get new time */
    GetDateTimeOffs (_lLOffset, &dtNew);

    if (pWindowData->ViewItem.view == OPEN_ANALOG)
        {
        /* get new DCF state */
        flRcvrStatusNew     = GetDCFStatus ();
        flRcvrStatusMaskNew = StatDCF.flMaskST & (MASK_ST_TZONE1 | MASK_ST_BATT);

        if ((_dt.minutes ^ dtNew.minutes) + (_dt.hours ^ dtNew.hours) +
            (_dt.day     ^ dtNew.day)     + (_dt.month ^ dtNew.month) +
            (_dt.year    ^ dtNew.year))
            {
            /* Redraw, as minute has been changed */
            ClkDrawClock (_hpsBuffer, DCLK_HANDS, somThis, &dtNew, flRcvrStatusNew);
            ClkUpdateScreen (_hps, somThis, NULL);
            bUpdate = TRUE;

            if (_ulPresence & PRESENCE_DISP_SEC_HAND && !(_ulPresence & PRESENCE_DISP_MINIMIZE))
                {
                GpiSetMix (_hps, FM_INVERT);
                ClkDrawHand (_hps, somThis, HT_SECOND, dtNew.seconds);
                }
            }

        /* start audio output, if installed and activated */
        if (dtNew.minutes == 0 && dtNew.seconds == 0 && bIsAudio)
            WinPostMsg (hwnd, WM_USER_STARTPLAY, MPFROMSHORT (dtNew.hours), 0);

        // ??? clock is drawn twice on minute change????
        if (flRcvrStatusNew != _flRcvrStatus || flRcvrStatusMaskNew != _flRcvrStatusMask)
            {
            if ((flRcvrStatusNew ^ _flRcvrStatus) &
                (STATUS_TZONE1 | STATUS_TZONE2    | STATUS_CARRIER |
                 STATUS_BATT   | STATUS_TIMEVALID | STATUS_ISDRIVER) ||
                flRcvrStatusMaskNew != _flRcvrStatusMask)
                {
                /* draw status display */
                ClkDrawClock (_hpsBuffer, DCLK_ALL, somThis, &dtNew, flRcvrStatusNew);
                ClkUpdateScreen (_hps, somThis, NULL);
                bUpdate = TRUE;
                _flRcvrStatus     = flRcvrStatusNew;
                _flRcvrStatusMask = flRcvrStatusMaskNew;

                if (_ulPresence & PRESENCE_DISP_SEC_HAND && !(_ulPresence & PRESENCE_DISP_MINIMIZE))
                    {
                    GpiSetMix (_hps, FM_INVERT);
                    ClkDrawHand (_hps, somThis, HT_SECOND, dtNew.seconds);
                    }
                }
            }

        if (_ulPresence & PRESENCE_DISP_SEC_HAND && !(_ulPresence & PRESENCE_DISP_MINIMIZE) && !bUpdate)
            {
            /* otherwise just delete and redraw second hand */
            GpiSetMix (_hps, FM_INVERT);
            ClkDrawHand (_hps, somThis, HT_SECOND, _dt.seconds);
            ClkDrawHand (_hps, somThis, HT_SECOND, dtNew.seconds);
            }

        }
    else
        {
        ClkDrawDigitalTime (hwnd, &dtNew,
            (_dt.minutes ^ dtNew.minutes) + (_dt.hours ^ dtNew.hours) +
            (_dt.day     ^ dtNew.day)     + (_dt.month ^ dtNew.month) +
            (_dt.year    ^ dtNew.year) ?
            DCLK_ALL : DCLK_HANDS);
        }

    _dt = dtNew;

    if (!_ulTimeChangeCheck)
        /* initialize on first pass */
        _ulTimeChangeCheck = (ULONG)GetTimeLowOffs (_lLOffset);
    else
        {
        /* Quick check, if another session has change the time */
        ulTime = (ULONG)GetTimeLowOffs (_lLOffset);
        if (ulTime > (_ulTimeChangeCheck + 60) ||
            _ulTimeChangeCheck > (ulTime + 60))
            {
            /* time has been changed for more than 60 seconds */
            DebugE (D_ALARM, "ClkTimer", "TimeChange");
            if (!AlarmSetTimer (somThis, &dtNew))
                _wpSaveDeferred (somSelf);
            _wpRefreshClockView (somSelf);          // face only is sufficient! ???
            }
        _ulTimeChangeCheck = ulTime;
        }

    return;
    }

/*******************************************************************\
    ClkStartAudioPlay: Calculates number of gongs depending on 
                       time and plays the corresponding WAV file,
                       if possible and necessary
    Input: hwnd:    window handle
           ulHours: hours of time
\*******************************************************************/
VOID ClkStartAudioPlay (HWND hwnd, PWINDOWDATA pWindowData, ULONG ulHours)
    {
    WPDCF77     *somSelf;
    HOBJECT      hObject;

    somSelf = pWindowData->somSelf;
    hObject = _wpQueryHandle (somSelf);

    if ((hObject == hClsRingingObject) && (szClsGongFile[0] != '\0'))
        {
        if (bClsIs1PerHour)
            ulHours = 1;
        else
            {
            ulHours = ulHours % 12;
            ulHours = ulHours ? ulHours : 12;
            }

        if (ClkMciLoadFile (hwnd, IDX_WINDOWPTR, szClsGongFile, ulHours))
            ClkMciStartPlayBack (hwnd, IDX_WINDOWPTR, WM_PLAY_FINISHED);
        }

    return;
    }

/*******************************************************************\
    ClkPresparam: Handles changes of presentation parameters
                  PP_BACKGROUNDCOLOR and PP_FONTNAMESIZE
    Input: hwnd:   window handle
           ulType: PresParam type: PP_*
\*******************************************************************/
VOID ClkPresparam (HWND hwnd, ULONG ulType)
    {
    PWINDOWDATA pWindowData;
    WPDCF77     *somSelf;
    WPDCF77Data *somThis;
    RGB         rgb;
    LONG        lCount;
    PBYTE       pBuffer;

    /* Initialize access to instance data */
    pWindowData = (PWINDOWDATA)WinQueryWindowPtr (hwnd, QWL_USER);
    somSelf     = pWindowData->somSelf;
    somThis     = pWindowData->somThis;

    switch (ulType)
        {
        case PP_BACKGROUNDCOLOR:
            WinQueryPresParam (hwnd, ulType, 0, NULL, sizeof (RGB), (PVOID)&rgb,
                QPF_PURERGBCOLOR);

            _clrFace    = (rgb.bRed<<16) + (rgb.bGreen<<8) + rgb.bBlue;
            _clrBackgnd = _clrDateOnAnalog = _clrFace;
            DebugULx (D_DRAG, "ClkWndProc", "RGB-value", _clrFace);
            break;

        case PP_FONTNAMESIZE:
            if (!DosAllocMem ((PPVOID)&pBuffer, CCHMAXFONTSIZENAME + sizeof (FONTMETRICS),
                             PAG_COMMIT | PAG_READ | PAG_WRITE))
                {
                WinQueryPresParam (hwnd, ulType, 0, NULL, CCHMAXFONTSIZENAME, (PVOID)pBuffer, 0);

                lCount = 1;
                GpiQueryFonts (_hpsBuffer, QF_PUBLIC, strchr (pBuffer, '.')+1, &lCount,
                    sizeof (FONTMETRICS), (PFONTMETRICS)&pBuffer[CCHMAXFONTSIZENAME]);

                DebugE (D_DRAG, "ClkWndProc => szFacename",   ((PFONTMETRICS)(pBuffer+CCHMAXFONTSIZENAME))->szFacename);
                DebugE (D_DRAG, "ClkWndProc => szFamilyname", ((PFONTMETRICS)(pBuffer+CCHMAXFONTSIZENAME))->szFamilyname);

                if (lCount == 1 && ((PFONTMETRICS)(pBuffer+CCHMAXFONTSIZENAME))->fsDefn & FM_DEFN_OUTLINE)
                    {
                    memset (&_fAttrD, 0, sizeof (FATTRS));
                    memset (&_fAttrT, 0, sizeof (FATTRS));
                    _fAttrD.usRecordLength = _fAttrT.usRecordLength = sizeof (FATTRS);
                    _fAttrD.fsFontUse      = _fAttrT.fsFontUse      = 0;
                    _fAttrD.fsType         = _fAttrT.fsType         = FATTR_FONTUSE_OUTLINE;
                    strcpy (_fAttrD.szFacename, ((PFONTMETRICS)(pBuffer+CCHMAXFONTSIZENAME))->szFacename);
                    strcpy (_fAttrT.szFacename, ((PFONTMETRICS)(pBuffer+CCHMAXFONTSIZENAME))->szFacename);
                    strcpy (_szFontFamilyD,     ((PFONTMETRICS)(pBuffer+CCHMAXFONTSIZENAME))->szFamilyname);
                    strcpy (_szFontFamilyT,     ((PFONTMETRICS)(pBuffer+CCHMAXFONTSIZENAME))->szFamilyname);
                    }
                else
                    {
                    WinAlarm (HWND_DESKTOP, WA_ERROR);
                    return;
                    }

                DosFreeMem (pBuffer);
                }
            break;
        }

    _wpRefreshClockView (somSelf);
    _wpSaveDeferred (somSelf);
    return;
    }

/*******************************************************************\
    ClkDrawString: Output of a string into the rectable prcl.
                   The characters are transformed in a way to fit
                   the complete string into the rectangle.
    Input: hps:       presentation space
           ulFGColor: foreground color
           pszString: string
           prcl:      pointer to RECTL structure
\*******************************************************************/
VOID ClkDrawString (HPS hps, ULONG ulFGColor, PCHAR pszString, PRECTL prcl)
    {
    USHORT      cx, cy;
    ULONG       ulc;
    SIZEF       sizef;
    POINTL      ptl;
    POINTL      aptlCharPos[12];                    // buffer for draw positions
    FONTMETRICS fm;

    /* set color and text mode */
    GpiSetColor (hps, ulFGColor);
    GpiSetCharMode (hps, CM_MODE3);

    /* Preliminary size of character box */
    sizef.cx = sizef.cy = MAKEFIXED (TESTSIZE, 0);  // set drawing aspect to nominal value 1:1, set size to 12
    GpiSetCharBox (hps, &sizef);

    /* CALCULATION OF CHARACTER WIDTH:                                              */
    /* Thereto the width of the complete string in calculated based on a            */
    /* character cx x cy (180/Pi = 57.3):                                           */
    /*                   l = Xende - Xanfang + tan(sl)*dy                           */
    /*    Approximation: l = Xende - Xanfang + sl*lMaxAscender/57.3                 */
    /* New character size is then x = TESTSIZE * cx/l                               */
    /* Abbreviations: sl:  character angle for italic                               */
    /*                cx:  target string length                                     */
    /*                dy:  empirically determined hight (about lMaxAscender * 1.4)  */
    /* CALCULATION OF CHARACTER HEIGHT:                                             */
    /* The height of a character box is determined via lMaxAscender and an          */
    /* empirical constant (14): cy = cy*14/lMaxAscender                             */
    cx = LOUSHORT (prcl->xRight - prcl->xLeft);
    cy = LOUSHORT (prcl->yTop   - prcl->yBottom);

    ptl.x = prcl->xLeft;
    ptl.y = prcl->yBottom;

    ulc = strlen (pszString);
    GpiQueryCharStringPosAt (hps, &ptl, 0, strlen (pszString), pszString, NULL, aptlCharPos);
    GpiQueryFontMetrics (hps, sizeof (FONTMETRICS), &fm);

    sizef.cy = (FIXED)(65536. * ((double)(cy * 14)/(double)fm.lMaxAscender));
    sizef.cx = (FIXED)(65536 * (double)TESTSIZE * (double)cx /
        ((double)(aptlCharPos[ulc].x - aptlCharPos[0].x) +
        (double)fm.lMaxAscender * (double)(fm.sCharSlope&0x1FF) / 81.86));

    /* draw string */
    GpiSetCharBox (hps, &sizef);
    GpiCharStringAt (hps, &ptl, strlen (pszString), pszString);

    return;
    }

/*******************************************************************\
    ClkDrawDigitalTime: draw digital clock
    Input: hwnd:       window handle
           pdt:        DATETIME structure
           ulPresence: presentation flags
           ulMode:     mode: DCLK_ALL or DCLK_HANDS
\*******************************************************************/
VOID ClkDrawDigitalTime (HWND hwnd, PDATETIME pdt, ULONG ulMode)
    {
    RECTL  rcl;
    RECTL  rclTime;
    RECTL  rclDate;
    CHAR   achTime[10];
    CHAR   achFinalDate[12];
    WPDCF77Data *somThis;

    somThis = ((PWINDOWDATA)WinQueryWindowPtr (hwnd, QWL_USER))->somThis;

    GetDateTime (pdt, MODE_PM | MODE_YEAR_2D, achTime, achFinalDate);

    WinQueryWindowRect (hwnd, &rcl);
    if (!(_ulPresence & PRESENCE_DISP_MINIMIZE) || (ulMode == DCLK_ALL))
        WinFillRect (_hps, &rcl, _sclrBG.ulSurface);

    rcl.yBottom += (rcl.yTop - rcl.yBottom)/10;
    rcl.yTop    -= (rcl.yTop - rcl.yBottom)/20;
    rcl.xLeft   += (rcl.xRight - rcl.xLeft)/20;
    rcl.xRight  -= (rcl.xRight - rcl.xLeft)/20;

    switch (_ulPresence & (PRESENCE_DISP_TIME | PRESENCE_DISP_DATE))
        {
        case PRESENCE_DISP_DATE:
            rclDate = rcl;
            break;

        case PRESENCE_DISP_TIME | PRESENCE_DISP_DATE:
            if (!(_ulPresence & PRESENCE_DISP_MINIMIZE))
                {
                rclTime = rclDate = rcl;
                rclTime.yBottom = rclDate.yTop = (rcl.yTop + rcl.yBottom) / 2;
                rclTime.yBottom += (rclTime.yTop - rclTime.yBottom)/10;
                rclDate.yTop    -= (rclTime.yTop - rclTime.yBottom)/10;
                break;
                } /* else: continue to next case statement */

        case PRESENCE_DISP_TIME:
            rclTime = rcl;
            break;
        }

    if (_ulPresence & PRESENCE_DISP_MINIMIZE)
        {
        /* Iconized: Refresh on new minute */
        if (ulMode == DCLK_ALL)
            {
            GpiSetCharSet (_hps, LCID_TIME);
            achTime[5] = '\0';
            ClkDrawString (_hps, _ulclrTimeText, achTime, &rcl);
            }
        }
    else
        {
        if (_ulPresence & PRESENCE_DISP_TIME)
            {
            GpiSetCharSet (_hps, LCID_TIME);
            ClkDrawString (_hps, _ulclrTimeText, achTime, &rclTime);
            }
        }

    if (_ulPresence & PRESENCE_DISP_DATE && !(_ulPresence & PRESENCE_DISP_MINIMIZE))
        {
        GpiSetCharSet (_hps, LCID_DATE);
        ClkDrawString (_hps, _ulclrDateText, achFinalDate, &rclDate);
        }

    return;
    }

/*******************************************************************\
    ClkRefresh: Initiates a redraw of the clock
    Input: somThis: pointer to instance data
\*******************************************************************/
VOID ClkRefresh (WPDCF77Data *somThis)
    {
    _bBufferDirty = TRUE;
    ClkSetRGBColors (somThis);
    ClkSetFonts (somThis);
    WinInvalidateRect (_hwndClient, NULL, TRUE);
    return;
    }

/*******************************************************************\
    ClkShowFrameControls: hide/show switch title bar 
    Input: hwndFrame: window handle of frame window
           bShow:     TRUE:  show title bar
                      FALSE: hide title bar
\*******************************************************************/
VOID ClkShowFrameControls (HWND hwndFrame, BOOL bShow)
    {
    WPDCF77     *somSelf;
    WPDCF77Data *somThis;
    HWND        hwnd;

    somSelf = (WPDCF77 *)WinQueryWindowPtr (hwndFrame, QWL_USER);
    somThis = WPDCF77GetData (somSelf);

    if (bShow)
        hwnd = hwndFrame;
    else
        hwnd = HWND_OBJECT;

    WinSetParent (_hwndTitleBar, hwnd, bShow);
    WinSetParent (_hwndSysMenu,  hwnd, bShow);
    WinSetParent (_hwndMinMax,   hwnd, bShow);

    WinSendMsg (hwndFrame, WM_UPDATEFRAME,
                MPFROMLONG (FCF_TITLEBAR | FCF_SYSMENU | FCF_MINMAX | FCF_MENU),
                MPVOID);

    if (bShow)
        WinInvalidateRect (hwndFrame, NULL, TRUE);
    return;
    }

/*******************************************************************\
    ClkSetFonts: copy FATTR structure for the fonts out of
    			 instance data
    Input: somThis: pointer to instance data
\*******************************************************************/
VOID ClkSetFonts (WPDCF77Data *somThis)
    {
    /* query font out of instance data */
    memcpy (&_fAttrDate, &_fAttrD, sizeof (FATTRS));
    memcpy (&_fAttrTime, &_fAttrT, sizeof (FATTRS));
    strcpy (_szFontFamilyDate, _szFontFamilyD);
    strcpy (_szFontFamilyTime, _szFontFamilyT);

    return;
    }

/*******************************************************************\
    ClkSetRGBColors: calculate shading of RGB colors
    Input: somThis: pointer to instance data
\*******************************************************************/
VOID ClkSetRGBColors (WPDCF77Data *somThis)
    {

    /* copy color values */
    _sclrFace.ulSurface       = _clrFace;
    _sclrBG.ulSurface         = _clrBackgnd;
    _sclrHHand.ulSurface      = _clrHHand;
    _sclrMHand.ulSurface      = _clrMHand;
    _sclrMajorTicks.ulSurface = _clrTick;
    _sclrMinorTicks.ulSurface = _clrFace;
    _sclrRing.ulSurface       = _clrFace;
    _sclrDate.ulSurface       = _clrDateOnAnalog;
    _ulclrTransparent         = _clrTransparent;
    _ulclrDateText            = _clrDateText;
    _ulclrTimeText            = _clrTimeText;

    /* fill color tables */
    ClkShadeLight (&_sclrMajorTicks);
    ClkShadeLight (&_sclrMinorTicks);
    ClkShadeLight (&_sclrFace);
    ClkShadeLight (&_sclrRing);
    ClkShadeLight (&_sclrHHand);
    ClkShadeLight (&_sclrMHand);
    ClkShadeLight (&_sclrBG);
    ClkShadeLight (&_sclrDate);

    _sclrHHand.ulShade = RGB_BLACK;
    _sclrMHand.ulShade = RGB_BLACK;

    return;
    }

/*******************************************************************\
    ClkShadeLight: determine shading of a color and fill
                COLORS structure
    Input: psColors: pointer to COLORS structure
\*******************************************************************/
VOID ClkShadeLight (PCOLORS psColors)
    {
    typedef union _RGBLONG
        {
        RGB     rgb;
        LONG    lng;
        } RGBLONG;
    RGBLONG  rgbSurface, rgbShade, rgbLight;

    /* delete all colors */
    rgbSurface.lng = rgbShade.lng = rgbLight.lng = 0L;

    /* load surface color */
    rgbSurface.lng = psColors->ulSurface;

    /* calculate shade */
    rgbShade.rgb.bBlue  = ClkShadeRGBByte (rgbSurface.rgb.bBlue);
    rgbShade.rgb.bRed   = ClkShadeRGBByte (rgbSurface.rgb.bRed);
    rgbShade.rgb.bGreen = ClkShadeRGBByte (rgbSurface.rgb.bGreen);

    /* calculate bright color */
    rgbLight.rgb.bBlue  = ClkLightRGBByte (rgbSurface.rgb.bBlue);
    rgbLight.rgb.bRed   = ClkLightRGBByte (rgbSurface.rgb.bRed);
    rgbLight.rgb.bGreen = ClkLightRGBByte (rgbSurface.rgb.bGreen);

    /* copy colors to output buffer */
    psColors->ulShade = rgbShade.lng;
    psColors->ulLight = rgbLight.lng;

    return;
    }

/*******************************************************************\
    ClkShadeRGBByte: determine RGB value of the shadow of a
    				 base color
    Input: brgb: base color: normal brightness
    return:      base color: shaded brightness
\*******************************************************************/
BYTE ClkShadeRGBByte (BYTE brgb)
    {
    #define SHADER   ( (BYTE) 0x50)

    return (brgb > SHADER) ? (brgb - SHADER) : 0;
    }

/*******************************************************************\
    ClkLightRGBByte: determine RGB value of an illuminated
    			     base color
    Input: brgb: base color: normal brightness
    return:      base color: illuminated brightness
\*******************************************************************/
BYTE ClkLightRGBByte (BYTE brgb)
    {

    #define LIGHTER  ( (BYTE) 0x40)

    return (brgb < (0xFF - LIGHTER)) ? (brgb + LIGHTER) : 0xFF;
    }
