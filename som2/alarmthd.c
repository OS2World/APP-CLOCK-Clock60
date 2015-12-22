/***************************************************************************\
 *
 * PROGRAM NAME: WPDCF77
 * -------------
 *
 * VERSION: 6.0
 * --------
 *
 * MODULE NAME: ALARMTHD
 * ------------
 *
 * DESCRIPTION:
 * ------------
 *   1. Thread for alarm function
 *   2. Thread for timer
 *
 * NOTES:
 * ------
 *   The application uses only one timer thread, which posts WM_1STIMER messages
 *   in a 1sec interval. The receiving window handles are transferred via the 
 *   _wpAdd1sTimer method and removed by the _wpRemove1sTimer method.
 *
 * FUNCTIONS:
 * -----------
 *   AlarmThread ()
 *   AlarmInit ()
 *   AlarmStart ()
 *   AlarmSetTimer ()
 *   th1sTimer ()
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
#define INCL_WINMENUS
#define INCL_GPITRANSFORMS
#define INCL_WINHELP
#define INCL_WINWINDOWMGR
#define INCL_WINMESSAGEMGR
#define INCL_DOSSEMAPHORES
#define INCL_DOSDATETIME
#define INCL_DOSQUEUES
#define INCL_DOSPROCESS
#define INCL_DOSERRORS
#define INCL_WINDIALOGS

#include <os2.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "dcfioctl.h"

#pragma info(noeff)         // suppress SOM compiler informationals

#include "wpdcf77.ih"
#include "wpdcf.h"

#include "clkdata.h"
#include "clkmem.h"
#include "res.h"
#include "alarmthd.h"
#include "udtime.h"
#include "debug.h"

/*--------------------------------------------------------------*\
 *  Global definitions and variables for this module            *
\*--------------------------------------------------------------*/
#define HOURPERDAY    24                            // hours per day
#define HOURS_24S     (24L * 60L * 60L)             // s per day
#define HOURS_24MS    (1000L * HOURS_24S)           // ms per day

static ULONG ulFreq[] = {262,  294,  330, 349, 392, 440,  494,  523,
                         587,  659,  699, 784, 880, 988, 1047, 1175,
                         1319, 1397,1568,1760,1976, 0};

BOOL IsAlarmActive (WPDCF77Data *somThis)
    {
    return _dtAlarm.bIsAlarmActive;
    }

/*******************************************************************\
    AlarmInit: Initialisation of alarm timer
    Input: somThis: pointer to instance data
    return:  return value of DosAsyncTimer
\*******************************************************************/
APIRET AlarmInit (WPDCF77Data *somThis)
    {
    APIRET rc;

    if (_timerResources.ulDays || _timerResources.ulTimerValue)
        {
        DebugULd (D_ALARM, "AlarmInit", "Start timer, seconds", _timerResources.ulTimerValue/1000);
        rc = DosAsyncTimer (_timerResources.ulTimerValue,
                            (HSEM)_timerResources.hSemTimer,
                            &_timerResources.hTimer);
        DebugULx (D_ALARM, "AlarmInit", "HSEM", _timerResources.hSemTimer);
        DebugULx (D_ALARM, "AlarmInit", "HTIMER", _timerResources.hSemTimer);
        }
    else
        {
        DebugE (D_ALARM, "AlarmInit", "No timer needed");
        _dtAlarm.bIsAlarmActive = FALSE;
        _timerResources.hTimer  = NULLHANDLE;
        rc = NO_ERROR;
        }

    _timerResources.ulPostReason = TR_TMR_EXPIRED;

    return rc;
    }

