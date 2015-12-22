/***************************************************************************\
 *
 * PROGRAM NAME: WPDCF77
 * -------------
 *
 * VERSION: 6.0
 * --------
 *
 * MODULE NAME: UDTIME
 * ------------
 *
 * DESCRIPTION:
 * ------------
 *   Responsible for the settings pages to set 
 *   date/time and alarm time
 *
 * NOTES:
 * ------
 *
 * FUNCTIONS:
 * -----------
 *   TimeDlgProc ()
 *   AlarmDlgProc ()
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
#define INCL_WINDIALOGS
#define INCL_WINBUTTONS
#define INCL_WINSYS
#define INCL_WINMESSAGEMGR
#define INCL_WINSHELLDATA
#define INCL_WINFRAMEMGR
#define INCL_WINPOINTERS
#define INCL_WINSWITCHLIST
#define INCL_WINWINDOWMGR
#define INCL_WINSTDSPIN
#define INCL_DOSNLS
#define INCL_DOSMISC
#define INCL_GPITRANSFORMS
#define INCL_DOSSEMAPHORES
#define INCL_DOSDATETIME

#include <os2.h>
#include <string.h>
#include <stdlib.h>

#include "dcfioctl.h"

#pragma info(noeff)         // suppress SOM compiler informationals

#include "wpdcf77.ih"
#include "wpdcf.h"

#include "clock.h"
#include "clkmem.h"
#include "res.h"
#include "clkdata.h"
#include "tloc.h"
#include "alarmthd.h"
#include "locale.h"
#include "udtime.h"
#include "debug.h"

/*--------------------------------------------------------------*\
 *  Global definitions and variables for this module            *
\*--------------------------------------------------------------*/
#define SECPERHOUR  3600        // Seconds per hour

static struct _ALRMCTLS
    {
    USHORT  usID;
    ULONG   usMode;
    } AlarmCtls[] = {{DID_ALRM_SOUND,    AM_AUDIO},
                     {DID_ALRM_POPUP,    AM_MSGBOX},
                     {DID_ALRM_LAUNCH,   AM_LAUNCH},
                     {DID_ALRM_SINGLE,   AM_SINGLE},
                     {DID_ALRM_MON,      AM_MON},
                     {DID_ALRM_TUE,      AM_TUE},
                     {DID_ALRM_WED,      AM_WED},
                     {DID_ALRM_THU,      AM_THU},
                     {DID_ALRM_FRI,      AM_FRI},
                     {DID_ALRM_SAT,      AM_SAT},
                     {DID_ALRM_SUN,      AM_SUN}};

/*******************************************************************\
    UpdateDTSpinBtn: Update of spin buttons in the date/time and
    				 alarm settings page. Only controls with 
    				 changed contents will be considered.
    Input: hwnd:   Window-Handle of settings page
           pdtNew: Current time / date
           pdtOld: Current state of spin buttons
\*******************************************************************/
VOID UpdateDTSpinBtn (HWND hwnd, PDATETIME pdtNew, PDATETIME pdtOld)
    {
    /* Year: Update also number of day of month */
    if (pdtNew->year != pdtOld->year)
        {
        WinSendDlgItemMsg (hwnd, DID_SETYEAR, SPBM_SETCURRENTVALUE,
                           MPFROMSHORT (pdtNew->year), MPVOID);
        WinSendDlgItemMsg (hwnd, DID_SETDAY, SPBM_SETLIMITS,
                           (MPARAM)DaysInMonth (pdtNew), (MPARAM)1);
        }
    /* Month: Update also number of days */
    if (pdtNew->month != pdtOld->month)
        {
        WinSendDlgItemMsg (hwnd, DID_SETMONTH, SPBM_SETCURRENTVALUE,
                           MPFROMCHAR (pdtNew->month), MPVOID);
        WinSendDlgItemMsg (hwnd, DID_SETDAY, SPBM_SETLIMITS,
                           (MPARAM)DaysInMonth (pdtNew), (MPARAM)1);
        }
    /* Day */
    if (pdtNew->day != pdtOld->day)
        WinSendDlgItemMsg (hwnd, DID_SETDAY, SPBM_SETCURRENTVALUE,
                           MPFROMCHAR (pdtNew->day), MPVOID);
    /* Time */
    if (pdtNew->hours != pdtOld->hours)
        WinSendDlgItemMsg (hwnd, DID_SETHOUR, SPBM_SETCURRENTVALUE,
                           MPFROMCHAR (pdtNew->hours), MPVOID);
    if (pdtNew->minutes != pdtOld->minutes)
        WinSendDlgItemMsg (hwnd, DID_SETMINUTE, SPBM_SETCURRENTVALUE,
                           MPFROMCHAR (pdtNew->minutes), MPVOID);
    if (pdtNew->seconds != pdtOld->seconds)
        WinSendDlgItemMsg (hwnd, DID_SETSEC, SPBM_SETCURRENTVALUE,
                           MPFROMCHAR (pdtNew->seconds), MPVOID);

    /* Update strucOldDT */
    memcpy (pdtOld, pdtNew, sizeof (DATETIME));

    return;
    }

