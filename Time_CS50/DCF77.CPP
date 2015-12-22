/***************************************************************************\
 *
 * PROGRAMMNAME: TIME_CS
 * -------------
 *
 * VERSION: 6.0
 * --------
 *
 * MODULNAME: DCF77
 * ----------
 *
 * BESCHREIBUNG:
 * -------------
 *   Klasse zur Kommunikation mit dem DCF77-Treiber
 *
 * HINWEISE:
 * ---------
 *
 * METHODEN:
 * -----------
 *   dcf77QueryData::dcf77QueryData ()
 *   dcf77QueryData::~dcf77QueryData ()
 *   dcf77QueryData::getDCFDataPtr ()
 *   dcf77QueryData::isOK ()
 *   dcf77QueryData::getDCFData ()
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
#define INCL_DOSDEVICES
#define INCL_DOSDEVIOCTL
#define INCL_DOSFILEMGR
#include <os2.h>

#include <string.h>

#include "sharemem.hpp"
#include "dcf77.hpp"

#define DEVICENAME          "DCF77$"        // Device-Name des Treibers

dcf77QueryData::dcf77QueryData ()
    {
    ULONG  ul;

    pDCFShMem = NULL;

    /* Schnittstelle zum DCF-Treiber ”ffnen */
    if (DosOpen ((PSZ)DEVICENAME, &hDrvr, &ul, 0, FILE_NORMAL,
                  OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,
                  OPEN_SHARE_DENYREADWRITE | OPEN_ACCESS_READWRITE,
                  NULL))
        {
        hDrvr = -1;                         // es existiert kein SYS-Treiber

        pDCFShMem = new DCF77Communication;
        if ((pDCFShMem->getDCFShMemPtr () == NULL) ||
            !(pDCFShMem->getDCFShMemPtr ()->flProgs & PROG_DRIVER))
            {
            delete pDCFShMem;
            pDCFShMem = NULL;               // es existiert kein PROG_DRIVER
            }
        }

    if (hDrvr != -1 || pDCFShMem)
        {
        /* Es existiert ein Treiber im System */
        memset (&dataDCF, '\0', sizeof (DCF77_ALLDATA));
        getDCFData ();
        if (!(dataDCF.dcf77Status.cVerMajor | dataDCF.dcf77Status.cVerMinor))
            {
            if (hDrvr != -1)
                DosClose (hDrvr);
            else
                delete pDCFShMem;

            hDrvr     = -1;
            pDCFShMem = NULL;
            }
        }

    return;
    }

dcf77QueryData::~dcf77QueryData ()
    {
    if (pDCFShMem)
        delete pDCFShMem;
    }

/*******************************************************************\
    getDCFDataPtr: Bestimmen des Zeigers auf DCF77_ALLDATA-Struktur.
    return: NULL:  Fehler
            sonst: Zeiger auf DCF77_ALLDATA
\*******************************************************************/
DCF77_ALLDATA *dcf77QueryData::getDCFDataPtr ()
    {
    return isOK () ? &dataDCF : NULL;
    }

/*******************************************************************\
    GetDCFStatus: Bestimmen des Zustandes des DCF-Empfanges.
    Es wird eine statische Struktur StatDCF gefllt,
    der flStatus-Wert als return-Wert bergeben.
    return: flStatus-Wert
\*******************************************************************/
BOOL dcf77QueryData::isOK ()
    {
    return (hDrvr == -1 && pDCFShMem == NULL) ? FALSE : TRUE;
    }

/*******************************************************************\
    GetDCFData: Lesen von Status/Offset/Uhrzeit vom DCF-Treiber.
    Es wird eine statische Struktur DCF_DATA gefllt.
    return: TRUE: ok
\*******************************************************************/
BOOL dcf77QueryData::getDCFData ()
    {
    ULONG  ulcParm, ulcData;
    BYTE   bParmPacket;
    BOOL   bRC = FALSE;

    if (hDrvr != -1)                        // Lesen aus dem SYS-Treiber
        {
        /* Status lesen */
        ulcParm = ulcData = 0;
        bParmPacket = 0;
        if (!DosDevIOCtl (hDrvr, IOCTL_DCF77, DCF77_GETSTATUS,
                         &bParmPacket,         sizeof (bParmPacket),  &ulcParm,
                         &dataDCF.dcf77Status, sizeof (DCF77_STATUS), &ulcData))
            {
            if (dataDCF.dcf77Status.cVerMajor >= 2)
                {
                /* Offset lesen */
                ulcParm = ulcData = 0;
                bParmPacket = 0;
                if (!DosDevIOCtl (hDrvr, IOCTL_DCF77, DCF77_QUERYOFFSET,
                                 &bParmPacket,         sizeof (bParmPacket),  &ulcParm,
                                 &dataDCF.dcf77Offset, sizeof (DCF77_OFFSET), &ulcData))
                    {
                    /* Uhrzeit lesen */
                    ulcParm = ulcData = 0;
                    bParmPacket = 0;
                    if (!DosDevIOCtl (hDrvr, IOCTL_DCF77, DCF77_GETDATETIME,
                                      &bParmPacket,           sizeof (bParmPacket),    &ulcParm,
                                      &dataDCF.dcf77DateTime, sizeof (DCF77_DATETIME), &ulcData))
                        {
                        bRC = TRUE;
                        }
                    }
                }
            }
        }
    else if (pDCFShMem)                     // Lesen aus dem SYS-Treiber
        {
        dataDCF = pDCFShMem->getDCFShMemPtr ()->dcf77Data;
        bRC     = TRUE;
        }

    if (!bRC)
        memset (&dataDCF, '\0', sizeof (DCF77_ALLDATA));

    return bRC;
    }

