/***************************************************************************\
 *
 * PROGRAMMNAME: TIME_CS
 * -------------
 *
 * VERSION: 6.0
 * --------
 *
 * MODULNAME: SHAREMEM
 * ----------
 *
 * BESCHREIBUNG:
 * -------------
 *   Klasse zur Kommunikation mit WPS-Uhr Åber Shared Memory
 *
 * HINWEISE:
 * ---------
 *
 * METHODEN:
 * -----------
 *   DCF77Communication::DCF77Communication ()
 *   DCF77Communication::~DCF77Communication ()
 *   DCF77Communication::CreateShMem ()
 *   DCF77Communication::isClient ()
 *   DCF77Communication::isDriver ()
 *   DCF77Communication::isServer ()
 *   DCF77Communication::isClock ()
 *   DCF77Communication::isVersion ()
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
#define INCL_DOSMEMMGR
#define INCL_DOSPROCESS
#include <os2.h>

#include <string.h>

#include "dcfioctl.h"
#include "sharemem.hpp"

DCF77Communication::DCF77Communication ()
    {
    flThisProg = 0;

    if (DosGetNamedSharedMem ((PPVOID)&pdcf77ShMem, (PSZ)SHMEMNAME, PAG_READ | PAG_WRITE))
        pdcf77ShMem = NULL;             // Fehler: Shared-Memory nicht gefunden

    return;
    }

DCF77Communication::~DCF77Communication ()
    {
    if (pdcf77ShMem)
        pdcf77ShMem->flProgs &= ~flThisProg;
    return;
    }

VOID DCF77Communication::CreateShMem (ULONG flProg)
    {
    if (pdcf77ShMem == NULL)
        {
        if (DosAllocSharedMem ((PPVOID)&pdcf77ShMem, (PSZ)SHMEMNAME, sizeof (DCF77SharedMem),
                               PAG_COMMIT | PAG_WRITE | PAG_READ))
            return;                         // Fehler: Shared-Memory nicht gefunden
        else
            memset (pdcf77ShMem, '\0', sizeof (DCF77SharedMem));
        }

    if (pdcf77ShMem->flProgs & flProg)
        pdcf77ShMem = NULL;                 // Fehler: Programm ist bereits aktiv
    else
        {
        flThisProg = flProg;
        pdcf77ShMem->flProgs |= flProg;
        }
    return;
    }

BOOL DCF77Communication::isClient ()
    {
    return pdcf77ShMem->flProgs & PROG_CLIENT ? TRUE : FALSE;
    }

BOOL DCF77Communication::isDriver ()
    {
    return pdcf77ShMem->flProgs & PROG_DRIVER ? TRUE : FALSE;
    }

BOOL DCF77Communication::isServer ()
    {
    return pdcf77ShMem->flProgs & PROG_SERVER ? TRUE : FALSE;
    }

BOOL DCF77Communication::isClock ()
    {
    return pdcf77ShMem->flProgs & PROG_CLOCK ? TRUE : FALSE;
    }

BOOL DCF77Communication::isVersion ()
    {
    return pdcf77ShMem->dcf77Data.bSize     == sizeof (DCF77_ALLDATA) &&
           pdcf77ShMem->dcf77Data.bVerMajor == PROG_VER_MAJOR         &&
           pdcf77ShMem->dcf77Data.bVerMinor == PROG_VER_MINOR;
    }

