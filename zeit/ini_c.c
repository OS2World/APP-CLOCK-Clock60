/***************************************************************************\
 *
 * PROGRAMMNAME: ZEIT
 * -------------
 *
 * VERSION: 5.4
 * --------
 *
 * MODULNAME: INI_C
 * ----------
 *
 * BESCHREIBUNG: gemeinsame Init-Funktionen fÅr ZEIT
 * -------------
 *
 * HINWEISE:
 * ---------
 *
 * FUNKTIONEN:
 * -----------
 * toupper
 * getUInt
 * getInt
 * getIValue
 * printToScreen
 * printToScreenCR
 * printUShortToScreen
 * printDemoMsg
 * endOfString
 * endOfToken
 * firstToken
 * nextToken
 * getToken
 * parseToken
 * parse
 * cini
 * f_cini

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
#define INCL_DOSPROCESS
#include <os2.h>

#pragma check_stack(off)
#include "dhcalls.h"
#include "zeit_c.h"
#include "tmr0_idc.h"
#include "trace.h"

#define STDOUT      1
#define STDIN       0
#define STDERROR    2

#define CCHMAXDATA  200

/* Speicherbereich fÅr Default-Werte aus Konfigurationsdatei */
extern DCF77_DATA DataDCFOrg;
extern CHAR       szConfigFile[CCHMAXPATH];

/* Konfigurationsvariable */
extern UCHAR  ucPortTyp;
extern UCHAR  ucBitPos;
extern UCHAR  ucPol;
extern UCHAR  ucPower;
extern UCHAR  ucAutoMode;
extern UCHAR  ucTimeZone;
extern UCHAR  ucSavety;
extern SHORT  sOffset;
extern USHORT usIntervall;
extern USHORT usPortAddr;

/* Timer-Variable */
extern USHORT usThreshold;                      // Schwellwert fÅr BitlÑnge (usThrBitlen) in ms
extern USHORT usTickLen;                        // TicklÑnge des Timers in ms
extern SHORT  usTTicks;                         // Zehntel-Ticks fÅr Sys-Timer

/* Schwellwerte fÅr Biterkennung */
extern USHORT usThrCarrier;                     // Erkennung TrÑgerausfall; 3.2s
extern USHORT usThrPolarity;                    // Erkennung PolaritÑt;     1.5s
extern USHORT usThrMinute;                      // Erkennung Minutenimpuls; 1.5s
extern USHORT usThrBitlen;                      // Erkennung BitpolaritÑt;  variabel

/* Modulinterne Variable */
USHORT trimm;

/***********************************************************************/
/*  Daten aus _INIDATA-Segment                                         */
/***********************************************************************/
extern USHORT usIsQuiet;

/* Fehlertexte (_INIDATA-Segment) */
extern const PCHAR szErrTooManyArgs;
extern const PCHAR szErrRead;
extern const PCHAR szErrFileNotFound1;
extern const PCHAR szErrFileNotFound2;
extern const PCHAR szErrOutOfRange;
extern const PCHAR szErrWrongParameter;
extern const PCHAR szCopyright;

/***********************************************************************/
/*  Definitionen, Daten fÅr Argumentparser                             */
/***********************************************************************/
/* RÅckgabewerte der getXInt-Funktionen */
#define PARM_DEFAULT        0       // Default-Wert wurde angefordert (Leerstring)
#define PARM_OK             1       // Wert gÅltig
#define PARM_ERR            2       // Wert enthÑlt ungÅltige Zeichen

/* usTyp in PARSESTRUCT */
#define OPTION_UNSIGNED     0x00    // Argument ist ein positiver Integer
#define OPTION_FUNSIGNED    0x80    // Argument ist ein positiver Integer; Zahl mu· vorhanden sein
#define OPTION_SIGNED       0x01    // Argument ist ein vorzeichenbehafteter Integer
#define OPTION_INDEX        0x02    // pusDest zeigt auf ein SWITCHSTRUCT-Array
#define OPTION_SWITCH       0x03    // Schalter; der Default wird nach pusDest geschrieben

