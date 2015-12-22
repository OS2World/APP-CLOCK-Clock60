#define INCL_DOSDEVICES
#define INCL_DOSDEVIOCTL
#define INCL_DOSFILEMGR
#define INCL_DOSPROCESS
#define INCL_DOSDATETIME
#define INCL_DOSPROCESS
#include <os2.h>
#ifdef DEBUG
#include <stdio.h>
#include <iostream.h>
#endif
#include "Inout.hpp"
#include "mgdef.h"

struct Linectrl {
   UCHAR zdatabits;
   UCHAR zparity;
   UCHAR zstopbits;
};

USHORT  usBPS = 300; // Baud

char *coms[]={"COM1","COM2","COM3","COM4","COM5","COM6","COM7","COM8","COM9"};
int comcnt = 9;

Inout::~Inout()
{
    if(clockhandle)DosClose(clockhandle);
};

void Inout::setPortNr(UCHAR serialPortNr)
{
    show(cout << "Inout::setPortNr" << int(serialPortNr) << endl;);
    USHORT rc=0;
    int        initvalue = 0;
    ULONG   action;
    ULONG   ullength=0;
    Linectrl lineparams;
    lineparams.zstopbits = (UCHAR)2;
    lineparams.zparity = (UCHAR)0;
    lineparams.zdatabits = (UCHAR)8;
    if ((serialPortNr > comcnt)||(serialPortNr < 1)) return;
    if ((serielle == serialPortNr)&&(clockhandle)) return;
    if (serialPortNr <= comcnt)serielle = serialPortNr;
    if (clockhandle) DosClose(clockhandle);
    clockhandle=0;
    show(cout << "Try open " << coms[serielle-1] << endl;);
    rc=DosOpen((PSZ)coms[serielle-1],&clockhandle,&action,
                        0,FILE_NORMAL,FILE_OPEN,
                        OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYNONE,NULL);
    if (! rc) {
        if (rc = DosDevIOCtl(clockhandle,
                         IOCTL_ASYNC,
                         ASYNC_STOPTRANSMIT,
                         NULL,0,NULL,
                         NULL,0,NULL)){
           //printf("Cannot STOP TRANSMIT at COM-PORT - Rc = %d\n\r", rc );
           initvalue = 1;
        }
        // Serielle vorhanden und zug„nglich
        if (rc = DosDevIOCtl(clockhandle,
                         IOCTL_ASYNC,
                         ASYNC_SETBAUDRATE,
                         (PULONG) & usBPS,
                         sizeof(usBPS),
                         &ullength,
                         NULL,0,NULL)){
            //printf("Cannot set speed at COM-PORT - Rc = %d\n\r", rc );
           initvalue = 1;
        }
        if (rc = DosDevIOCtl(clockhandle,
                         IOCTL_ASYNC,
                         ASYNC_SETLINECTRL,
                         (PULONG) & lineparams,
                         sizeof(lineparams),
                         &ullength,
                         NULL,0,NULL)){
           initvalue = 1;
        //printf("Cannot set linectrl COM-PORT - Rc = %d\n\r", rc );
        }
        setInitSequenz();
    }else
      show(cout <<"open fail"<< endl ;);
    if (initvalue)  {DosClose(clockhandle);clockhandle=0;}
}

