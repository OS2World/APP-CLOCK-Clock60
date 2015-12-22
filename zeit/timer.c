/***************************************************************************\
 *
 * PROGRAMMNAME: ZEIT
 * -------------
 *
 * VERSION: 5.4
 * --------
 *
 * MODULNAME: TIMER
 * ----------
 *
 * BESCHREIBUNG: Timermodul fÅr HR-Timer
 * -------------
 *
 * HINWEISE:
 * ---------
 *
 * FUNKTIONEN:
 * -----------
 *   getticcnt
 *   attachTimer
 *   registerTimer
 *
 *  Ver.    Date      Comment
 *  ----    --------  -------
 *  4.1     11.03.96   erste Version mit HR-Timer-Support
 *  4.2     13.12.98   Korrektur im HR-Timer-Support
 *  5.0     22.06.00   Neuzusammenstellung der Version
 *  5.10    28.01.01   Korrektur der Minutenanzeige
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
#include <infoseg.h>
#include <dhcalls.h>
#include "zeit_c.h"
#include "tmr0_idc.h"
#include "dcfioctl.h"
#include "trace.h"

/* globales Info-Segment */
extern struct InfoSegGDT far *pInfoSeg;

/* Timer-Variable */
extern USHORT usTimerMode;                      // verwendeter Timer-Typ
extern unsigned int ticcount;
extern char initflag;
extern USHORT usTickLen;                        // TicklÑnge des Timers in ms
extern SHORT  usTTicks;                         // Zehntel-Ticks fÅr Sys-Timer
extern BOOL   bIsHRTimerAvail;                  // HR-Timer verfÅgbar

/* Schwellwerte fÅr Biterkennung */
extern USHORT usThrCarrier;                     // Erkennung TrÑgerausfall; 3.2s
extern USHORT usThrPolarity;                    // Erkennung PolaritÑt;     1.5s
extern USHORT usThrMinute;                      // Erkennung Minutenimpuls; 1.5s
extern USHORT usThrBitlen;                      // Erkennung BitpolaritÑt;  variabel

TIMER0_ATTACH_DD DDTable;
PTMRFN           ptmrfn;

/* Bestimmen der Zahl der SchleifendurchlÑufe wÑhrend */
/* eines Systemtimer-Ticks (32ms)                     */
void getticcnt (void)
    {
    USHORT port = 0x201;                        // willkÅrlicher Port; 201 (Game) ist unkritisch

    initflag = 0;
    while (initflag == 0);                      // warten bis zum nÑchsten Tick

    while (initflag == 1)                       // einen vollstÑndigen Tick ausmessen
        {
        ticcount++;
        getbit(port);
        }

    return;
    }

void attachTimer (void)
    {
    bIsHRTimerAvail = FALSE;

    /* IDC-Entry von TIMER0$ suchen */
    DDTable.pfn = NULL;
    ptmrfn      = NULL;
    if (DevHelp_AttachDD ((NPSZ)"TIMER0$ ", (NPBYTE)&DDTable))
        return;                                 // TIMER0.SYS nicht geladen

    if (!DDTable.pfn)
        return;                                 // Fehler in TIMER0.SYS

    ptmrfn = DDTable.pfn;
    bIsHRTimerAvail = TRUE;
    return;
    }

BOOL registerTimer (USHORT newTimerMode, USHORT tickLen)
    {
    switch (newTimerMode)
        {
        case TIMER_MODE_SYS:
        case TIMER_MODE_SYSI:
            // Timermodus unterstÅtzt nur System-TicklÑnge
            tickLen = pInfoSeg->SIS_MinSlice;

            /* System-Timer registrieren */
            if (DevHelp_SetTimer ((NPFN)timerHandler) != 0)
                return FALSE;

            usThrCarrier  = THR_SYS_CARRIER;
            usThrPolarity = THR_SYS_POLARITY;
            usThrMinute   = THR_SYS_MINUTE;
            usThrBitlen   = THR_SYS_BITLEN;

            /* HR-Timer deregistrieren */
            if (ptmrfn != NULL && usTimerMode == TIMER_MODE_HR)
                ptmrfn (TMR0_DEREG, (ULONG)iTimerHandler, 0L);
            break;

        case TIMER_MODE_HR:
            // vorlÑufig nur Default-TicklÑnge unterstÅtzt
            tickLen = THR_TICKLEN;

            /* HR-Timer registrieren */
            if (ptmrfn == NULL)
                return FALSE;
            if (ptmrfn (TMR0_REG, (ULONG)iTimerHandler, (ULONG)tickLen) != 0)
                return FALSE;

            usThrCarrier  = THR_HR_CARRIER;
            usThrPolarity = THR_HR_POLARITY;
            usThrMinute   = THR_HR_MINUTE;
            usThrBitlen   = THR_HR_BITLEN;

            /* System-Timer deregistrieren */
            if (usTimerMode == TIMER_MODE_SYS || usTimerMode == TIMER_MODE_SYSI)
                DevHelp_ResetTimer ((NPFN)timerHandler);
            break;

        default:
            return FALSE;
        }

    usTickLen   = tickLen;
    usTimerMode = newTimerMode;
    return TRUE;
    }

