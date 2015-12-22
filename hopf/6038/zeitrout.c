/***************************************************************************\
 *
 * PROGRAMMNAME: HOPF6038
 * -------------
 *
 * VERSION: 6.0
 * --------
 *
 * MODULNAME: ZEITROUT
 * ----------
 *
 * BESCHREIBUNG: Strategieroutine
 * -------------
 *
 * HINWEISE:
 * ---------
 *
 * FUNKTIONEN:
 * -----------
 *   strncmp
 *   GetWeekday
 *   CorrectTLOC
 *   setLong
 *   getLong
 *   setMode
 *   goodSat
 *   getTime
 *   AfterSwitch
 *   CalcOffset
 *   AddOffset
 *   ioctl_fctn
 *   minuteTimer
 *   timer_fctn
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
#pragma check_stack (off)
#include "os2.h"
#include "hopf6038.h"
#include "tloc.h"
#include "dcfioctl.h"
#include "common.h"

static USHORT usMaskSt   = MASK_ST_TZONE1    +    // Info Åber Zeitzone 1
                           MASK_ST_TZONE2    +    // Info Åber Zeitzone 2
                           MASK_ST_CARRIER   +    // Info Åber TrÑger vorhanden
                           MASK_ST_TIMEVALID +    // Info Åber Zeit gÅltig
                           MASK_ST_SET       +    // Info Åber Zahl der SetzvorgÑnge
                           MASK_ST_COORDINATES;   // Info Åber Ortskoordinaten

static short clocksetcnt = 0;
static LONG  lBreite;
static LONG  lLaenge;
static SHORT sflSet = 0;

DCF77_DATETIME dt;
short dt_weekday;
short utcflag      = 1;
short offset       = 0;
short portaddr     = 0x280;
short intervall    = 10;
short intervallcnt = 2;


char         chTimeZone[20];
TLOC_DEF     tloc_spring;
TLOC_DEF     tloc_fall;
static SHORT sTimeOffset;


/* Externe Methoden und Variable im masm - code */
void outbyte (short port, short value);
UCHAR inbyte (short port);
int TestSeg (void far *, USHORT);
ULONG ShiftR (ULONG, USHORT);
ULONG ShiftL (ULONG, USHORT);
void INT_3 (void);

int strncmp (const char *psz1, const char *psz2, unsigned int usMax)
{
    USHORT usCnt;
    SHORT  sResult;

    for (usCnt=0; usCnt<usMax; usCnt++)
    {
        if ((psz1[usCnt] == '\0') && (psz2[usCnt] == '\0'))
            return 0;

        sResult = psz1[usCnt] - psz2[usCnt];
        if (sResult != 0)
            return sResult;
    } /* endfor */
    return 0;
}

/*******************************************************************\
    GetWeekday: Berechnung des Wochentags aus dem Datum.
                Die Berechnung erfolgt unabhÑngig vom GÅltigkeits-
                beginn des Gregorianischen Kalenders
    Eingang:    pdt: DCF77_DATETIME mit day, month, year
                Das Jahr wird vierstellig angegeben.
    Return:     0=So, 1=Mo, 2=Di, 3=Mi, 4=Do, 5=Fr, 6=Sa
\*******************************************************************/
int GetWeekday (TLOC_DEF *ptloc)
{
    SHORT c_month;
    SHORT c_year;
    SHORT days;

    c_month = (ptloc->usMonth + 9) % 12;
    c_year  = dt.year - ((c_month > 9) ? 1 : 0);

    days  = ((c_month * 26)+ 5)/ 10;            /* RND (M * 2, 6)*/
    days += (c_year * 5)/ 4;                    /* FLOOR (J * 1, 25)*/
    days += ptloc->usFirstDay + 1;

    return days % 7;
}

