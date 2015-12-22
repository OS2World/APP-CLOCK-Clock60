/***************************************************************************\
 *
 * PROGRAM NAME: WPDCF77
 * -------------
 *
 * VERSION: 6.0
 * --------
 *
 * MODULE NAME: SZ_WZ
 * ------------
 *
 * DESCRIPTION:
 * ------------
 *   Parsing of file for daylight saving time (TZTAB)
 *
 * NOTES:
 * ------
 *
 * FUNCTIONS:
 * -----------
 *   isblank ()
 *   stripSpace ()
 *   removeString ()
 *   strtoi ()
 *   strtosi ()
 *   strtoiv ()
 *   readLine ()
 *   queryType ()
 *   parseLine ()
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
 *  1.00    10-23-94  First release
 *  2.00    06-16-95  Extensions
 *  3.00    10-30-95  Client/Server support; SOM2
 *  4.00    03-15-96  Support for hopf
 *  4.20    10-15-96  Support for hopf GPS
 *  4.30    17-05-97  Support for HR-Timer
 *  4.40    01-31-00  Multimedia support, bug fixing
 *  5.10    02-10-01  Bug fixing position of icon view
 *  5.20    06-09-01  Bug fixing audio data
 *  5.30    01-16-02  Implementation of reminder function
 *  5.40    11-22-03  Bug fixing reminder function
 *  6.00    02-15-04  USB support
 *
 *  Copyright (C) noller & breining software 1995...2007
 *
\******************************************************************************/
#define INCL_DOSFILEMGR
#define INCL_DOSERRORS
#define INCL_DOSDATETIME
#include <os2.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "tloc.h"
#include "sz_wz.h"

#ifndef __32BIT__
#pragma check_stack (off)

/*******************************************************************\
    isblank: checks, if character is a white space.
    return:  BOOL:   TRUE if "c" is white space
\*******************************************************************/
BOOL isblank (CHAR c)
    {
    switch (c)
        {
        case ' ':
        case '\t':
            return TRUE;
            break;
        }
    return FALSE;
    }
#endif

/*******************************************************************\
    stripSpace: Removes leading spaces of a string.
    Input:  PSZ pszBuffer: pointer to string
    return: PSZ:           ppinter to first non-blank character
\*******************************************************************/
PSZ stripSpace (PSZ pszBuffer)
    {
    SHORT i;

    for (i = 0; pszBuffer[i] != '\0'; i++)
        if (!isblank (pszBuffer[i]))
            break;

    return pszBuffer+i;
    }

/*******************************************************************\
    removeString: Removes tokens at beginning of a string, which
                  are no digits or '-' (search for decimal number)
    input: PSZ pszBuffer: pointer to string
\*******************************************************************/
PSZ removeString (PSZ psz)
    {
    SHORT i;

    for (i = 0; psz[i] != '\0'; i++)
        if (psz[i] >= '0' && psz[i] <= '9' || psz[i] == '-')
            break;

    return psz+i;
    }

/*******************************************************************\
    strtoi: Converts a string into an integer. The converted number 
            will be removed from string.
    input:  PSZ psz:         source string
    output: PSHORT psResult: result
    return: PSZ:  pointer to first character after the integer
            NULL: no interger found
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
    strtosi: Converts a string into a single integer. The converted
             integer will be removed from string.
    input:  PSZ psz:         source string
    output: PSHORT psResult: result
    return: PSZ:  pointer to first non blank after the integer
            NULL: no integer found
\*******************************************************************/
PSZ strtosi (PSZ psz, PSHORT psResult)
    {
    if ((psz = strtoi (psz, psResult)) != NULL)
        return stripSpace (psz);
    else
        return NULL;
    }

/*******************************************************************\
    strtoiv: Converts a string into an integer interval.
             The converted integers will be removed from the string.
    input:  PSZ psz:         source string
    output: PUSHORT pusFirst: start of interval
            PUSHORT pusLast:  end of interval
                              0 => no interval
    return: PSZ:  pointer to character after valid number / interval
            NULL: invalid interval
\*******************************************************************/
PSZ strtoiv (PSZ pszLine, PUSHORT pusFirst, PUSHORT pusLast)
    {
    PSZ    pszTemp;

    pszTemp = strtoi (pszLine, (PSHORT)pusFirst);
    if (pszTemp == NULL)
        return NULL;                           // invalid start

    if ((*pszTemp != ' ') && (*pszTemp != '-'))
       return NULL;                            // invalid start

    pszLine = stripSpace (pszTemp);

    if (*pszLine == '-')
        {
           pszTemp = stripSpace(pszLine+1);
           return strtosi (pszTemp, (PSHORT)pusLast);
        }
    else
        *pusLast = 0;

    return pszLine;
    }

