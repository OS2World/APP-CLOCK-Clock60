#define INCL_DOSMEMMGR
#define INCL_DOSPROCESS
#include <os2.h>

#include <string.h>

#include "dcfioctl.h"
#include "sharemem.hpp"

DCF77Communication::DCF77Communication (ULONG flProg)
    {
    if (DosGetNamedSharedMem ((PPVOID)&pdcf77ShMem, (PSZ)SHMEMNAME, PAG_READ | PAG_WRITE))
        {
        if (DosAllocSharedMem ((PPVOID)&pdcf77ShMem, (PSZ)SHMEMNAME, sizeof (DCF77SharedMem),
                               PAG_COMMIT | PAG_WRITE | PAG_READ))
            {
            pdcf77ShMem = NULL;             // Fehler: Shared-Memory nicht gefunden
            return;
            }
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

DCF77Communication::~DCF77Communication ()
    {
    if (pdcf77ShMem)
        pdcf77ShMem->flProgs &= ~flThisProg;
    return;
    }

DCF77SharedMem *DCF77Communication::getDCFShMemPtr ()
    {
    return pdcf77ShMem;
    }

DCF77_ALLDATA *DCF77Communication::getDCFDataPtr ()
    {
    return &dcf77Data;
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