/*******************************************************************\
 *                       Exported Functions                        *
\*******************************************************************/
/*******************************************************************\
    GetDateTimeOffs: Retrieves current time and date and adds the
                     local time offset out of _lLOffset.
    Input:  somThis: Pointer to instance data
    Output: pdt:     Pointer to DATETIME structure
\*******************************************************************/
VOID GetDateTimeOffs (LONG lOffset, PDATETIME pdt)
    {
    CHAR  cDaysInMonth;
    UCHAR cDayDiff;

    DosGetDateTime (pdt);

    if (lOffset)          // nothing to do for lOffset == 0
        {
        /* add hours */
        pdt->hours += lOffset;

        /* add day */
        cDayDiff = ((signed char)pdt->hours + 24) / 24 - 1;
        pdt->day    += cDayDiff;
        pdt->weekday = (pdt->weekday + cDayDiff + 7) % 7;

        /* add month */
        cDaysInMonth = DaysInMonth (pdt);
        pdt->month += ((signed char)pdt->day + cDaysInMonth - 1) / cDaysInMonth - 1;

        /* add year */
        pdt->year += ((signed char)pdt->month + 11) / 12 - 1;

        /* correct */
        pdt->hours = ((signed char)pdt->hours + 24) % 24;
        pdt->month = ((signed char)pdt->month + 11) % 12 + 1;
        if ((signed char)pdt->day < 1)
            cDaysInMonth = DaysInMonth (pdt);
        pdt->day = (pdt->day + cDaysInMonth - 1) % cDaysInMonth + 1;
        }

    return;
    }

/*******************************************************************\
    GetTimeLowOffs: Returns the lower part of the number of seconds
                    since 1-1-1970. An offset in hours will be
                    considered.
    Input:   lOffset: Offset in hours
    return:  number of seconds since 1-1-1970
\*******************************************************************/
LONG GetTimeLowOffs (LONG lOffset)
    {
    LONG lSeconds;

    DosQuerySysInfo (QSV_TIME_LOW, QSV_TIME_LOW, &lSeconds, sizeof (LONG));

    return lSeconds + SECPERHOUR * lOffset;
    }

VOID TimeInitDlg (HWND hwnd, WPDCF77 *somSelf)
    {
    WPDCF77Data *somThis;

    somThis = WPDCF77GetData (somSelf);
    WinSetWindowULong (hwnd, QWL_USER, (ULONG)somSelf);
    _pIDataTime = allocZeroMem (sizeof (TIMEINSTDATA));

    /* Initialize spin buttons for time */
    WinSendDlgItemMsg (hwnd, DID_SETHOUR,   SPBM_SETLIMITS, MPFROMLONG (23), MPFROMLONG (0));
    WinSendDlgItemMsg (hwnd, DID_SETMINUTE, SPBM_SETLIMITS, MPFROMLONG (59), MPFROMLONG (0));
    WinSendDlgItemMsg (hwnd, DID_SETSEC,    SPBM_SETLIMITS, MPFROMLONG (59), MPFROMLONG (0));

    /* Initialize spin buttons for date */
    WinSendDlgItemMsg (hwnd, DID_SETYEAR,  SPBM_SETLIMITS, MPFROMLONG (99), MPFROMLONG (0));
    WinSendDlgItemMsg (hwnd, DID_SETMONTH, SPBM_SETLIMITS, MPFROMLONG (12), MPFROMLONG (1));
    WinSendDlgItemMsg (hwnd, DID_SETDAY,   SPBM_SETLIMITS,
                       MPFROMLONG (DaysInMonth (&_pIDataTime->strucDateTime)), MPFROMLONG (1));

    memset (&_pIDataTime->strucDateTime, 0, sizeof (DATETIME));
    WinSendMsg (hwnd, WM_UPDATESPB, 0, 0);
    _wpAdd1sTimer (somSelf, hwnd);

    return;
    }

