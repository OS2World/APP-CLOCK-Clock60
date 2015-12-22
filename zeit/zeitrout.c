/***************************************************************************\
 *
 * PROGRAMMNAME: ZEIT
 * -------------
 *
 * VERSION: 5.4
 * --------
 *
 * MODULNAME: ZEITROUT
 * ----------
 *
 * BESCHREIBUNG: Strategieroutine des Treibers + Hilfsfunktionen
 * -------------
 *
 * HINWEISE:
 * ---------
 *
 * FUNKTIONEN:
 * -----------
 *   initdata
 *   strcpy
 *   ruecksetzen
 *   bittochar
 *   getbSignal
 *   fillDataDCF
 *   shortToHShort
 *   ioctl
 *   getcom
 *   getbit
 *   getbit_auto
 *   getTime
 *   timerHandler
 *   start
 *   insertThrValue
 *   calcThreshold
 *
 *  Ver.    Date      Comment
 *  ----    --------  -------
 *  4.10    11.03.96   erste Version mit HR-Timer-Support
 *  4.20    13.12.98   Korrektur im HR-Timer-Support
 *  5.00    06-26-00   Zusammenstellung neuer Release
 *  5.10    01-28-01   Korrektur der Minutenanzeige
 *  5.20    06-09-01   Fehlerkorrektur Audiodaten
 *  5.30    01-16-02  Implementierung der Erinnerungsfunktion
 *  5.40    11-22-03  Fehlerkorrektur Erinnerungsfunktion
 *
 *  Copyright (C) noller & breining software 1995...2003
 *
\******************************************************************************/
#pragma check_stack(off)
#define INCL_ERRORS
#include <os2.h>

#include "common.h"
#include "dcfioctl.h"
#include "zeit_c.h"
#include "tmr0_idc.h"
#include "trace.h"

typedef enum {unknown, ok, nok} TMR_STATE;

typedef union _DCF_IOCTL
    {
    DCF77_STATUS    pStatus;
    DCF77_DATETIME  pDateTime;
    DCF77_OFFSET    pOffset;
    DCF77_DATA      pData;
    DCF77_LEVEL     pLevel;
    CHAR            szCfgFileName[CCHMAXPATH];
    } DCF_IOCTL;
typedef DCF_IOCTL far *PDCF_IOCTL;

/* Speicherbereich fÅr Default-Werte aus Konfigurationsdatei */
DCF77_DATA DataDCFOrg;
CHAR       szConfigFile[CCHMAXPATH] = "";

/* Konfigurationsvariable */
UCHAR  ucPortTyp   = GAME;
UCHAR  ucBitPos    = 0;
UCHAR  ucPol       = POSITIVE;
UCHAR  ucPower     = 0;
UCHAR  ucAutoMode  = AUTOSTART;
UCHAR  ucTimeZone  = TIME_LOCAL;
UCHAR  ucSavety    = 2;
SHORT  sOffset     = 0;
USHORT usIntervall = 60;
USHORT usPortAddr  = 0;

#define COPYRIGHTLENGTH 24
static char hername[] = "(c) Geppert             "; /* lÑnge >= 24 */
static char retname[] = "(c) noller & breining   "; /* lÑnge >= 24 */

#ifdef _TRACE_
    USHORT iTrace[2];
#endif

/* Daten fÅr Zeit-Auswertung */
UCHAR  data[60];            //static???
static DCF77_DATETIME dt;
USHORT minold      = 0;     //static???
USHORT hourold     = 0;     //static???
USHORT dayold      = 0;     //static???
USHORT monold      = 0;     //static???
USHORT yearold     = 0;     //static???
USHORT goodmin     = 0;     //static???
USHORT badmin      = 0;     //static???
USHORT uscClockSet = 0;     // Zahl der erfolgreichen SetzvorgÑnge

USHORT flStatus    = 0;     //static???         // Status, siehe STATUS_*-Werte in dcfioctl.h

/* Timer-Variable */
extern USHORT usTimerMode;                      // verwendeter Timer-Typ
USHORT usTickLen;                               // TicklÑnge des Timers in ms
SHORT  usTTicks;                                // Zehntel-Ticks fÅr Sys-Timer
BOOL   bIsHRTimerAvail;                         // HR-Timer verfÅgbar
USHORT usThreshold;                             // Schwellwert fÅr BitlÑnge (usThrBitlen) in ms