/*******************************************************************\
    AlarmStart: Start/Stop of an alarm
    Input: somThis: pointer to instance data
    return:  TRUE:  ok
             FALSE: error occurred during AlarmSetTimer
\*******************************************************************/
BOOL AlarmStart (WPDCF77Data *somThis, AlarmStartAction action)
    {
    DATETIME dtActual;

    switch (action)
        {
        case activate:
            if (_dtAlarm.usMode & AM_REMINDER)
                _dtAlarm.bIsAlarmActive = FALSE;
            else
                _dtAlarm.bIsAlarmActive = TRUE;
            break;

        case start:
            _dtAlarm.bIsAlarmActive = TRUE;
            break;

        case stop:
            _dtAlarm.bIsAlarmActive = FALSE;
            break;

        case toggle:
            _dtAlarm.bIsAlarmActive = !_dtAlarm.bIsAlarmActive;
            break;
        }

    /* has an alarm time been specified? */
    if (!_dtAlarm.bIsAlarmActive)
        {
        /* alarm has been stopped */
        DebugE (D_ALARM, "AlarmStart", "Alarm RESET");
        _timerResources.ulDays       = 0;                // Number of days
        _timerResources.ulTimerValue = 0;                // Number of milliseconds
        _timerResources.ulPostReason = TR_ALARMSTOPPED;

        if (_dtAlarm.usMode & AM_REMINDER)
            _timerResources.ulTimerValue = 60000 * _dtAlarm.usReminderTime;

        /* post semaphore, to set up timer with new data */
        DosPostEventSem (_timerResources.hSemTimer);
        return TRUE;
        }
    else
        {
        DebugE (D_ALARM, "AlarmStart", "Alarm STARTED");
        GetDateTimeOffs (_lLOffset, &dtActual);
        return AlarmSetTimer (somThis, &dtActual);
        }
    }

/*******************************************************************\
    AlarmReStart: Set up of a multiple alarm
    Input: somThis: pointer to instance data
    return:  TRUE:  ok
             FALSE: error occurred during AlarmSetTimer
\*******************************************************************/
BOOL AlarmReStart (WPDCF77Data *somThis)
    {
    DATETIME dtActual;

    DebugE (D_ALARM, "AlarmReStart", "Alarm RESTARTED");
    GetDateTimeOffs (_lLOffset, &dtActual);
    return AlarmSetTimer (somThis, &dtActual);
    }