/* flArgTyp in PARSESTRUCT */
#define OPTION_TYP_G        0x0001  // Option 'G' wurde geparst
#define OPTION_TYP_S        0x0002  // Option 'S' wurde geparst
#define OPTION_TYP_C        0x0004  // Option 'C' wurde geparst
#define OPTION_TYP_I        0x0008  // Option 'I' wurde geparst
#define OPTION_TYP_B        0x0010  // Option 'B' wurde geparst
#define OPTION_TYP_W        0x0020  // Option 'W' wurde geparst
#define OPTION_TYP_O        0x0040  // Option 'O' wurde geparst
#define OPTION_TYP_U        0x0080  // Option 'U' wurde geparst
#define OPTION_TYP_P        0x0100  // Option 'P' wurde geparst
#define OPTION_TYP_T        0x0200  // Option 'T' wurde geparst
#define OPTION_TYP_R        0x0400  // Option 'R' wurde geparst
#define OPTION_TYP_Q        0x0800  // Option 'Q' wurde geparst

/* Definition eines Optionstyps */
typedef struct _PARSESTRUCT
    {
    CHAR    c;                      // Kennbuchstabe
    USHORT  flArgTyp;               // Flagbit
    USHORT  usTyp;                  // Optionstyp
    USHORT  usMin;                  // Minimalwert
    USHORT  usMax;                  // Maximalwert
    USHORT  usDefault;              // Defaultwert
    PVOID   pusDest;                // Zieladdresse fÅr geparsten Wert
    } PARSESTRUCT;

/* MehrfacheintrÑge fÅr OPTION_INDEX */
typedef struct _SWITCHSTRUCT
    {
    PVOID   pVal;                   // Zeiger auf die Zielvariable; letzter Eintrag == NULL
    USHORT  val;                    // neuer Wert fÅr pVal; letzter Eintrag enthÑlt Zahl der Bytes in *pVal
    USHORT  flArgMask;              // nicht setzen, wenn flArgMask & flArgParsed
    } SWITCHSTRUCT;
typedef SWITCHSTRUCT far *PSWITCHSTRUCT;

#define LENTRY(a)   {NULL, sizeof ((a)), 0}

/* Listen fÅr MehrfacheintrÑge bei OPTION_INDEX-Optionen */
SWITCHSTRUCT arstrucBit[] =     {{&ucBitPos, 0x00, 0}, {&ucAutoMode, AUTOSTART, 0}, {NULL, 0, 0},
                                 {&ucBitPos, 0x01, 0}, {&ucAutoMode, AUTOOFF,   0}, {NULL, 0, 0},
                                 {&ucBitPos, 0x02, 0}, {&ucAutoMode, AUTOOFF,   0}, {NULL, 0, 0},
                                 {&ucBitPos, 0x04, 0}, {&ucAutoMode, AUTOOFF,   0}, {NULL, 0, 0},
                                 {&ucBitPos, 0x08, 0}, {&ucAutoMode, AUTOOFF,   0}, {NULL, 0, 0},
                                 {&ucBitPos, 0x10, 0}, {&ucAutoMode, AUTOOFF,   0}, {NULL, 0, 0},
                                 {&ucBitPos, 0x20, 0}, {&ucAutoMode, AUTOOFF,   0}, {NULL, 0, 0},
                                 {&ucBitPos, 0x40, 0}, {&ucAutoMode, AUTOOFF,   0}, {NULL, 0, 0},
                                 {&ucBitPos, 0x80, 0}, {&ucAutoMode, AUTOOFF,   0}, {NULL, 0, 0},
                                 LENTRY (ucBitPos),    LENTRY (ucAutoMode)};

