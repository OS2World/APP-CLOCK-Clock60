/***************************************************************************\
 *
 * PROGRAMMNAME: ZEIT
 * -------------
 *
 * VERSION: 5.4
 * --------
 *
 * MODULNAME: COMMON
 * ----------
 *
 * BESCHREIBUNG: gemeinsame Funktionen fÅr ZEIT, 6036 und 6038-Treiber
 * -------------
 *
 * HINWEISE:
 * ---------
 *
 * FUNKTIONEN:
 * -----------
 *   atoi
 *   lower
 *   hextoint
 *   bin2bcd
 *   bcd2bin
 *   lastDay
 *   setClock
 *
 *  Ver.    Date      Comment
 *  ----    --------  -------
 *  5.00    06-21-00  Erste Version
 *  5.10    01-28-01   Korrektur der Minutenanzeige
 *  5.20    06-09-01   Fehlerkorrektur Audiodaten
 *  5.30    01-16-02  Implementierung der Erinnerungsfunktion
 *  5.40    11-22-03  Fehlerkorrektur Erinnerungsfunktion
 *
 *  Copyright (C) noller & breining software 1995...2003
 *
\******************************************************************************/
#define INCL_DOSFILEMGR
#include <os2.h>
#include "common.h"

#define STDOUT      1
#define STDIN       0
#define STDERROR    2

#pragma check_stack (off)

short atoi (char far *a)
{
    short i=0, n=0, v=1;

    while (a[i] == ' ')
        i++;
    if (a[i] == '-')
    {
        v = -1;
        ++i;
    }
    for (; a[i]>='0'&& a[i]<='9' ; ++i)
        n = 10*n + (a[i]-'0');
    return n*v;
}

char lower (char a)
{
    if (a >= 'A' && a <= 'Z')
        return a + 'a' -'A';
    else
        return a;
}

short hextoint (char far *data)
{
    short i, n=0;
    char a;

    a = lower (data[0]);
    for (i = 0; (a >= '0' && a <= '9') || (a >= 'a' && a <= 'f'); i++)
    {
       if (a >= 'a' && a <= 'f')
           n = 16*n+ (a-'a'+10);
       else
           n = 16*n+ (a-'0');
       a = lower (data[i+1]);
    }
    return n;
}

USHORT bin2bcd (USHORT us)
{
    return ((us/10)*0x10 + (us%10));
}

USHORT bcd2bin (USHORT us)
{
    return ((us/0x10)*10 + (us%0x10));
}

/*******************************************************************\
    LastDay:    Berechnung der Zahl der Tage eines Monats
    Eingang:    month: Monat; Januar = 1; ...; Dezember = 12
                year:  Jahr; vierstellig angeben
    Return:     Zahl der Tage des Monats
\*******************************************************************/
SHORT lastDay (int month, int year)
{
    static SHORT sDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    SHORT        rc;

    month--;
    rc = sDays[month];

    /* special handling for february: leap year = 29 days */
    if (month == 1)
    {
        if ((year % 4) == 0)
        {
            if ((year % 100) == 0)
            {
                if ((year % 400) == 0)
                    rc = 29;
            }
            else
                rc = 29;
        }
    }

    return rc;
}

USHORT setClock (DCF77_DATETIME *pdt)
{
    SHORT sStatus;

    /* Zeitincrement deaktivieren */
    outbyte (0x70, 11);
    sStatus = inbyte (0x71);
    outbyte (0x70, 11);
    outbyte (0x71, sStatus | 0x80);

    /* Zeit setzen */
    outbyte (0x70, 0);
    outbyte (0x71, bin2bcd (pdt->seconds));
    outbyte (0x70, 2);
    outbyte (0x71, bin2bcd (pdt->minutes));
    outbyte (0x70, 4);
    outbyte (0x71, bin2bcd (pdt->hours));
    outbyte (0x70, 7);
    outbyte (0x71, bin2bcd (pdt->day));
    outbyte (0x70, 8);
    outbyte (0x71, bin2bcd (pdt->month));
    outbyte (0x70, 9);
    outbyte (0x71, bin2bcd (pdt->year % 100));

    /* Zeitincrement wieder einschalten */
    outbyte (0x70, 11);
    outbyte (0x71, sStatus);

    return TRUE;
}

