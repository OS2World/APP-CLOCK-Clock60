#define INCL_DOSMEMMGR
#define INCL_DOSPROCESS
#include <os2.h>

#include <stdio.h>

#include "dcfioctl.h"

#define DCF77SharedMem  SHMEM

int main (void)
    {
    PSHMEM pShMem;

    if (DosGetNamedSharedMem ((PPVOID)&pShMem, SHMEMNAME, PAG_READ | PAG_WRITE))
        {
        printf ("No shared memory\n");
        return 0;
        }

    printf ("SharedMem::flProgs:                   %x\n", pShMem->flProgs);
    printf ("SharedMem::ulTermReason:              %x\n", pShMem->ulTermReason);
    printf ("SharedMem::bVerMajor:                 %d\n", pShMem->dcf77Data.bVerMajor);
    printf ("SharedMem::bVerMinor:                 %d\n", pShMem->dcf77Data.bVerMinor);
    printf ("SharedMem::ulInterval:                %d\n", pShMem->dcf77Data.ulInterval);
    printf ("SharedMem::bSendSrvrTime:             %s\n", pShMem->dcf77Data.bSendSrvrTime ? "True" : "False");
    printf ("SharedMem::dcf77Offset.sOffset:       %d\n", pShMem->dcf77Data.dcf77Offset.sOffset);
    printf ("SharedMem::dcf77Offset.flUTC:         %d\n", pShMem->dcf77Data.dcf77Offset.flUTC);
    printf ("SharedMem::dcf77Status.cDrvType:      %d\n", pShMem->dcf77Data.dcf77Status.cDrvType);
    printf ("SharedMem::dcf77Status.cVerMajor:     %d\n", pShMem->dcf77Data.dcf77Status.cVerMajor);
    printf ("SharedMem::dcf77Status.cVerMinor:     %d\n", pShMem->dcf77Data.dcf77Status.cVerMinor);
    printf ("SharedMem::dcf77Status.bPolarity:     %d\n", pShMem->dcf77Data.dcf77Status.bPolarity);
    printf ("SharedMem::dcf77Status.flStatus:    0x%X\n", pShMem->dcf77Data.dcf77Status.flStatus);
    printf ("SharedMem::dcf77Status.uscBadSeconds: %d\n", pShMem->dcf77Data.dcf77Status.uscBadSeconds);
    printf ("SharedMem::dcf77Status.uscGoodSeconds:%d\n", pShMem->dcf77Data.dcf77Status.uscGoodSeconds);
    printf ("SharedMem::dcf77Status.uscTimeSet:    %d\n", pShMem->dcf77Data.dcf77Status.uscTimeSet);
    printf ("SharedMem::dcf77Status.flMaskST     0x%x\n", pShMem->dcf77Data.dcf77Status.flMaskST);
    printf ("SharedMem::dcf77Status.flMaskSS     0x%x\n", pShMem->dcf77Data.dcf77Status.flMaskSS);
    printf ("SharedMem::dcf77Status.flMaskTO     0x%x\n", pShMem->dcf77Data.dcf77Status.flMaskTO);
    printf ("SharedMem::dcf77Status.flMaskIV     0x%x\n", pShMem->dcf77Data.dcf77Status.flMaskIV);
    printf ("SharedMem::dcf77Status.flMaskLN     0x%x\n", pShMem->dcf77Data.dcf77Status.flMaskLN);
    printf ("SharedMem::dcf77DateTime.hours:       %d\n", pShMem->dcf77Data.dcf77DateTime.hours);
    printf ("SharedMem::dcf77DateTime.minutes:     %d\n", pShMem->dcf77Data.dcf77DateTime.minutes);
    printf ("SharedMem::dcf77DateTime.seconds:     %d\n", pShMem->dcf77Data.dcf77DateTime.seconds);
    printf ("SharedMem::dcf77DateTime.day:         %d\n", pShMem->dcf77Data.dcf77DateTime.day);
    printf ("SharedMem::dcf77DateTime.month:       %d\n", pShMem->dcf77Data.dcf77DateTime.month);
    printf ("SharedMem::dcf77DateTime.year:        %d\n", pShMem->dcf77Data.dcf77DateTime.year);
    printf ("SharedMem::dcf77DateTime.timezone:    %d\n", pShMem->dcf77Data.dcf77DateTime.timezone);
    printf ("SharedMem::dcf77Data.bSupply        0x%x\n", pShMem->dcf77Data.dcf77Data.bSupply);
    printf ("SharedMem::dcf77Data.bSignal        0x%x\n", pShMem->dcf77Data.dcf77Data.bSignal);
    printf ("SharedMem::dcf77Data.bPort          0x%x\n", pShMem->dcf77Data.dcf77Data.bPort);
    printf ("SharedMem::dcf77Data.bcRepeat         %d\n", pShMem->dcf77Data.dcf77Data.bcRepeat);
    printf ("SharedMem::dcf77Data.usIOAddr       0x%x\n", pShMem->dcf77Data.dcf77Data.usIOAddr);
    printf ("SharedMem::dcf77Data.usSetInterval    %d\n", pShMem->dcf77Data.dcf77Data.usSetInterval);
    printf ("SharedMem::dcf77Data.usThreshold      %d\n", pShMem->dcf77Data.dcf77Data.usThreshold);

    return 0;
    }