unsigned int ticcount = 0;
UCHAR  uccSavety    = 0;                        // ZÑhler fÅr EmpfangsÅberprÅfung
USHORT traeger      = 100;
USHORT uscIntervall = 0;                        // ZÑhler fÅr Update-Intervall
CHAR   initflag     = 0;

/* Schwellwerte fÅr Biterkennung */
USHORT usThrCarrier;                            // Erkennung TrÑgerausfall; 3.2s
USHORT usThrPolarity;                           // Erkennung PolaritÑt;     1.5s
USHORT usThrMinute;                             // Erkennung Minutenimpuls; 1.5s
USHORT usThrBitlen;                             // Erkennung BitpolaritÑt;  variabel

/* Schwellwertberechnung H/L-Impulse */
UCHAR ucThrValues[THR_ARRAYSIZE];               // Array fÅr Schwellwertberechnung
UCHAR ucThrMaxValue[2] = {255, 255};            // 2 Maximalwerte im Array
UCHAR ucThrMaxIndex[2];                         // 2 Indizes der Maximalwerte im Array

static USHORT usLevelMax = 0;                   // Grî·e des Puffers fÅr Bit-Level
static USHORT uscLevel   = 0;                   // nÑchstes freies Byte Bit-Level-Puffer

void initdata (void)
    {
    int j;
    for (j = 0; j < 60; j++)
        data[j] = 1;

    dt.seconds = 0;
    }

PCHAR strcpy (PCHAR pszDest, const PCHAR pszSource)
    {
    PCHAR pszTemp = pszDest;
    PCHAR pszSrc  = pszSource;

    do
        {
        *pszTemp++ = *pszSrc;
        } while (*pszSrc++ != '\0');

    return pszDest;
    }

/* rÅcksetzen der Daten fÅr Zeitwin */
VOID ruecksetzen (VOID)
    {
    dt.hours   = 0;
    dt.minutes = 0;
    dt.day     = 0;
    dt.month   = 0;
    dt.year    = 0;
    flStatus  &= ~STATUS_TIMEVALID;

    return;
    }

CHAR bittochar (CHAR *cdata, CHAR length)
    {
    CHAR retvalue = 0;

    if (cdata[0] != 0)
        retvalue = 1;
    if (length > 1)
        {
        if (cdata[1] != 0)
            retvalue += 2;
        if (length > 2)
            {
            if (cdata[2] != 0)
                retvalue += 4;
            if (length > 3)
                {
                if (cdata[3] != 0)
                    retvalue += 8;
                }
            }
        }

    return retvalue;
    }

BYTE getbSignal (VOID)
    {
    BYTE bSignal;
    BYTE ucBP = ucBitPos;

    for (bSignal = 0; ucBP != 0; bSignal += 1)
        {
        if (ucBP & 1)
            break;
        ucBP >>= 1;
        }

    if ((ucAutoMode == AUTOON)||(ucAutoMode == AUTOSTART))
        bSignal = SIGNAL_SCAN;
    return bSignal;
    }

VOID fillDataDCF (PDCF77_DATA pDataDCF)
    {
    pDataDCF->bSupply = ucPower & 7;
    pDataDCF->bSignal = getbSignal ();
    switch (ucPortTyp)
        {
        case GAME:
            pDataDCF->bPort   = PORT_GAME;
            break;
        case GAMEX:
            pDataDCF->bPort   = PORT_GAMEX;
            break;
        case COM:
            pDataDCF->bSignal = SIGNAL_SERIAL;
        case SER:
            switch (usPortAddr)
                {
                case 0x3F8:
                    pDataDCF->bPort = PORT_SER1;
                    break;
                case 0x2F8:
                    pDataDCF->bPort = PORT_SER2;
                    break;
                case 0x3E8:
                    pDataDCF->bPort = PORT_SER3;
                    break;
                case 0x2E8:
                    pDataDCF->bPort = PORT_SER4;
                    break;
                }
            break;

        case COMX:
            pDataDCF->bSignal = SIGNAL_SERIAL;
        case SERX:
            pDataDCF->bPort   = PORT_SERX;
            break;
        }

    pDataDCF->usThreshold   = usThreshold;
    pDataDCF->usTicklen     = usTickLen;
    pDataDCF->usTimerMode   = usTimerMode;
    pDataDCF->bcRepeat      = (BYTE)ucSavety;
    pDataDCF->usIOAddr      = usPortAddr;
    pDataDCF->usSetInterval = usIntervall;

    return;
    }