/*******************************************************************\
    AlarmSetTimer: Set timer value for the alarm thread
    Input: somThis: pointer to instance data
             pdtActual: pointer to DATETIME structure for current time
    return:  TRUE:  alarm time has been corrected
             FALSE: alarm tiem erroneous
\*******************************************************************/
BOOL AlarmSetTimer (WPDCF77Data *somThis, PDATETIME pdtActual)
    {
    LONG  lc1, lc2, lchms, ly, lDays;
    SHORT sM;
    BOOL  bRC;

    _timerResources.ulDays = _timerResources.ulTimerValue = 0;

    if (_dtAlarm.usMode & AM_REMINDER)
        {
        _timerResources.ulDays       = 0;                                   // Number of Days
        _timerResources.ulTimerValue = 60 * 1000 * _dtAlarm.usReminderTime; // Number of Milliseconds
        DebugULd (D_ALARM, "AlarmSetTimer", "Reminder; time interval", _timerResources.ulTimerValue);

        bRC = TRUE;                                                         // FALSE on negative alarm time
        }
    else if (_dtAlarm.usMode & AM_ACTIVE)
        {
        /* hours between alarm and now */
        lchms = _dtAlarm.dtAlarm.hours - pdtActual->hours;

        if (_dtAlarm.usMode & AM_SINGLE)
            {
            DebugE (D_ALARM, "AlarmSetTimer", "Single alarm");

            /* days from 1900 until alarm time */
            ly = _dtAlarm.dtAlarm.year - ((sM = _dtAlarm.dtAlarm.month - 3) < 0 ? 1 : 0);
            lc1 = ly * 1461 / 4;
            lc2 = (_dtAlarm.dtAlarm.month + (sM < 0 ? 13 : 1)) * 153 / 5;
            lDays = lc1 + lc2 + _dtAlarm.dtAlarm.day + (ly - 1900)/400 - (ly - 1900)/100;

            /* days from 1900 until now */
            ly = pdtActual->year - ((sM = pdtActual->month - 3) < 0 ? 1 : 0);
            lc1 = ly * 1461 / 4;
            lc2 = (pdtActual->month + (sM < 0 ? 13 : 1)) * 153 / 5;
            lDays -= lc1 + lc2 + pdtActual->day + (ly - 1900)/400 - (ly - 1900)/100;
            }
        else
            {
            DebugE (D_ALARM, "AlarmSetTimer", "Multiple alarm");

            lDays = 0;
            lchms = (lchms + HOURPERDAY) % HOURPERDAY;
            }

        if ((lDays < 0) || (lchms < 0) && (lDays == 0))             // error: negative alarm time
            {
            DebugE (D_ALARM, "AlarmSetTimer", "-- time < 0 --");
            bRC = FALSE;
            }
        else
            {
            /* number of minutes */
            lchms *= 60;
            lchms += _dtAlarm.dtAlarm.minutes - pdtActual->minutes;

            /* number of seconds */
            lchms *= 60;
            lchms += _dtAlarm.dtAlarm.seconds - pdtActual->seconds;

            if (lchms <= 0)
                {
                lchms += HOURS_24S;
                if (_dtAlarm.usMode & AM_SINGLE)
                    lDays--;
                }

            DebugULd (D_ALARM, "AlarmSetTimer", "lDays", lDays);
            DebugULd (D_ALARM, "AlarmSetTimer", "TVal ", lchms);

            if (lDays >= 0)
                {
                _timerResources.ulDays       = lDays;               // number of days
                _timerResources.ulTimerValue = 1000 * lchms;        // number of milliseconds
                bRC = TRUE;
                }
            else
                bRC = FALSE;                                        // FALSE on negative alarm time
            }
        }
    else
        {
        DebugE (D_ALARM, "AlarmSetTimer", "Alarm stopped");
        bRC = FALSE;
        }

    _dtAlarm.bIsAlarmActive = bRC;

    /* post semaphore to re-set timer */
    _timerResources.ulPostReason = TR_VALUECHANGED;                 // start timer
    DosPostEventSem (_timerResources.hSemTimer);

    DebugULx (D_ALARM, "AlarmSetTimer", "rc", bRC);
    return bRC;
    }

