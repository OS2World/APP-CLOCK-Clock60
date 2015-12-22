/***************************************************************************\
 *
 * PROGRAMMNAME: TIMECLNT
 * -------------
 *
 * VERSION: 6.0
 * --------
 *
 * MODULNAME: TIMECLNT
 * ----------
 *
 * BESCHREIBUNG:
 * -------------
 *   Client-Programm zur öbertragung der Uhrzeit Åber NetBIOS
 *
 * HINWEISE:
 * ---------
 *   Programmaufruf: TIMECLNT [DEBUG]
 *      Bei Angabe von DEBUG wird die Ausgabe in die Tracedatei
 *      $$DCF$$.DBG geschrieben
 *
 * FUNKTIONEN:
 * -----------
 *   BreakHandler ()
 *   thWatchdog ()
 *   thReceive ()
 *   main ()
 *   wrtTrace ()
 *
 *  Ver.    Date      Comment
 *  ----    --------  -------
 *  5.00    07-11-00  y2k-Fehlerbehebung
 *  5.20    06-09-01  Fehlerkorrektur Audiodaten
 *  5.30    01-16-02  Implementierung der Erinnerungsfunktion
 *  5.40    11-22-03  Fehlerkorrektur Erinnerungsfunktion
 *
 *  Copyright (C) noller & breining software 1995...2004
 *
\******************************************************************************/
#define INCL_DOSDATETIME
#define INCL_DOSERRORS
#define INCL_DOSSEMAPHORES
#define INCL_DOSPROCESS
#include <os2.h>

#include <signal.h>
#include <setjmp.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "dcfioctl.h"
#include "sharemem.hpp"
#include "netbios.hpp"
#include "dcf77.hpp"
#include "timeclnt.hpp"

/*******************************************************************
   Debug-Definitionen; zu TRACE-Zwecken beim Compilieren
      "DEBUG" angeben!
 *******************************************************************/
#define DEBUG

#ifdef DEBUG
  int  _flag;
  void wrtTrace (PCHAR pszTitle, ...);

  #define dprintDT(sz,D,M,Y,h,m,s)      if (_flag) wrtTrace (sz, D, M, Y, h, m, s)
  #define dprintUL(sz,l)                if (_flag) wrtTrace (sz, l)
  #define dprintS(sz)                   if (_flag) wrtTrace (sz)
#else
  #define dprintDT(sz,D,M,Y,h,m,s)
  #define dprintUL(sz,l)
  #define dprintS(sz)
#endif /* DEBUG */

/* Break und jbuf dienen zur ^C-Steuerung;             *\
 * Break = b_init: Signalhandler setzt Break = b_break *
 *                 => Schleife wird nicht gestartet    *
 * Break = b_running: Normalzustand; jbuf ist gesetzt  *
 *                 => ^C springt auf diese Marke       *
 * Break = b_break: Es wurde bereits ^C erkannt        *
 *                 => Programm beenden; Destruktoren   *
\*                    werden aufgerufen!               */
static enum BREAK {b_init, b_running, b_break} Break = b_init;
static jmp_buf jbuf;

static HEV hevWatchdog = NULLHANDLE;
static HEV hevReceive  = NULLHANDLE;

struct ReceiveData
    {
    DCF77Communication *pdcf77Comm;
    NetBIOS            *pnb30;
    USHORT              usAdapter;
    };

static void BreakHandler (int sig)
    {
    /* BreakHandler neu registrieren */
    signal (sig,  (_SigFunc)BreakHandler);
    switch (Break)
        {
        case b_init:
            fputs (ERROR_BREAK, stderr);
            Break = b_break;
            break;

        case b_running:
            longjmp (jbuf, 1);
            break;
        }

    return;
    }

void _Optlink thWatchdog (void *arg)
    {
    ULONG ulInterval, ulcPost;
    DCF77SharedMem *pdcf77ShMem = (DCF77SharedMem *)arg;

    while (TRUE)
        {
        ulInterval = pdcf77ShMem->flProgs & PROG_CLOCK ? pdcf77ShMem->dcf77Data.ulInterval : DEFAULT_INTERVAL;
        if (ulInterval < DEFAULT_INTERVAL)
            ulInterval = DEFAULT_INTERVAL;

        if (DosWaitEventSem (hevWatchdog, ulInterval*1500) == ERROR_TIMEOUT)
            {
            fputs (ERROR_SERVER, stderr);
            pdcf77ShMem->dcf77Data.dcf77Status.flStatus = 0;
            }
        else
            DosResetEventSem (hevWatchdog, &ulcPost);
        }
    }