SWITCHSTRUCT arstrucSer[] =     {{&usPortAddr, 0x000, 0}, {&ucPortTyp, SER, 0}, {&ucPol, NEGATIVE, 0}, {&ucBitPos, 0x0, 0}, {&ucAutoMode, AUTOSTART, 0}, {NULL, 0, 0},
                                 {&usPortAddr, 0x3F8, 0}, {&ucPortTyp, SER, 0}, {&ucPol, NEGATIVE, 0}, {&ucBitPos, 0x0, 0}, {&ucAutoMode, AUTOSTART, 0}, {NULL, 0, 0},
                                 {&usPortAddr, 0x2F8, 0}, {&ucPortTyp, SER, 0}, {&ucPol, NEGATIVE, 0}, {&ucBitPos, 0x0, 0}, {&ucAutoMode, AUTOSTART, 0}, {NULL, 0, 0},
                                 {&usPortAddr, 0x3E8, 0}, {&ucPortTyp, SER, 0}, {&ucPol, NEGATIVE, 0}, {&ucBitPos, 0x0, 0}, {&ucAutoMode, AUTOSTART, 0}, {NULL, 0, 0},
                                 {&usPortAddr, 0x2E8, 0}, {&ucPortTyp, SER, 0}, {&ucPol, NEGATIVE, 0}, {&ucBitPos, 0x0, 0}, {&ucAutoMode, AUTOSTART, 0}, {NULL, 0, 0},
                                 LENTRY (usPortAddr),     LENTRY (ucPortTyp),    LENTRY (ucPol),       LENTRY (ucBitPos),   LENTRY (ucAutoMode)};

SWITCHSTRUCT arstrucCom[] =     {{&usPortAddr, 0x000, 0}, {&ucPortTyp, COM, 0}, {&ucPol, POSITIVE, 0}, {&ucBitPos, 0x0, 0}, {&ucAutoMode, AUTOSTART, 0}, {NULL, 0, 0},
                                 {&usPortAddr, 0x3F8, 0}, {&ucPortTyp, COM, 0}, {&ucPol, POSITIVE, 0}, {&ucBitPos, 0x1, 0}, {&ucAutoMode, AUTOSTART, 0}, {NULL, 0, 0},
                                 {&usPortAddr, 0x2F8, 0}, {&ucPortTyp, COM, 0}, {&ucPol, POSITIVE, 0}, {&ucBitPos, 0x1, 0}, {&ucAutoMode, AUTOSTART, 0}, {NULL, 0, 0},
                                 {&usPortAddr, 0x3E8, 0}, {&ucPortTyp, COM, 0}, {&ucPol, POSITIVE, 0}, {&ucBitPos, 0x1, 0}, {&ucAutoMode, AUTOSTART, 0}, {NULL, 0, 0},
                                 {&usPortAddr, 0x2E8, 0}, {&ucPortTyp, COM, 0}, {&ucPol, POSITIVE, 0}, {&ucBitPos, 0x1, 0}, {&ucAutoMode, AUTOSTART, 0}, {NULL, 0, 0},
                                 LENTRY (usPortAddr),     LENTRY (ucPortTyp),   LENTRY (ucPol),        LENTRY (ucBitPos),   LENTRY (ucAutoMode)};

SWITCHSTRUCT arstrucGame[] =    {{&usPortAddr, 0x201, 0}, {&ucPortTyp, GAME, 0}, {&ucPol, POSITIVE, 0}, {&ucBitPos, 0x00, 0}, {&ucAutoMode, AUTOSTART, 0}, {NULL, 0, 0},
                                 {&usPortAddr, 0x201, 0}, {&ucPortTyp, GAME, 0}, {&ucPol, POSITIVE, 0}, {&ucBitPos, 0x10, 0}, {&ucAutoMode, AUTOOFF,   0}, {NULL, 0, 0},
                                 {&usPortAddr, 0x201, 0}, {&ucPortTyp, GAME, 0}, {&ucPol, POSITIVE, 0}, {&ucBitPos, 0x20, 0}, {&ucAutoMode, AUTOOFF,   0}, {NULL, 0, 0},
                                 LENTRY (usPortAddr),     LENTRY (ucPortTyp),    LENTRY (ucPol),        LENTRY (ucBitPos),    LENTRY (ucAutoMode)};

