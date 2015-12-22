/***************************************************************************\
 *
 * PROGRAMMNAME: TIMESERV
 * -------------
 *
 * VERSION: 6.0
 * --------
 *
 * MODULNAME: TIMESERV
 * ----------
 *
 * BESCHREIBUNG:
 * -------------
 *   Server-Programm zur öbertragung der Uhrzeit Åber NetBIOS
 *
 * HINWEISE:
 * ---------
 *   Programmaufruf: TIMESERV [DEBUG]
 *      Bei Angabe von DEBUG wird die Ausgabe in die Tracedatei
 *      $$DCF$$.DBG geschrieben
 *
 * FUNKTIONEN:
 * -----------
 *   BreakHandler ()
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
#define INCL_DOSSEMAPHORES
#define INCL_DOSDATETIME
#include <os2.h>

#include <signal.h>
#include <setjmp.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "..\dcfioctl.h"
#include "..\sharemem.hpp"
#include "..\netbios.hpp"
#include "..\dcf77.hpp"
#include "timeserv.hpp"

/*******************************************************************
   Debug-Definitionen; zu Trace-Zwecken beim Compilieren
      "DEBUG" angeben!
 *******************************************************************/
#define DEBUG

#ifdef DEBUG
  int _flag;
  void wrtTrace (PCHAR pszTitle, ...);

  #define dprintDT(sz,D,M,Y,h,m,s)      if (_flag) wrtTrace (sz, D, M, Y, h, m, s)
  #define dprintS(sz)                   if (_flag) wrtTrace (sz)
#else
  #define dprintDT(sz,D,M,Y,h,m,s)
  #define dprintS(sz)
#endif /* DEBUG */

#define INTERVAL_ERRWAIT    1

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

int main (int argc, char *argv[])
    {
    ULONG           ulInterval, ulWaitInterval, ulErrInterval;
    ULONG           ulcPost;
    BOOL            bValid;
    HEV             hevServer;
    USHORT          usAdapter;
    DCF77SharedMem *pdcf77ShMem;
    DCF77_ALLDATA   SrvrData;

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

    /* Semaphore anlegen */
    if (DosCreateEventSem ((PSZ)SEMNAME, &hevServer, 0, FALSE))
        {
        fputs (ERROR_SEMAPHORE, stderr);
        return 1;
        }

    /* Kommunikation mit dem Uhr-Treiber aufbauen */
    /*  Die Daten werden Åber das IOCTL-Interface */
    /*  abgeholt und in einem privaten Speicher   */
    /*  abgelegt.                                 */
    dcf77QueryData dcf77Drv;
    if (!dcf77Drv.isOK ())
        {
        fputs (ERROR_DRIVER, stderr);
        return 1;                       // Fehler: Treiber nicht gefunden
        }

    /* Shared memory initialisieren */
    DCF77Communication dcf77Comm;
    dcf77Comm.CreateShMem (PROG_SERVER);
    if (dcf77Comm.getDCFShMemPtr () == NULL)
        {
        fputs (ERROR_ALLOC, stderr);    // Fehler beim allokieren des Shared memory
        return 1;                       //   oder Programm bereits aktiv
        }

    if (dcf77Comm.isClient ())
        {
        fputs (ERROR_CLIENT, stderr);
        return 1;                       // Fehler: LAN::Time-Client bereits aktiv
        }

    /* Netbios-Verbindung aufbauen */
    NetBIOS nb30;

    if (!nb30.isOK (0))             /* ??? */
        {
        fputs (ERROR_NETBIOS, stderr);
        return 1;                       // Fehler: NetBIOS-Problem
        }

    /* ^C-Handling */
    if (setjmp (jbuf))
        return 0;

    if (Break == b_break)
        return 0;

    Break = b_running;

    ulErrInterval = 0;
    usAdapter     = 0;
    while (TRUE)
        {
        /* Daten holen */
        dcf77Drv.getDCFData ();
        dprintDT ((dcf77Drv.getDCFDataPtr ()->dcf77Status.flStatus & STATUS_TIMEVALID) ? TXT_VALID : TXT_INVALID,
            dcf77Drv.getDCFDataPtr ()->dcf77DateTime.day,
            dcf77Drv.getDCFDataPtr ()->dcf77DateTime.month,
            dcf77Drv.getDCFDataPtr ()->dcf77DateTime.year,
            dcf77Drv.getDCFDataPtr ()->dcf77DateTime.hours,
            dcf77Drv.getDCFDataPtr ()->dcf77DateTime.minutes,
            dcf77Drv.getDCFDataPtr ()->dcf77DateTime.seconds);

        /* Intervall holen, falls Uhr aktiv */
        pdcf77ShMem = dcf77Comm.getDCFShMemPtr ();
        ulInterval = pdcf77ShMem->flProgs & PROG_CLOCK ? pdcf77ShMem->dcf77Data.ulInterval : DEFAULT_INTERVAL;
        if (ulInterval < DEFAULT_INTERVAL)
            ulInterval = DEFAULT_INTERVAL;
        ulWaitInterval = ulInterval;

        if (dcf77Drv.getDCFDataPtr ()->dcf77Status.flStatus & STATUS_TIMEVALID)
            bValid = TRUE;
        else
            {
            /* fehlerhafte Uhrzeit; Falls ulInterval abgelaufen ist => Trotzdem versenden */
            bValid = FALSE;
            ulErrInterval += INTERVAL_ERRWAIT;
            ulWaitInterval = INTERVAL_ERRWAIT;
            }

        /* Im Fehlerfall je nach Einstellung Serverzeit senden */
        if (!bValid && (ulErrInterval > ulInterval) && pdcf77ShMem->dcf77Data.bSendSrvrTime)
            {
            DATETIME dt;
            PDCF77_DATETIME pdt = &(dcf77Drv.getDCFDataPtr ()->dcf77DateTime);

            DosGetDateTime (&dt);
            pdt->hours    = dt.hours;
            pdt->minutes  = dt.minutes;
            pdt->seconds  = dt.seconds;
            pdt->day      = dt.day;
            pdt->month    = dt.month;
            pdt->year     = dt.year;
            pdt->timezone = TZ_UNK;
            }

        if (bValid || (ulErrInterval > ulInterval))
            {
            /* Es sind Daten zum Versenden vorhanden */
            SrvrData = *dcf77Drv.getDCFDataPtr ();
            SrvrData.bSize      = sizeof (DCF77_ALLDATA);
            SrvrData.bVerMajor  = PROG_VER_MAJOR;
            SrvrData.bVerMinor  = PROG_VER_MINOR;
            SrvrData.ulInterval = ulInterval;

            /* éndern der Maskierung */
            SrvrData.dcf77Status.flMaskTO &= !MASK_TO_ALL;

            /* Daten senden */
            dprintS (TXT_BEGINSEND);
            nb30.sendData (usAdapter, (void *)&SrvrData, sizeof (DCF77_ALLDATA));
            if (!nb30.isOK (usAdapter))
                {
                fputs (ERROR_WRITE, stderr);
                dprintS (TXT_ENDSENDERR);
                }
            else
                dprintS (TXT_ENDSENDOK);
            ulErrInterval = 0;
            }
        dprintS ("\n");

        usAdapter = nb30.getNextAvlAdapter (usAdapter);

        DosWaitEventSem (hevServer, ulWaitInterval * 1000 / nb30.getNumOfAdapters ());
        DosResetEventSem (hevServer, &ulcPost);
        }
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