/*******************************************************************\
    CorrectTLOC:Umrechnen der Zeit in tloc (local time) nach UTC
    Eingang:    ptloc: Zeiger auf TLOC_DEF-Struktur mit Eingangswert
    Ausgang:    ptloc: Ergebnis
\*******************************************************************/
void CorrectTLOC (TLOC_DEF *ptloc)
{
    ptloc->usMinutes += (ptloc->sTimeDiff % 60);
    ptloc->usHours   += (ptloc->sTimeDiff / 60);

    if (ptloc->usMinutes > 50000)       // < 0
    {
        ptloc->usMinutes += 60;
        ptloc->usHours--;
    }

    if (ptloc->usMinutes > 60)
    {
        ptloc->usMinutes -= 60;         // sollte nie vorkommen
        ptloc->usHours++;
    }

    if (ptloc->usHours > 23)
    {
        ptloc->usHours -= 24;
        ptloc->usFirstDay++;

        if (ptloc->usLastDay != 0)      // Intervall, also ggf korrigieren
            ptloc->usLastDay++;

        if (ptloc->usLastWeekday == 0)  // Genau ein Wochentag...
        {
            ptloc->usFirstWeekday++;
            ptloc->usFirstWeekday %= 7;
        }
    }

    if (ptloc->usHours > 50000)         // negativ
    {
        ptloc->usHours += 24;
        ptloc->usFirstDay--;

        if (ptloc->usLastDay != 0)      // Intervall, also ggf korrigieren
            ptloc->usLastDay--;

        if (ptloc->usLastWeekday == 0)  // Genau ein Wochentag...
        {
            ptloc->usFirstWeekday += 6; // Quasi -1
            ptloc->usFirstWeekday %= 7;
        }
    }
}

void setLong (UCHAR ucCmd, LONG lVal)
{
    SHORT  port;
    SHORT  pos;

    port = portaddr + OFFSET_INPUT;

    for (pos=24; pos >= 0; pos -=8)
    {
        outbyte (port, (USHORT)(ShiftR ((ULONG)lVal, pos) & 0xFF));
        port++;
    } /* endfor */

    port = portaddr + OFFSET_INPUT_CMD;
    outbyte (port, ucCmd);
}

LONG getLong (USHORT usOffset)
{
    SHORT  port;
    SHORT  pos;
    ULONG  ulResult;

    port = portaddr + usOffset;
    ulResult = 0;

    for (pos=24; pos >= 0; pos -=8)
    {
        ulResult += ShiftL ((ULONG)(inbyte (port)& 0xFF), pos);
        port++;
    } /* endfor */

    return (LONG)ulResult;
}

void setMode (UCHAR ucMode)
{
    SHORT  port;
    UCHAR  ucSetCode;

    port = portaddr + OFFSET_INPUT;
    ucSetCode  = (ucMode + 2)& 3;
    ucSetCode |= (ucMode & 4);
    outbyte (port++, ucSetCode);

    port = portaddr + OFFSET_INPUT_CMD;
    outbyte (port, SET_MODE);
}

int goodSat (void)
{
    SHORT sOffset, sSatCnt;

    sSatCnt = 0;
    for (sOffset=OFFSET_SN_SAT1; sOffset<=OFFSET_SN_SAT6; sOffset+=2)
    {
       if (inbyte (portaddr + sOffset) > 30)
           sSatCnt++;
    }
    return sSatCnt;
}