void _Optlink thReceive (void *arg)
    {
    DCF77_ALLDATA       dcf77Data;
    DCF77SharedMem     *pdcf77ShMem;
    DCF77Communication *pdcf77Comm;
    DCF77_DATETIME     *pdcf77dt;
    NetBIOS            *pnb30;
    DATETIME            dt;
    BOOL                bValid;

    dprintS (TXT_RECEIVE);

    pdcf77Comm  = ((ReceiveData *)arg)->pdcf77Comm;
    pdcf77ShMem = pdcf77Comm->getDCFShMemPtr ();
    pnb30       = ((ReceiveData *)arg)->pnb30;

    while (TRUE)
        {
        dprintS (TXT_ENTERGET);
        bValid = pnb30->getDataBlock (((ReceiveData *)arg)->usAdapter,
                                      (void *)&dcf77Data,
                                      sizeof (DCF77_ALLDATA));

        /* Daten in den Zielpuffer umkopieren; andere Threads blockieren */
        dprintS (TXT_EXITGET);
        DosEnterCritSec ();
        pdcf77ShMem->dcf77Data = dcf77Data;
        DosExitCritSec ();
        DosPostEventSem (hevWatchdog);

        /* Hat der Server die richtige Version? */
        if (!pdcf77Comm->isVersion ())
            {
            pdcf77ShMem->ulTermReason = TERM_VERSION;

            DCF77SharedMem *pShMem = pdcf77Comm->getDCFShMemPtr ();
            if (pShMem)
                {
                dprintUL (TXT_VERSMAJOR, pShMem->dcf77Data.bVerMajor);
                dprintUL (TXT_VERSMINOR, pShMem->dcf77Data.bVerMinor);
                dprintUL (TXT_VERCMAJOR, PROG_VER_MAJOR);
                dprintUL (TXT_VERCMINOR, PROG_VER_MINOR);
                }

            dprintS (TXT_VERSION);
            DosPostEventSem (hevReceive);
            break;
            }

        /* Uhr setzen */
        memset (&dt, '\0', sizeof (DATETIME));
        pdcf77dt    = &(pdcf77ShMem->dcf77Data.dcf77DateTime);
        dt.hours    = pdcf77dt->hours;
        dt.minutes  = pdcf77dt->minutes;
        dt.seconds  = pdcf77dt->seconds;
        dt.day      = pdcf77dt->day;
        dt.month    = pdcf77dt->month;
        dt.year     = pdcf77dt->year;
        APIRET rc   = DosSetDateTime (&dt);
        dprintUL (TXT_SETDATETIME, rc);

        dprintDT (bValid ? TXT_VALID : TXT_INVALID,
                  dcf77Data.dcf77DateTime.day,
                  dcf77Data.dcf77DateTime.month,
                  dcf77Data.dcf77DateTime.year,
                  dcf77Data.dcf77DateTime.hours,
                  dcf77Data.dcf77DateTime.minutes,
                  dcf77Data.dcf77DateTime.seconds);
        }

    return;
    }

int main (int argc, char *argv[])
    {
    USHORT         uscAdapter;
    ReceiveData   *pRecData;

#ifdef DEBUG
    /* Argumente parsen */
    _flag = 0;
    if (argc == 2)
        if (stricmp (argv[1], "DEBUG") == 0)
            _flag = 1;
#endif /* DEBUG */

    /* Signal-Handler installieren */
    signal (SIGINT,  (_SigFunc)BreakHandler);
    signal (SIGTERM, (_SigFunc)BreakHandler);
    signal (SIGBREAK,(_SigFunc)BreakHandler);

    /* Shared memory initialisieren */
    DCF77Communication dcf77Comm;
    dcf77Comm.CreateShMem (PROG_CLIENT);
    if (dcf77Comm.getDCFShMemPtr () == NULL)
        {
        fputs (ERROR_CLIENT, stderr);   // Fehler beim allokieren des Shared memory
        return 1;                       //   oder Programm bereits aktiv
        }

    if (dcf77Comm.isServer ())
        {
        fputs (ERROR_ALLOC, stderr);
        return 1;                       // Fehler: LAN::Time-Server bereits aktiv
        }

    /* Netbios-Verbindung aufbauen */
    NetBIOS nb30;

    if (nb30.getNumOfAdapters () == 0)
        {
        fputs (ERROR_NETBIOS, stderr);  // Fehler: NetBIOS-Problem oder keine
        return 1;                       //   Netzwerkadapter installiert
        }

    /* Watchdog zur ServerÅberwachung initialisieren */
    DosCreateEventSem (NULL, &hevWatchdog, 0, FALSE);
    if (_beginthread (thWatchdog, NULL, 0x8000, (void *)dcf77Comm.getDCFShMemPtr ()) == -1)
        {
        fputs (ERROR_THREAD, stderr);
        return 1;                       // Fehler: Thread kann nicht erzeugt werden
        }

    /* Thread fÅr jedes LAN-Adapter starten */
    DosCreateEventSem (NULL, &hevReceive, 0, FALSE);
    uscAdapter = 0;
    do
        {
        pRecData = new ReceiveData;
        pRecData->pdcf77Comm = &dcf77Comm;
        pRecData->pnb30      = &nb30;
        pRecData->usAdapter  = uscAdapter;
        if (_beginthread (thReceive, NULL, 0x8000, (void *)pRecData) == -1)
            {
            fputs (ERROR_THREAD, stderr);
            return 1;               // Fehler: Thread kann nicht erzeugt werden
            }
        } while ((uscAdapter = nb30.getNextAvlAdapter (uscAdapter)) != 0);

    /* ^C-Handling */
    if (setjmp (jbuf))
        return 0;

    if (Break == b_break)
        return 0;

    Break = b_running;

    dprintS (TXT_WAIT);

    /* Thread 1 wartet auf Fehler */
    DosWaitEventSem (hevReceive, SEM_INDEFINITE_WAIT);

    return 1;
    }

#ifdef DEBUG
void wrtTrace (PCHAR sz, ...)
    {
    va_list  parg;
    FILE    *pTracefile = NULL;

    pTracefile = fopen ("$$DCF$$.DBG", "a");

    if (!pTracefile)
        return;

    va_start (parg, sz);            /* Zeiger auf Argumentliste holen */
    vfprintf (pTracefile, sz, parg);
    va_end (parg);                  /* fertig mit Argumentliste */

    fclose (pTracefile);

    return;
    }

#endif /* DEBUG */

