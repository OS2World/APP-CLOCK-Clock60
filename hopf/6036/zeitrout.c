/***************************************************************************\
 *
 * PROGRAMMNAME: HOPF6036
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
 *   getTime
 *   ioctl_fctn
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
#include "hopf6036.h"
#include "dcfioctl.h"
#include "common.h"

static USHORT usMaskSt = MASK_ST_ANT       +    // Info ber Ersatzantenne
                         MASK_ST_HLEAP     +    // Info ber Stundensprung
                         MASK_ST_SLEAP     +    // Info ber Schaltsekunde
                         MASK_ST_TZONE1    +    // Info ber Zeitzone 1
                         MASK_ST_TZONE2    +    // Info ber Zeitzone 2
                         MASK_ST_CARRIER   +    // Info ber Tr„ger vorhanden
                         MASK_ST_TIMEVALID +    // Info ber Zeit gltig
                         MASK_ST_SET       ;    // Info ber Zahl der Setzvorg„nge

static SHORT  uscClockSet = 0;                  // Zahl der erfolgreichen Setzvorg„nge
static USHORT minold      = 0;
static USHORT hourold     = 0;
static USHORT dayold      = 0;
static USHORT monold      = 0;
static USHORT yearold     = 0;
static DCF77_DATETIME dt;

short utcflag    = 0;
short offset     = 0;
short portaddr   = 0x280;

USHORT usIntervall  = 60;
static USHORT uscIntervall = 0;
static UCHAR  ucSavety     = 2;
static USHORT uccSavety    = 0;

/* Externe Methoden und Variable im masm - code */
void outbyte (short port, short value);
UCHAR inbyte (short port);
void INT_3 (void);

int getTime (void)
{
    int my_offset;
    int stat_val;

    stat_val = inbyte (portaddr + OFFSET_STAT);
    if (((stat_val & HSTAT_DCFTIME) == 0) ||          // no DCF time
        (stat_val == 0xFF))                           // HW not found
       return FALSE;

    dt.timezone = (SHORT)TZ_DCF;
    dt.hours    = bcd2bin (inbyte (portaddr + OFFSET_HOUR));
    dt.minutes  = bcd2bin (inbyte (portaddr + OFFSET_MIN ));
    dt.seconds  = bcd2bin (inbyte (portaddr + OFFSET_SEC ));
    dt.day      = bcd2bin (inbyte (portaddr + OFFSET_DAY ));
    dt.month    = bcd2bin (inbyte (portaddr + OFFSET_MON ));
    dt.year     = bcd2bin (inbyte (portaddr + OFFSET_YEAR));
    dt.year    += (dt.year > 90 ? 1900 : 2000);

    /* Zeitoffset */
    my_offset = offset;
    if (utcflag)
    {
        if (stat_val & HSTAT_TZONE1)
            my_offset -= 1;
        if (stat_val & HSTAT_TZONE2)
            my_offset -= 2;
    } /* endif */

    dt.hours += offset;
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
        if (dt.day < 1)
        {
            dt.month--;
            if (dt.month < 1)
            {
                dt.month += 12;
                dt.year--;
            } /* endif month */
            dt.day = lastDay (dt.month, dt.year);
        } /* endif */
    } /* endif */

    /* Plausibilit„tsprfung */
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
        dayold++;                           // Der letzte Tag des Monats wird nicht mehr geprft
        }

    return TRUE;
}