int getTime (void)
{
    SHORT stat_val;

    if (sflSet & SET_IN_WORK)                   // while Set in Work
         return FALSE;                          // do not read anything

    if ((inbyte (portaddr + OFFSET_MODE) & 3)!= 1)
    {
         sflSet |= SET_FLAG_MODE | SET_IN_WORK;
         return FALSE;
    }

    stat_val = inbyte (portaddr + OFFSET_STAT);
    if (((stat_val & HSTAT_DCFTIME) == 0) ||    // no DCF time
         (stat_val == 0xFF))                    // HW not found
       return FALSE;

    dt.hours    = bcd2bin (inbyte (portaddr + OFFSET_HOUR));
    dt.minutes  = bcd2bin (inbyte (portaddr + OFFSET_MIN ));
    dt.seconds  = bcd2bin (inbyte (portaddr + OFFSET_SEC ));
    dt.day      = bcd2bin (inbyte (portaddr + OFFSET_DAY ));
    dt.month    = bcd2bin (inbyte (portaddr + OFFSET_MON ));
    dt.year     = bcd2bin (inbyte (portaddr + OFFSET_YEAR));
    dt_weekday  =          inbyte (portaddr + OFFSET_DOW );

    dt.year += (dt.year > 90 ? 1900 : 2000);

    return TRUE;
}

BOOL AfterSwitch (PTLOC_DEF ptloc)
{
    USHORT usDeltaDay;

    usDeltaDay = ptloc->usFirstDay;
    if (dt.month < ptloc->usMonth)
        return FALSE;
    if (dt.month > ptloc->usMonth)
        return TRUE;

    if (ptloc->usLastWeekday == 0)          // one DayOfWeek
        usDeltaDay += (7 + ptloc->usFirstWeekday - GetWeekday (ptloc))% 7;

    if (dt.day < usDeltaDay)
        return FALSE;
    if (dt.day > usDeltaDay)
        return TRUE;

    if (dt.hours < ptloc->usHours)
        return FALSE;
    if (dt.hours > ptloc->usHours)
        return TRUE;
    return (dt.minutes >= ptloc->usMinutes);
}

SHORT CalcOffset (void)
{
    SHORT sOffs = offset * 60;

    if (utcflag)
        return sOffs;

    if (AfterSwitch (&tloc_spring))
        if (AfterSwitch (&tloc_fall))
            sOffs -= tloc_fall.sTimeDiff;
        else
            sOffs -= tloc_spring.sTimeDiff;
    else
        sOffs -= tloc_fall.sTimeDiff;

    return sOffs;
}

VOID AddOffset (SHORT my_offset)
{
    dt.minutes += my_offset % 60;
    dt.hours   += my_offset / 60;

    if (dt.minutes > 60)
    {
       dt.minutes -= 60;
       dt.hours++;
    } /* endif */

    if (dt.minutes < 0)
    {
       dt.minutes += 60;
       dt.hours--;
    } /* endif */

    if (dt.hours > 23)
    {
        dt.hours -= 24;
        dt.day++;
        if (dt.day > lastDay (dt.month, dt.year))
        {
            dt.day -= lastDay (dt.month, dt.year);
            dt.month++;
            if (dt.month > 12)
            {
                dt.month -= 12;
                dt.year++;
            } /* endif month */
        } /* endif day */
    } /* endif hours */

    if (dt.hours < 0)
    {
        dt.hours += 24;
        dt.day--;
        if (dt.day > 1)
        {
            dt.month--;
            dt.day += lastDay (dt.month, dt.year);
            if (dt.month < 1)
            {
                dt.month += 12;
                dt.year--;
            } /* endif month */
        } /* endif */
    } /* endif */

    return;
}