// WM_UPDATESPB (User Message): change corresponding spin button, if date/time has changed.
// (_pIDataTime->strucDateTime != strucDTOld)
VOID TimeUpdateSpb (HWND hwnd)
    {
    WPDCF77     *somSelf;
    WPDCF77Data *somThis;
    DATETIME     strucDTOld;            // for comparison

    somSelf = (WPDCF77 *)WinQueryWindowULong (hwnd, QWL_USER);
    somThis = WPDCF77GetData (somSelf);

    memcpy (&strucDTOld, &_pIDataTime->strucDateTime, sizeof (DATETIME));
    GetDateTimeOffs (_lLOffset, &_pIDataTime->strucDateTime);
    UpdateDTSpinBtn (hwnd, &_pIDataTime->strucDateTime, &strucDTOld);

    return;
    }

VOID TimeControl (HWND hwnd, MPARAM mp1)
    {
    WPDCF77     *somSelf;
    WPDCF77Data *somThis;
    ULONG        ulValue;

    if (SHORT2FROMMP (mp1) == SPBN_ENDSPIN)
        {
        somSelf = (WPDCF77 *)WinQueryWindowULong (hwnd, QWL_USER);
        somThis = WPDCF77GetData (somSelf);
        WinSendDlgItemMsg (hwnd, SHORT1FROMMP (mp1), SPBM_QUERYVALUE,
                           &ulValue, MPFROM2SHORT (0, SPBQ_UPDATEIFVALID));
        switch (SHORT1FROMMP (mp1))
            {
            case DID_SETYEAR:
                _pIDataTime->strucDateTime.year = ulValue;
                break;

            case DID_SETMONTH:
                _pIDataTime->strucDateTime.month = ulValue;
                break;

            case DID_SETDAY:
                _pIDataTime->strucDateTime.day = ulValue;
                break;

            case DID_SETHOUR:
                _pIDataTime->strucDateTime.hours = ulValue;
                break;

            case DID_SETMINUTE:
                _pIDataTime->strucDateTime.minutes = ulValue;
                break;

            case DID_SETSEC:
                _pIDataTime->strucDateTime.seconds = ulValue;
                _pIDataTime->strucDateTime.hundredths = 0;
                break;
            }

        /* correct number of days of month, if necessary */
        if ((SHORT1FROMMP (mp1) == DID_SETYEAR) ||
            (SHORT1FROMMP (mp1) == DID_SETMONTH))
            {
            WinSendDlgItemMsg (hwnd, DID_SETDAY, SPBM_SETLIMITS,
                               (MPARAM)DaysInMonth (&_pIDataTime->strucDateTime),
                               (MPARAM)1);
            WinSendDlgItemMsg (hwnd, DID_SETDAY, SPBM_QUERYVALUE,
                               &ulValue,
                               MPFROM2SHORT (0, SPBQ_UPDATEIFVALID));
            _pIDataTime->strucDateTime.day = ulValue;
            }

        /* spin buttons are up-to-date */
        DosSetDateTime (&_pIDataTime->strucDateTime);
        }

    return;
    }

VOID TimeClose (HWND hwnd)
    {
    WPDCF77     *somSelf;
    WPDCF77Data *somThis;

    somSelf = (WPDCF77 *)WinQueryWindowULong (hwnd, QWL_USER);
    somThis = WPDCF77GetData (somSelf);
    _wpRemove1sTimer (somSelf, hwnd);

    freeMem ((PPVOID)&_pIDataTime);

    return;
    }

