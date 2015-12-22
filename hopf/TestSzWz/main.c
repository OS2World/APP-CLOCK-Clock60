#include <os2.h>
#include <stdio.h>
#include "dcfioctl.h"
#include "tloc.h"

#define CCHMAXLEN   200

int main (void)
    {
    TLOC_DEF  dt;
    HFILE     hFile;
    char      szLine[CCHMAXLEN];
    char     *szFileName = "tztab";

    OpenTZFile (szFileName, &hFile);

    /* Alle Eintr„ge auslesen */
    while (GetNextLineOfType (hFile, szLine, CCHMAXLEN, heading))
        {
        printf ("neue Zeile: %s\n", szLine);
        }

    /* Ersten Eintrag parsen */
    RewindTZFile (hFile);
//    MoveToTZNo (hFile, 1, szLine, CCHMAXLEN);
    FindEntry  (hFile, szLine, CCHMAXLEN, "EST5EDT");

    while (GetTZString (hFile, szLine, CCHMAXLEN, &dt))
        {
        printf ("Intervall: Stunden:       %d\n", dt.usHours);
        printf ("           Minuten:       %d\n", dt.usMinutes);
        printf ("           Tag:           %d...%d\n", dt.usFirstDay, dt.usLastDay);
        printf ("           Monat:         %d\n", dt.usMonth);
        printf ("           Jahr:          %d...%d\n", dt.usFirstYear, dt.usLastYear);
        printf ("           Wochentag:     %d...%d\n", dt.usFirstWeekday, dt.usLastWeekday);
        printf ("           Zeitdifferenz: %d\n", dt.sTimeDiff);
        }

    CloseTZFile (hFile);
    return 0;
    }
