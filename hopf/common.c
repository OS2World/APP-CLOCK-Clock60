/***************************************************************************\
 *
 * PROGRAMMNAME: HOPF603x
 * -------------
 *
 * VERSION: 6.0
 * --------
 *
 * MODULNAME: COMMON
 * ----------
 *
 * BESCHREIBUNG: gemeinsame Funktionen fÅr 6036 und 6038-Treiber
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
 *   printtoscreen
 *   printchartoscreen
 *
 *  Ver.    Date      Comment
 *  ----    --------  -------
 *  5.00    06-21-00  Erste Version
 *  5.20    06-09-01  Fehlerkorrektur Audiodaten
 *
 *  Copyright (C) noller & breining software 1995...2004
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

void printtoscreen (char far *data)
{
    USHORT fsize;
    SHORT  size = 0;

    while (data[size] != 0)
        size++;
    if (size > 0)
        DosWrite (STDOUT, data, size, &fsize);

    return;
}

void printchartoscreen (USHORT data)
{
    USHORT fsize, cnt = 0, zws;
    CHAR   field[20];

    if (data == 0)
    {
        DosWrite (STDOUT, "0", 1, &fsize);
        return;
    }
    while (data > 0)
    {
        zws = data % 10;
        field[cnt] = (CHAR)zws + '0';
        data = data / 10;
        cnt ++;
        field[cnt] = 0;
        if (cnt >18)
            data = 0;
    }
    DosWrite (STDOUT, field, cnt, &fsize);

    return;
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
    SHORT validflag = 0;
    SHORT writeok;

    for (writeok = 0; writeok < 10; writeok++)
    {
        /* versuche bis zu 10 mal Datum zu setzen */
        outbyte (0x70, 10);
        validflag=inbyte (0x71);
        if ((validflag & 0x80)== 0)
        {
           outbyte (0x70, 11);
           validflag = inbyte (0x71);
           outbyte (0x70, 11);
           outbyte (0x71, validflag | 0x80);
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
           outbyte (0x70, 11);
           outbyte (0x71, validflag & 0x7F);
           return TRUE;
       }
    }
    return FALSE;
}

