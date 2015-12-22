        .286p
        page    60,80
        title   DCF_DD  -  DCF Device Driver

;/***********************************************************************/
;/*                                                                     */
;/* Driver Name: ZEIT.SYS - DCF77 device driver for external devices    */
;/*              ---------------------------------------------------    */
;/*                                                                     */
;/* Source File Name: ZEITINI.ASM                                       */
;/*                                                                     */
;/* Descriptive Name: Device Driver fÅr externe DCF77 Empfangsmodule    */
;/*                                                                     */
;/* Function: - Assemblerroutinen fÅr Treiberinitialisierung            */
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

        include devhlp.inc                              ; Definition der Device help calls
        include zeit_c.inc

        public  SInfoSeg
        public  _pInfoSeg
        extrn   _cini:          near
        extrn   _attachTimer:   near
        extrn   _registerTimer: near
        extrn   DeviceHelp:     dword
        extrn   IniArgOfs:      word
        extrn   IniArgSeg:      word

PCHAR       typedef     far ptr byte                    ; typedef fÅr C-Module

CR          equ         <0DH>                           ; Carriage return
LF          equ         <0AH>                           ; Linefeed
CRLF        catstr      CR,<,>,LF
CRLF0       catstr      CRLF,<,0>

;-----  Request Header
rh          segment     at 0
rh_len          db      ?                               ; length of request header
rh_unit         db      ?                               ; unit CSEG (not used)
rh_command      db      ?                               ; command CSEG
rh_status       dw      ?                               ; status
rh_rsvd         dw      2 dup(?)                        ; reserved for DOS
rh_q_link       dw      2 dup(?)                        ; request queue linkage
rh_DSEG         equ     $                               ; DSEG appropriate to the operation

;-----  Request Header DSEG for INIT command
                org     rh_DSEG-rh
rh_no_units     db      ?                               ; number of units (not used)
rh_end_addr     dd      ?                               ; ending address of device driver
rh_bpb          dd      ?                               ; Addresse der Kommandozeile
rh_drive        db      ?

;-----  Request Header DSEG for IOCTL command
                org     rh_DSEG-rh
rh_ioctl_categ  db      ?                               ; category CSEG
rh_ioctl_funct  db      ?                               ; function CSEG
rh_ioctl_parm   dd      ?                               ; addr of parms (offset and selector)
rh_ioctl_buff   dd      ?                               ; addr of buffer (offset and selector)

rh          ends

;*******************************************************************************
; _DATA segment
;*******************************************************************************
_DATA       segment     word public 'DATA'
_pInfoSeg       label   dword                           ; far-Zeiger auf globales Infoseg
OInfoSeg        word    ?                               ; Offset:  Infoseg
SInfoSeg        word    ?                               ; Segment: Infoseg
_DATA       ends

;*******************************************************************************
; _INIDATA segment
;*******************************************************************************
_INIDATA    segment     word public 'INIDATA'

dsegend     label       byte                            ; Ende des Datensegments
            public      dsegend
            public      _szErrTooManyArgs
            public      _szErrRead
            public      _szErrFileNotFound1
            public      _szErrFileNotFound2
            public      _szErrOutOfRange
            public      _szErrWrongParameter
            public      _szCopyright
            public      _usIsQuiet

;-----  Fehlertexte
szErrTooManyArgs        byte    "Zu viele Argumente in Parameterzeile", CRLF0
szErrRead               byte    "Fehler beim Lesen der Konfigurationsdatei", CRLF0
szErrFileNotFound1      byte    "Konfigurationsdatei "
szErrFileNotFound2      byte    " nicht gefunden", CRLF0
szErrOutOfRange         byte    "falscher Parameterwert", CRLF0
szErrWrongParameter     byte    "fehlerhafter Parameter", CRLF0
szCopyright             byte    "DCF77 - Treiber fÅr OS/2 Ver.5.1; (C) noller & breining software, 1996...2001", CRLF0

_szErrTooManyArgs       PCHAR   szErrTooManyArgs
_szErrRead              PCHAR   szErrRead
_szErrFileNotFound1     PCHAR   szErrFileNotFound1
_szErrFileNotFound2     PCHAR   szErrFileNotFound2
_szErrOutOfRange        PCHAR   szErrOutOfRange
_szErrWrongParameter    PCHAR   szErrWrongParameter
_szCopyright            PCHAR   szCopyright

_usIsQuiet              word    0

_INIDATA    ends

;*******************************************************************************
; CSEG segment
;*******************************************************************************
_INITEXT    segment     word public 'CODE'
            assume      cs:CGROUP, ds:DGROUP, es:rh

csegend     label       byte                            ; Ende des Codesegments
            public      csegend

;******************* START OF SPECIFICATIONS ************************
;
; SUBROUTINE NAME:  initialize
;
; DESCRIPTIVE NAME: PDD initialize routine
;
; FUNCTION: This entry point is the PDD's initialization routine.
;           This routine will do the device driver initialization,
;           (save devhlp address and set up overlay point)
;
; ENTRY POINT: initialize
;       LINKAGE: CALL FAR
;
; INPUT: NONE
;
; EXIT-NORMAL: EAX = NONE
;
; EXIT-ERROR:  EAX = NONE
;
; INTERNAL REFERENCES:  NONE
;
; EXTERNAL REFERENCES:  NONE
;
;********************************************************************
initialize proc far public

        mov     ax, word ptr rh_end_addr[bx]            ; Get address for devhlp routines
        mov     word ptr DeviceHelp, ax                 ;   and store in DeviceHelp
        mov     ax, word ptr rh_end_addr+2[bx]
        mov     word ptr DeviceHelp+2, ax

;-----  Timer einhÑngen
        push    es
        push    bx

        mov     al, DHGETDOSV_SYSINFOSEG                ; VAR index = System InfoSeg
        mov     dl, DevHlp_GetDOSVar                    ; DevHlp Funktion
        call    DeviceHelp
        mov     es, ax                                  ; es:bx = InfoSeg Adresse
        mov     ax, es:[bx]                             ; Segment des InfoSeg holen
        mov     SInfoSeg, ax
        mov     OInfoSeg, 0

        call    _attachTimer
        push    0
        push    TIMER_MODE_SYSI                         ; vorlÑufig System-Timer registrieren
        call    _registerTimer                          ;   nach InitComplete kann der korrekte
        add     sp, 4
        pop     bx                                      ;   Timer bestimmt werden
        pop     es

        .386P
        push    es
        push    bx
        mov     eax, dword ptr rh_bpb[bx]
        push    eax
        call    _cini                                   ; Kommandozeilenparser
        add     sp, 4
        pop     bx
        pop     es
        .286P

;-----  the next 2 lines set up the address of the end of the real
;       device driver. the init cseg will be deleted by dos
        mov     word ptr rh_end_addr[bx],   offset csegend
        mov     word ptr rh_end_addr[bx+2], offset dsegend

        ret
initialize endp

_INITEXT    ends

DGROUP      group       _DATA, _INIDATA
CGROUP      group       _INITEXT

            end