USHORT ioctl (PVOID pParm, PDCF_IOCTL pIOBuffer, USHORT usCategFkt)
    {
    switch (usCategFkt)
        {
        case (USHORT)((IOCTL_DCF77 << 8) + DCF77_GETSTATUS):
            if (TestSeg ((PUSHORT)pIOBuffer, sizeof (DCF77_STATUS)) == FALSE)
                return STATUS_INV_PARM;

            flStatus &= ~STATUS_SCAN;
            if ((ucAutoMode == AUTOON) || (ucAutoMode == AUTOSTART))
                flStatus |= STATUS_SCAN;

            pIOBuffer->pStatus.cDrvType        = DRVTYPE_EXTMOD;        // Treiber ist ZEIT.SYS
            pIOBuffer->pStatus.cVerMajor       = VERSION_MAJOR;         // Hauptversionsnummer
            pIOBuffer->pStatus.cVerMinor       = VERSION_MINOR;         // Unterversionsnummer
            pIOBuffer->pStatus.bPolarity       = ucPol;                 // PolaritÑt 0=negativ, 1=positiv
            pIOBuffer->pStatus.bIsHRTimerAvail = (BYTE)bIsHRTimerAvail; // 1 = HR-Timer verfÅgbar
            pIOBuffer->pStatus.flStatus        = flStatus;              // Status, siehe STATUS_*-Werte
            pIOBuffer->pStatus.uscBadSeconds   = badmin;                // empfangene falsche Zeiten
            pIOBuffer->pStatus.uscGoodSeconds  = goodmin;               // empfangene korrekte Zeiten
            pIOBuffer->pStatus.uscTimeSet      = uscClockSet;           // Zahl der SetzvorgÑnge
            pIOBuffer->pStatus.flMaskST        = MASK_ST_ALL - MASK_ST_COORDINATES - MASK_ST_BATT;
            pIOBuffer->pStatus.flMaskSS        = MASK_SS_ALL - MASK_SS_HOPF - MASK_SS_USB;
            pIOBuffer->pStatus.flMaskTO        = MASK_TO_ALL;
            pIOBuffer->pStatus.flMaskIV        = MASK_IV_ALL;
            pIOBuffer->pStatus.flMaskLN        = MASK_LN_ALL;
            return 0;

        case (USHORT)((IOCTL_DCF77 << 8) + DCF77_GETDATETIME):
            if (TestSeg ((PUSHORT)pIOBuffer, sizeof (DCF77_DATETIME)) == FALSE)
                return STATUS_INV_PARM;
            pIOBuffer->pDateTime.hours    = dt.hours;
            pIOBuffer->pDateTime.minutes  = dt.minutes;
            pIOBuffer->pDateTime.seconds  = dt.seconds;
            pIOBuffer->pDateTime.day      = dt.day;
            pIOBuffer->pDateTime.month    = dt.month;
            pIOBuffer->pDateTime.year     = dt.year;
            pIOBuffer->pDateTime.timezone = dt.timezone;
            return 0;

        case (USHORT)((IOCTL_DCF77 << 8) + DCF77_QUERYOFFSET):
            if (TestSeg ((PUSHORT)pIOBuffer, sizeof (DCF77_OFFSET)) == FALSE)
                return STATUS_INV_PARM;
            pIOBuffer->pOffset.flUTC   = (BYTE)ucTimeZone;     // 0 = MEZ/MESZ; 1 = UTC
            pIOBuffer->pOffset.sOffset = (SHORT)sOffset;       // Zeitoffset -3...+2 Stunden
            return 0;

        case (USHORT)((IOCTL_DCF77 << 8) + DCF77_QUERYDATA):
            if (TestSeg ((PUSHORT)pIOBuffer, sizeof (DCF77_DATA)) == FALSE)
                return STATUS_INV_PARM;

            if (TestSeg ((PUSHORT)pParm, sizeof (BYTE)) == FALSE)
                return STATUS_INV_PARM;

            switch (*((PBYTE)pParm))
                {
                case CMD_DATA_ACTUAL:
                    usThreshold = usThrBitlen*usTickLen + usTTicks*usTickLen/4; // ??? wird nur hier wirklich gebraucht!!!
                    fillDataDCF (&pIOBuffer->pData);
                    break;

                case CMD_DATA_DEFAULT:
                    pIOBuffer->pData = DataDCFOrg;
                    break;

                default:
                    return STATUS_INV_PARM;
                }
            return 0;

        case (USHORT)((IOCTL_DCF77 << 8) + DCF77_SETOFFSET):
            if (TestSeg ((PUSHORT)pIOBuffer, sizeof (DCF77_OFFSET)) == FALSE)
                return STATUS_INV_PARM;
            if (pIOBuffer->pOffset.flUTC == 0)
                ucTimeZone = TIME_LOCAL;
            else
                ucTimeZone = TIME_UTC;
            if ((pIOBuffer->pOffset.sOffset > -13 ) && (pIOBuffer->pOffset.sOffset < 13))   // Werte checken -3...+2, s. o. ???
                sOffset = pIOBuffer->pOffset.sOffset;
            else
                return STATUS_INV_PARM;
            return 0;

        case (USHORT)((IOCTL_DCF77 << 8) + DCF77_SETDATA):
            if (TestSeg ((PUSHORT)pIOBuffer, sizeof (DCF77_DATA)) == FALSE)
                return STATUS_INV_PARM;

            /* Hardware-Typ und Portadresse */
            switch (pIOBuffer->pData.bPort)
                {
                case PORT_GAME:
                    ucPortTyp  = GAME;
                    usPortAddr = 0x201;
                    break;
                case PORT_GAMEX:
                    ucPortTyp  = GAMEX;
                    usPortAddr = pIOBuffer->pData.usIOAddr;
                    break;
                case PORT_SER1:
                    ucPortTyp  = SER;
                    usPortAddr = 0x3F8;
                    break;
                case PORT_SER2:
                    ucPortTyp  = SER;
                    usPortAddr = 0x2F8;
                    break;
                case PORT_SER3:
                    ucPortTyp  = SER;
                    usPortAddr = 0x3E8;
                    break;
                case PORT_SER4:
                    ucPortTyp  = SER;
                    usPortAddr = 0x2E8;
                    break;
                case PORT_SERX:
                    ucPortTyp  = SERX;
                    usPortAddr = pIOBuffer->pData.usIOAddr;
                    break;
                default:
                    //??? Fehlerfall!
                    break;
                }

            /* Seriell-Modus (öbertragung Åber RxD-Leitung) */
            if (pIOBuffer->pData.bSignal & SIGNAL_SERIAL)
                {
                if (ucPortTyp == SER)
                    {
                    ucPortTyp    = COM;
                    ucAutoMode = AUTOOFF;
                    }
                if (ucPortTyp == SERX)
                    {
                    ucPortTyp    = COMX;
                    ucAutoMode = AUTOOFF;
                    }
                }

            /* Setz-Intervall */
            if (pIOBuffer->pData.usSetInterval > 0)
                usIntervall = (USHORT)pIOBuffer->pData.usSetInterval;

            /* WiederholprÅfung */
            if (pIOBuffer->pData.bcRepeat > 0)
                ucSavety = (UCHAR)pIOBuffer->pData.bcRepeat;

            /* Stromversorgung externer Module */  // power? (0/-1); ???
            if (pIOBuffer->pData.bSupply & SUPPLY_OFF)
                ucPower = -1;
            else
                ucPower = pIOBuffer->pData.bSupply & 0x03;

            /* Bitposition fÅr SignalÅbertragung */
            if (pIOBuffer->pData.bSignal & SIGNAL_SCAN)
                ucAutoMode = AUTOSTART;
            else
                {
                ucBitPos = 1 << (pIOBuffer->pData.bSignal & SIGNAL_MASK);
                ucAutoMode = AUTOOFF;
                }

            /* Timertyp bestimmen/setzen */
            if (pIOBuffer->pData.usTimerMode != usTimerMode)
                {
                if (pIOBuffer->pData.usTimerMode == TIMER_MODE_SYS ||
                    pIOBuffer->pData.usTimerMode == TIMER_MODE_HR)
                    {
                    if (!registerTimer (pIOBuffer->pData.usTimerMode,
                                        pIOBuffer->pData.usTicklen))
                        return STATUS_GEN_FAIL;         // Fehler: Timer nicht vorhanden
                    }
                else
                    return STATUS_INV_PARM;             // Fehler: falscher Timertyp
                }

            usThreshold = pIOBuffer->pData.usThreshold;
            usThrBitlen = usThreshold / usTickLen;
            if (pIOBuffer->pData.usTimerMode == TIMER_MODE_SYS)
                {
                // System-Timer: usThrBitlen enthÑlt die Zahl der Ticks fÅr die Schwelle
                //               usTTicks enthÑlt die 1/4 Ticks fÅr die Schwelle
                usTTicks = (SHORT)(usThreshold % usTickLen * 4 / usTickLen);
                }
            else
                {
                // HR-Timer: usThrBitlen enthÑlt den Startwert fÅr die Zahl der Ticks
                usTTicks = 0;
                }

            return 0;

        case (USHORT)((IOCTL_DCF77 << 8) + DCF77_GETCFGFILE):
            if (TestSeg ((PUSHORT)pIOBuffer, CCHMAXPATH) == FALSE)
                return STATUS_INV_PARM;

            /* Dateinamen der Konfigurationsdatei umkopieren */
            f_strcpy (pIOBuffer->szCfgFileName, szConfigFile);
            return 0;

        case (USHORT)((IOCTL_DCF77 << 8) + DCF77_GETLVLDATA):
            if (TestSeg ((PUSHORT)pIOBuffer, sizeof (DCF77_LEVEL)) == FALSE)
                return STATUS_INV_PARM;
            if (TestSeg ((PUSHORT)pIOBuffer, pIOBuffer->pLevel.usSize + sizeof (pIOBuffer->pLevel.usSize)) == FALSE)
                return STATUS_INV_PARM;

            if (!LvlIsReady ())
                return STATUS_IN_USE;

            pIOBuffer->pLevel.usSize = LvlWait (pIOBuffer->pLevel.bLevel, pIOBuffer->pLevel.usSize);
            return 0;
        }

    return STATUS_BAD_CMD;
    }

