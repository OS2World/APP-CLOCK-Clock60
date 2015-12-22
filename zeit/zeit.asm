        .286p
        page    60,80
        title   DCF_DD  -  DCF Device Driver

;/***********************************************************************/
;/*                                                                     */
;/* Driver Name: ZEIT.SYS - DCF77 device driver for external devices    */
;/*              ---------------------------------------------------    */
;/*                                                                     */
;/* Source File Name: ZEIT.ASM                                          */
;/*                                                                     */
;/* Descriptive Name: Device Driver fÅr externe DCF77 Empfangsmodule    */
;/*                                                                     */
;/* Function: - Strategieroutine mit Funktionsdispatcher                */
;/*           - Assembler-Unterprogramme                                */
;/*                                                                     */
;/*---------------------------------------------------------------------*/
;/* Options:                                                            */
;/*                                                                     */
;/*---------------------------------------------------------------------*/
;/* Assemble: keine Optionen                                            */
;/*---------------------------------------------------------------------*/
;/*                                                                     */
;/* Change Log                                                          */
;/*                                                                     */
;/* Version  Date       Comment                                         */
;/* -------  ----       -------                                         */
;/* 4.10     11.03.96   erste Version mit HR-Timer-Support              */
;/* 4.20     13.12.98   Korrektur im HR-Timer-Support                   */
;/* 5.00     22.06.00   kleinere Anpassungen (Buildumgebung, Texte)     */
;/* 5.10     28.01.01   Korrektur der Minutenanzeige                    */
;/* 5.20     09.06.01   Fehlerkorrektur Audiodaten                      */
;/* 5.30     16.01.02   Implementierung der Erinnerungsfunktion         */
;/* 5.40     22.11.03   Fehlerkorrektur Erinnerungsfunktion             */
;/*                                                                     */
;/*  Copyright (C) noller & breining software 2003                      */
;/*                                                                     */
;/***********************************************************************/
        .model  small

option M510

        include reqpkt.inc              ; Definitionen Dev Header
        include devhlp.inc              ; Definitionen device help calls
        include devsym.inc              ; Definitionen Error etc.
        include zeit_c.inc

option NOM510

;-----  Request Header
rh          segment     at 0
rh_len          db      ?               ; length of request header
rh_unit         db      ?               ; unit CSEG (not used)
rh_command      db      ?               ; command CSEG
rh_status       dw      ?               ; status
rh_rsvd         dw      2 dup (?)       ; reserved for DOS
rh_q_link       dw      2 dup (?)       ; request queue linkage
rh_ioctl_categ  db      ?               ; category CSEG
rh_ioctl_funct  db      ?               ; function CSEG
rh_ioctl_parm   dd      ?               ; addr of parms (offset and selector)
rh_ioctl_buff   dd      ?               ; addr of buffer (offset and selector)
rh          ends

_BSS        segment     word public 'BSS'
_BSS        ends

c_common    segment     word public 'BSS'
c_common    ends

_INITEXT    segment     word public 'CODE'
_INITEXT    ends

public  __acrtused                      ; trick to force in startup
        __acrtused = 9876h              ; funny value not easily
                                        ; matched in SYMDEB

;*************************************************************
; CONST segment
;*************************************************************
CONST       segment     word public 'CONST'

function_table  label   word
                dw      offset initialize               ;  0
                dw      offset bad_command              ;  1: media_check
                dw      offset bad_command              ;  2: build_bpb
                dw      offset bad_command              ;  3
                dw      offset bad_command              ;  4: read
                dw      offset bad_command              ;  5: non_destr_read
                dw      offset bad_command              ;  6: input_status
                dw      offset bad_command              ;  7: input_flush
                dw      offset bad_command              ;  8: write
                dw      offset bad_command              ;  9: write_verify
                dw      offset bad_command              ;  A: output_status
                dw      offset bad_command              ;  B: output_flush
                dw      offset bad_command              ;  C
                dw      offset dev_open                 ;  D
                dw      offset dev_close                ;  E
                dw      offset bad_command              ;  F: remov_media
                dw      offset ioctl                    ; 10
                dw      offset bad_command              ; 11: reset_media
                dw      offset bad_command              ; 12: get_logical_drv_map
                dw      offset bad_command              ; 13: set_logical_drv_map
                dw      offset bad_command              ; 14: deinstall
                dw      offset bad_command              ; 15
                dw      offset bad_command              ; 16: partitionable
                dw      offset bad_command              ; 17: get_hd_map
                dw      offset bad_command              ; 18
                dw      offset bad_command              ; 19
                dw      offset bad_command              ; 1A
                dw      offset bad_command              ; 1B
                dw      offset bad_command              ; 1C: shutdown
                dw      offset bad_command              ; 1D: get_drv_caps
                dw      offset bad_command              ; 1E
                dw      offset init_cpl                 ; 1F

