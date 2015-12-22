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
 *  Ver.    Date      Comment
 *  ----    --------  -------
 *  5.00    06-21-00  Erste Version
 *  5.20    06-09-01  Fehlerkorrektur Audiodaten
 *
 *  Copyright (C) noller & breining software 1995...2004
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
void printtoscreen (char far *data);
void printchartoscreen (USHORT data);
SHORT lastDay (int month, int year);
USHORT setClock (DCF77_DATETIME *pdt);

#endif /* COMMON_H */