/* Parsertabelle fÅr zur VerfÅgung stehende Optionstypen */
PARSESTRUCT arstrucParse[] =    {{'G',  OPTION_TYP_G, OPTION_INDEX,      1,      2,  0,        (PUSHORT)arstrucGame},
                                 {'S',  OPTION_TYP_S, OPTION_INDEX,      1,      4,  0,        (PUSHORT)arstrucSer},
                                 {'C',  OPTION_TYP_C, OPTION_INDEX,      1,      4,  0,        (PUSHORT)arstrucCom},
                                 {'I',  OPTION_TYP_I, OPTION_UNSIGNED,   0, 0xFFFF, 60,        &usIntervall},
                                 {'B',  OPTION_TYP_B, OPTION_INDEX,      1,      8,  0,        (PUSHORT)arstrucBit},
                                 {'W',  OPTION_TYP_W, OPTION_UNSIGNED,   0,      3,  3,        &ucPower},
                                 {'O',  OPTION_TYP_O, OPTION_SIGNED,   -12,     12,  0,        &sOffset},
                                 {'U',  OPTION_TYP_U, OPTION_SWITCH,     0,      0,  TIME_UTC, &ucTimeZone},
                                 {'P',  OPTION_TYP_P, OPTION_FUNSIGNED,  0, 0xFFFF,  0,        &usPortAddr},
                                 {'T',  OPTION_TYP_T, OPTION_FUNSIGNED, 50,    250,  160,      &trimm},
                                 {'R',  OPTION_TYP_R, OPTION_FUNSIGNED,  0,      9,  0,        &ucSavety},
                                 {'Q',  OPTION_TYP_Q, OPTION_SWITCH,     0,      0,  TRUE,     &usIsQuiet},
                                 {'\0', 0,            0,                 0,      0,  0,        NULL}};

USHORT flArgParsed = 0;             // bisher geparster Options-Typ (OPTION_TYP_*-Flags)

#ifdef DEMO
PCHAR demotxt = {"\033[2J"                                                                     \
                 "                DCF77 - Treiber von Michael Geppert \r\n"                    \
                 "                     noller & breining software\r\n"                         \
                 "                         Version 6.00\r\n\n"                                 \
                 "Dieser ShareWare - DCF77-Treiber ist ein voll funktionsfÑhiges Produkt,\r\n" \
                 "das  sich von der  Vollversion nur  durch die  Ausgabe  dieser  Meldung\r\n" \
                 "unterscheidet. Die Vollversion erhalten Sie im Handel oder bei:\r\n"         \
                 "       noller & breining software\r\n"                                       \
                 "       Lauffener Str. 4\r\n"                                                    \
                 "       71522 Backnang\r\n\n"};
PDCF77_DATA pDCFDemo = NULL;
#endif

/***********************************************************************/
/*  Funktionsprototypen                                                */
/***********************************************************************/
/* Funktionen in _INITEXT */
CHAR toupper (CHAR a);
USHORT getUInt (PUSHORT pValue, PSZ far *ppszString);
USHORT getInt (PSHORT pValue, PSZ far *ppszString);
BOOL getIValue (PSWITCHSTRUCT pstrucSw, USHORT index);
BOOL endOfString (PSZ string);
PSZ firstToken (PSZ string);
PSZ parseToken (PSZ pszString);
PSZ getToken (PSZ string);
PSZ nextToken (PSZ string);
PSZ endOfToken (PSZ string);
BOOL parse (PSZ pszString);
void cini (PSZ argline);
void printToScreen (PSZ string);
void printToScreenCR (PSZ string);
void printUShortToScreen (USHORT data);

/* Funktionen in _TEXT */
void far f_cini (void);

#pragma alloc_text(_INITEXT, toupper, getInt, getUInt, getIValue, printToScreen, printToScreenCR, printUShortToScreen)
#pragma alloc_text(_INITEXT, endOfString, endOfToken, firstToken, nextToken, getToken, parseToken, parse, cini)

#ifdef DEMO
VOID printDemoMsg (VOID);
#pragma alloc_text(_INITEXT, printDemoMsg)
#endif /* DEMO */

CHAR toupper (CHAR a)
    {
    if (a >= 'a' && a <= 'z')
        return a + 'A' -'a';

    return a;
    }

