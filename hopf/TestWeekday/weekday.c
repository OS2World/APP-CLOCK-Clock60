#include "stdio.h"
#include "math.h"
#include <os2.h>

USHORT usMonth = 2;
USHORT usFirstDay = 1;
USHORT usYear = 96;

/* Jahreszahl vierstellig */
/*******************************************************************\
    GetWeekday: Berechnung des Wochentags aus dem Datum.
                Die Berechnung erfolgt unabh„ngig vom Gltigkeits-
                beginn des Gregorianischen Kalenders
    Eingang:    pdt: DCF77_DATETIME mit day, month, year
    Return:     0=So, 1=Mo, 2=Di, 3=Mi, 4=Do, 5=Fr, 6=Sa
\*******************************************************************/
int GetWeekday (void)
{
    SHORT c_month;
    SHORT c_year;
    SHORT days;

    c_month = (usMonth + 9) % 12;
    c_year  = usYear - ((c_month > 9) ? 1 : 0);

    days  = ((c_month * 26) + 5) / 10;           /* RND   (M * 2,6)   */
    days += (c_year * 5) / 4;                    /* FLOOR (J * 1,25)  */
    days += usFirstDay + 1;

    return days % 7;
}

int main (int argc, char *argv[])
{
    short day, month, year;
    char  *wd[] = {"So", "Mo", "Di", "Mi", "Do", "Fr", "Sa"};
    int   rc;

    if (argc != 4)
        return 1;

    usFirstDay = atoi (argv[1]);
    usMonth    = atoi (argv[2]);
    usYear     = atoi (argv[3]);

    rc  = GetWeekday ();

    printf ("%d.%d.%d\n", usFirstDay, usMonth, usYear);
    printf ("Wochentag: %d: %s\n", rc,  wd[rc]);

    return 0;
}

