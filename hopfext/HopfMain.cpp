#define INCL_DOSPROCESS
#define INCL_DOSERRORS
#define INCL_DOSSEMAPHORES
#define INCL_DOSMISC
#define INCL_VIO
#define INCL_KBD
#include <os2.h>
#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#include <setjmp.h>
#include <string.h>
#ifdef DEBUG
#include <iostream.h>
#endif

// Statische Variable
int serielle = 2;
int showhelp = -1;
int democnt=1;

#include "ShareMem.hpp"
#include "InitClass.hpp"
#include "Inout.hpp"
#include "mgdef.h"
#include "AusgabeTxt.h"

#define DEVICENAME          "DCF77$"        // Device-Name des Treibers

static enum BREAK {b_init, b_running, b_break} Break = b_init;
static jmp_buf jbuf;
static HEV hev = 0;

static void BreakHandler (int sig)
{
    // BreakHandler neu registrieren
    signal (sig,  (_SigFunc)BreakHandler);
    switch (Break){
        case b_init:
            show(fputs (ERROR_BREAK, stderr););
            Break = b_break;
            break;
        case b_running:
            longjmp (jbuf, 1);
            break;
    }
    return;
}

void setClockAbility(SHMEM *data)
{
    show(cout << "main::setClockAbility" <<endl;)
    if (data == NULL)return;
    memset(&(data->dcf77Data),0,sizeof(DCF77_ALLDATA));
    data->dcf77Data.dcf77Status.cVerMajor  = PROG_VER_MAJOR;
    data->dcf77Data.dcf77Status.cVerMinor  = PROG_VER_MINOR;
    data->dcf77Data.bSize                  = sizeof (DCF77_ALLDATA);
    data->dcf77Data.dcf77Status.flMaskST   = MASK_ST_TIMEVALID | MASK_ST_CARRIER | MASK_ST_SET|MASK_ST_BATT;
    data->dcf77Data.dcf77Status.flMaskSS   = MASK_SS_SER1|MASK_SS_SER2|MASK_SS_SER3|MASK_SS_SER4|MASK_SS_SERX;
    data->dcf77Data.dcf77Status.flMaskTO   = 0;
    data->dcf77Data.dcf77Status.flMaskIV   = MASK_IV_UPDATE;
    data->dcf77Data.dcf77Status.flMaskLN   = 0;
    data->dcf77Data.dcf77Status.uscTimeSet = 0;
    data->dcf77Data.dcf77Status.uscGoodSeconds = 0;
}

void showtxt (char *txt[])
{
    USHORT cb=0;
    for (USHORT cnt = 0; txt[cnt][0] != 0; cnt++){
        for (USHORT cnt2=0,cb=0; txt[cnt][cnt2] != '\0'; cnt2++,cb++);
        VioSetCurPos (cnt+2,5,NULL);
        VioWrtTTY (txt[cnt],cb,NULL);
    }
};