USHORT getUInt (PUSHORT pValue, PSZ far *ppszString)
    {
    USHORT usRc = 0;
    UCHAR  c;
    BOOL   bIsHex = FALSE;

    c = **ppszString;
    if (c == '\0' || c == ' ' || c == '\t' || c == '\r')
        return PARM_DEFAULT;

    /* Ist die Zahl eine Hex-Zahl? */
    if (c == '0' && *(*ppszString + 1) == 'x')
        {
        bIsHex = TRUE;
        *ppszString += 2;
        c = toupper (**ppszString);
        }

    while (c != ' ' && c != '\t' && c != '\0' && c != '\r')
        {
        c -= '0';
        if (!bIsHex)
            {
            if (c <= 9 && (usRc == 6553 && c < 6 || usRc < 6553))
                usRc = usRc * 10 + (USHORT)c;
            else
                return PARM_ERR;
            }
        else
            {
            if (usRc > 0xFFF)
                return PARM_ERR;
            if (c <= 9)
                usRc = usRc * 16 + (USHORT)c;
            else if (c >= 'A'-'0' && c <= 'F'-'0')
                usRc = usRc * 16 + (USHORT)c - 'A' + '9' + 1;
            else
                return PARM_ERR;
            }
        c = toupper (*++*ppszString);
        }

    *pValue = usRc;
    return PARM_OK;
    }

USHORT getInt (PSHORT pValue, PSZ far *ppszString)
    {
    SHORT sRc = 0;
    SHORT sVz = 1;
    UCHAR c;

    c = **ppszString;
    switch (c)
        {
        case '\0':
        case ' ':
        case '\t':
        case '\r':
            return PARM_DEFAULT;

        case '-':
            sVz = -1;
            c = *++*ppszString;
            break;

        case '+':
            c = *++*ppszString;
            break;
        }

    if (getUInt ((PUSHORT)&sRc, ppszString) != PARM_OK)
        return PARM_ERR;

    *pValue = sRc * sVz;
    return PARM_OK;
    }

BOOL getIValue (PSWITCHSTRUCT pstrucSw, USHORT index)
    {
    PSWITCHSTRUCT p;
    BOOL   bRC = FALSE;
    USHORT i, ix;

    // letzte Zeile suchen: erster Eintrag mit pVal==NULL && val!=0
    for (p = pstrucSw; p->pVal != NULL || p->val == 0; p++)
        {}

    // Zeile fÅr Parameter suchen und Daten in Zielvariable kopieren
    for (ix = 0; pstrucSw->pVal != NULL; ix++, pstrucSw++)
        {
        while (pstrucSw->pVal != NULL)
            {
            if (ix == index)
                {
                if ((pstrucSw->flArgMask & flArgParsed) == 0)
                    for (i = 0; i < p->val; i++)
                        ((PBYTE)(pstrucSw->pVal))[i] = ((PBYTE)&(pstrucSw->val))[i];
                bRC = TRUE;
                break;
                }
            pstrucSw++;
            }
        pstrucSw++;
        }

    return bRC;
    }

VOID printToScreen (PSZ string)
    {
    USHORT size;

    if (string != NULL)
        {
        size = f_strlen (string);
        if (size > 0)
            DosWrite (STDOUT, string, size, &size);
        }

    return;
    }

/*******************************************************************\
    printToScreenCR: Gibt einen String auf der Konsole aus. Vor dem
                   ersten String soll die Copyright-Info ausgegeben
                   werden, falls der Quiet-Mode nicht gesetzt ist.
    Eingang: string:  Ausgabestring
\*******************************************************************/
VOID printToScreenCR (PSZ string)
    {
    static BOOL bCIsPrinted = FALSE;
    USHORT size;

    if (!bCIsPrinted && !usIsQuiet)
        {
        printToScreen (szCopyright);
        bCIsPrinted = TRUE;
        }

    printToScreen (string);
    return;
    }

VOID printUShortToScreen (USHORT data)
    {
    USHORT index = 4;
    CHAR   string[5];

    do
        {
        string[index--] = (char)(data % 10) + '0';
        data /= 10;
        } while (data > 0);

    index++;
    DosWrite (STDOUT, string+index, 5-index, &index);
    return;
    }