Inout::Inout(UCHAR ser)
{
   show(cout << "Inout::Inout" << int(ser) << endl;);
   clockhandle=0;
   serielle = 2;
   setPortNr(ser);
}
int Inout::setInitSequenz()
{
    show(cout << "Inout::setInitSequenz" << endl;);
    USHORT rc=0;
    DCBINFO dcfbino;
    ULONG   ullength=0;
    if (clockhandle == 0)return -1;
    //printf("setInitSequenz()\n");
    if (rc = DosDevIOCtl(clockhandle,
                         IOCTL_ASYNC,
                         ASYNC_GETDCBINFO,
                         NULL,0,NULL,
                         (PULONG) &dcfbino,
                         sizeof(dcfbino),
                         &ullength)){
        //printf("Cannot get DCFINF COM-PORT - Rc = %d\n\r", rc );
        return(1);
    }
    dcfbino.fbCtlHndShake &= 0xBC;
    dcfbino.fbFlowReplace &= 0x3F;
    dcfbino.usReadTimeout = 1650;
    //printf("dcfbino.Flags1 %x\n",(dcfbino.fbCtlHndShake));
    //printf("dcfbino.Flags2 %x\n",(dcfbino.fbFlowReplace));
    if (rc = DosDevIOCtl(clockhandle,
                         IOCTL_ASYNC,
                         ASYNC_SETDCBINFO,
                         (PULONG) &dcfbino,
                         sizeof(dcfbino),
                         &ullength,
                         NULL,0,NULL)){
        //printf("Cannot set1 DCBINFO COM-PORT - Rc = %d\n\r", rc );
        return(1);
    }
    DosSleep(500);//for (i = 0;i<1000000;i++);
    //printf("dcfbino.Flags1 %x\n",(dcfbino.fbCtlHndShake));
    //printf("dcfbino.Flags2 %x\n",(dcfbino.fbFlowReplace));
    dcfbino.fbCtlHndShake |= 0x01;
    dcfbino.fbFlowReplace |= 0x40;
    if (rc = DosDevIOCtl(clockhandle,
                         IOCTL_ASYNC,
                         ASYNC_SETDCBINFO,
                         (PULONG) &dcfbino,
                         sizeof(dcfbino),
                         &ullength,
                         NULL,0,NULL)){
        //printf("Cannot set2 DCBINFO COM-PORT - Rc = %d\n\r", rc );
        return(1);
    }
    DosSleep(50);
    //printf("dcfbino.Flags1 %x\n",(dcfbino.fbCtlHndShake));
    //printf("dcfbino.Flags2 %x\n",(dcfbino.fbFlowReplace));
    dcfbino.fbCtlHndShake &= 0xFE;
    dcfbino.fbFlowReplace &= 0xBF;
    if (rc = DosDevIOCtl(clockhandle,
                         IOCTL_ASYNC,
                         ASYNC_SETDCBINFO,
                         (PULONG) &dcfbino,
                         sizeof(dcfbino),
                         &ullength,
                         NULL,0,NULL)){
        //printf("Cannot set2 DCBINFO COM-PORT - Rc = %d\n\r", rc );
        return(1);
    }
    return 0;
}

int Inout::readDate(UCHAR *buffer)
{
    show(cout << "Inout::readDate" << endl;);
    USHORT rc;
    DCBINFO dcfbino;
    ULONG   ullength=0;
    for (int i = 0; i < 16 ; i++)buffer[i]=0;

    if (clockhandle == 0){
       setPortNr(serielle);
       if (clockhandle == 0)return -1;
    }
    if (rc = DosDevIOCtl(clockhandle,
                         IOCTL_ASYNC,
                         ASYNC_SETBREAKON,
                         NULL,0,NULL,
                         (PUSHORT) &dcfbino,
                         sizeof(USHORT),
                         &ullength)){
        show(cout << "Cannot set3 SETBREAKON - Rc = %d\n\r"<<  rc << endl;);
        return(1);
    }
    if (rc = DosRead(clockhandle,buffer,16,&ullength)){
        show(cout << "Cannot read COM-PORT - Rc = %d\n\r"<<  rc << endl;);
        return(1);
    }
    if (ullength < 16) {
        show(cout << "Timeout COM-PORT"<< endl;);
        return(1);
    }
    show(for (int i = 0;i < ullength; i++) cout << buffer[i];cout << endl;);
    if (rc = DosDevIOCtl(clockhandle,
                         IOCTL_ASYNC,
                         ASYNC_SETBREAKOFF,
                         NULL,0,NULL,
                         (PUSHORT) &dcfbino,
                         sizeof(USHORT),
                         &ullength)){
        show(cout << "Cannot set3 SETBREAKOFF - Rc = %d\n\r"<<  rc << endl;);
        return(1);
    }
    for (i = 0; i < 16 ; i++)if (buffer[i]!=0) return 0;
    return 1;
}