UCHAR getcom (VOID)
    {
    UCHAR zw;

    /* jetzt eventuell die Uhr aktivieren */
    if ((ucPower >= 0) && (ucPower < 4))
        {
        zw = (UCHAR)inbyte (usPortAddr + 4);
        zw = zw & 0xFC;
        zw = zw | ucPower;
        outbyte (usPortAddr + 4, zw);
        }

    if (ucPol == POSITIVE)
        {
        /* jetzt noch serielle Schnittstelle initialisieren */
        ucPol = NEGATIVE;
        outbyte (usPortAddr + 3, 0x83); /* LCR: 8bit/1Stop/No Parity/Set DivRegisters */
        outbyte (usPortAddr, 0);        /* Teiler-Register setzen auf 0x900 (2304) */
        outbyte (usPortAddr + 1, 0x09); /*   bei 1,8432MHz entspricht das 50bit/s  */
        outbyte (usPortAddr + 3, 0x03); /* LCR: 8bit/1Stop/No Parity */
        TRACE (17, NULL, 0);
        }

    zw = (UCHAR)inbyte (usPortAddr + 5);
    if (!(zw & 0x01))
        {
        LvlComIns (0xFF);               /* Pegel in Pegel-Puffer eintragen */
        return -1;                      /* keine Daten im Receiver Holding Register */
        }

    zw = (UCHAR)inbyte (usPortAddr);
#ifdef _TRACE_
    iTrace[0] = 1;
    iTrace[1] = zw;
#endif
    TRACE (DCF77_DATASET, (char near *)iTrace, sizeof (iTrace));

    /* bei 100ms Impuls: 20ms Startbit, 4 Datenbits; LSB zuerst Åbertragen => RHR-Wert = 0xF0 */
    /* bei 200ms Impuls: 20ms Startbit, 8 Datenbits; LSB zuerst Åbertragen => RHR-Wert = 0x00 */
    LvlComIns (zw);                     /* Pegel in Pegel-Puffer eintragen */
    zw = (zw & 0x3F) == 0 ? 1 : 0;
    return zw;
    }

