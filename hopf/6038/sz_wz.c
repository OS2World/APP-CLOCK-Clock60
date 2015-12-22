/***************************************************************************\
 *
 * PROGRAMMNAME: WPDCF77
 * -------------
 *
 * VERSION: 6.0
 * --------
 *
 * MODULNAME: SZ_WZ
 * ----------
 *
 * BESCHREIBUNG: Parsen der Datei fÅr Sommer-/Winterzeit-Umschaltzeitpunkte
 * -------------
 *
 * HINWEISE:
 * ---------
 *
 * FUNKTIONEN:
 * -----------
 *   isblank ()
 *   RemoveSpace ()
 *   RemoveString ()
 *   strtoi ()
 *   strtosi ()
 *   strtoiv ()
 *   GetLine ()
 *   QueryType ()
 *   ParseLine ()
 *   OpenTZFile ()
 *   RewindTZFile ()
 *   CloseTZFile ()
 *   GetNextLineOfType ()
 *   MoveToTZNo ()
 *   GetTZString ()
 *   FindEntry ()
 *   IsValidHandle ()
 *
 *  Ver.    Date      Comment
 *  ----    --------  -------
 *  4.20    10-15-96  UnterstÅtzung fÅr hopf GPS
 *  4.40    01-31-00  Multimedia-UnterstÅtzung, Bugfixing
 *  5.00    26-06-00  Neuzusammenstellung der Version
 *  5.20    06-09-01  Fehlerkorrektur Audiodaten
 *  5.30    01-16-02  Implementierung der Erinnerungsfunktion
 *  5.40    11-22-03  Fehlerkorrektur Erinnerungsfunktion
 *
 *  Copyright (C) noller & breining software 1995...2004
 *
\******************************************************************************/
#define INCL_DOSFILEMGR
#define INCL_DOSERRORS
#include <os2.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "dcfioctl.h"
#include "tloc.h"

USHORT DaysInMonth (DATETIME *pDT);

#ifndef __32BIT__
#pragma check_stack (off)

/*******************************************************************\
    isblank: PrÅft ob Zeichen ein White Space ist.
    Return:  BOOL:   TRUE if "c" is white space
\*******************************************************************/
BOOL isblank (CHAR c)
    {
    switch (c)
        {
        case ' ':
        case '\t':
            return TRUE;
            break;
        } /* endswitch */
    return FALSE;
    }
#endif

/*******************************************************************\
    RemoveSpace: Entfernt fÅhrende Leerzeichen am Anfang eines
                 Strings.
    Eingang: PSZ pszBuffer: Zeiger auf String
    Return:  PSZ:  Zeiger auf erstes non-Blank
\*******************************************************************/
PSZ RemoveSpace (PSZ pszBuffer)
    {
    SHORT i;

    for (i = 0; pszBuffer[i] != '\0'; i++)
        if (!isblank (pszBuffer[i]))
            break;

    return (pszBuffer+i);
    }

/*******************************************************************\
    RemoveString: Entfernt am Beginn eines Strings Tokens, die nicht
                  zu dezimalen Integerzahlen gehîren.
    Eingang: PSZ pszBuffer: Zeiger auf String
\*******************************************************************/
PSZ RemoveString (PSZ psz)
    {
    SHORT i;

    for (i = 0; psz[i] != '\0'; i++)
        if (psz[i] >= '0' && psz[i] <= '9' || psz[i] == '-')
            break;

    return (psz+i);
    }

/*******************************************************************\
    strtoi: Konvertiert einen String in eine int-Zahl. Die
            ausgewertete Zahl wird aus dem String entfernt.
    Eingang: PSZ psz:         Quellstring
    Ausgang: PSHORT psResult: Ergebnis
    Return:  PSZ:  Zeiger auf erstes Zeichen nach der Zahl
             NULL: Keine Zahl gefunden
\*******************************************************************/
PSZ strtoi (PSZ psz, PSHORT psResult)
    {
    SHORT  c;
    SHORT  sVz = 1;
    PSZ    pszTemp;

    *psResult = 0;

    switch (*psz)
        {
        case '-':
            sVz = -1;
        case '+':
            psz++;
            break;
        }

    pszTemp = psz;

    while ((c = *psz) != 0)
        {
        c -= '0';
        if ((c > 9) || (c < 0))
            break;
        *psResult = *psResult * 10 + c;
        psz++;
        }

    *psResult *= sVz;

    return (pszTemp != psz) ? psz : NULL;
    }