/*******************************************************************\
    TimeDlgProc: Settings-page to set date/time
    Input: hwnd: window handle
           msg:  message type
           mp1:  1. parameter
           mp2:  2. parameter
\*******************************************************************/
MRESULT EXPENTRY TimeDlgProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
    {
    switch (msg)
        {
        case WM_INITDLG:
            TimeInitDlg (hwnd, (WPDCF77 *)mp2);
            return (MRESULT)TRUE;

        case WM_COMMAND:
            return 0;

        case WM_1STIMER:
            WinSendMsg (hwnd, WM_UPDATESPB, MPVOID, MPVOID);
            return 0;

        case WM_UPDATESPB:
            TimeUpdateSpb (hwnd);
            return 0;

        case WM_CONTROL:
            TimeControl (hwnd, mp1);
            return 0;

        case WM_DESTROY:
            WinSendMsg (hwnd, WM_CLOSE, 0, 0);
            return 0;

        case WM_CLOSE:
            TimeClose (hwnd);
            break;
        }

    return WinDefDlgProc (hwnd, msg, mp1, mp2);
    }

/*******************************************************************\
    ShowAlrmCtls: Set visibility state of controls in 
                  alarm dialog box:
        AM_REMINDER: 1 = Reminder spin button active
                     0 = time controls are active; and:
                     AM_SINGLE: 1 = date controls are active
                                0 = day of week active
    Input: hwnd:   window handle of dialog box
           usMode: see AM_* - flags in ALARMTIME-Structure
\*******************************************************************/
VOID ShowAlrmCtls (HWND hwnd, USHORT usMode)
    {
    USHORT uscID;
    CHAR   szText[CCHMAXPGNAME];
    BOOL   bStateTime, bStateDate, bStateDay;

    if (usMode & AM_REMINDER)
        {
        bStateTime = FALSE;
        bStateDate = FALSE;
        bStateDay  = FALSE;
        }
    else
        {
        bStateTime = TRUE;
        if (usMode & AM_SINGLE)
            {
            bStateDate = TRUE;
            bStateDay  = FALSE;
            }
        else
            {
            bStateDate = FALSE;
            bStateDay  = TRUE;
            }
        }

    /* Controls in DID_TXT_GROUP1 */
    for (uscID = 0; uscID < 3; uscID++)
        WinShowWindow (WinWindowFromID (hwnd, DID_SETHOUR + uscID), bStateTime);

    WinShowWindow (WinWindowFromID (hwnd, DID_SETREMINDER), !bStateTime);
    WinShowWindow (WinWindowFromID (hwnd, DID_TXT_HOURS),    bStateTime);
    WinShowWindow (WinWindowFromID (hwnd, DID_TXT_SECONDS),  bStateTime);

    /* Controls in DID_TXT_GROUP2 */
    for (uscID = 0; uscID < 3; uscID++)
        {
        WinShowWindow (WinWindowFromID (hwnd, DID_SETDAY  + uscID), bStateDate);
        WinShowWindow (WinWindowFromID (hwnd, DID_TXT_DAY + uscID), bStateDate);
        }

    for (uscID = 0; uscID < 7; uscID++)
        {
        WinShowWindow (WinWindowFromID (hwnd, DID_ALRM_MON + uscID), bStateDay);
        WinShowWindow (WinWindowFromID (hwnd, DID_TXT_MON  + uscID), bStateDay);
        }

    WinShowWindow (WinWindowFromID (hwnd, DID_ALRM_SINGLE), bStateTime);
    WinShowWindow (WinWindowFromID (hwnd, DID_TXT_GROUP2),  bStateTime);

    /* Title of group boxes */
    WinLoadString (hab, hmod, (usMode & AM_SINGLE) ? IDS_ALRM_DATE : IDS_ALRM_DAILY, CCHMAXPGNAME, szText);
    WinSetDlgItemText (hwnd, DID_TXT_GROUP2, szText);

    WinLoadString (hab, hmod, (usMode & AM_REMINDER) ? IDS_ALRM_REMIND : IDS_ALRM_TIME, CCHMAXPGNAME, szText);
    WinSetDlgItemText (hwnd, DID_TXT_GROUP1, szText);

    return;
    }

