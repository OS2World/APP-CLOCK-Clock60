/***************************************************************************\
 *
 * PROGRAMMNAME: HOPF6038
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
 *  5.00    06-15-00  y2k-Bugfix
 *  5.20    06-09-01  Fehlerkorrektur Audiodaten
 *  5.30    01-16-02  Implementierung der Erinnerungsfunktion
 *  5.40    11-22-03  Fehlerkorrektur Erinnerungsfunktion
 *
 *  Copyright (C) noller & breining software 1995...2004
 *
\******************************************************************************/
#define INCL_DOSFILEMGR
#define INCL_DOSPROCESS
#include "os2.h"
#include "hopf6038.h"
#include "tloc.h"
#include "dcfioctl.h"
#include "common.h"

#define STDIN         0
#define STDOUT        1
#define STDERROR      2

#define READONLY_FLAG 0

#pragma check_stack (off)

extern DCF77_DATETIME dt;
extern short portaddr;
extern short intervall;
extern short utcflag;
extern short offset;

extern char     chTimeZone[20];
extern TLOC_DEF tloc_spring;
extern TLOC_DEF tloc_fall;

static char     szLine[CCHMAXLEN];
static char     szFileName[CCHMAXLEN];
static TLOC_DEF tloc;

void setMode (UCHAR);
UCHAR inbyte (short port);
void CorrectTLOC (TLOC_DEF *ptloc);
void INT_3 (void);
int getTime ();

SHORT cini (unsigned char far *strCfg)
{
    SHORT sCnt, sCnt2;
    SHORT sError;
    HFILE hFile;

    sCnt=0;
    sError = FALSE;

    printtoscreen ("DCF77-driver for hopf 6038-card; (C) noller & breining software, 1996...2000\r\n");

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
                       portaddr = hextoint (& (strCfg[sCnt]));
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
                       intervall = atoi (& (strCfg[sCnt]));
                    }
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

                case 'F':
                case 'f':
                    do
                    {
                        sCnt++;
                    } while (strCfg[sCnt] == ' ');
                    if ((strCfg[sCnt] == ':') || (strCfg[sCnt] == '='))
                    {
                       sCnt2 = 0;
                       do
                       {
                           sCnt++;
                       } while (strCfg[sCnt] == ' ');
                       while ((strCfg[sCnt] != ' ') && (strCfg[sCnt] != '\0'))
                       {
                           szFileName[sCnt2++] = strCfg[sCnt++];
                           if (sCnt2 == CCHMAXLEN)
                               break;
                       } /* endwhile */
                       szFileName[sCnt2] = '\0';
                    }
                    break;

                case 'Z':
                case 'z':
                    do
                    {
                        sCnt++;
                    } while (strCfg[sCnt] == ' ');
                    if ((strCfg[sCnt] == ':') || (strCfg[sCnt] == '='))
                    {
                       sCnt2 = 0;
                       do
                       {
                           sCnt++;
                       } while (strCfg[sCnt] == ' ');
                       while ((strCfg[sCnt] != ' ') && (strCfg[sCnt] != '\0'))
                       {
                           chTimeZone[sCnt2++] = strCfg[sCnt++];
                           if (sCnt2 == 19) break;
                       } /* endwhile */
                       chTimeZone[sCnt2] = '\0';
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

    if (sError == FALSE)
    {
        setMode (3 | 4);
        getTime ();

        OpenTZFile (szFileName, &hFile);
        FindEntry (hFile, szLine, CCHMAXLEN, chTimeZone);

        while (GetTZString (hFile, szLine, CCHMAXLEN, &tloc))
        {
            if ((dt.year == tloc.usFirstYear) || (dt.year <= tloc.usLastYear))
            {
                if (tloc.usMonth < 7)
                    tloc_spring = tloc;
                else
                    tloc_fall = tloc;
                utcflag = 0;
            }
        }
        CloseTZFile (hFile);

        CorrectTLOC (&tloc_spring);
        CorrectTLOC (&tloc_fall);
    } /* endif */

    return sError;
}