main (int arc,char * arcv[])
{
    DCF77Communication  dcfc(PROG_DRIVER);
    UCHAR buffer[20];
    ULONG ulInterval= 1;
    ULONG ulcPost;
    USHORT rc;
    SHMEM *data = dcfc.getDCFShMemPtr();
    DosError(FERR_DISABLEHARDERR);
    // Signal-Handler installieren
    signal (SIGINT,  (_SigFunc)BreakHandler);
    signal (SIGTERM, (_SigFunc)BreakHandler);
    signal (SIGBREAK,(_SigFunc)BreakHandler);

    if (dcfc.getDCFShMemPtr () == NULL){
        USHORT pfWait = VP_WAIT;
        VioPopUp (&pfWait,NULL);
        showtxt (aktivtxt);
        DosSleep(5000);
        VioEndPopUp (NULL);
        return 0;                       //   oder Programm bereits aktiv
    }
    HFILE  hDrvr;
    ULONG  ul;
    if (!DosOpen ((PSZ)DEVICENAME, &hDrvr, &ul, 0, FILE_NORMAL,
                  OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,
                  OPEN_SHARE_DENYREADWRITE | OPEN_ACCESS_READWRITE,
                  NULL)){
        DosClose(hDrvr);
        USHORT pfWait = VP_WAIT;
        VioPopUp (&pfWait,NULL);
        showtxt (treibertxt);
        DosSleep(5000);
        VioEndPopUp (NULL);
        return 0;                       //   oder Treiber bereits aktiv
    }

    data->dcf77Data.dcf77Data.usSetInterval = 1;
    data->dcf77Data.dcf77Status.flStatus = 0;
    InitClass iniclass(sizeof(inidata)/sizeof(Initc_Pro),inidata);
    iniclass.analyse(arc,arcv);

    if (showhelp == 0){
        for (int cnt = 0; helptxt[cnt][0]!= NULL; cnt++) printf("%s\n",helptxt[cnt]);
        return(0);
    }


    Inout hopfclock((UCHAR)serielle);
    if (hopfclock.getPortNr() <= 4) data->dcf77Data.dcf77Data.bPort = hopfclock.getPortNr();
    else data->dcf77Data.dcf77Data.bPort = PORT_SERX;
    data->dcf77Data.dcf77Data.usIOAddr = hopfclock.getPortNr();
    if (hopfclock.isok())data->dcf77Data.dcf77Status.flStatus &= STATUS_ISDRIVER;
    else                      data->dcf77Data.dcf77Status.flStatus |= ~STATUS_ISDRIVER;
    int cnt =0;
    //DosCreateEventSem (SEMNAME, &hev, 0, FALSE);
    if (setjmp (jbuf))
        return 0;

    if (Break == b_break)
        return 0;

    Break = b_running;
    setClockAbility(data);

    // //////////////////////////////////////////////////
    // Ausgabe des Demo Textes
    // //////////////////////////////////////////////////
#ifdef DEMO
    USHORT pfWait = VP_WAIT;
    VioPopUp (&pfWait,NULL);
    showtxt (demotxt);
    KBDKEYINFO info;
    KbdCharIn(&info,0,NULL);
    VioEndPopUp (NULL);
#endif

    while (1) {
        if ( ((data->dcf77Data.dcf77Data.bPort != hopfclock.getPortNr())&&
              (data->dcf77Data.dcf77Data.bPort != PORT_SERX)) ||
             ((data->dcf77Data.dcf77Data.bPort == PORT_SERX)&&
              (data->dcf77Data.dcf77Data.usIOAddr != hopfclock.getPortNr()))){
            data->dcf77Data.dcf77Status.flStatus &= ~STATUS_CARRIER;
            data->dcf77Data.dcf77Status.flStatus &= ~STATUS_TIMEVALID;
            cnt = 0;
            if ((data->dcf77Data.dcf77Data.bPort >=PORT_SER1)&& (data->dcf77Data.dcf77Data.bPort <=PORT_SER4))
                data->dcf77Data.dcf77Data.usIOAddr = data->dcf77Data.dcf77Data.bPort ;
            hopfclock.setPortNr((UCHAR)(data->dcf77Data.dcf77Data.usIOAddr));

            if (hopfclock.getPortNr() <= PORT_SER4)
                data->dcf77Data.dcf77Data.bPort = hopfclock.getPortNr();
            else
                data->dcf77Data.dcf77Data.bPort = PORT_SERX;
            data->dcf77Data.dcf77Data.usIOAddr = hopfclock.getPortNr();
        }
        if ((data->dcf77Data.ulInterval != ulInterval) && (data->dcf77Data.ulInterval > 0)){
            ulInterval = data->dcf77Data.ulInterval;
            cnt = 0;
            show(cout << "Interval :" << ulInterval<< " neu:" << data->dcf77Data.dcf77Data.usSetInterval <<endl;)
        }
        data->dcf77Data.ulInterval = ulInterval ;
        //show(cout << " cnt: " << cnt << " ullinterval: " << ulInterval;if ((cnt %2) == 0)cout <<endl;);
        if (cnt-- <= 0){
            cnt = ulInterval *120;
            show(cout << "Lese Uhr aus" <<endl;);
            if (hopfclock.readDate(buffer) == 0){
                show(cout << "Zeit erfolgreich erfaát => Tr„ger" <<endl;);
                data->dcf77Data.dcf77Status.flStatus &= ~STATUS_TIMEVALID;
                data->dcf77Data.dcf77Status.flStatus |= STATUS_CARRIER;
                if (buffer[14] & 0x01){
                    show(cout << "Zeit fr gltig gesetzt => Time valid" <<endl;);
                    DATETIME datetime;
                    DosGetDateTime(&datetime);
                    for (int i = 0 ; i < 13 ; i++)buffer[i] -= '0';
                    datetime.seconds = buffer[4]*10 + buffer[5];
                    datetime.minutes = buffer[2]*10 + buffer[3];
                    datetime.hours   = buffer[0]*10  + buffer [1];
                    datetime.hundredths = 65;
                    datetime.day     = buffer[7]*10  + buffer [8];
                    datetime.month   = buffer[9]*10  + buffer [10];
                    datetime.year    = 1900+buffer[11]*10+buffer [12];
                    if (datetime.year < 1980)
                        datetime.year += 100;
                    show (cout << "Zeit: "<< int(datetime.hours) << ":" << int(datetime.minutes) << ":" << int(datetime.seconds););
                    show (cout << "Datum: "<< int(datetime.day) <<"."<< int(datetime.month) << "." << int(datetime.year)<< endl;);
                    datetime.weekday = buffer[6]%7;
                    data->dcf77Data.dcf77DateTime.hours   = datetime.hours;
                    data->dcf77Data.dcf77DateTime.minutes = datetime.minutes;
                    data->dcf77Data.dcf77DateTime.seconds = datetime.seconds;
                    data->dcf77Data.dcf77DateTime.day     = datetime.day;
                    data->dcf77Data.dcf77DateTime.month   = datetime.month;
                    data->dcf77Data.dcf77DateTime.year    = datetime.year;
                    data->dcf77Data.dcf77DateTime.timezone = buffer[13];
                    data->dcf77Data.dcf77Status.flStatus |= STATUS_TIMEVALID;
                    data->dcf77Data.dcf77Status.uscGoodSeconds++;
#ifdef DEMO
                    if (democnt > 0) {
                        if (rc = DosSetDateTime(&datetime)){
                            show(printf("Cannot set Date - Rc = %d\n\r", rc );)
                        }else{
                            data->dcf77Data.dcf77Status.uscTimeSet++;
                            democnt--;
                        }
                    }
#endif
#ifndef DEMO
                    if (rc = DosSetDateTime(&datetime)){
                        show(printf("Cannot set Date - Rc = %d\n\r", rc ););
                    }else
                        data->dcf77Data.dcf77Status.uscTimeSet++;
#endif
                    if (buffer[14] & 0x04)
                        data->dcf77Data.dcf77Status.flStatus |= STATUS_BATT;
                    else
                        data->dcf77Data.dcf77Status.flStatus &= ~STATUS_BATT;
                }
            }else{
                data->dcf77Data.dcf77Status.flStatus &= ~STATUS_CARRIER;
                show(cout << ".";);
            }
        }
        DosSleep(500);
   }
   return(0);
}
