/***************************************************************************\
 *
 * PROGRAMMNAME: WPDCF77
 * -------------
 *
 * VERSION: 5.4
 * --------
 *
 * MODULNAME: HOTSPOT
 * ----------
 *
 * BESCHREIBUNG:
 * -------------
 *   Festlegen des Hotspot-Punktes fÅr Bitmaps
 *
 * HINWEISE:
 * ---------
 *
 * FUNKTIONEN:
 * -----------
*    readBuffer ()
*    getBitmapData ()
*    writeBitmapData ()
*    main ()
 *
 *  Ver.    Date      Comment
 *  ----    --------  -------
 *  1.00    12-31-99  First Release
 *  5.20    06-09-01  Fehlerkorrektur Audiodaten
 *  5.30    01-16-02  Implementierung der Erinnerungsfunktion
 *  5.40    11-22-03  Fehlerkorrektur Erinnerungsfunktion
 *
 *  Copyright (C) noller & breining software 1999...2003
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

VOID getBitmapData (PSZ pszFilename, PPOINTL pptlHotspot, PPOINTL pptlSize)
    {
    HFILE            hFile;                     // Dateihandling
    ULONG            ulAction;
    ULONG            rc;
    UBMPFILEHEADER   unBmpFileHdr;              // Bitmap-Header lesen
    BOOL             bRC;
    ULONG            ulcInfoHeader;
    ULONG            cx, cy;
    ULONG            ulcPalette;                // Palette lesen
    ULONG            ulSize;
    PVOID            pOffset;
    ULONG            ulcData;                   // Bitmap-Daten lesen

    /* Datei îffnen */
    bRC = DosOpen (pszFilename,
                   &hFile,
                   &ulAction,
                   0,
                   FILE_NORMAL,
                   OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,
                   OPEN_FLAGS_FAIL_ON_ERROR | OPEN_SHARE_DENYWRITE | OPEN_ACCESS_READONLY,
                   NULL) ? FALSE : TRUE;

    if (!bRC)
        return;

    memset (&unBmpFileHdr, 0, sizeof (UBMPFILEHEADER));

    /* BITMAPFILEHEADER bis BITMAPINFOHEADER-LÑnge lesen und öberprÅfung */
    bRC = readBuffer (hFile, (PVOID)&unBmpFileHdr,
                      sizeof (BITMAPFILEHEADER) - sizeof (BITMAPINFOHEADER) + sizeof (ULONG));

    if (!bRC || (unBmpFileHdr.bfh.usType != BFT_BMAP))
        goto Error;

    /* Bestimmen des Bitmaptyps */
    switch (unBmpFileHdr.bfh.bmp.cbFix)     // (ohne PaletteneintrÑge)
        {
        case sizeof (BITMAPINFOHEADER):
        case sizeof (WINBITMAPINFOHEADER):
        case sizeof (BITMAPINFOHEADER2):
            break;

        default:
            goto Error;
        }
    ulcInfoHeader = unBmpFileHdr.bfh.bmp.cbFix;

    /* Lesen des Restes des BITMAPFILEHEADER(2)s */
    if (!readBuffer (hFile, (PVOID)&unBmpFileHdr.bfh.bmp.cx, ulcInfoHeader - sizeof (ULONG)))
        goto Error;

    /* Bestimmen der LÑnge der BITMAPINFO-Struktur einschl. Paletteninfo */
    switch (unBmpFileHdr.bfh.bmp.cbFix)
        {
        case sizeof (BITMAPINFOHEADER):
            (*pptlSize).x = unBmpFileHdr.bfh.bmp.cx;
            (*pptlSize).y = unBmpFileHdr.bfh.bmp.cy;
            break;

        case sizeof (BITMAPINFOHEADER2):
        case sizeof (WINBITMAPINFOHEADER):
            (*pptlSize).x = unBmpFileHdr.bfh2.bmp2.cx;
            (*pptlSize).y = unBmpFileHdr.bfh2.bmp2.cy;
            break;
        }

    /* Bestimmen des Hotspot-Punktes */
    (*pptlHotspot).x = (LONG)unBmpFileHdr.bfh.xHotspot;
    (*pptlHotspot).y = (LONG)unBmpFileHdr.bfh.yHotspot;

    goto Exit;

Error:
Exit:
    /* Datei schlie·en */
    DosClose (hFile);
    return;
    }

VOID writeBitmapData (PSZ pszFilename, PPOINTL ptlHotspot)
    {
    HFILE            hFile;                     // Dateihandling
    ULONG            ulAction;
    ULONG            rc;
    UBMPFILEHEADER   unBmpFileHdr;              // Bitmap-Header lesen
    BOOL             bRC;
    ULONG            ulcInfoHeader;
    USHORT           x, y;

    /* Datei îffnen */
    bRC = DosOpen (pszFilename,
                   &hFile,
                   &ulAction,
                   0,
                   FILE_NORMAL,
                   OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,
                   OPEN_FLAGS_FAIL_ON_ERROR | OPEN_SHARE_DENYREADWRITE | OPEN_ACCESS_READWRITE,
                   NULL) ? FALSE : TRUE;

    if (!bRC)
        return;

    x = (SHORT)ptlHotspot->x;
    y = (SHORT)ptlHotspot->y;

    DosSetFilePtr (hFile, 6, FILE_BEGIN, &ulAction);
    DosWrite (hFile, &x, 2, &ulAction);
    DosWrite (hFile, &y, 2, &ulAction);
    DosClose (hFile);

    return;
    }

int main (int argc, char *argv[])
    {
    POINTL ptlHotspot, ptlNewHotspot;
    POINTL ptlSize;
    CHAR   buffer[128];
    LONG   value;

    if (argc != 2)
        {
        printf ("Syntax: %s <bitmap-file>\n\n", argv[0]);
        printf ("Dieses Programm dient zur Festlegung des Punktes, an denen die Zeigerachse den\n");
        printf ("Zeiger durchstî·t. Ein Zeiger wird immer in der 12-Uhr-Stellung gezeichnet,\n");
        printf ("der Koordinatenursprung liegt im mathematischen Sinne immer unten links.\n\n");
        printf ("Dieses Programm arbeitet interaktiv,\nd. h. die Koordinaten werden einzeln erfragt\n");
        return 0;
        }

    getBitmapData (argv[1], &ptlHotspot, &ptlSize);
    printf ("Actual Size of Bitmap:    (cx/cy) = (%d/%d)\n", ptlSize.x, ptlSize.y);
    printf ("Actual Hotspot of Bitmap: (cx/cy) = (%d/%d)\n", ptlHotspot.x, ptlHotspot.y);

    for (value = -1; value < 0 || value > ptlSize.x; )
        {
        printf ("\nNew Hotspot x-value: (0...%d): ", ptlSize.x); fflush (stdout);
        scanf ("%d", &value); fflush (stdin);
        }
    ptlNewHotspot.x = value;

    for (value = -1; value < 0 || value > ptlSize.y; )
        {
        printf ("\nNew Hotspot y-value: (0...%d): ", ptlSize.y); fflush (stdout);
        scanf ("%d", &value); fflush (stdin);
        }
    ptlNewHotspot.y = value;

    writeBitmapData (argv[1], &ptlNewHotspot);

    return 0;
    }

