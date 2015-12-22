/***************************************************************************\
 *
 * PROGRAMMNAME: HOPF6036
 * -------------
 *
 * VERSION: 6.0
 * --------
 *
 * MODULNAME: INIT
 * ----------
 *
 * BESCHREIBUNG: Initialisierungsmodul fÅr Device Driver
 * -------------
 *
 * HINWEISE:
 * ---------
 *
 * FUNKTIONEN:
 * -----------
 *   cini
 *
 *  Ver.    Date      Comment
 *  ----    --------  -------
 *  4.10          97  Erste Version
 *  5.00    06-21-00  y2k-Bugfix
 *  5.20    06-09-01  Fehlerkorrektur Audiodaten
 *  5.30    01-16-02  Implementierung der Erinnerungsfunktion
 *  5.40    11-22-03  Fehlerkorrektur Erinnerungsfunktion
 *
 *  Copyright (C) noller & breining software 1995...2004
 *
\******************************************************************************/
#define INCL_DOSFILEMGR
#define INCL_DOSPROCESS
#define STDOUT      1
#define STDIN       0
#define STDERROR    2
#define READONLY_FLAG 0
#include <os2.h>
#include "common.h"

#pragma check_stack (off)

extern short portaddr;
extern short utcflag;
extern short offset;

extern USHORT usIntervall;

int getTime (void);

short cini ( unsigned char far* strCfg)
{
    SHORT sCnt;
    SHORT sError;

    sCnt=0;
    sError = FALSE;

    printtoscreen ("DCF77-driver for hopf 6036-card Ver.6.0; (C) noller & breining software, 1996...2004\r\n");

    while (strCfg[sCnt] != '\0')
    {
        if ((strCfg[sCnt] == '-') || (strCfg[sCnt] == '/'))
        {
            sCnt++;
            switch (strCfg[sCnt])
            {
                case 'P':
                case 'p':
                    do
                    {
                        sCnt++;
                    } while (strCfg[sCnt] == ' ');
                    if ((strCfg[sCnt] == ':') || (strCfg[sCnt] == '='))
                    {
                       do
                       {
                           sCnt++;
                       } while (strCfg[sCnt] == ' ');
                       portaddr = hextoint (&(strCfg[sCnt]));
                    }
                    break;

                case 'I':
                case 'i':
                    do
                    {
                        sCnt++;
                    } while (strCfg[sCnt] == ' ');
                    if ((strCfg[sCnt] == ':') || (strCfg[sCnt] == '='))
                    {
                       do
                       {
                           sCnt++;
                       } while (strCfg[sCnt] == ' ');
                       usIntervall = atoi (& (strCfg[sCnt]));
                    }
                    break;

                case 'U':
                case 'u':
                    utcflag = 1;
                    break;

                case 'O':
                case 'o':
                    do
                    {
                        sCnt++;
                    } while (strCfg[sCnt] == ' ');
                    if ((strCfg[sCnt] == ':') || (strCfg[sCnt] == '='))
                    {
                       do
                       {
                           sCnt++;
                       } while (strCfg[sCnt] == ' ');
                       offset = atoi (& (strCfg[sCnt]));
                    }
                    break;

                default:
                    sError = TRUE;
                    printtoscreen ("Invalid parameter in CONFIG.SYS\r\n");
                    break;
            }
        }
        sCnt++;
    }
    getTime ();

    return sError;
}