/*******************************************************************\
    AlarmThread: Alarm-Thread
    Input: somSelf: Pointer to Instance
\*******************************************************************/
VOID _Optlink AlarmThread (WPDCF77 *somSelf)
    {
    WPDCF77Data *somThis;
    PROGDETAILS ProgDetails;
    DATETIME    dtActual;
    BOOL        bAlarmOK;
    ULONG       ulPostCount;
    SHORT       si;
    APIRET      rc;

    somThis = WPDCF77GetData (somSelf);

    /* create event-semaphore for this timer */
    if ((rc = DosCreateEventSem (NULL,
                                 &_timerResources.hSemTimer,
                                 DC_SEM_SHARED,
                                 FALSE)) != NO_ERROR)
        {
        /* error: unable to create semaphore */
        _timerResources.hSemTimer = 0;
        }
    else
        {
        /* start timer */
        if (!(rc = AlarmInit (somThis)))
            {
            /* stay in loop until cancel message */
            for (;;)
                {
                DebugE (D_ALARM, "AlarmThread", "Wait loop entered");

                /* Possible events for post of semaphore:   */
                /* 1. timer elapsed       (TR_TMR_EXPIRED)  */
                /* 2. PM stopped alarm    (TR_ALARMSTOPPED) */
                /* 3. timer value changed (TR_VALUECHANGED) */
                rc = DosWaitEventSem ((HEV)_timerResources.hSemTimer, SEM_INDEFINITE_WAIT);
                if ((rc == ERROR_INVALID_HANDLE) || (rc == ERROR_NOT_ENOUGH_MEMORY))
                    break;

                DebugULd (D_ALARM, "AlarmThread", "Semaphore posted, rc", rc);
                DosResetEventSem ((HEV)_timerResources.hSemTimer, &ulPostCount);

                switch (_timerResources.ulPostReason)
                    {
                    /* alarm time changed */
                    case TR_VALUECHANGED:
                        DebugE (D_ALARM, "AlarmThread", "TR_VALUECHANGED");
                        if (_timerResources.hTimer != NULLHANDLE)
                            {
                            DosStopTimer (_timerResources.hTimer);
                            _timerResources.hTimer = NULLHANDLE;
                            DebugE (D_ALARM, "AlarmThread", "Timer stopped");
                            }
                        rc = AlarmInit (somThis);
                        break;

                    /* timer elapsed */
                    case TR_TMR_EXPIRED:
                        if (_timerResources.ulDays)
                            {
                            /* alarm time not yet reached */
                            DebugE (D_ALARM, "AlarmThread", "Timer elapsed");
                            _timerResources.ulDays--;
                            _timerResources.ulTimerValue = HOURS_24MS;
                            DebugULd (D_ALARM, "AlarmThread", "Still days", _timerResources.ulDays);
                            DebugULd (D_ALARM, "AlarmThread", "Still Secs", _timerResources.ulTimerValue/1000);
                            rc = AlarmInit (somThis);
                            }
                        else
                            {
                            /* alarm time reached */
                            DebugE (D_ALARM, "AlarmThread", "--- ALARM ---");
                            bAlarmOK = _dtAlarm.bIsAlarmActive;
                            if (!(_dtAlarm.usMode & (AM_SINGLE | AM_REMINDER)))
                                {
                                GetDateTimeOffs (_lLOffset, &dtActual);
                                /* conversion of weekday: Monday = day 0; Sunday = day 6 */
                                /* AM_MON is bit 8                                       */
                                dtActual.weekday = (dtActual.weekday + 6) % 7 + 8;
                                if (!(_dtAlarm.usMode & (1 << dtActual.weekday)))
                                    bAlarmOK = FALSE;
                                }
                            if (bAlarmOK && _dtAlarm.usMode & (AM_ACTIVE | AM_REMINDER))
                                {
                                if (_dtAlarm.usMode & AM_MSGBOX)
                                    {
                                    /* message box */
                                    WinPostMsg (_hwndClient, WM_MESSAGE,
                                                (MPARAM)IDMSG_ALARMEXPIRED,
                                                (MPARAM)(MB_OK | MB_INFORMATION));
                                    }
                                if (_dtAlarm.usMode & AM_AUDIO)
                                    {
                                    /* alarm sound */
                                    DebugE (D_ALARM, "AlarmThread => sound file: ", _szAlarmSoundFile);
                                    DebugE (D_ALARM, "AlarmThread => audio supp: ", (bIsAudio ? "TRUE" : "FALSE"));
                                    if ((_szAlarmSoundFile[0] != '\0') && bIsAudio)
                                        {
                                        DebugE (D_ALARM, "AlarmThread", "Post message");
                                        WinPostMsg (_hwndClient, WM_PLAY_MMFILE, 0, 0);
                                        }
                                    else
                                        {
                                        for (si = 0; ulFreq[si] != 0; si++)
                                            DosBeep (ulFreq[si], 50);
                                        }
                                    }
                                if (_dtAlarm.usMode & AM_LAUNCH)
                                    {
                                    /* start program */
                                    ProgDetails.Length          = sizeof (PROGDETAILS);
                                    ProgDetails.progt.progc     = PROG_DEFAULT;
                                    ProgDetails.progt.fbVisible = SHE_VISIBLE;
                                    ProgDetails.pszTitle        = NULL;
                                    ProgDetails.pszExecutable   = _szAlarmProg;
                                    ProgDetails.pszParameters   = _szAlarmParm;
                                    ProgDetails.pszStartupDir   = _szAlarmDir;
                                    ProgDetails.pszIcon         = NULL;
                                    ProgDetails.pszEnvironment  = NULL;
                                    ProgDetails.swpInitial.fl   = 0;
                                    WinStartApp (NULLHANDLE, &ProgDetails, _szAlarmParm, NULL, 0);
                                    }
                                }
                            if (_dtAlarm.usMode & (AM_SINGLE | AM_REMINDER))
                                {
                                _dtAlarm.bIsAlarmActive = FALSE;
                                _timerResources.ulDays = _timerResources.ulTimerValue = 0;
                                _timerResources.hTimer = NULLHANDLE;
                                _wpRefreshClockView (somSelf);
                                _wpSaveDeferred (somSelf);
                                DebugE (D_ALARM, "AlarmThread", "timerResources reset");
                                }
                            else
                                AlarmReStart (somThis);
                            }
                        break;

                    /* stop timer, end thread */
                    case TR_ALARMSTOPPED:

                    /* Error: invalid value */
                    default:
                        DebugE (D_ALARM, "AlarmThread", "TR_ALARMSTOPPED");
                        if (_timerResources.hTimer != NULLHANDLE)
                            {
                            DosStopTimer (_timerResources.hTimer);
                            _timerResources.hTimer = NULLHANDLE;
                            DebugE (D_ALARM, "AlarmThread", "Timer stopped");
                            }
                        rc = NO_ERROR;
                        break;
                    }

                if (rc != NO_ERROR)
                    break;
                }
            }
        }

    /* post error message */
    DebugULx (D_ALARM, "AlarmThread", "RetCode", rc);
    WinPostMsg (_hwndClient, WM_MESSAGE, (MPARAM)IDMSG_ERRORSTARTALARM, (MPARAM)(MB_OK | MB_ERROR));

    /* close event semaphore */
    if (_timerResources.hSemTimer)
        DosCloseEventSem (_timerResources.hSemTimer);

    memset (&_timerResources, '\0', sizeof (TIMER_RESOURCES));

    _endthread();
    return;
    }