#ifdef DEMO
VOID printDemoMsg (VOID)
    {
    CHAR   daten[CCHMAXDATA];
    USHORT fsize;
    USHORT cnt;

    printToScreenCR (demotxt);
    pDCFDemo = &DataDCFOrg;

    for (cnt = 0; cnt < 3; cnt++)
        {
        DosBeep (500, 300);
        DosSleep (200L);
        }
    DosBeep (1200, 150);
    printToScreenCR ("    bitte Return - Taste drÅcken !\r\n");
    DosRead (STDIN, daten, 1, &fsize);

    return;
    }
#endif /* DEMO */

BOOL endOfString (PSZ string)
    {
    return *string == '\0' || *string == '\r';
    }

PSZ endOfToken (PSZ string)
    {
    PSZ psz;

    if (*string == '"')
        {
        /* Token endet beim nÑchsten '"' */
        for (psz = string+1; !endOfString (psz); psz++)
            if (*psz == '"')
                break;
        }
    else
        {
        /* Token endet mit nÑchstem Leerzeichen */
        for (psz = string; !endOfString (psz); psz++)
            if (*psz == ' ' || *psz == '\t')
                break;
        }

    return psz;
    }

PSZ firstToken (PSZ string)
    {
    while (*string == ' ' || *string == '\t')
        string++;

    return string;
    }

PSZ nextToken (PSZ string)
    {
    PSZ psz;

    /* aktuelles Token-Ende suchen */
    psz = endOfToken (string);
    if (*psz == '"')
        psz++;

    /* nÑchsten Token-Anfang suchen */
    for (; !endOfString (psz); psz++)
        if (*psz != ' ' && *psz != '\t')
            break;

    return psz;
    }

PSZ getToken (PSZ string)
    {
    PSZ psz;

    psz = string + (*string == '"' ? 1 : 0);
    *endOfToken (string) = '\0';

    return psz;
    }

PSZ parseToken (PSZ pszString)
    {
    PARSESTRUCT *pstrucParse;
    CHAR         c = toupper (*pszString);
    USHORT       value;

    if (c != '\0')
        {
        pszString++;

        /* Suchen des Zeichens in der PARSESTRUCT-Tabelle */
        for (pstrucParse = arstrucParse; pstrucParse->c != '\0'; pstrucParse++)
            {
            if (pstrucParse->c == c)
                {
                switch (pstrucParse->usTyp)
                    {
                    case OPTION_UNSIGNED:
                    case OPTION_FUNSIGNED:
                        value = pstrucParse->usDefault;
                        switch (getUInt (&value, &pszString))
                            {
                            case PARM_DEFAULT:
                                if (pstrucParse->usTyp == OPTION_FUNSIGNED)
                                    printToScreenCR (szErrWrongParameter);
                                break;

                            case PARM_OK:
                                if (value < pstrucParse->usMin || value > pstrucParse->usMax)
                                    {
                                    printToScreenCR (szErrOutOfRange);
                                    value = pstrucParse->usDefault;
                                    }
                                break;

                            default:
                                printToScreenCR (szErrWrongParameter);
                            }
                        *((PUSHORT)pstrucParse->pusDest) = value;
                        goto Exit;

                    case OPTION_SIGNED:
                        value = pstrucParse->usDefault;
                        switch (getInt ((PSHORT)&value, &pszString))
                            {
                            case PARM_OK:
                                if ((SHORT)value < (SHORT)pstrucParse->usMin ||
                                    (SHORT)value > (SHORT)pstrucParse->usMax)
                                    {
                                    printToScreenCR (szErrOutOfRange);
                                    value = pstrucParse->usDefault;
                                    }
                                break;

                            case PARM_ERR:
                                printToScreenCR (szErrWrongParameter);
                                break;
                            }
                        *((PUSHORT)pstrucParse->pusDest) = value;
                        goto Exit;

                    case OPTION_SWITCH:
                        if (*pszString == ' ' || *pszString == '\t' || endOfString (pszString))
                            *((PUSHORT)pstrucParse->pusDest) = pstrucParse->usDefault;
                        else
                            printToScreenCR (szErrWrongParameter);
                        goto Exit;

                    case OPTION_INDEX:
                        value = pstrucParse->usDefault;
                        switch (getUInt (&value, &pszString))
                            {
                            case PARM_OK:
                                if (value < pstrucParse->usMin || value > pstrucParse->usMax)
                                    {
                                    printToScreenCR (szErrOutOfRange);
                                    value = pstrucParse->usDefault;
                                    }
                                break;

                            case PARM_ERR:
                                printToScreenCR (szErrWrongParameter);
                                break;
                            }
                        if (getIValue ((PSWITCHSTRUCT)(pstrucParse->pusDest), value) == FALSE)
                            printToScreenCR (szErrWrongParameter);
                        goto Exit;
                    }
                }
            }
        }

Exit:
    flArgParsed |= pstrucParse->flArgTyp;
    return pszString;
    }

