#define INCL_DOSMEMMGR
#define INCL_DOSPROCESS
#include <os2.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dcfioctl.h"

int main (int argc, char *argv[])
    {
    PSHMEM pShMem;

    if (DosGetNamedSharedMem ((PPVOID)&pShMem, SHMEMNAME, PAG_READ | PAG_WRITE))
        {
        printf ("No shared memory\n");
        return 0;
        }

    if (argc == 1)
        {
        printf ("Syntax: mem [flush|driver|noserver|noclient|status|maskST [maskSS [maskTO [maskIV [maskLN]]]]\n");
        return 0;
        }

    if (argc == 2 && strcmp (argv[1], "flush") == 0)
        {
        memset (pShMem, '\0', sizeof (SHMEM));
        return 0;
        }

    if (argc == 2 && strcmp (argv[1], "driver") == 0)
        {
        pShMem->flProgs = PROG_DRIVER;
        return 0;
        }

    if (argc == 2 && strcmp (argv[1], "noserver") == 0)
        {
        pShMem->flProgs &= ~PROG_SERVER;
        return 0;
        }
    if (argc == 2 && strcmp (argv[1], "noclient") == 0)
        {
        pShMem->flProgs &= ~PROG_CLIENT;
        return 0;
        }

    if (argc == 2 && strcmp (argv[1], "status") == 0)
        {
        pShMem->dcf77Data.dcf77Status.flStatus |= STATUS_BATT + STATUS_TZONE1 + STATUS_CARRIER + STATUS_TIMEVALID;
        return 0;
        }

    switch (argc)
        {
        case 6:
            pShMem->dcf77Data.dcf77Status.flMaskLN = strtol (argv[5], NULL, 0);
        case 5:
            pShMem->dcf77Data.dcf77Status.flMaskIV = strtol (argv[4], NULL, 0);
        case 4:
            pShMem->dcf77Data.dcf77Status.flMaskTO = strtol (argv[3], NULL, 0);
        case 3:
            pShMem->dcf77Data.dcf77Status.flMaskSS = strtol (argv[2], NULL, 0);
        case 2:
            pShMem->dcf77Data.dcf77Status.flMaskST = strtol (argv[1], NULL, 0);
        }

    return 0;
    }