VOID AlarmSetAllControls (HWND hwnd, WPDCF77Data *somThis)
    {
    USHORT   usItem;
    DATETIME strucDTOld;            // for comparison

    memset (&strucDTOld, 0, sizeof (DATETIME));
    UpdateDTSpinBtn (hwnd, &_pIDataAlarm1->strucDateTime, &strucDTOld);
    WinSendDlgItemMsg (hwnd, DID_SETREMINDER, SPBM_SETCURRENTVALUE,
                       MPFROMSHORT (_pIDataAlarm1->usReminderTime), MPVOID);

    /* initialize check and radio buttons */
    if (_pIDataAlarm1->usMode & AM_REMINDER)
        usItem = DID_ALRM_REMINDER;
    else if (_pIDataAlarm1->usMode & AM_ACTIVE)
        usItem = DID_ALRM_ON;
    else
        usItem = DID_ALRM_OFF;
    WinCheckButton (hwnd, usItem, 1);

    for (usItem = 0; usItem < sizeof (AlarmCtls) / sizeof (struct _ALRMCTLS); usItem++)
        {
        WinCheckButton (hwnd, AlarmCtls[usItem].usID,
                        _pIDataAlarm1->usMode & AlarmCtls[usItem].usMode ? TRUE : FALSE);
        }

    ShowAlrmCtls (hwnd, _pIDataAlarm1->usMode);

    return;
    }

VOID AlarmInitPrivateData (WPDCF77Data *somThis)
    {
    // For alarm function:
    //   If currently no active alarm: load strucDateTime witch current time
    //   If currently active alarm:    load strucDateTimeA with alarm time
    //                                 AM_SINGLE:  use alarm date
    //                                 !AM_SINGLE: use current date
    GetDateTimeOffs (_lLOffset, &_pIDataAlarm1->strucDateTime);
    if (_dtAlarm.usMode & AM_ACTIVE)
        {
        _pIDataAlarm1->strucDateTime.hours   = _dtAlarm.dtAlarm.hours;
        _pIDataAlarm1->strucDateTime.minutes = _dtAlarm.dtAlarm.minutes;
        _pIDataAlarm1->strucDateTime.seconds = _dtAlarm.dtAlarm.seconds;
        if (_dtAlarm.usMode & AM_SINGLE)
            {
            _pIDataAlarm1->strucDateTime.day     = _dtAlarm.dtAlarm.day;
            _pIDataAlarm1->strucDateTime.month   = _dtAlarm.dtAlarm.month;
            _pIDataAlarm1->strucDateTime.year    = _dtAlarm.dtAlarm.year;
            }
        }
    _pIDataAlarm1->usReminderTime = _dtAlarm.usReminderTime;
    _pIDataAlarm1->usMode = _dtAlarm.usMode;

    DebugULd (D_DLG, "AlarmDlgProc", "Day",   _pIDataAlarm1->strucDateTime.day);
    DebugULd (D_DLG, "AlarmDlgProc", "Month", _pIDataAlarm1->strucDateTime.month);
    DebugULd (D_DLG, "AlarmDlgProc", "Year",  _pIDataAlarm1->strucDateTime.year);

    return;
    }

VOID AlarmInitDlg (HWND hwnd, WPDCF77 *somSelf)
    {
    WPDCF77Data *somThis;

    somThis = WPDCF77GetData (somSelf);
    WinSetWindowULong (hwnd, QWL_USER, (ULONG)somSelf);
    _pIDataAlarm1 = allocZeroMem (sizeof (ALARMINSTDATA1));

    /* initialize spin buttons for time */
    WinSendDlgItemMsg (hwnd, DID_SETREMINDER, SPBM_SETLIMITS, MPFROMLONG (120), MPFROMLONG (1));
    WinSendDlgItemMsg (hwnd, DID_SETHOUR,     SPBM_SETLIMITS, MPFROMLONG (23),  MPFROMLONG (0));
    WinSendDlgItemMsg (hwnd, DID_SETMINUTE,   SPBM_SETLIMITS, MPFROMLONG (59),  MPFROMLONG (0));
    WinSendDlgItemMsg (hwnd, DID_SETSEC,      SPBM_SETLIMITS, MPFROMLONG (59),  MPFROMLONG (0));

    /* initialize spin buttos for date */
    WinSendDlgItemMsg (hwnd, DID_SETYEAR,  SPBM_SETLIMITS, MPFROMLONG (2100), MPFROMLONG (1980));
    WinSendDlgItemMsg (hwnd, DID_SETMONTH, SPBM_SETLIMITS, MPFROMLONG (12),   MPFROMLONG (1));
    WinSendDlgItemMsg (hwnd, DID_SETDAY,   SPBM_SETLIMITS,
                       MPFROMLONG (DaysInMonth (&_pIDataAlarm1->strucDateTime)), MPFROMLONG (1));

    AlarmInitPrivateData (somThis);
    AlarmSetAllControls (hwnd, somThis);

    return;
    }