UCHAR getbit (USHORT port)
    {
    UCHAR zw;

    zw = (UCHAR)inbyte (port);          /* Port einlesen */
    return (zw ^ ucPol) & ucBitPos;     /* BitpolaritÑt korrigieren */
    }

UCHAR getbit_auto (USHORT port)
    {
    static UCHAR firstbyte = 0;         /* erstes gelesenes Byte nach Automatik-Aktivierung */
    UCHAR zw, ebit;

    zw = (UCHAR)inbyte (port) & 0xF0;   /* Port einlesen */

    /* Automatik auf Start */
    if (ucAutoMode == AUTOSTART)
        {
        firstbyte = zw;                 /* Erstes Byte behalten */
        ucAutoMode = AUTOON;
        }

    /* Auto ein und BitÑnderung */
    if (ucAutoMode == AUTOON && firstbyte != zw)
        {
        /* Neue Bitposition auf 1 Bit beschrÑnken und sichern */
        ucBitPos = firstbyte ^ zw;
        if (ucBitPos & 0x10)
            ucBitPos = 0x10;
        if (ucBitPos & 0x20)
            ucBitPos = 0x20;
        if (ucBitPos & 0x40)
            ucBitPos = 0x40;
        if (ucBitPos & 0x80)
            ucBitPos = 0x80;
        ucAutoMode = AUTOSCANNED;
        }

    zw = (zw ^ ucPol) & ucBitPos;       /* BitpolaritÑt korrigieren */
    LvlInsert (zw);                     /* Pegel in Pegel-Puffer eintragen */
    return zw;
    }