/*******************************************************************\
    readLine: Reads a line from file with handle hFile; max ulLen
             characters will be read, further characters until end 
             of line will be removed.
    input:  HFILE hFile:   file handle
            PSZ pszBuffer: pointer to destination buffer
            IOTYPE ulLen:  size of buffer
    return: TRUE:  line read
            FALSE: EOF
\*******************************************************************/
BOOL readLine (HFILE hFile, PSZ pszBuffer, IOTYPE ioLen)
    {
    IOTYPE ioRead;
    APIRET rc;
    ULONG  ulNewPos;
    USHORT i, j;
    LONG  lPos;
    UCHAR uch;

    /* fill buffer with data */
    rc = DosRead (hFile, pszBuffer, ioLen-1, &ioRead);
    if (rc != 0 || ioRead == 0)
        return FALSE;
    pszBuffer[ioRead] = '\0';

    /* search CRLF as end of line */
    for (i=0; i<ioRead; i++)
        if (pszBuffer[i] == '\r' || pszBuffer[i] == '\n')
            break;

    /* Is the end of line in the already read buffer? */
    for (j=i; j<ioRead; j++)
        if (pszBuffer[j] != '\r' && pszBuffer[j] != '\n')
            break;

    pszBuffer[i] = '\0';

    if (j < ioLen-1)
        /* buffer was long enough for line, some characters have to be written back */
        lPos = ioRead-j;
    else
        {
        /* buffer was too small, the real end of line has to be searched for; */
        /* these additional characters will be ignored                        */
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
    queryType: Determines the type of the line out of the TLOC-file
    input:  PSZ pszLine: pointer to line buffer
    return: TokType: typ of the line
\*******************************************************************/
TokType queryType (PSZ pszLine)
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
    parseLine: Decomposes a line of type 'entryline'
    input:  PSZ pszLine:   pointer to line buffer
    output: PTLOC_DEF pdt: pointer to result structure
    return: TRUE: parsing was successful
            FALSE: line contains inconsistent data
\*******************************************************************/
BOOL parseLine (PSZ pszLine, PTLOC_DEF pdt)
    {
    SHORT  sTemp;
    USHORT usHours;                             // change-over time: houers
    USHORT usMinutes;                           // change-over time: minutes
    USHORT usFirstDay, usLastDay;               // change-over date: (interval) days
    USHORT usMonth;                             // change-over date: month
    USHORT usFirstYear, usLastYear;             // change-over date: interval years
    USHORT usFirstWeekday, usLastWeekday;       // change-over date: (interval) day of week
    SHORT  sTimeDiff;                           // time zone: difference to UTC

    /************ parse string ************/
    /* evaluate: minutes */
    pszLine = strtosi (pszLine, (PSHORT)&usMinutes);
    if (pszLine == NULL)
        return FALSE;

    /* evaluate: hours */
    pszLine = strtosi (pszLine, (PSHORT)&usHours);
    if (pszLine == NULL)
        return FALSE;

    /* evaluate: (interval) day*/
    pszLine = strtoiv (pszLine, &usFirstDay, &usLastDay);
    if (pszLine == NULL)
        return FALSE;

    /* evaluate: month */
    pszLine = strtosi (pszLine, (PSHORT)&usMonth);
    if (pszLine == NULL)
        return FALSE;

    /* evaluate: interval year */
    pszLine = strtoiv (pszLine, &usFirstYear, &usLastYear);
    if (pszLine == NULL)
        return FALSE;

    /* evaluate: (interval) day of week */
    pszLine = strtoiv (pszLine, &usFirstWeekday, &usLastWeekday);
    if (pszLine == NULL)
        return FALSE;

    /* evaluate: time difference */
    pszLine = removeString (pszLine);
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

    /************ plausibility checck ************/
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
 *                       Exported functions                        *
\*******************************************************************/

/*******************************************************************\
    OpenTZFile: Open input file
    input:  PSZ pszFileName: name of file
    output: PHFILE phFile:   file handle
    return: APIRET: return code of DosOpen
\*******************************************************************/
APIRET OpenTZFile (PSZ pszFileName, PHFILE phFile)
    {
    IOTYPE  ioAction;

    /* open file */
    return DosOpen (pszFileName, phFile, &ioAction, 0,
                    FILE_NORMAL,
                    OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,
                    OPEN_FLAGS_FAIL_ON_ERROR | OPEN_SHARE_DENYWRITE | OPEN_ACCESS_READONLY,
                    0);
    }

/*******************************************************************\
    RewindTZFile: Reset file pointer
    input:  HFILE hFile: file handle
    return: APIRET: return code of DosSetFilePtr
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
    CloseTZFile: Close file
    input:  HFILE hFile: file handle
    return: APIRET: return code of DosClose
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
    GetNextLineOfType: Gets next line of requested type out of file. 
    input:  HFILE hFile:    file handle
            USHORT usLen:   length of result buffer pszLine
            TokType usType: type of requested line
    output: PSZ pszLine:    new line
    return: TRUE:  line found
            FALSE: no line available
\*******************************************************************/
BOOL GetNextLineOfType (HFILE hFile, PSZ pszLine, USHORT usLen, TokType usType)
    {
    while (readLine (hFile, pszLine, usLen))
        {
        pszLine = stripSpace (pszLine);
        if (queryType (pszLine) == usType)
            return TRUE;
        }
    return FALSE;
    }

/*******************************************************************\
    MoveToTZNo: Sets file pointer to entry with the number us
                (Type heading)
    input:  HFILE hFile:   file handle
            USHORT us:     entry number
            USHORT usLen:  length of result buffer
    output: PSZ pszLine:   contents of requested line (heading)
    return: TRUE: entry found
            FALSE: no entry found
\*******************************************************************/
BOOL MoveToTZNo (HFILE hFile, USHORT us, PSZ pszLine, USHORT usLen)
    {
    USHORT  usActualNumber;

    /* read heading lines */
    for (usActualNumber = 0; usActualNumber < us;)
        {
        if (!GetNextLineOfType (hFile, pszLine, usLen, heading))
            return FALSE;
        usActualNumber++;
        }

    return TRUE;
    }

/*******************************************************************\
    GetTZString: Evaluates the next line of type eheading.
                 On exit, the file pointer points to the
                 following line.
    input:  HFILE hFile:   file handle
            USHORT usLen:  length of line buffer
    output: PSZ pszLine:   contents of evaluated line
            PTLOC_DEF pdt: result structure
    return: TRUE: entry found
            FALSE: no entry found
\*******************************************************************/
BOOL GetTZString (HFILE hFile, PSZ pszLine, USHORT usLen, PTLOC_DEF pdt)
    {
    BOOL bRC = FALSE;

    /* loop: evaluate lines until next heading or match */
    while (readLine (hFile, pszLine, usLen))
        {
        if (queryType (pszLine) == heading)
            break;
        if (parseLine (pszLine, pdt))
            {
            bRC = TRUE;
            break;
            }
        }

    return bRC;
    }

/*******************************************************************\
    FindEntry: Evaluates the line of type heading, which contains 
               the passed TimeZone string. On exit, the file pointer
               points to the following line.
               NOTE: In 16-bit programs, the string buffers must be
                     located in the standard data segment.
    input:  HFILE hFile:   file handle
            PSZ pszLine:   line buffer
            USHORT usLen:  length of line buffer
            PSZ pszTZone:  timezone-string
    return: 0:             no entry found
            sonst:         index of entry
\*******************************************************************/
USHORT FindEntry (HFILE hFile, PSZ pszLine, USHORT usLen, PSZ pszTZone)
    {
    USHORT index = 0;

    if (pszTZone == NULL)
        return 0;

    /* read heading lines */
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
    IsValidHandle: Checks, if the passed handle is a valid
                   file handle
    input:  HFILE hFile:   file handle
    return: TRUE:          handle is file handle
            FALSE:         handle is no file handle
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