int ioctl_fctn (UCHAR far *newdate, USHORT categUfkt)
{
    USHORT categ = (USHORT)categUfkt /256;
    USHORT fkt   = (USHORT)categUfkt %256;
    UCHAR  ucVal;
    SHORT  i;
    SHORT  port;

    if (categ == IOCTL_DCF77){
        if (fkt == DCF77_GETSTATUS){
            // Neue FKT
            DCF77_STATUS far *status = (DCF77_STATUS far *)newdate;
            if (TestSeg (newdate, sizeof (DCF77_STATUS))== 1)
                return 0x8103;
            status->cDrvType      = DRVTYPE_HOPF6038;   // Treiber ist ZEIT.SYS
            status->cVerMajor     = VERSION_MAJOR;      // Hauptversionsnummer
            status->cVerMinor     = VERSION_MINOR;      // Unterversionsnummer
            status->bPolarity     = 0;                  // PolaritÑt 0=negativ, 1=positiv
            status->uscBadSeconds = 0;                  // empfangene falsche Zeiten
            status->uscGoodSeconds= 0;                  // empfangene korrekte Zeiten
            status->uscTimeSet    = clocksetcnt;        // Zahl der SetzvorgÑnge
            status->flStatus      = 0;
            status->flMaskST      = usMaskSt;
            status->flMaskSS      = MASK_SS_HOPF;
            status->flMaskTO      = MASK_TO_ALL;
            status->flMaskIV      = MASK_IV_UPDATE;
            status->flMaskLN      = 0;

            ucVal = inbyte (portaddr + OFFSET_STAT);
            if (! (utcflag))
            {
               if (sTimeOffset == - (tloc_spring.sTimeDiff))
                                         status->flStatus |= STATUS_TZONE1;
               if (sTimeOffset == - (tloc_fall.sTimeDiff))
                                         status->flStatus |= STATUS_TZONE2;
            }
            if ((ucVal != 0xFF)&& (ucVal & HSTAT_DCFTIME))
                                         status->flStatus |= STATUS_TIMEVALID;

            if (goodSat ())status->flStatus |= STATUS_CARRIER;

            return 0;
        } // Ende Getstatus

        if (fkt == DCF77_GETDATETIME){
            DCF77_DATETIME far *datetime = (DCF77_DATETIME far *)newdate;
            if (TestSeg (newdate, sizeof (DCF77_DATETIME))== 1)
                return 0x8103;

            if (getTime ())
            {
              sTimeOffset = CalcOffset ();
              AddOffset (sTimeOffset);
              datetime->hours    = dt.hours;
              datetime->minutes  = dt.minutes;
              datetime->seconds  = dt.seconds;
              datetime->day      = dt.day;
              datetime->month    = dt.month;
              datetime->year     = dt.year;
            }
            datetime->timezone = TZ_GPS;
            return 0;
        } // Ende GetDate

        if (fkt == DCF77_QUERYOFFSET ){
            DCF77_OFFSET far *offs = (DCF77_OFFSET far *)newdate;
            if (TestSeg (newdate, sizeof (DCF77_OFFSET))== 1)
                return 0x8103;

            offs->flUTC =   utcflag;
            offs->sOffset = offset;  // Zeitoffset -3...+2 Stunden
            return 0;
        }

        if (fkt == DCF77_QUERYDATA){
            DCF77_DATA far *data = (DCF77_DATA far *)newdate;
            if (TestSeg (newdate, sizeof (DCF77_DATA))== 1)
                return 0x8103;

            data->bSupply       = 0;
            data->bcRepeat      = 0;
            data->usIOAddr      = (USHORT)portaddr;
            data->usSetInterval = (USHORT)intervall;
            data->usThreshold   = 0;
            data->bPort         = PORT_HOPF6036;
            return 0;
        }

        if (fkt == DCF77_SETOFFSET){
            DCF77_OFFSET far *offs = (DCF77_OFFSET far *)newdate;
            if (TestSeg (newdate, sizeof (DCF77_OFFSET))== 1)
                return 0x8103;

            utcflag = offs->flUTC;
            if ((offs->sOffset > -3 )&& (offs->sOffset < 4))
                offset = (char)offs->sOffset;
            else
                return 0x8103;
            return 0;
        }

        if (fkt == DCF77_SETDATA){
            DCF77_DATA far *data = (DCF77_DATA far *)newdate;
            if (TestSeg (newdate, sizeof (DCF77_DATA))== 1)
                return 0x8103;

            portaddr = data->usIOAddr;
            if (data->usSetInterval > 0)
            {
                intervall    = (short)data->usSetInterval;
                intervallcnt = intervall;
            }
            return 0;
       }

        if (fkt == DCF77_SETPORT){
            DCF77_PORT far *data = (DCF77_PORT far *)newdate;
            if (TestSeg (newdate, sizeof (DCF77_PORT))== 1)
                return 0x8103;

            for (i=0; i<data->bNum; i++){
               port = portaddr + data->ppData[i].bPortOfs;
               outbyte (port, data->ppData[i].bValue);
            } /* endfor */
            return 0;
        }

        if (fkt == DCF77_GETMILLI){
            DCF77_MILLI far *data = (DCF77_MILLI far *)newdate;
            if (TestSeg (newdate, sizeof (DCF77_MILLI))== 1)
                return 0x8103;

               if ((inbyte (portaddr + OFFSET_KENNUNG1)== 0x58)
                && (inbyte (portaddr + OFFSET_KENNUNG2)== 0x4E))
                  data->usValue = inbyte (portaddr + OFFSET_MS_LSB)
                               + (inbyte (portaddr + OFFSET_MS_MSB)* 256);
               else
                  data->usValue = 0xFFFF;
            return 0;
        }

        if (fkt == DCF77_SETCOORD){
            DCF77_COORD far *data = (DCF77_COORD far *)newdate;
            if (TestSeg (newdate, sizeof (DCF77_COORD))== 1)
                return 0x8103;
            lBreite = data->Breite;
            lLaenge = data->Laenge;
            sflSet |= SET_FLAG_BREITE | SET_FLAG_LAENGE | SET_IN_WORK;
            return 0;
        }

        if (fkt == DCF77_QUERYCOORD){
            DCF77_COORD far *data = (DCF77_COORD far *)newdate;
            if (TestSeg (newdate, sizeof (DCF77_COORD))== 1)
                return 0x8103;
            data->Breite = getLong (OFFSET_BREITE);
            data->Laenge = getLong (OFFSET_LAENGE);
            return 0;
        }

        if (fkt == DCF77_SETWZSZ){
            DCF77_WZSZ far *data = (DCF77_WZSZ far *)newdate;
            if (TestSeg (newdate, sizeof (DCF77_WZSZ))== 1)
                return 0x8103;

            if ((data->sDtSwitch1.usMonth < 7)
             && (data->sDtSwitch2.usMonth > 6))
            {
               tloc_spring = data->sDtSwitch1;
               tloc_fall   = data->sDtSwitch2;
            }
            else if ((data->sDtSwitch2.usMonth < 7)
                  && (data->sDtSwitch1.usMonth > 6))
            {
               tloc_spring = data->sDtSwitch2;
               tloc_fall   = data->sDtSwitch1;
            }
            else
                return 0x8103;

            CorrectTLOC (&tloc_spring);
            CorrectTLOC (&tloc_fall);
            return 0;
        }

    }
}

void minuteTimer (void)
{
    intervallcnt--;

    if (intervallcnt <= 0)
    {
        if (getTime ())// valid time
        {
            AddOffset (CalcOffset ());
            if (setClock (&dt))// is successfull
            {
                 clocksetcnt++;
            }
        }
        intervallcnt = intervall;
    } /* endif */
}

void timer_fctn (void)// alle 5 Sekunden
{
    static short counter = 12;

    counter--;
    if (counter <= 0)
    {
        minuteTimer ();
        counter = 12;
    }

    if (sflSet | SET_FLAG_MODE)
    {
          setMode (3 | 4);
          sflSet &= (~SET_FLAG_MODE);
    }
    else if (sflSet | SET_FLAG_BREITE)
    {
          setLong (SET_BREITE, lBreite);
          sflSet &= (~SET_FLAG_BREITE);
    }
    else if (sflSet | SET_FLAG_LAENGE)
    {
          setLong (SET_LAENGE, lLaenge);
          sflSet &= (~SET_FLAG_LAENGE);
    }
    else
          sflSet =  0;
}

