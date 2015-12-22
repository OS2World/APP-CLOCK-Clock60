/***************************************************************************\
 *
 * PROGRAM NAME: WPDCF77
 * -------------
 *
 * VERSION: 6.0
 * --------
 *
 * MODULE NAME: ENTRYFIELD
 * ------------
 *
 * DESCRIPTION:
 * ------------
 *   Subclass of ENTRYFIELD with drop-possibilities
 *
 * NOTES:
 * ------
 *
 * FUNCTIONS:
 * -----------
 *   SubclassEntryField ()
 *   EntryFieldDlgProc ()
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
#define INCL_WINSTDDRAG
#define INCL_WINWINDOWMGR

#include <os2.h>
#include <string.h>

#include "entryfield.h"

/* may be global, because this address is constant in system address space */
static PFNWP pfnwpEntryFieldProc;       // old WindowProc for WinSubclassWindow

CHAR *szRenderingFormat[] =
    {
    NULL,
    "DRF_BITMAP",
    "DRF_WAVE",
    };

CHAR *getRenderingFormat (ULONG ulRenderingType)
    {
    if (ulRenderingType < sizeof (szRenderingFormat) / sizeof (szRenderingFormat[0]))
        return szRenderingFormat[ulRenderingType];

    return NULL;
    }

/*******************************************************************\
    EntryFieldDlgProc: Dialog prozedure for entry field with
                       drop support
    Input: hwnd: window handle
           msg:  message type
           mp1:  parameter 1
           mp2:  parameter 2
\*******************************************************************/
MRESULT EXPENTRY EntryFieldDlgProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
    {
    CHAR        szDir[CCHMAXPATH];
    PDRAGINFO   pDragInfo;
    PDRAGITEM   pDragItem;
    USHORT      usIndicator, cItems;
    ULONG       ulBytes;
    ULONG       ulRenderingType;

    switch (msg)
        {
        case DM_DRAGOVER:
            pDragInfo = (PDRAGINFO)mp1;

            /* access to DRAGINFO structure */
            DrgAccessDraginfo (pDragInfo);

            /* access makes sense? */
            switch (pDragInfo->usOperation)
                {
                /* default operation */
                case DO_DEFAULT:
                    pDragItem = DrgQueryDragitemPtr (pDragInfo, 0);
                    ulBytes   = DrgQueryStrName (pDragItem->hstrContainerName,
                                                 sizeof (szDir),
                                                 szDir);
                    if (!ulBytes)
                        {
                        DrgFreeDraginfo (pDragInfo);
                        return MRFROM2SHORT (DOR_NODROPOP, 0);
                        }
                    break;

                /* DOR_NODROPOP, if operation does not make sense or is unknown */
                default:
                    DrgFreeDraginfo (pDragInfo);
                    return MRFROM2SHORT (DOR_NODROPOP, 0);
                }

            cItems = DrgQueryDragitemCount (pDragInfo);

            /* operation does not make sense, if    */
            /* more than 1 object has been selected */
            if (cItems != 1)
                {
                DrgFreeDraginfo (pDragInfo);
                return MPFROM2SHORT (DOR_NODROPOP, 0);
                }

            ulRenderingType = WinQueryWindowULong (hwnd, QWL_USER);

            /* check rendering format */
            if (DrgVerifyRMF (pDragItem, "DRM_OS2FILE", szRenderingFormat[ulRenderingType]))
                usIndicator = DOR_DROP;
            else
                usIndicator = DOR_NEVERDROP;

            /* free DRAGINFO structure */
            DrgFreeDraginfo (pDragInfo);

            return MRFROM2SHORT (usIndicator, DO_COPY);

        case DM_DROP:
            pDragInfo = (PDRAGINFO)mp1;

            /* access to DRAGINFO structure */
            DrgAccessDraginfo (pDragInfo);

            /* allow access? */
            switch (pDragInfo->usOperation)
                {
                /* default operation */
                case DO_COPY:
                    pDragItem = DrgQueryDragitemPtr (pDragInfo, 0);
                    ulBytes   = DrgQueryStrName (pDragItem->hstrContainerName,
                                                 sizeof (szDir),
                                                 szDir);

                    if (!ulBytes)
                        return MRFROM2SHORT (DOR_NODROPOP, 0);
                    break;

                default:
                    DrgFreeDraginfo (pDragInfo);
                    return MRFROM2SHORT (DOR_NODROPOP, 0);
                }

            cItems = DrgQueryDragitemCount (pDragInfo);

            /* operation does not make sense, if    */
            /* more than 1 object has been selected */
            if (cItems != 1)
                {
                DrgFreeDraginfo (pDragInfo);
                return (MPFROM2SHORT (DOR_NODROPOP, 0));
                }

            ulRenderingType = WinQueryWindowULong (hwnd, QWL_USER);

            /* check rendering format */
            if (DrgVerifyRMF (pDragItem, "DRM_OS2FILE", szRenderingFormat[ulRenderingType]))
                usIndicator = DOR_DROP;
            else
                usIndicator = DOR_NEVERDROP;

            /* determine file name and enter to entry field */
            ulBytes = strlen (szDir);
            DrgQueryStrName (pDragItem->hstrSourceName,
                             sizeof (szDir) - ulBytes,
                             szDir + ulBytes);

            WinSetWindowText (hwnd, szDir);

            /* free DRAGINFO structure */
            DrgFreeDraginfo (pDragInfo);

            return MRFROMLONG (0);
        }

    return (*pfnwpEntryFieldProc)(hwnd, msg, mp1, mp2);
    }

/*******************************************************************\
    SubclassEntryField: Subclass of entry field DlgProc, to
                        support drop of files
    Input: hwnd: window handle of entry fields
\*******************************************************************/
VOID SubclassEntryField (HWND hwnd)
    {
    pfnwpEntryFieldProc = WinSubclassWindow (hwnd, EntryFieldDlgProc);
    return;
    }
