        .286p
        page    60,80
        title   DCF_DD  -  DCF Device Driver

;/***********************************************************************/
;/*                                                                     */
;/* Driver Name: ZEIT.SYS - DCF77 device driver for external devices    */
;/*              ---------------------------------------------------    */
;/*                                                                     */
;/* Source File Name: TRACE.ASM                                         */
;/*                                                                     */
;/* Descriptive Name: Trace Code Execution Routine                      */
;/*                                                                     */
;/* Function: Dieses Modul dient der Fehlersuche in Treibern mit Hilfe  */
;/*           des TRACEFMT-Utilities von OS/2.                          */
;/*           TraceIt wird gerufen mit dem Minor Code in DX, dem        */
;/*           Zeiger auf den Trace-Puffer in DS:SI und der Pufferl„nge  */
;/*           in CX.                                                    */
;/*                                                                     */
;/*---------------------------------------------------------------------*/
;/* Assemble: keine Optionen                                            */
;/*---------------------------------------------------------------------*/
;/*                                                                     */
;/* Change Log                                                          */
;/*                                                                     */
;/* Version  Date       Comment                                         */
;/* -------  ----       -------                                         */
;/* 1.0      28.12.93   erste Version                                   */
;/* 4.1      11.03.96   erste Version mit HR-Timer-Support              */
;/* 4.2      13.12.98   Korrektur im HR-Timer-Support                   */
;/* 5.0      22.06.00   kleinere Anpassungen (Buildumgebung, Texte)     */
;/* 5.10     28.01.01   Korrektur der Minutenanzeige                    */
;/* 5.20     09.06.01   Fehlerkorrektur Audiodaten                      */
;/* 5.30     16.01.02   Implementierung der Erinnerungsfunktion         */
;/* 5.40     22.11.03   Fehlerkorrektur Erinnerungsfunktion             */
;/*                                                                     */
;/*  Copyright (C) noller & breining software 2003                      */
;/*                                                                     */
;/***********************************************************************/
        .model  small

;-----------------------------------------------------------------------
;         Allgemeine Definitionen
;-----------------------------------------------------------------------
option M510
        include devhlp.inc              ; Device driver header definitions.
        include infoseg.inc             ; InfoSeg-Definitions.
option NOM510

_DATA       segment     word public 'DATA'
        extrn   DeviceHelp:     dword
        extrn   SInfoSeg:       word
_DATA       ends

TRACE_MAJOR   equ   0F5h                ; Major Trace Code

;/***********************************************************************/
;/*       C o d e s e g m e n t                                         */
;/***********************************************************************/
_TEXT       segment     word public 'CODE'
            assume      cs:CGROUP, ds:DGROUP, es:NOTHING

        .386p

;-----  TraceIt: Trace Information in Trace-Buffer schreiben
;       Eingang: DS:SI : Trace Buffer
;                CX    : Trace Buffer L„nge
;                DX    : Trace Minor Code
TraceIt proc    near public uses ax bx es

        cli                                     ; Clear interrupts

                                                ; Calculate position and bit
        mov     ax, 0FH - ((TRACE_MAJOR + 8) and 0FH)
        mov     bx, 2*(TRACE_MAJOR / 16)        ; of TRACE code bit in GDT
        mov     es, [SInfoSeg]                  ; Set ES to GDT segment
        bt      word ptr es:[InfoSegGDT.SIS_mec_table+bx], ax
        jnc     Exit                            ; Leave if bit cleared

        mov     bx, cx                          ; BX : Trace buffer size
        cmp     si, 0                           ; Trace buffer size = 0,
        ja      @f                              ;   if NULL-pointer
        mov     bx, si
@@:     mov     cx, dx                          ; CX : Minor trace code
        mov     ax, TRACE_MAJOR                 ; AX : Major trace code
        mov     dl, DevHlp_RAS                  ; DL : write trace code
        call    [DeviceHelp]

Exit:   sti                                     ; Enable interrupts again
        RET
TraceIt endp

;-----  C-Interface zu TraceIt; Aufrufsyntax:
;       void trace (short minor, char far *buffer, short size)
trace   proc    near C public uses si di, minor:word, buffer:near ptr byte, buffsize:word

        mov     si, buffer
        mov     dx, minor
        mov     cx, buffsize
        call    TraceIt
        RET
trace   endp

_TEXT       ends

DGROUP      group       _DATA
CGROUP      group       _TEXT

            end