BOOL parse (PSZ pszString)
    {
    pszString = firstToken (pszString);
    while (!endOfString (pszString))
        {
        if (*pszString == '-' || *pszString == '/')
            pszString = parseToken (++pszString);
        else
            return FALSE;
        pszString = nextToken (pszString);
        }

    /* WÑhrend dem Parsen ist der Systemtimer TIMER_MODE_SYS aktiv */
    usThreshold = trimm;
    usTTicks    = (SHORT)(10*trimm/usTickLen % 10) * 4/10;
    usThrBitlen = trimm / usTickLen;

    if (flArgParsed & OPTION_TYP_P)
        switch (ucPortTyp)
            {
            case GAME:
                ucPortTyp = GAMEX;
                break;
            case SER:
                ucPortTyp = SERX;
                break;
            case COM:
                ucPortTyp = COMX;
                break;
            }

    return TRUE;
    }

VOID cini (PSZ argline)
    {
    PSZ    pszToken;
    PSZ    pszFileName;
    CHAR   daten[CCHMAXDATA];
    HFILE  hFile;
    APIRET rc;
    USHORT usSize, usAction;

    /* Optionsdatei bestimmen und îffnen */
    pszToken = firstToken (argline);
    pszToken = nextToken (argline);
    f_strcpy ((PSZ)daten, pszToken);
    pszToken = nextToken (pszToken);
    pszFileName = getToken (daten);

    if (*pszToken != '\0')
        {
        printToScreenCR (szErrTooManyArgs);
        goto _exit;                         // Fehleraussprung: zu viele Argumente
        }

    f_strcpy (szConfigFile, pszFileName);   // Konfigurationsdateinamen merken

    rc = DosOpen (pszFileName, &hFile, &usAction, 0L,
                  FILE_READONLY, FILE_OPEN,
                  OPEN_SHARE_DENYWRITE | OPEN_ACCESS_READONLY, 0L);

    /* Lesen der Parameterdatei */
    if (rc == 0)
        {
        rc = DosRead (hFile, daten, CCHMAXDATA, &usSize);
        if (rc)
            {
            printToScreenCR (szErrRead);
            goto _exit;                     // Fehleraussprung: Fehler beim Lesen der Datei
            }
        daten[usSize >= CCHMAXDATA ? CCHMAXDATA-1 : usSize] = '\0';
        DosClose (hFile);
        }
    else
        {
        printToScreenCR (szErrFileNotFound1);
        printToScreenCR (pszFileName);
        printToScreenCR (szErrFileNotFound2);
        goto _exit;                         // Fehleraussprung: Fehler beim ôffnen der Datei
        }

    /* Parsen der Konfigurationsdaten */
    if (parse (daten) == FALSE)
        {
        printToScreenCR (szErrWrongParameter);
        goto _exit;
        }

#ifdef DEMO
    printDemoMsg ();
#endif

_exit:
    printToScreenCR (NULL);
    f_cini ();
    return;
    }

VOID far f_cini (VOID)
    {
#ifdef DEMO
    fillDataDCF (pDCFDemo);
#else
    fillDataDCF (&DataDCFOrg);
#endif /* DEMO */
    start ();
    getticcnt ();

    return;
    }
