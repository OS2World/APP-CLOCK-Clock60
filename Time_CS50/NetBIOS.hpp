#ifndef _NETBIOS_
#define _NETBIOS_

#define E32TO16

typedef unsigned char byte;
typedef unsigned short int  word;
typedef unsigned long int dword;
typedef void * _Seg16 address;

#include <netb_1_c.h>
#include <netb_2_c.h>
#include <netb_4_c.h>

#define NBNAME      "TIMESERV$"
#define MAX_ADAPTER 8

//*****************************************************************************
// Class definition for NetBIOS
//*****************************************************************************
class NetBIOS
    {
        struct network_control_block NCB;
        unsigned char   ucNcbNum[MAX_ADAPTER];
        CHAR            szName[MAX_ADAPTER][sizeof (NBNAME) + 3];   // zweistellige Dezimalzahl am Ende
        BYTE            bAvailableAdapters;                         // verfÅgbare Adapter (Bit0 -> Adapter 0)
        BYTE            bInitializedAdapters;                       // initialisierte Adapter
        USHORT          uscAdapters;                                // Zahl der verfÅgbaren Adapter

        USHORT          getAvailableAdapters (VOID);
        BOOL            isAvailableAdapter (USHORT usAdapter);
        VOID            clearNCB (VOID);
        USHORT          resetNetBios (USHORT usAdapter);
        VOID            openSession (USHORT usAdapter);

    public:
                        NetBIOS ();
                        ~NetBIOS ();
        USHORT          getNextAvlAdapter (USHORT usAdapter);
        BOOL            isOK (USHORT usAdapter);
        BOOL            sendData (USHORT usAdapter, void *pData, ULONG ulcData);
        BOOL            getDataBlock (USHORT usAdapter, void *pData, ULONG ulcData);
        USHORT          getNumOfAdapters (VOID);
    };

#endif /* _NETBIOS_ */