MCW             equ     ($-function_table)/2-1          ; hîchster zulÑssiger Befehlscode

CONST       ends

;*************************************************************
; DSEG segment
;*************************************************************
_DATA       segment     word public 'DATA'

        public  DeviceHelp
        public  IniArgSeg               ; öbergabeparameter
        public  IniArgOfs               ; öbergabeparameter
        public  _usTimerMode

        extern  _usTickLen: word

;-----  Device Header
DDHd    SysDev  <-1,                \   ; Zeiger auf nÑchsten Device Header
                 DEV_CHAR_DEV +     \   ; Device Attribute
                 DEV_30 +           \
                 DEVLEV_3,          \
                 offset strategy,,  \   ; Strategie-Routine
                 'DCF77$'>              ; Device Name
DevCaps dword   DEV_16MB +          \   ; Char-DD mit voller Addressierbarkeit
                DEV_INITCOMPLETE        ; InitComplete Request-Paket wird unterstÅtzt

DeviceHelp      dword   ?               ; device help interface pointer
IniArgSeg       word    ?               ; öbergabeparmeter
IniArgOfs       word    ?               ; öbergabeparameter

InitComplete    word    0               ; 1 = InitComplete beendet
_usTimerMode    word    TIMER_MODE_UNK  ; Modus, siehe TIMER_MODE_x

pBuffLvl        label   dword           ; Bit-Level-Puffer
pBuffLvlL       word    0
pBuffLvlH       word    0
hLockLvl        label   dword           ; Lock handle fÅr Bit-Level-Puffer
hLockLvlL       word    0
hLockLvlH       word    0
uscLvl          word    0               ; FÅllstand fÅr Bit-Level-Puffer
usMaxLvl        word    0               ; Puffergrî·e des Bit-Level-Puffers


_DATA       ends

_INIDATA    segment     word public 'INIDATA'
_INIDATA    ends

;*************************************************************
; CSEG segment
;*************************************************************

_TEXT       segment     word public 'CODE'
            assume      cs:CGROUP, ds:DGROUP, es:rh

        extrn   initialize:     far
        extrn   _registerTimer: near
        extrn   _ioctl:         near
        extrn   _timerHandler:  far

;-----  Device Strategy Entry Point
strategy proc   far
;       INT     3
        mov     rh_status[bx], 0100h    ; store successful return CSEG and
                                        ;   request complete status in
                                        ;   request header status word

;-----  Test, ob der High-Resolution-Timer registriert werden soll
        cmp     _usTimerMode, TIMER_MODE_SYSI
        jne     strat1
        cmp     InitComplete, 0         ; Timer0 darf erst nach InitComplete
        je      strat1                  ;   registriert werden
        push    es
        push    bx
        push    0
        push    TIMER_MODE_HR
        call    _registerTimer
        add     sp, 4
        or      ax, ax
        jnz     @f
        mov     _usTimerMode, TIMER_MODE_SYS
@@:     pop     bx
        pop     es

strat1: mov     al, rh_command[bx]      ; route control based on
        cmp     al, MCW                 ;   command CSEG
        ja      bad_command             ; zu hoher Funktions-Code
        sal     al, 1                   ; set up to
        cbw                             ;   take the
        mov     si, ax                  ;   correct jump into the table
        jmp     function_table[si]

;-----  ungÅltige Command-Codes
bad_command::
        mov     rh_status[bx], 8103h    ; store unknown command error and
                                        ;    request complete status in
                                        ;    request header status word
dev_open::
dev_close::
        ret
strategy endp

outbyte proc    near C public, port:word, value:word
        mov     dx, port
        mov     ax, value
        out     dx, al
        RET
outbyte endp

inbyte  proc    near C public, port:word
        mov     dx, port
        in      al, dx
        RET
inbyte  endp

f_strlen proc   far C public uses di es, source: far ptr byte

        cld

        mov     al, 0
        mov     cx, 0ffffh
        les     di, source
 repne  scasb
        not     cx                                     ; StringlÑnge ohne Terminator '\0'
        dec     cx

        mov     ax, cx
        RET
f_strlen endp

f_strcpy proc   far C public uses si di es ds, dest:far ptr byte, source: far ptr byte

        les     di, dest
        lds     si, source
lp:     lodsb
        stosb
        or      al, al
        jnz     lp

        mov     dx, es
        mov     ax, word ptr dest
        RET
f_strcpy endp

f_memcpy proc   far C public uses si di es ds, dest:far ptr byte, source: far ptr byte, count: word

        mov     cx, count
        les     di, dest
        lds     si, source
 rep    movsb

        mov     dx, es
        mov     ax, word ptr dest
        RET
f_memcpy endp

