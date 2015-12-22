/***************************************************************************\
 *
 * PROGRAMMNAME: TIME_CS
 * -------------
 *
 * VERSION: 6.0
 * --------
 *
 * MODULNAME: NETBIOS
 * ----------
 *
 * BESCHREIBUNG:
 * -------------
 *   Klasse zur Kommunikation Åber das NetBIOS-Protokoll
 *
 * HINWEISE:
 * ---------
 *
 * METHODEN:
 * -----------
 *   NetBIOS::NetBIOS ()
 *   NetBIOS::~NetBIOS ()
 *   NetBIOS::clearNCB ()
 *   NetBIOS::resetNetBios ()
 *   NetBIOS::getAvailableAdapters ()
 *   NetBIOS::getNextAvlAdapter ()
 *   NetBIOS::getNumOfAdapters ()
 *   NetBIOS::openSession ()
 *   NetBIOS::isAvailableAdapter ()
 *   NetBIOS::isOK ()
 *   NetBIOS::sendData ()
 *   NetBIOS::getDataBlock ()
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
#include <os2.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "NetBIOS.hpp"

NetBIOS::NetBIOS ()
    {
    USHORT usAdapter;

    memset (ucNcbNum, '\0', sizeof (ucNcbNum));
    bInitializedAdapters = 0;
    bAvailableAdapters   = getAvailableAdapters ();

    for (usAdapter = 0; usAdapter < MAX_ADAPTER; usAdapter++)
        openSession (usAdapter);
    }

NetBIOS::~NetBIOS ()
    {
    USHORT usAdapter;

    for (usAdapter = 0; usAdapter < 8; usAdapter++)
        {
        /* DELETE NAME */
        if (isOK (usAdapter))
            {
            // NCB lîschen
            clearNCB ();

            NCB.ncb_command  = NB_DELETE_NAME_WAIT;
            NCB.ncb_lana_num = usAdapter;
            memcpy (NCB.ncb_name, szName[usAdapter], strlen (szName[usAdapter]));
            NETBIOS ((char * _Seg16)&NCB);
            }
        }
    }

VOID NetBIOS::clearNCB (VOID)
    {
    memset (&NCB, '\0', sizeof (NCB));
    return;
    }

USHORT NetBIOS::resetNetBios (USHORT usAdapter)
    {

    // NCB lîschen
    clearNCB ();

    // NCB initialisieren
    NCB.ncb_command     = NB_RESET_WAIT;
    NCB.ncb_lsn         = 0;                    // Resourcen-Anforderung
    NCB.ncb_callname[0] = 1;                    // 1 Session
    NCB.ncb_callname[1] = 1;                    // 1 Command
    NCB.ncb_callname[2] = 1;                    // 1 Name in log table
    NCB.ncb_lana_num    = usAdapter;            // Adapter Nummer

    // NCB senden
    return NETBIOS ((char * _Seg16)&NCB);
    }

USHORT NetBIOS::getAvailableAdapters (VOID)
    {
    USHORT usc, usMask;

    for (uscAdapters = usc = usMask = 0; resetNetBios (usc) == 0 && usc < MAX_ADAPTER; usc++)
        {
        uscAdapters++;
        usMask |= 1 << usc;
        }

    return usMask;
    }

USHORT NetBIOS::getNextAvlAdapter (USHORT usAdapter)
    {
    USHORT usc;

    for (usc = 0; usc < MAX_ADAPTER; usc++)
        {
        usAdapter++;
        usAdapter %= MAX_ADAPTER;
        if (isAvailableAdapter (usAdapter))
            break;
        }

    return usAdapter;
    }

USHORT NetBIOS::getNumOfAdapters (VOID)
    {
    return uscAdapters;
    }

VOID NetBIOS::openSession (USHORT usAdapter)
    {
    USHORT  uscName;

    if (!isAvailableAdapter (usAdapter))
        return;

    /* ADD_GROUP_NAME: 10 Versuche, bis ein freier Name gefunden wird */
    /* Dieser Fall sollte bei ADD_GROUP_NAME im Gegensatz zu ADD_NAME */
    /* nicht auftreten.                                               */
    strcpy (szName[usAdapter], NBNAME);
    for (uscName = 0; uscName < 10; uscName++)
        {
        // NCB lîschen
        clearNCB ();

        // NCB initialisieren
        NCB.ncb_command  = NB_ADD_GROUP_NAME_WAIT;
        NCB.ncb_lana_num = usAdapter;
        _itoa (uscName, szName[usAdapter] + sizeof (NBNAME) - 1, 10);
        memcpy (NCB.ncb_name, szName[usAdapter], strlen (szName[usAdapter]));

        // NCB senden
        if (!NETBIOS ((char * _Seg16)&NCB))
            {
            ucNcbNum[usAdapter] = NCB.ncb_num;
            bInitializedAdapters |= 1 << usAdapter;
            break;
            }
        else
            bInitializedAdapters &= ~(1 << usAdapter);
        }

    return;
    }

BOOL NetBIOS::isAvailableAdapter (USHORT usAdapter)
    {
    return bAvailableAdapters & (1 << usAdapter) ? TRUE : FALSE;
    }

BOOL NetBIOS::isOK (USHORT usAdapter)
    {
    return bInitializedAdapters & (1 << usAdapter) ? TRUE : FALSE;
    }

BOOL NetBIOS::sendData (USHORT usAdapter, void* pData, ULONG ulcData)
    {
    if (!isOK (usAdapter))
        openSession (0);

    if (isOK (usAdapter))
        {
        // NCB lîschen
        clearNCB ();

        /* SEND BROADCAST */
        NCB.ncb_command        = NB_SEND_BROADCAST_DATAGRAM_WAIT;
        NCB.ncb_num            = ucNcbNum[usAdapter];
        NCB.ncb_buffer_address = pData;
        NCB.ncb_length         = ulcData;
        NCB.ncb_lana_num       = usAdapter;
        bInitializedAdapters &= NETBIOS ((char * _Seg16)&NCB) ? ~(1 << usAdapter) : 0xFF;
        }

    return isOK (usAdapter);
    }

BOOL NetBIOS::getDataBlock (USHORT usAdapter, void *pData, ULONG ulcData)
    {
    BOOL bRC = FALSE;

    if (!isOK (usAdapter))
        openSession (0);

    if (isOK (usAdapter))
        {
        /* RECEIVE BROADCAST */
        memset (&NCB, '\0', sizeof (NCB));
        NCB.ncb_command        = NB_RECEIVE_BROADCAST_DATAGRAM_W;
        NCB.ncb_num            = ucNcbNum[usAdapter];
        NCB.ncb_buffer_address = pData;
        NCB.ncb_length         = ulcData;
        NCB.ncb_lana_num       = usAdapter;
        bInitializedAdapters &= NETBIOS ((char * _Seg16)&NCB) ? ~(1 << usAdapter) : 0xFF;
        if (isOK (usAdapter))
            bRC = memcmp ((char *)NCB.ncb_callname, NBNAME, sizeof (NBNAME) - 1) ? FALSE : TRUE;
        }

    return bRC;
    }