VOID AlarmControl (HWND hwnd, MPARAM mp1)
    {
    WPDCF77     *somSelf;
    WPDCF77Data *somThis;
    BOOL         bState;
    ULONG        ulValue;

    somSelf = (WPDCF77 *)WinQueryWindowULong (hwnd, QWL_USER);
    somThis = WPDCF77GetData (somSelf);
    switch (SHORT2FROMMP (mp1))
        {
        case SPBN_ENDSPIN:
            WinSendDlgItemMsg (hwnd, SHORT1FROMMP (mp1), SPBM_QUERYVALUE,
                &ulValue, MPFROM2SHORT (0, SPBQ_UPDATEIFVALID));
            switch (SHORT1FROMMP (mp1))
                {
                case DID_SETYEAR:
                    _pIDataAlarm1->strucDateTime.year = ulValue;
                    break;

                case DID_SETMONTH:
                    _pIDataAlarm1->strucDateTime.month = ulValue;
                    break;

                case DID_SETDAY:
                    _pIDataAlarm1->strucDateTime.day = ulValue;
                    break;

                case DID_SETHOUR:
                    _pIDataAlarm1->strucDateTime.hours = ulValue;
                    break;

                case DID_SETMINUTE:
                    _pIDataAlarm1->strucDateTime.minutes = ulValue;
                    break;

                case DID_SETSEC:
                    _pIDataAlarm1->strucDateTime.seconds = ulValue;
                    _pIDataAlarm1->strucDateTime.hundredths = 0;
                    break;

                case DID_SETREMINDER:
                    _pIDataAlarm1->usReminderTime = ulValue;
                    break;
                }

            /* correct number of days of month, if necessary */
            if ((SHORT1FROMMP (mp1) == DID_SETYEAR) ||
                (SHORT1FROMMP (mp1) == DID_SETMONTH))
                {
                WinSendDlgItemMsg (hwnd, DID_SETDAY, SPBM_SETLIMITS,
                                   (MPARAM)DaysInMonth (&_pIDataAlarm1->strucDateTime),
                                   (MPARAM)1);
                WinSendDlgItemMsg (hwnd, DID_SETDAY, SPBM_QUERYVALUE,
                                   &ulValue,
                                   MPFROM2SHORT (0, SPBQ_UPDATEIFVALID));
                _pIDataAlarm1->strucDateTime.day = ulValue;
                }
            return;

        case BN_CLICKED:
        case BN_DBLCLICKED:
            switch (SHORT1FROMMP (mp1))
                {
                case DID_ALRM_ON:
                    _pIDataAlarm1->usMode &= ~AM_REMINDER;
                    _pIDataAlarm1->usMode |=  AM_ACTIVE;
                    break;

                case DID_ALRM_REMINDER:
                    _pIDataAlarm1->usMode &= ~AM_ACTIVE;
                    _pIDataAlarm1->usMode |=  AM_REMINDER;
                    break;

                case DID_ALRM_OFF:
                    _pIDataAlarm1->usMode &= ~(AM_ACTIVE | AM_REMINDER);
                    break;

                case DID_ALRM_SINGLE:
                    bState = (WinQueryButtonCheckstate (hwnd, DID_ALRM_SINGLE) == 1);
                    _pIDataAlarm1->usMode &= ~AM_SINGLE;
                    _pIDataAlarm1->usMode |= (bState ? AM_SINGLE : 0);
                    break;
                }
            ShowAlrmCtls (hwnd, _pIDataAlarm1->usMode);
            return;
        }

    return;
    }