VOID getTime (VOID)
    {
    SHORT  sOffs = sOffset;
    SHORT  sParity1;
    SHORT  sParity2;
    SHORT  sParity3;
    USHORT usc;

    TRACE (DCF77_EVAL_START, NULL, 0);
    badmin++;
    flStatus &= ~STATUS_TIMEVALID;

    /* PrÅfsumme bestimmen */
    sParity1 = sParity2 = sParity3 = 0;
    for (usc =  0; usc<29; usc++)
        sParity1 += data[usc];
    for (usc = 29; usc<36; usc++)
        sParity2 += data[usc];
    for (usc = 36; usc<59; usc++)
        sParity3 += data[usc];

    /* Zeit bestimmen */
    TRACE (DCF77_EVAL_DATA, NULL, 0);
    if (((sParity1 & 0x1)!=0) || ((sParity2 & 0x1)!=0) || ((sParity3 & 0x1)!=0))
        {
        TRACE (DCF77_EVAL_ERR_PARITY, NULL, 0);
        ruecksetzen ();
        return;
        }
    dt.timezone = (SHORT)TZ_DCF;
    dt.seconds  = 0;
    dt.minutes  = bittochar (&data[21],4) + 10*bittochar (&data[25],3);
    dt.hours    = bittochar (&data[29],4) + 10*bittochar (&data[33],2);
    dt.day      = bittochar (&data[36],4) + 10*bittochar (&data[40],2);
    dt.month    = bittochar (&data[45],4) + 10*bittochar (&data[49],1);
    dt.year     = bittochar (&data[50],4) + 10*bittochar (&data[54],4);

    if (data[15] == 0)
        flStatus &= ~STATUS_ANT;
    else
        flStatus |= STATUS_ANT;
    if (data[16] == 0)
        flStatus &= ~STATUS_HLEAP;
    else
        flStatus |= STATUS_HLEAP;
    if (data[17] == 0)
        flStatus &= ~STATUS_TZONE1;
    else
        flStatus |= STATUS_TZONE1;
    if (data[18] == 0)
        flStatus &= ~STATUS_TZONE2;
    else
        flStatus |= STATUS_TZONE2;
    if (data[19] == 0)
        flStatus &= ~STATUS_SLEAP;
    else
        flStatus |= STATUS_SLEAP;
    if ((dt.minutes > 59) || (dt.hours > 23) || (dt.year > 99) ||
        (dt.month   < 1)  || (dt.month > 12) ||
        (dt.day     < 1)  || (dt.day   > 31))
        {
        TRACE (DCF77_EVAL_ERR, NULL, 0);
        ruecksetzen ();
        return;
        }

    dt.year += (dt.year > 90 ? 1900 : 2000);

    /* Zeitoffset */
    TRACE (DCF77_EVAL_OFFSET, NULL, 0);
    if (ucTimeZone == TIME_UTC)
        {
        if (flStatus & STATUS_TZONE1)
            sOffs -= 2;                     // wirklich? fÅr alle Zeitzonen? ???
        else if (flStatus & STATUS_TZONE2)
            sOffs -= 1;
        }

    dt.hours = dt.hours + sOffs;
    if (dt.hours > 23)                      // Offset positioniert auf nachfolgenden Tag
    {
        dt.hours = 0;
        dt.day++;
        if (dt.day > lastDay (dt.month, dt.year))
        {
            dt.day = 1;
            dt.month++;
            if (dt.month > 12)
            {
                dt.month = 1;
                dt.year++;
            }
        }
    }
    else if (dt.hours < 0)                  // Offset positioniert auf vorhergehenden Tag
    {
        dt.hours += 24;
        dt.day--;
        if (dt.day < 1)
        {
            dt.month--;
            if (dt.month < 1)
            {
                dt.month = 12;
                dt.year--;
            }
            dt.day = lastDay (dt.month, dt.year);
        }
    }

    /* PlausibilitÑtsprÅfung */
    if (dt.minutes == minold && dt.hours == hourold &&
        dt.day     == dayold && dt.month == monold  &&
        dt.year    == yearold)
        uccSavety++;                        // Wieder 1 min ok
    else
        uccSavety = 0;                      // Fehler bei der Wiederholung

    yearold = dt.year;
    monold  = dt.month;
    dayold  = dt.day;
    hourold = dt.hours;
    minold  = dt.minutes + 1;
    if (minold > 59)
        {
        minold = 0;
        hourold++;
        }
    if (hourold == 24)
        {
        hourold = 0;
        dayold++;                           // Der letzte Tag des Monats wird nicht mehr geprÅft
        }

    /* Status korrigieren; ab hier ist die empfangene Zeit korrekt */
    goodmin++;
    badmin--;
    flStatus |= STATUS_TIMEVALID;

    if (ucSavety > uccSavety)
        return;

    uccSavety = ucSavety;                   // NÑchste Zeit darf wieder gesetzt werden, wenn sie korrekt ist

    /* Zeit nur nach Setzintervall setzen */
    if (++uscIntervall < usIntervall)
        return;
    uscIntervall = 0;

    /* Zeit in Uhrenchip setzen */
    TRACE (DCF77_EVAL_OK, NULL, 0);
    setClock (&dt);
    uscClockSet++;

    return;
    }

