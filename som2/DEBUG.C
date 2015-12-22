/***************************************************************************\
 *
 * PROGRAM NAME: WPDCF77
 * -------------
 *
 * VERSION: 6.0
 * --------
 *
 * MODULE NAME: DEBUG
 * ------------
 *
 * DESCRIPTION:
 * ------------
 *   Provides debug functions
 *
 * NOTES:
 * ------
 *
 * FUNCTIONS:
 * -----------
 *   myReplacementForSOMOutChar ()
 *   DbgDlgProc ()
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
#define INCL_WIN
#define INCL_DOS
#define INCL_GPITRANSFORMS

#include <OS2.H>
#include <stdlib.h>

#include "dcfioctl.h"

#pragma info(noeff)         // suppress SOM compiler informationals

#include "wpdcf77.ih"
#include "wpdcf.h"

#include "res.h"
#include "debug.h"

#ifdef DEBUG
ULONG ulDebugMask = D_ALARM + D_INI + D_MENU + D_WND + D_DRAG +
                    D_DIAG + D_DLG + D_MINMAX + D_STAT;
CHAR  szDebug[128];
#endif /* DEBUG */

/* Define your replacement routine */
int SOMLINK myReplacementForSOMOutChar (char c)
    {
#ifdef DEBUG
    pmprintf ("%c", c);
#endif /* DEBUG */

    return 1;
    }

#ifdef DEBUG
MRESULT EXPENTRY DbgDlgProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
    {
    static ULONG ulResult;
    CHAR szText[9];

    switch (msg)
        {
        case WM_INITDLG:
            ulResult = 0;
            WinSendDlgItemMsg (hwnd, DID_TL_0 + SOM_TraceLevel, BM_SETCHECK,
                MPFROMSHORT (1), 0);
            WinSendDlgItemMsg (hwnd, DID_WL_0 + SOM_WarnLevel, BM_SETCHECK,
                MPFROMSHORT (1), 0);
            WinSendDlgItemMsg (hwnd, DID_AL_0 + SOM_AssertLevel, BM_SETCHECK,
                MPFROMSHORT (1), 0);
            WinSendDlgItemMsg (hwnd, DID_DM, EM_SETTEXTLIMIT, MPFROMSHORT (8), MPVOID);
            WinSetDlgItemText (hwnd, DID_DM, _itoa (ulDebugMask, szText, 16));
            WinSendDlgItemMsg (hwnd, DID_DT, MLM_INSERT, MPFROMP (D_TEXT_MASK), MPVOID);
            return (MRESULT)TRUE;

        case WM_CONTROL:
            switch (LONGFROMMP (mp1))
                {
                case DID_TL_0 + (BN_CLICKED<<16):
                case DID_TL_1 + (BN_CLICKED<<16):
                case DID_TL_2 + (BN_CLICKED<<16):
                    ulResult &= ~(3 << SHFT_TRACE);
                    ulResult += (SHORT1FROMMP (mp1) - DID_TL_0) << SHFT_TRACE;
                    break;

                case DID_WL_0 + (BN_CLICKED<<16):
                case DID_WL_1 + (BN_CLICKED<<16):
                case DID_WL_2 + (BN_CLICKED<<16):
                    ulResult &= ~(3 << SHFT_WARN);
                    ulResult += (SHORT1FROMMP (mp1) - DID_WL_0) << SHFT_WARN;
                    break;
                case DID_AL_0 + (BN_CLICKED<<16):
                case DID_AL_1 + (BN_CLICKED<<16):
                case DID_AL_2 + (BN_CLICKED<<16):
                    ulResult &= ~(3 << SHFT_ASSERT);
                    ulResult += (SHORT1FROMMP (mp1) - DID_AL_0) << SHFT_ASSERT;
                    break;

                case DID_DM + (EN_KILLFOCUS<<16):
                    WinQueryDlgItemText (hwnd, DID_DM, 9, szText);
                    ulDebugMask = strtoul (szText, NULL, 16);
                    break;
                }
            return 0;

        case WM_CLOSE:
            WinDismissDlg (hwnd, ulResult);
            return 0;
        }

    return WinDefDlgProc (hwnd, msg, mp1, mp2);
    }
#endif /* DEBUG */