VOID AlarmCommand (HWND hwnd, USHORT usCmd, USHORT usSource)
    {
    WPDCF77     *somSelf;
    WPDCF77Data *somThis;

    if (usSource == CMDSRC_PUSHBUTTON)
        {
        somSelf = (WPDCF77 *)WinQueryWindowPtr (hwnd, QWL_USER);
        somThis = WPDCF77GetData (somSelf);
        switch (usCmd)
            {
            case DID_CANCEL:
                AlarmInitPrivateData (somThis);
                AlarmSetAllControls (hwnd, somThis);
                break;

            case DID_STANDARD:
                memset ((PVOID)&(_pIDataAlarm1->strucDateTime), '\0', sizeof (DATETIME));
                _pIDataAlarm1->usReminderTime = 0;
                _pIDataAlarm1->usMode = 0;
                AlarmSetAllControls (hwnd, somThis);
                break;
            }
        }

    return;
    }

VOID AlarmClose (HWND hwnd)
    {
    WPDCF77     *somSelf;
    WPDCF77Data *somThis;
    USHORT       usItem;

    somSelf = (WPDCF77 *)WinQueryWindowULong (hwnd, QWL_USER);
    somThis = WPDCF77GetData (somSelf);

    if (_pIDataAlarm1->usMode & (AM_ACTIVE | AM_REMINDER))
        {
        for (usItem = 0; usItem < sizeof (AlarmCtls) / sizeof (struct _ALRMCTLS); usItem++)
            if (WinQueryButtonCheckstate (hwnd, AlarmCtls[usItem].usID))
                _pIDataAlarm1->usMode |= AlarmCtls[usItem].usMode;
            else
                _pIDataAlarm1->usMode &= ~AlarmCtls[usItem].usMode;

        if (_pIDataAlarm1->usMode & AM_REMINDER)
            _dtAlarm.usReminderTime  = _pIDataAlarm1->usReminderTime;
        else
            {
            _dtAlarm.dtAlarm.hours   = _pIDataAlarm1->strucDateTime.hours;
            _dtAlarm.dtAlarm.minutes = _pIDataAlarm1->strucDateTime.minutes;
            _dtAlarm.dtAlarm.seconds = _pIDataAlarm1->strucDateTime.seconds;
            if (_pIDataAlarm1->usMode & AM_SINGLE)
                {
                _dtAlarm.dtAlarm.day     = _pIDataAlarm1->strucDateTime.day;
                _dtAlarm.dtAlarm.month   = _pIDataAlarm1->strucDateTime.month;
                _dtAlarm.dtAlarm.year    = _pIDataAlarm1->strucDateTime.year;
                }
            }
        }

    _dtAlarm.usMode = _pIDataAlarm1->usMode;

    AlarmStart (somThis, activate);
    _wpSaveDeferred (somSelf);
    _wpRefreshClockView (somSelf);

    freeMem ((PPVOID)&_pIDataAlarm1);

    return;
    }

/*******************************************************************\
    AlarmDlgProc: Dialog procedure for the alarm settings page
    Input: hwnd: window handle
           msg:  message type
           mp1:  1. parameter
           mp2:  2. parameter
\*******************************************************************/
MRESULT EXPENTRY AlarmDlgProc (HWND hwnd, ULONG usMsg, MPARAM mp1, MPARAM mp2)
    {
    switch (usMsg)
        {
        case WM_INITDLG :
            AlarmInitDlg (hwnd, (WPDCF77 *)mp2);
            return (MRESULT)TRUE;

        case WM_CONTROL:
            AlarmControl (hwnd, mp1);
            return 0;

        case WM_COMMAND:
            AlarmCommand (hwnd, SHORT1FROMMP (mp1), SHORT1FROMMP (mp2));
            return 0;

        case WM_DESTROY:
            WinSendMsg (hwnd, WM_CLOSE, MPVOID, MPVOID);
            return 0;

        case WM_CLOSE:
            AlarmClose (hwnd);
            break;
        }

    return WinDefDlgProc (hwnd, usMsg, mp1, mp2);
    }
