/***************************************************************************\
 *
 * PROGRAM NAME: WPDCF77
 * -------------
 *
 * VERSION: 6.0
 * --------
 *
 * MODULE NAME: BITMAP
 * ------------
 *
 * DESCRIPTION:
 * ------------
 *   Routines to handle and display bitmaps
 *
 * NOTES:
 * ------
 *
 * FUNCTIONS:
 * -----------
 *   readBuffer ()
 *   calcDataSize ()
 *   loadBitmap ()
 *   createBitmap ()
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
#define INCL_DOSFILEMGR
#define INCL_GPI
#define INCL_DOS

#include <os2.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>

#include "bitmap.h"
#include "clkmem.h"

typedef union _UBMPFILEHEADER
    {
    BITMAPFILEHEADER    bfh;
    BITMAPFILEHEADER2   bfh2;
    } UBMPFILEHEADER;
typedef UBMPFILEHEADER *PUBMPFILEHEADER;

typedef union _UBMPINFO
    {
    BITMAPINFO          bmi;
    BITMAPINFO2         bmi2;
    } UBMPINFO;
typedef UBMPINFO *PUBMPINFO;

BOOL readBuffer (HFILE hFile, PVOID pBuffer, ULONG ulcBuffer)
    {
    ULONG ulLen;

    if (!DosRead (hFile, pBuffer, ulcBuffer, &ulLen) &&
        ulLen == ulcBuffer)
        return TRUE;
    else
        return FALSE;
    }

ULONG calcDataSize (ULONG cx, ULONG cy, ULONG ulBitsPerPel)
    {
    ULONG ulcData;

    ulcData  = (cx * ulBitsPerPel +7) / 8;
    ulcData += (4 - (ulcData%4)) & 0x3;
    ulcData *= cy;

    return ulcData;
    }

ULONG loadBitmap (PSZ pszFilename, PUBMPINFO *ppunBmpInfo, PBYTE *ppData, PPOINTL pHotspot)
    {
    HFILE            hFile;                     // file handling
    ULONG            ulAction;
    ULONG            rc;
    UBMPFILEHEADER   unBmpFileHdr;              // read bitmap header
    BOOL             bRC;
    ULONG            ulcInfoHeader;
    ULONG            cx, cy;
    ULONG            ulcPalette;                // read palette
    ULONG            ulSize;
    PVOID            pOffset;
    ULONG            ulcData;                   // read bitmap data

    *ppunBmpInfo = NULL;
    *ppData      = NULL;
    rc           = BMPERR_INVBMP;

    /* open file */
    bRC = DosOpen (pszFilename,
                   &hFile,
                   &ulAction,
                   0,
                   FILE_NORMAL,
                   OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,
                   OPEN_FLAGS_FAIL_ON_ERROR | OPEN_SHARE_DENYWRITE | OPEN_ACCESS_READONLY,
                   NULL) ? FALSE : TRUE;

    if (!bRC)
        return BMPERR_OPENERR;

    memset (&unBmpFileHdr, 0, sizeof (UBMPFILEHEADER));

    /* read BITMAPFILEHEADER up to BITMAPINFOHEADER-length and check */
    bRC = readBuffer (hFile, (PVOID)&unBmpFileHdr,
                      sizeof (BITMAPFILEHEADER) - sizeof (BITMAPINFOHEADER) + sizeof (ULONG));

    if (!bRC || (unBmpFileHdr.bfh.usType != BFT_BMAP))
        goto Error;

    /* determine bitmap type */
    switch (unBmpFileHdr.bfh.bmp.cbFix)     // (without palette entries)
        {
        case sizeof (BITMAPINFOHEADER):
        case sizeof (WINBITMAPINFOHEADER):
        case sizeof (BITMAPINFOHEADER2):
            break;

        default:
            goto Error;
        }
    ulcInfoHeader = unBmpFileHdr.bfh.bmp.cbFix;

    /* read the rest of BITMAPFILEHEADER(2)s */
    if (!readBuffer (hFile, (PVOID)&unBmpFileHdr.bfh.bmp.cx, ulcInfoHeader - sizeof (ULONG)))
        goto Error;

    /* determine the length of the BITMAPINFO structure including palette info */
    switch (ulcInfoHeader)
        {
        case sizeof (BITMAPINFOHEADER):
            ulcPalette = unBmpFileHdr.bfh.bmp.cPlanes * unBmpFileHdr.bfh.bmp.cBitCount;
            pOffset    = &unBmpFileHdr.bfh.bmp;
            ulcData    = calcDataSize (unBmpFileHdr.bfh.bmp.cx, unBmpFileHdr.bfh.bmp.cy, ulcPalette);
            cx = unBmpFileHdr.bfh.bmp.cx;
            cy = unBmpFileHdr.bfh.bmp.cy;
            break;

        case sizeof (WINBITMAPINFOHEADER):
            ulcInfoHeader = sizeof (BITMAPINFOHEADER2);

        case sizeof (BITMAPINFOHEADER2):
            ulcPalette = unBmpFileHdr.bfh2.bmp2.cPlanes * unBmpFileHdr.bfh2.bmp2.cBitCount;
            pOffset    = &unBmpFileHdr.bfh2.bmp2;
            if (unBmpFileHdr.bfh2.bmp2.ulCompression == BCA_UNCOMP)
                ulcData = calcDataSize (unBmpFileHdr.bfh2.bmp2.cx, unBmpFileHdr.bfh2.bmp2.cy, ulcPalette);
            else
                ulcData = unBmpFileHdr.bfh2.bmp2.cbImage;
            cx = unBmpFileHdr.bfh2.bmp2.cx;
            cy = unBmpFileHdr.bfh2.bmp2.cy;
            break;
        }

    if (ulcPalette >= 24)
        ulSize = ulcPalette = 0;
    else
        {
        ulcPalette = 1 << ulcPalette;
        if (ulcInfoHeader == sizeof (BITMAPINFOHEADER))
            ulSize  = ulcPalette * sizeof (RGB);
        else
            ulSize  = ulcPalette * sizeof (RGB2);
        }

    /* allocate memory for BITMAPINFO(2) structure */
    *ppunBmpInfo = allocMem (ulcInfoHeader + ulSize);
    if (*ppunBmpInfo == NULL)
        goto Error;

    /* fill BITMAPINFO structure with already read data */
    memcpy (*ppunBmpInfo, pOffset, ulcInfoHeader);
    (*ppunBmpInfo)->bmi.cbFix = ulcInfoHeader;  // correction for windows bitmaps

    /* read rest of BITMAPINFO structure (palette data) from file */
    if (ulcPalette)
        {
        if (ulcInfoHeader == sizeof (BITMAPINFOHEADER))
            pOffset = &((*ppunBmpInfo)->bmi.argbColor);
        else
            pOffset = &((*ppunBmpInfo)->bmi2.argbColor);

        if (!(bRC = readBuffer (hFile, (PVOID)pOffset, ulSize)))
            goto Error_Free;
        }

    /* read bitmap data */
    *ppData = allocMem (ulcData);
    if (*ppData == NULL)
        goto Error_Free;

    if (!(bRC = readBuffer (hFile, (PVOID)*ppData, ulcData)))
        {
        freeMem ((PPVOID)ppData);
        ppData = NULL;
        goto Error_Free;
        }

    /* determine hotspot point */
    if (pHotspot)
        {
        (*pHotspot).x = (LONG)unBmpFileHdr.bfh.xHotspot;
        (*pHotspot).y = (LONG)unBmpFileHdr.bfh.yHotspot;
        if ((*pHotspot).x >= cx)
            (*pHotspot).x  = cx;
        if ((*pHotspot).y >= cy)
            (*pHotspot).y  = cy;
        }

    rc = BMPERR_OK;
    goto Exit;

