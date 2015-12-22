/***************************************************************************\
 *
 * PROGRAM NAME: WPDCF77
 * -------------
 *
 * VERSION: 6.0
 * --------
 *
 * MODULE NAME: CLKMEM
 * ------------
 *
 * DESCRIPTION:
 * ------------
 *   Routines for memory management
 *
 * NOTES:
 * ------
 *
 * FUNCTIONS:
 * -----------
 *    memnchr ()
 *    allocZeroMem ()
 *    allocMem ()
 *    freeMem ()
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
#define INCL_GPITRANSFORMS
#define INCL_DOSPROCESS

#include <os2.h>
#include <memory.h>

#include "clkmem.h"

/*******************************************************************\
    allocZeroMem: Allocates memory from OS-memory and zeroes it out.
    Input:  ulc: length of buffer
    return: pointer to buffer; NULL => error
\*******************************************************************/
PVOID allocZeroMem (ULONG ulc)
    {
    PVOID p;

    DosAllocMem (&p, ulc, PAG_COMMIT | PAG_READ | PAG_WRITE);
    memset (p, '\0', ulc);
    return p;
    }

/*******************************************************************\
    allocMem: Allocates memory from OS-memory
    Input:  ulc: length of buffer
    return: pointer to buffer; NULL => error
\*******************************************************************/
PVOID allocMem (ULONG ulc)
    {
    PVOID p;

    DosAllocMem (&p, ulc, PAG_COMMIT | PAG_READ | PAG_WRITE);
    return p;
    }

/*******************************************************************\
    freeMem: frees a buffer which was allocates with one of the 
             above functions
    Input: ppBuffer: pointer to pointer of buffer
\*******************************************************************/
VOID freeMem (PPVOID ppBuffer)
    {
    DosFreeMem (*ppBuffer);
    *ppBuffer = NULL;
    return;
    }