iTimerHandler proc far C public
        sti
        call    _timerHandler
        RET
iTimerHandler endp

DevHelp_SetTimer proc  far Pascal public uses si di, TimerHandler:word

        mov     ax, TimerHandler                ; Offset Timer-Handler
        mov     dl, DevHlp_SetTimer
        call    DeviceHelp
        mov     ax, 0
        jnc     Exit
        mov     ax, 1
Exit:   RET
DevHelp_SetTimer endp

DevHelp_ResetTimer proc far Pascal public uses si di, TimerHandler:word

        mov     ax, TimerHandler
        mov     dl, DevHlp_ResetTimer
        call    DeviceHelp
        RET
DevHelp_ResetTimer endp

DevHelp_AttachDD proc   far Pascal public uses si di, DDName:word, IDCTable:word

        mov     bx, DDName                      ; Device-Driver Name
        mov     di, IDCTable                    ; Ergebnis-Struktur
        mov     dl, DevHlp_AttachDD
        call    DeviceHelp
        mov     ax, 0
        jnc     Exit
        mov     ax, 1
Exit:   RET
DevHelp_AttachDD endp

LvlIsReady proc near C public

        mov     ax, 1
        cmp     usMaxLvl, 0
        je      Exit
        mov     ax, 0

Exit:   RET
LvlIsReady endp

        .386p
LvlWait proc    near C public uses si di, pBuffer:far ptr byte, uscBuffer:word

;-----  IOCTL-Datenbereich locken
        mov     ax, word ptr pBuffer[2]         ; ax: Puffer-Segment
        mov     bx, 0100H                       ; longterm Lock; blockieren, bis verfÅgbar
        mov     dl, DevHlp_Lock
        call    DeviceHelp
        jc      Error                           ; Fehler: Segment kann nicht gelockt werden
        mov     hLockLvlL, bx                   ; Lock handle sichern
        mov     hLockLvlH, ax

;-----  IOCTL-Puffer in physikalische Adresse konvertieren
        push    ds
        mov     si, ds
        mov     es, si
        lds     si, pBuffer                     ; virtuelle Adresse des Puffers im Strategie-Kontext
        mov     dl, DevHlp_VirtToPhys
        call    es:DeviceHelp
        pop     ds
        jc      Exit                            ; Fehler: gelocktes Segment kann nicht konvertiert werden

        mov     pBuffLvlL, bx                   ; Zeiger auf Puffer initialisieren
        mov     pBuffLvlH, ax
        mov     uscLvl, 0                       ; FÅllgrad-ZÑhler initialisieren
        mov     ax, uscBuffer
        mov     usMaxLvl, ax                    ; Puffergrî·e kopieren

WaitLp: cli                                     ; Wakeup-Test mit maskierten Interrupts!
        mov     ax, usMaxLvl                    ; usMaxLvl ist nach einem Block 0
        cmp     uscLvl, ax
        jae     Exit                            ; Wakeup-Test
        mov     bx, pBuffLvlL                   ; Event_id
        mov     ax, pBuffLvlH
        mov     cx, 0FFFFH                      ; Timeout
        mov     di, cx
        mov     dx, DevHlp_ProcBlock            ; ProcBlock, unterbrechbar
        call    DeviceHelp
        jnc     WaitLp

Exit:   sti
        mov     bx, hLockLvlL                   ; Lock auf Puffer wieder entfernen
        mov     ax, hLockLvlH
        mov     dl, DevHlp_Unlock
        call    DeviceHelp

Error:  mov     usMaxLvl, 0
        mov     ax, uscLvl

        RET
LvlWait endp


;********************************************************
;  SUBROUTINE NAME:  LvlInsert
;
;  DESCRIPTIVE NAME:  EinfÅgen eines Byte in Puffer
;
;  FUNCTION:  FÅgt ein Byte in den Puffer fÅr die
;             Darstellung der EmpfangsqualitÑt ein.
;             LvlWait wird wieder freigegeben, sobald
;             der Puffer voll ist.
;
;  NOTES:
;
;  INPUT:  Stack: einzufÅgenden Byte
;*******************************************************
LvlInsert proc  near C public uses si di, chr:byte

        mov     cx, usMaxLvl                    ; cx wird auch als Segment-LÑnge fÅr PhysToVirt benutzt
        or      cx, cx
        jz      Exit

        cli
        mov     ax, pBuffLvlH                   ; physikalische Adresse
        mov     bx, pBuffLvlL
        mov     dx, 0100H + DevHlp_PhysToVirt   ; Ergebnis in ES:DI
        call    DeviceHelp
        jc      Error

        add     di, uscLvl
        mov     al, chr
        mov     es:[di], al
        sti

        inc     uscLvl
        mov     ax, usMaxLvl
        cmp     uscLvl, ax
        jb      Exit