/*******************************************************************\
    strtosi: Konvertiert einen String in eine einzelne int-Zahl. Die
             ausgewertete Zahl wird aus dem String entfernt.
    Eingang: PSZ psz:         Quellstring
    Ausgang: PSHORT psResult: Ergebnis
    Return:  PSZ:  Zeiger auf erstes Non-Blank nach der Zahl
             NULL: Keine Zahl gefunden
\*******************************************************************/
PSZ strtosi (PSZ psz, PSHORT psResult)
    {
    if ((psz = strtoi (psz, psResult)) != NULL)
        return RemoveSpace(psz);
    else
        return NULL;
    }

/*******************************************************************\
    strtoiv: Konvertiert einen String in eine int-Zahlenintervall.
             Die ausgewerteten Zahlen werden aus dem String entfernt.
    Eingang: PSZ psz:         Quellstring
    Ausgang: PUSHORT pusFirst: Start des Intervalls
             PUSHORT pusLast:  Ende des Intervalls;
                               0 => kein Intervall
    Return:  PSZ:  Zeiger hinter gÅltige Zahl / gelesenes Intervall
             NULL: ungÅltiges Intervall
\*******************************************************************/
PSZ strtoiv (PSZ pszLine, PUSHORT pusFirst, PUSHORT pusLast)
    {
    PSZ    pszTemp;

    pszTemp = strtoi (pszLine, (PSHORT)pusFirst);
    if (pszTemp == NULL)
        return NULL;                           // Start ungÅltig

    if ((*pszTemp != ' ') && (*pszTemp != '-'))
       return NULL;                            // Start ungÅltig

    pszLine = RemoveSpace(pszTemp);

    if (*pszLine == '-')
        {
           pszTemp = RemoveSpace(pszLine+1);
           return strtosi(pszTemp, (PSHORT)pusLast);
        }
    else
        *pusLast = 0;

    return pszLine;
    }

/*******************************************************************\
    GetLine: Holt eine Zeile aus der Datei in hFile; Es werden max.
             ulLen Zeichen geholt, weitere Zeichen bis zum Zeilen-
             ende werden entfernt.
    Eingang: HFILE hFile:   Dateihandle
             PSZ pszBuffer: Zeiger auf Zielpuffer
             IOTYPE ulLen:   Puffergrî·e
    Return:  TRUE:  Zeile gelesen
             FALSE: EOF
\*******************************************************************/
BOOL GetLine (HFILE hFile, PSZ pszBuffer, IOTYPE ioLen)
    {
    IOTYPE ioRead;
    APIRET rc;
    ULONG  ulNewPos;
    USHORT i, j;
    LONG  lPos;
    UCHAR uch;

    /* Puffer mit Daten fÅllen */
    rc = DosRead (hFile, pszBuffer, ioLen-1, &ioRead);
    if (rc != 0 || ioRead == 0)
        return FALSE;
    pszBuffer[ioRead] = '\0';

    /* CRLF fÅr Zeilenende suchen */
    for (i=0; i<ioRead; i++)
        if (pszBuffer[i] == '\r' || pszBuffer[i] == '\n')
            break;

    /* Ist das Zeilenende noch im bereits gelesenen Puffer? */
    for (j=i;j<ioRead; j++)
        if (pszBuffer[j] != '\r' && pszBuffer[j] != '\n')
            break;

    pszBuffer[i] = '\0';

    if (j < ioLen-1)
        /* Puffer war fÅr die Zeile lang genug, es mÅssen Zeichen zurÅckgeschrieben werden */
        lPos = ioRead-j;
    else
        {
        /* Puffer war zu klein, es mu· nach dem echten Zeilenende gesucht werden; */
        /* Diese Åbrigen Zeichen werden weggeworfen                               */
        while (!DosRead (hFile, &uch, 1, &ioRead))
            if ((uch != '\r' && uch != '\n' && uch != EOF ) || ioRead == 0)
                break;
        lPos = ioRead;
        }
#ifdef __32BIT__
    DosSetFilePtr (hFile, -lPos, FILE_CURRENT, &ulNewPos);
#else
    DosChgFilePtr (hFile, -lPos, FILE_CURRENT, &ulNewPos);
#endif
    return TRUE;
    }