int ioctl_fctn (UCHAR far *newdate, USHORT categUfkt)
{
    USHORT categ = (USHORT)categUfkt /256;
    USHORT fkt   = (USHORT)categUfkt %256;
    UCHAR  ucVal;
    SHORT  i;
    SHORT  port;

    if (categ == IOCTL_DCF77)
    {
        if (fkt == DCF77_GETSTATUS)
        {
            // Neue FKT
            DCF77_STATUS far *status = (DCF77_STATUS far *)newdate;
            if (TestSeg (newdate, sizeof (DCF77_STATUS)) == 1)
                return 0x8103;

            status->cDrvType        = DRVTYPE_HOPF6036;   // Treiber ist ZEIT.SYS
            status->cVerMajor       = VERSION_MAJOR;      // Hauptversionsnummer
            status->cVerMinor       = VERSION_MINOR;      // Unterversionsnummer
            status->bIsHRTimerAvail = FALSE;              // HR-Timer nicht verfgbar
            status->bPolarity       = 0;                  // Polarit„t 0=negativ, 1=positiv
            status->uscBadSeconds   = 0;                  // empfangene falsche Zeiten
            status->uscGoodSeconds  = 0;                  // empfangene korrekte Zeiten
            status->uscTimeSet      = uscClockSet;        // Zahl der Setzvorg„nge
            status->flStatus        = 0;
            status->flMaskST        = usMaskSt;
            status->flMaskSS        = MASK_SS_HOPF;
            status->flMaskTO        = MASK_TO_ALL;
            status->flMaskIV        = MASK_IV_UPDATE | MASK_IV_CHECK;
            status->flMaskLN        = 0;

            ucVal = inbyte (portaddr + OFFSET_STAT);
            if (ucVal != 0xFF)
            {
                if (ucVal & HSTAT_ANT)
                    status->flStatus |= STATUS_ANT;
                if (ucVal & HSTAT_HLEAP)
                    status->flStatus |= STATUS_HLEAP;
                if (ucVal & HSTAT_SLEAP)
                    status->flStatus |= STATUS_SLEAP;
                if (ucVal & HSTAT_TZONE1)
                    status->flStatus |= STATUS_TZONE1;
                if (ucVal & HSTAT_TZONE2)
                    status->flStatus |= STATUS_TZONE2;
                if (ucVal & HSTAT_DCFTIME)
                    status->flStatus |= STATUS_TIMEVALID + STATUS_CARRIER;
            } /* endif */

            return 0;
        } // Ende Getstatus

        if (fkt == DCF77_GETDATETIME)
        {
            DCF77_DATETIME far *datetime = (DCF77_DATETIME far *)newdate;
            if (TestSeg (newdate, sizeof (DCF77_DATETIME)) == 1)
                return 0x8103;

            if (getTime ())
            {
              datetime->hours    = dt.hours;
              datetime->minutes  = dt.minutes;
              datetime->seconds  = dt.seconds;
              datetime->day      = dt.day;
              datetime->month    = dt.month;
              datetime->year     = dt.year;
              datetime->timezone = dt.timezone;
            }
            return 0;
        } // Ende GetDate

        if (fkt == DCF77_QUERYOFFSET)
        {
            DCF77_OFFSET far *offs = (DCF77_OFFSET far *)newdate;
            if (TestSeg (newdate, sizeof (DCF77_OFFSET)) == 1)
                return 0x8103;

            offs->flUTC =   utcflag;
            offs->sOffset = offset;  // Zeitoffset -3...+2 Stunden
            return 0;
        }

        if (fkt == DCF77_QUERYDATA)
        {
            DCF77_DATA far *data = (DCF77_DATA far *)newdate;
            if (TestSeg (newdate, sizeof (DCF77_DATA)) == 1)
                return 0x8103;

            data->usThreshold   = 0;
            data->bcRepeat      = (BYTE)ucSavety;
            data->bSupply       = 0;
            data->usIOAddr      = (USHORT)portaddr;
            data->usSetInterval = usIntervall;
            data->bPort         = PORT_HOPF6036;
            return 0;
        }

        if (fkt == DCF77_SETOFFSET)
        {
            DCF77_OFFSET far *offs = (DCF77_OFFSET far *)newdate;
            if (TestSeg (newdate, sizeof (DCF77_OFFSET)) == 1)
                return 0x8103;

            utcflag = offs->flUTC;
            if ((offs->sOffset > -3 ) && (offs->sOffset < 4))
                offset = (char)offs->sOffset;
            else
                return 0x8103;
            return 0;
        }

        if (fkt == DCF77_SETDATA)
        {
            DCF77_DATA far *data = (DCF77_DATA far *)newdate;
            if (TestSeg (newdate, sizeof (DCF77_DATA)) == 1)
                return 0x8103;

            portaddr = data->usIOAddr;

            /* Setzintervall */
            if (data->usSetInterval > 0)
                usIntervall = data->usSetInterval;

            /* Wiederholprfung */
            if (data->bcRepeat > 0)
                ucSavety = (UCHAR)data->bcRepeat;

            return 0;
        }

        if (fkt == DCF77_SETPORT)
        {
            DCF77_PORT far *data = (DCF77_PORT far *)newdate;
            if (TestSeg (newdate, sizeof (DCF77_PORT)) == 1)
                return 0x8103;

            for (i=0; i<data->bNum; i++)
            {
               port = portaddr + data->ppData[i].bPortOfs;
               outbyte (port, data->ppData[i].bValue);
            } /* endfor */
            return 0;
        }

        if (fkt == DCF77_GETMILLI)
        {
            DCF77_MILLI far *data = (DCF77_MILLI far *)newdate;
            if (TestSeg (newdate, sizeof (DCF77_MILLI)) == 1)
                return 0x8103;

                if ((inbyte (portaddr+0x3B) == 0x58) && (inbyte (portaddr+0x3C) == 0x4E))
                    data->usValue = inbyte (portaddr+0x38) + (inbyte (portaddr+0x39) * 256);
                else
                    data->usValue = 0xFFFF;
            return 0;
        }
    }
}

void timer_fctn (void)
{
    if (ucSavety > uccSavety)
        return;

    uccSavety = ucSavety;                   // N„chste Zeit darf wieder gesetzt werden, wenn sie korrekt ist

    if (++uscIntervall < usIntervall)
        return;
    uscIntervall = 0;

    if ((getTime ()) && (setClock (&dt)))
        uscClockSet++;                      // Schreiben war erfolgreich
    else
        uscIntervall = usIntervall - 1;     // Schreiben hat nicht geklappt; neuer Versuch

}