VOID far _loadds timerHandler (VOID)
    {
    static UCHAR oldBit = 0;
    static UCHAR ucActualSecond = 0;
    static USHORT uscLow  = 0;
    static USHORT uscHigh = 0;
    SHORT  i;
    USHORT port;
    UCHAR  zw, bit;

    /* FÅr Init-Zeitpunkt zur CPU-Geschwindigkeitsbestimmung (fÅr n/10-Tick-Schleife) */
    initflag++;

    /* ZÑhler fÅr TrÑgererkennung; traeger == 0 => kein TrÑger */
    if (traeger > 0)
        traeger--;
    if (traeger == 0)
        {
        flStatus &= ~STATUS_CARRIER;
        ruecksetzen ();
        }

    /* SekundenzÑhler */
    if (dt.seconds > 59)
        {
        TRACE (DCF77_60S, NULL, 0);
        dt.seconds = 0;
        ruecksetzen ();
        }

    /* neues Bit bestimmen */
    if ((ucPortTyp == COM) || (ucPortTyp == COMX))
        {
        i = getcom();

        if (!(i & ~1))
            {
            data[dt.seconds] = i;
            dt.seconds++;
            uscHigh = 0;
            traeger = usThrCarrier;         // Neuer Impuls
            flStatus |= STATUS_CARRIER;
            }
        if (uscHigh > usThrMinute)          // 1.5s
            {
            /* Minutenzeichen */
            if (dt.seconds > 58)
                {
                getTime ();
                if (!(flStatus & STATUS_TIMEVALID))
                    ucPol   = POSITIVE;         // sicherheitshalber UART neu initialisieren
                }
            if (uscHigh > usThrMinute * 40) // 60s
                {
                badmin++;
                uscHigh = 0;
                ucPol   = POSITIVE;         // sicherheitshalber UART neu initialisieren
                }
            initdata();
            }
        uscHigh++;
        }
    else
        {
        if ((ucPortTyp == SER) || (ucPortTyp == SERX))
            {
            port = usPortAddr + 6;
            /* jetzt eventuell die Uhr aktivieren */
            if ((ucPower >= 0) && (ucPower < 4))
                {
                zw = (UCHAR) inbyte(port-2);
                zw = zw & 0xFC;
                zw = zw | ucPower;
                outbyte (port-2, zw);
                }
            }
        else
            /* Gameport */
            port = usPortAddr;

        if (getbit_auto (port) != 0)
            {
            /* 0 empfangen */
            if (oldBit == 1)
                {
                TRACE (DCF77_0TO1, &dt.seconds, 1);

                /* vorhergehendes Bit war 0 */
                traeger = usThrCarrier;                           /* Neuer Impuls, 3.2s */
                flStatus |= STATUS_CARRIER;
                if (ucActualSecond != dt.seconds || usTTicks == 0)
                    {
                    data[dt.seconds] = (uscHigh > usThrBitlen) ? 1 : 0;
                    if (usTimerMode == TIMER_MODE_HR && insertThrValue (uscHigh))
                        calcThreshold ();
#ifdef _TRACE_
                    iTrace[0] = 1;
                    iTrace[1] = uscHigh;
#endif
                    TRACE (DCF77_DATASET, (char near *)iTrace, sizeof (iTrace));
                    }
                if (uscHigh > usThrPolarity)
                    {
                    /* falsche PolaritÑt >= ucPol umdrehen !*/
                    ucPol ^= 0xFF;
                    TRACE (DCF77_CHGPOL, &ucPol, sizeof (ucPol));
                    initdata ();
                    ruecksetzen ();
                    }
                dt.seconds++;
                }
            /* Vorbereiten fÅr "1"-Empfang */
            oldBit  = 0;
            uscHigh = 0;
            uscLow++;
            }
        else
            {
            /* 1 empfangen */
            if (oldBit == 0)
                TRACE (DCF77_1TO0, NULL, 0);

            if (uscLow > usThrMinute)
                {
                /* Minutenzeichen */
                if (dt.seconds > 58)
                    {
                    TRACE (DCF77_MINUTE, data, 60);
                    getTime ();
                    TRACE (DCF77_RESULT, (char near *)&flStatus, sizeof (flStatus));
                    }
                else
                    {
                    flStatus &= ~STATUS_TIMEVALID;
                    badmin++;
                    }
                initdata ();
                }
            oldBit = 1;
            /* >>>>>>>>>>> noch ansehen */
            if (uscHigh == usThrBitlen && dt.seconds != ucActualSecond && usTTicks != 0)
                {
                // hticcnt:    ZÑhler fÅr 1/4 Ticks
                // realticcnt: ZÑhlerwert fÅr eingestellten n/4 Tick
                // ticcount:   SchleifendurchlÑufe fÅr 1 Tick
                // usTTicks:    Anzahl 1/4 Ticks (n)
                int hticcnt;
                int realticcnt;

                /* jetzt halbes Tic testen */
                hticcnt    = 0;
                realticcnt = ticcount*usTTicks/4;
                while (0 == getbit (port) && hticcnt < realticcnt)
                    hticcnt++;
                data[dt.seconds] = (hticcnt < realticcnt) ? 0 : 1;
                ucActualSecond = dt.seconds;
                /* Jetzt noch erreichen, da· kein zweiter impuls in dieser Sec. ausgewertet wird.*/
                }
            /* <<<<<<<<<<<<<<<<<<<<<<<<<< */
            uscLow = 0;
            uscHigh++;
            } /* endif */
        }
    }

