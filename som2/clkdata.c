/***************************************************************************\
 *
 * PROGRAM NAME: WPDCF77
 * -------------
 *
 * VERSION: 6.0
 * --------
 *
 * MODULE NAME: CLKDATA
 * ------------
 *
 * DESCRIPTION:
 * ------------
 *   Contains the global variables of the clock
 *
 * NOTES:
 * ------
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
#define INCL_DOSDATETIME

#include <os2.h>

#include "wpdcf77.h"

#include "clock.h"
#include "dcfioctl.h"
#include "clkdata.h"

#include "wpdcf.h"

/*--------------------------------------------------------------*\
 *  Global variables
\*--------------------------------------------------------------*/
HAB      hab;                       // anchor block is filled by clsInitData
HMODULE  hmod = NULLHANDLE;         // module handle

ULONG    ulcTimers = 0;             // number of registered timers
HWND     hwndTimers[MAXTIMERS];

HFILE    hDrvr = -1;                // handle for DCF77 IOCtl's; -1: driver not found
PSHMEM   pShMem = NULL;             // pointer to shared memory
UCHAR    cDrvType = 0;              // driver type

PUSHORT  pusAdjBuffer = NULL;       // pointer to buffer for antenna adjustment
ULONG    ulAdjBufferSize;           // size of pusAdjBuffer in byte
ULONG    ulMaxPixel;                // max. number of measurement points in pusAdjBuffer
TID      tidGraphThread = 0;        // thread handle for antenna adjustment

ULONG    ulDCFDelta;                // subsidence of carrier of DCF77 station
PUSHORT  pGraphData = NULL;         // pointer to buffer for reception quality
ULONG    ulIndexGrphData;           // index of currently handled entry in pGraphData
ULONG    ulcGraphInData;            // number of added data in pGraphData

ULONG    ulGraphView      = 0;      // view type for graphical window
WPDCF77 *somSelfGraphView = NULL;   // object pointer of instance, which has opened the graphical window

ULONG    ulStatusTStamp   = 0;      // time stamp for last status queryletzte Statusabfrage
ULONG    ulDataTStamp     = 0;      // time stamp for last data query
ULONG    ulOffsTStamp     = 0;      // time stamp for last offset query
ULONG    ulTimeTStamp     = 0;      // time stamp for last time query
ULONG    ulCoordTStamp    = 0;      // time stamp for last coordinate query
ULONG    ulTimerCntr      = 1;      // second counter for time stamp comparison
ULONG    ulGetDCFData     = GET_OK; // control variable for GetDCFData ()
DCF77_STATUS    StatDCF;            // buffer for DCF state
DCF77_DATETIME  DtDCF;              // buffer for date/time from driver
DCF77_COORD     CoordGPS;           // data structure for GPS position from driver

BOOL     bIsAudio;                  // TRUE: multimedia support installed on system

// persistent instance data of meta class
DCF77_DATA      DataDCF;            // data structure for driver settings
DCF77_OFFSET    OffsDCF;            // data structure for driver offset settings

CHAR     szLocName[CCHMAXTZONE];    // buffer for name of time zones in locale file
CHAR     szLocFile[CCHMAXPATH];     // path+filename of locale file

ULONG    ulSrvrInterval;            // interval for LAN::Time
BOOL     bSendSrvrTime;             // True: send server time, if reception error

CHAR     szClsGongFile[CCHMAXPATH]; // WAV file for hourly sound of the gong
BOOL     bClsIs1PerHour;            // TRUE: play WAV file only 1x per hour
HOBJECT  hClsRingingObject;         // instance handle of clock with sound