Error:  sti
        mov     bx, pBuffLvlL                   ; Event_id
        mov     ax, pBuffLvlH
        mov     dl, DevHlp_ProcRun
        call    DeviceHelp

        mov     usMaxLvl, 0
Exit:   RET
LvlInsert endp


;********************************************************
;  SUBROUTINE NAME:  LvlComIns
;
;  DESCRIPTIVE NAME:  EinfÅgen in Puffer
;
;  FUNCTION:  FÅgt Bytes in den Puffer fÅr die
;             Darstellung der EmpfangsqualitÑt ein.
;             LvlWait wird wieder freigegeben, sobald
;             der Puffer voll ist. Diese Routine wird
;             fÅr die Ausgabe von getcom benîtigt
;             chr = 0FFH:  EinfÅgen von 1 Byte "1"
;             chr = sonst: EinfÅgen von 8 Byte "0"/"1",
;                          je nach Anzahl von 0/1-Bits
;                          in chr; MSB zuerst
;
;  NOTES:
;
;  INPUT:  Stack: einzufÅgenden Byte
;*******************************************************
LvlComIns proc near C public uses si di, chr:byte

        cmp     chr, 0FFH
        jne     @f
        push    1                               ; Byte war FF -> 1x1 einfÅgen
        call    LvlInsert
        add     sp, 2
        jmp     short Exit

@@:     mov     cx, 7                           ; Byte != FF
        mov     dl, chr
_Loop:  xor     ax, ax                          ; Bit == 0 -> AX = 0
        bt      dx, cx                          ; Bit testen; Bit 7 zuerst, absteigend
        jnc     @f
        inc     ax                              ; Bit == 1 -> AX = 1
@@:     push    dx
        push    cx
        push    ax                              ; öbergabewert 0/1
        call    LvlInsert                       ; 2x einfÅgen, da 1 bit = 20ms
        call    LvlInsert                       ;    HR-Timer-Intervall = 10ms
        add     sp, 2
        pop     cx
        pop     dx
        dec     cx                              ; 8 DurchlÑufe
        jge     _Loop

        push    0                               ; 20 ms fehlen wegen Startbit
        call    LvlInsert
        call    LvlInsert
        add     sp, 2

Exit:   RET
LvlComIns endp

;********************************************************
;  SUBROUTINE NAME:  IOCTL
;
;  DESCRIPTIVE NAME:  IOCtl-Routine
;
;  FUNCTION:  Bearbeitet IOCtl-Requests
;
;  NOTES:  Fragt Zeit, Datum, etc.
;
;  INPUT:  ES:BX : Zeiger auf request packet
;*******************************************************
ioctl   proc    far uses si di

        mov     rh_status[bx], 0

;-----  Fill Port Funct
        push    bx
        push    es
        mov     al, rh_ioctl_funct[bx]
        mov     ah, rh_ioctl_categ[bx]
        push    ax
        push    dword ptr rh_ioctl_buff[bx]
        push    dword ptr rh_ioctl_parm[bx]
        call    _ioctl              ; lade Daten in Buffer
        add     sp, 10
        clc                         ; Clear CARRY = kein Fehler
@@:     pop     es
        pop     bx
        mov     rh_status[bx], ax
        or      rh_status[bx], STDON
        RET
ioctl   endp

;********************************************************
;  SUBROUTINE NAME:  init_cpl
;
;  DESCRIPTIVE NAME:  InitComplete-Routine
;
;  FUNCTION:  Setzt ein Flag, das dem Treiber
;             den InitComplete-Zustand anzeigt
;
;  NOTES:
;
;  INPUT:  ES:BX : Zeiger auf request packet
;*******************************************************
init_cpl proc   far public

        mov     InitComplete, 1
        ret
init_cpl endp


;-----  PrÅft, ob ein Parameter/Daten-Segment fÅr einen IOCtl existiert
;       Eingang: 1. Parameter: FAR PTR: Zeiger auf Segment,
;                2. Parameter: USHORT : MindestsegmentlÑnge
;       Ausgang: Ret-Value: Bool: 0 = Fehler, 1 = OK
TestSeg proc    near C public, pSeg:dword, segLen:word

        mov     cx, segLen
        mov     ebx, pSeg
        mov     dx, 0

        add     cx, bx
        shr     ebx, 16
        verr    bx
        mov     ax, 0
        jnz     Exit                            ; keine Zugriffsrechte fÅr Segment
        lsl     bx, bx
        cmp     bx, cx                          ; CY=1: Segment zu klein
        cmc
        rcl     ax, 1

Exit:   RET
TestSeg endp

        .286P

_TEXT       ends

DGROUP      group       CONST, _BSS, c_common, _DATA, _INIDATA
CGROUP      group       _TEXT, _INITEXT

            end         strategy