/*******************************************************************\
    QueryType: Bestimmt den Typ einer Zeile aus der TLOC-Datei
    Eingang: PSZ pszLine: Zeiger auf Zeilenpuffer
    Return:  TokType: Typ der Zeile
\*******************************************************************/
TokType QueryType (PSZ pszLine)
    {
    CHAR c;

    if ((c = pszLine[0]) >= '0' && c <= '9')
        return entryline;
    if (c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z')
        return heading;
    if (c == '#')
        return comment;

    return invalid;
    }

/*******************************************************************\
    ParseLine: Zerlegt eine Zeile vom Typ 'entryline'
    Eingang: PSZ pszLine:   Zeiger auf Zeilenpuffer
    Ausgang: PTLOC_DEF pdt: Zeiger auf Ergebnisstruktur
    Return:  TRUE: Parse war erfolgreich
             FALSE: Zeile enthÑlt inkonsistente Daten
\*******************************************************************/
BOOL ParseLine (PSZ pszLine, PTLOC_DEF pdt)
    {
    SHORT  sTemp;
    USHORT usHours;                             // Umschalt-Uhrzeit: Stunden
    USHORT usMinutes;                           // Umschalt-Uhrzeit: Minuten
    USHORT usFirstDay, usLastDay;               // Umschalt-Datum:   (Intervall) Tage
    USHORT usMonth;                             // Umschalt-Datum:   Monat
    USHORT usFirstYear, usLastYear;             // Umschalt-Datum:   Intervall Jahre
    USHORT usFirstWeekday, usLastWeekday;       // Umschalt-Datum:   (Intervall) Wochentag
    SHORT  sTimeDiff;                           // Zeitzone: Differenz zu UTC

    /************ String parsen ************/
    /* Auswertung: Minuten */
    pszLine = strtosi (pszLine, (PSHORT)&usMinutes);
    if (pszLine == NULL)
        return FALSE;

    /* Auswertung: Stunden */
    pszLine = strtosi (pszLine, (PSHORT)&usHours);
    if (pszLine == NULL)
        return FALSE;

    /* Auswertung: (Intervall) Tag */
    pszLine = strtoiv (pszLine, &usFirstDay, &usLastDay);
    if (pszLine == NULL)
        return FALSE;

    /* Auswertung: Monat */
    pszLine = strtosi (pszLine, (PSHORT)&usMonth);
    if (pszLine == NULL)
        return FALSE;

    /* Auswertung: Intervall Jahr */
    pszLine = strtoiv (pszLine, &usFirstYear, &usLastYear);
    if (pszLine == NULL)
        return FALSE;

    /* Auswertung: (Intervall) Wochentag */
    pszLine = strtoiv (pszLine, &usFirstWeekday, &usLastWeekday);
    if (pszLine == NULL)
        return FALSE;

    /* Auswertung: Zeitdifferenz */
    pszLine = RemoveString (pszLine);
    pszLine = strtosi (pszLine, &sTemp);
    if (pszLine == NULL)
        return FALSE;
    if ((*pszLine != ' ') && (*pszLine != ':') && (*pszLine != '\0'))
        return FALSE;

    sTimeDiff = 60 * sTemp;
    if (*pszLine == ':')
        {
        pszLine = strtoi (++pszLine, &sTemp);
        if (pszLine == NULL)
            return FALSE;
        sTimeDiff += sTemp;
        }

    /************ PlausibilitÑtsprÅfungen ************/
    if (usHours > 23 || usMinutes > 59 || usMonth > 12)
        return FALSE;
    if (usLastDay == 0 && usLastWeekday == 0 || usLastDay*usLastWeekday)
        return FALSE;
    if (usFirstDay > 31 || usLastDay > 31 || usFirstWeekday > 6 || usLastWeekday > 6)
        return FALSE;

    pdt->usHours        = usHours;
    pdt->usMinutes      = usMinutes;
    pdt->usFirstDay     = usFirstDay;
    pdt->usLastDay      = usLastDay;
    pdt->usMonth        = usMonth;
    pdt->usFirstYear    = usFirstYear;
    pdt->usLastYear     = usLastYear;
    pdt->usFirstWeekday = usFirstWeekday;
    pdt->usLastWeekday  = usLastWeekday;
    pdt->sTimeDiff      = sTimeDiff;

    return TRUE;
    }

/*******************************************************************\
 *                       Exportierte Funktionen                    *
\*******************************************************************/

/*******************************************************************\
    OpenTZFile: ôffnen des Eingabefiles
    Eingang: PSZ pszFileName: Name der Datei
    Ausgang: PHFILE phFile:   Dateihandle
    Return:  APIRET: Return-Code von DosOpen
\*******************************************************************/
APIRET OpenTZFile (PSZ pszFileName, PHFILE phFile)
    {
    IOTYPE  ioAction;

    /* Datei îffnen */
    return DosOpen (pszFileName, phFile, &ioAction, 0,
                    FILE_NORMAL,
                    OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,
                    OPEN_FLAGS_FAIL_ON_ERROR | OPEN_SHARE_DENYWRITE | OPEN_ACCESS_READONLY,
                    0);
    }

/*******************************************************************\
    RewindTZFile: ZurÅcksetzen des Dateizeigers
    Eingang: HFILE hFile: Dateihandle
    Return:  APIRET: Return-Code von DosSetFilePtr
\*******************************************************************/
APIRET RewindTZFile (HFILE hFile)
    {
    ULONG ulActual;

#ifdef __32BIT__
    return DosSetFilePtr (hFile, 0, FILE_BEGIN, &ulActual);
#else
    return DosChgFilePtr (hFile, 0L, FILE_BEGIN, &ulActual);
#endif
    }

/*******************************************************************\
    CloseTZFile: Schlie·en der Datei
    Eingang: HFILE hFile: Dateihandle
    Return:  APIRET: Return-Code von DosClose
\*******************************************************************/
APIRET CloseTZFile (HFILE hFile)
    {
    HFILE hFileTemp = hFile;
    hFile = -1;

    if (IsValidHandle (hFileTemp))
        return DosClose (hFileTemp);
    return ERROR_INVALID_HANDLE;
    }

/*******************************************************************\
    GetNextLineOfType: Holt die nÑchste Zeile aus der Datei, die
                       dem angeforderten Typ entspricht
    Eingang: HFILE hFile:    Dateihandle
             USHORT usLen:   LÑnge des Ergebnispuffers pszLine
             TokType usType: Typ der angeforderten Zeile
    Ausgang: PSZ pszLine:    Neue Zeile
    Return:  TRUE: Zeile gefunden
             FALSE: keine Zeile mehr verfÅgbar
\*******************************************************************/
BOOL GetNextLineOfType (HFILE hFile, PSZ pszLine, USHORT usLen, TokType usType)
    {
    while (GetLine (hFile, pszLine, usLen))
        {
        pszLine = RemoveSpace (pszLine);
        if (QueryType (pszLine) == usType)
            return TRUE;
        }
    return FALSE;
    }

/*******************************************************************\
    MoveToTZNo: Setzt den Dateizeiger auf den Eintrag mit der
                Nummer us (Typ heading)
    Eingang: HFILE hFile:   Dateihandle
             USHORT us:     Eintrag Nummer
             USHORT usLen:  LÑnge des Ergebnispuffers
    Ausgang: PSZ pszLine:   Inhalt der angeforderten Zeile (heading)
    Return:  TRUE: Eintrag gefunden
             FALSE: kein Eintrag gefunden
\*******************************************************************/
BOOL MoveToTZNo (HFILE hFile, USHORT us, PSZ pszLine, USHORT usLen)
    {
    USHORT  usActualNumber;

    /* öberschriftenzeilen einlesen */
    for (usActualNumber = 0; usActualNumber < us;)
        {
        if (!GetNextLineOfType (hFile, pszLine, usLen, heading))
            return FALSE;
        usActualNumber++;
        }

    return TRUE;
    }

/*******************************************************************\
    GetTZString: Wertet die nÑchste Zeile vom Typ heading aus.
                 Der Dateizeiger steht anschlie·end auf der
                 nachfolgenden Zeile
    Eingang: HFILE hFile:   Dateihandle
             USHORT usLen:  LÑnge des Zeilenpuffers
    Ausgang: PSZ pszLine:   Inhalt der untersuchten Zeile
             PTLOC_DEF pdt: Ergebnisstruktur
    Return:  TRUE: Eintrag gefunden
             FALSE: kein Eintrag gefunden
\*******************************************************************/
BOOL GetTZString (HFILE hFile, PSZ pszLine, USHORT usLen, PTLOC_DEF pdt)
    {
    BOOL bRC = FALSE;

    /* Schleife: Zeilen bis zur nÑchsten öberschrift bzw. Match untersuchen */
    while (GetLine (hFile, pszLine, usLen))
        {
        if (QueryType (pszLine) == heading)
            break;
        if (ParseLine (pszLine, pdt))
            {
            bRC = TRUE;
            break;
            }
        }

    return bRC;
    }

/*******************************************************************\
    FindEntry: Sucht die Zeile vom Typ heading, die den Åbergebenen
               TimeZone String enthÑlt. Der Dateizeiger steht
               anschlie·end auf der nachfolgenden Zeile
               ACHTUNG: Bei 16-Bit-Code mÅssen die String-Puffer
                        im Standard-Datensegment liegen.
    Eingang: HFILE hFile:   Dateihandle
             PSZ pszLine:   Zeilenpuffer
             USHORT usLen:  LÑnge des Zeilenpuffers
             PSZ pszTZone:  Zeitzonen-String
    Return:  0:             kein Eintrag gefunden
             sonst:         Index des Eintrages
\*******************************************************************/
USHORT FindEntry (HFILE hFile, PSZ pszLine, USHORT usLen, PSZ pszTZone)
    {
    USHORT index = 0;

    if (pszTZone == NULL)
        return 0;

    /* öberschriftenzeilen einlesen */
    while (GetNextLineOfType (hFile, pszLine, usLen, heading))
        {
        index++;
#ifdef __32BIT__
        if (strncmp (pszLine, pszTZone, 20) == 0 )
#else
        if (strncmp ((char _near *)pszLine, (char _near *)pszTZone, 20) == 0 )
#endif
            return index;
        }

    return 0;
    }

/*******************************************************************\
    IsValidHandle: PrÅft, ob das angegebene Handle ein gÅltiges
                   Dateihandle darstellt
    Eingang: HFILE hFile:   Dateihandle
    Return:  TRUE:          Handle ist Dateihandle
             FALSE:         Handle ist kein Dateihandle
\*******************************************************************/
BOOL IsValidHandle (HFILE hFile)
    {
#ifdef __32BIT__
    APIRET rc;
    ULONG  ulType;
    ULONG  ulAttr;

    rc = DosQueryHType (hFile, &ulType, &ulAttr);

    return (rc || (ulType & 0xFF)) ? FALSE : TRUE;
#else
    return (hFile > 2) ? TRUE : FALSE;
#endif
    }