/*******************************************************************\
    th1sTimer: timer thread
\*******************************************************************/
VOID _Optlink th1sTimer (void *pArg)
    {
    ULONG  i, ulPostCount;
    HTIMER hTimer;
    HEV    hevTimer;
    APIRET rc;

    pArg;       /* avoid compiler warning */

    DebugE (D_TIMER, "th1sTimer", "Enter");

    /* create semaphore */
    if (!DosCreateEventSem (NULL, &hevTimer, DC_SEM_SHARED, FALSE))
        {

        /* create timer */
        if (!DosStartTimer (1000, (HSEM)hevTimer, &hTimer))
            {

            DebugULx (D_TIMER, "th1sTimer", "HSEM", hevTimer);
            DebugULx (D_TIMER, "th1sTimer", "HTIMER", hTimer);

            /* timer loop; after each post of semaphore a message */
            /* is posted to each window in the hwndTimers list    */
            while (!(rc = DosWaitEventSem (hevTimer, SEM_INDEFINITE_WAIT)))
                {
                DebugE (D_TIMER, "th1sTimer", "Semaphore posted");
                DosResetEventSem (hevTimer, &ulPostCount);

                /* post message to hwndTimer[] */
                for (i = 0; i < MAXTIMERS; i++)
                    if (hwndTimers[i] != NULLHANDLE)
                        WinPostMsg (hwndTimers[i], WM_1STIMER, MPVOID, MPVOID);

                /* second counter for queries: Query occurs max. 1 time per second     */
                /* The old value will be returned on each query within the same second */
                ulTimerCntr++;
                }
            DebugULd (D_TIMER, "th1sTimer", "rc", rc);
            }
        else
            DebugE (D_TIMER, "th1sTimer", "Error 1");
        }
    else
        DebugE (D_TIMER, "th1sTimer", "Error 2");

    DebugE (D_TIMER | D_DIAG, "th1sTimer", "ERROR: Timer thread died");
    _endthread ();
    return;
    }
