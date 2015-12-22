/***************************************************************************\
 *
 * PROGRAMMNAME: ZEIT, HOPF603x
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
 *  Ver.    Date      Comment
 *  ----    --------  -------
 *  5.00    06-21-00  Erste Version
 *  5.10    01-28-01  Korrektur der Minutenanzeige
 *  5.20    06-09-01  Fehlerkorrektur Audiodaten
 *  5.30    01-16-02  Implementierung der Erinnerungsfunktion
 *  5.40    11-22-03  Fehlerkorrektur Erinnerungsfunktion
 *
 *  Copyright (C) noller & breining software 1995...2003
 *
\******************************************************************************/
#ifndef COMMON_H
#define COMMON_H

#include "dcfioctl.h"

short atoi (char far *a);
char lower (char a);
short hextoint (char far *data);
USHORT bin2bcd (USHORT us);
USHORT bcd2bin (USHORT us);
SHORT lastDay (int month, int year);
USHORT setClock (DCF77_DATETIME *pdt);

#endif /* COMMON_H */