Error_Free:
    /* free allocated memory */
    freeMem ((PPVOID)ppunBmpInfo);
    ppunBmpInfo = NULL;

Error:
Exit:
    /* close file */
    DosClose (hFile);
    return rc;
    }

/*******************************************************************\
    createBitmap: Loads a bitmap file and creates a bitmap handle
                  in the specified presentation space
                  According to OS/2 pointer files, a hotspot is 
                  supported.
    Input:  HPS      hps         : presentation space
            PSZ      pszFilename : file name
    Output: PHBITMAP phBM        : bitmap handle
            PPOINTL  pHotspot    : hotspot in pixel
\*******************************************************************/
ULONG createBitmap (HPS hps, PSZ pszFilename, PHBITMAP phBM, PPOINTL pHotspot)
    {
    ULONG     rc = BMPERR_OK;
    PUBMPINFO punBmpInfo;
    PBYTE     pData;

    *phBM = NULLHANDLE;

    if (strlen (pszFilename) && hps)
        {
        if ((rc = loadBitmap (pszFilename, &punBmpInfo, &pData, pHotspot)) == BMPERR_OK)
            {
            if (punBmpInfo && pData)
                {
                *phBM = GpiCreateBitmap (hps,
                                         (PBITMAPINFOHEADER2)punBmpInfo,
                                         CBM_INIT,
                                         pData,
                                         (PBITMAPINFO2)punBmpInfo);
                }
            if (*phBM == NULLHANDLE)
                rc = BMPERR_INVBMP;
            if (punBmpInfo)
                freeMem ((PPVOID)&punBmpInfo);
            if (pData)
                freeMem ((PPVOID)&pData);
            }
        }

    return rc;
    }