VOID start (void)
    {
    initdata ();
    ruecksetzen ();
    }

BOOL insertThrValue (USHORT bitlen)
    {
    USHORT us;

    /* ImpulslÑnge normieren und Bereich prÅfen */
    bitlen -= THR_10MS_MIN;
    if (bitlen > THR_ARRAYSIZE-1)
        {
        TRACE (DCF77_THREXCEEDED, NULL, 0);
        return FALSE;
        }

    /* Initialisierung des Arrays */
    if (ucThrMaxValue[0] == 255 || ucThrMaxValue[1] == 255)
        {
        for (us = 0; us < THR_ARRAYSIZE; us++)
            ucThrValues[us] = 0;
        }

    /* neuen Wert eintragen */
    ucThrValues[bitlen]++;
    return TRUE;
    }

VOID calcThreshold (VOID)
    {
    USHORT us, j;
    USHORT usPrevValue;
    BOOL   bFlag;

    /* Schwelle suchen */
    j = usPrevValue = 0;
    ucThrMaxValue[0] = ucThrMaxValue[1] =
    ucThrMaxIndex[0] = ucThrMaxIndex[1] = 0;
    for (us = 0; us < THR_ARRAYSIZE; us++)
        {
        if (ucThrValues[us] > usPrevValue)
            {
            /* Steigung > 0: Index j beibehalten */
            if (bFlag && us - ucThrMaxIndex[j] > THR_PEAK_DIST)
                {
                j ^= 1;
                bFlag = FALSE;
                }
            if (ucThrValues[us] > ucThrMaxValue[j])
                {
                ucThrMaxIndex[j] = us;
                ucThrMaxValue[j] = ucThrValues[us];
                }
            }
        else
            {
            /* Steigung <= 0: Index fÅr anderen Arrayeintrag vorbereiten */
            bFlag = TRUE;
            }
        usPrevValue = ucThrValues[us];
        }
    j = ucThrMaxIndex[0] - ucThrMaxIndex[1];
    if (j > 5 || j < -5)
        usThrBitlen = THR_10MS_MIN + (ucThrMaxIndex[0] + ucThrMaxIndex[1]) / 2;

    TRACE (DCF77_THR, (BYTE *)&usThrBitlen, sizeof (usThrBitlen));
    TRACE (DCF77_THRSET, ucThrValues, sizeof (ucThrValues));
    return;
    }

