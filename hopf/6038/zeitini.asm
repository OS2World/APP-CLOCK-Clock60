;**************************************************************************
;                   HOPF6036
;                   Init Module
;                   Copyright NB 1996...2004
;***************************************************************************
.286P

        page    60,80
        title   _DD   DCF Device Driver

      .MODEL  small,os_os2
        INCLUDE  devhlp.inc             ; DEFINITION OF DEVICE HELP CALLS

        EXTRN  _cini:near
        EXTRN  devhlp_ptr:dword
        EXTRN  IniArgOfs :word
        EXTRN  IniArgSeg :word
        page
;        EXTRN  data

;       Request Header

rh          segment at 0
rh_len      db  ?               ; length of request header
rh_unit     db  ?               ; unit CSEG (not used)
rh_command  db  ?               ; command CSEG
rh_status   dw  ?               ; status
rh_rsvd     dw  2 dup(?)        ; reserved for DOS
rh_q_link   dw  2 dup(?)        ; request queue linkage
rh_DSEG     equ $               ; DSEG appropriate to the operation

;       Request Header DSEG for INIT command
            org rh_DSEG-rh
rh_no_units db  ?               ; number of units (not used)
rh_end_addr dd  ?               ; ending address of device driver
rh_bpb      dd  ?               ; address of BPB array (not used)
rh_drive    db  ?

;       Request Header DSEG for IOCTL command
            org rh_DSEG-rh
rh_ioctl_categ db  ?            ; category CSEG
rh_ioctl_funct db  ?            ; function CSEG
rh_ioctl_parm  dd  ?            ; addr of parms (offset and selector)
rh_ioctl_buff  dd  ?            ; addr of buffer (offset and selector)

rh          ends

;
;*******************************************************************************
; _INIDATA segment
;*******************************************************************************

_INIDATA    SEGMENT   PUBLIC  'INIDATA'                                          ;@VDD

dsegend        label      byte
public dsegend

_INIDATA       ends

;*******************************************************************************
; CSEG segment
;*******************************************************************************

_TEXT    SEGMENT  PUBLIC 'CODE'                                           ;@VDD

        page
        assume  cs:_TEXT,ds:_INIDATA,es:rh   ; cs Register -> CSEG .....

        EXTRN    timer_handler:FAR


end_of_driver label byte                 ; last byte of device driver    @VDD
;******************* START OF SPECIFICATIONS ************************
;
; SUBROUTINE NAME:  initialize
;
; DESCRIPTIVE NAME: PDD initialize routine
;
; FUNCTION: This entry point is the TCA PDD's initialization routine.
;           This routine will do the device driver  initialization,
;           (save devhlp address and set up overlay point), as well
;           as register the PDD entry point with the VDD.
;
; ENTRY POINT: initialize
;       LINKAGE: CALL NEAR
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

initialize proc FAR

PUBLIC initialize

;       Initialize device driver - pass back end of driver address

;initialize:

        ASSUME  CS:_TEXT,DS:_INIDATA,ES:RH

STARTADDR:      ; Ab hier Test auf Ver„nderung

        mov     ax, word ptr rh_end_addr[bx] ; Get address for devhlp routines
        mov     word ptr devhlp_ptr,ax       ;   and store in devhlp_ptr
        mov     ax, word ptr rh_end_addr+2[bx]
        mov     word ptr devhlp_ptr+2,ax

        .386P

        push    es
        push    bx
        ;int 3; test
        mov     eax, dword ptr rh_bpb[bx]
        push    eax
        call    _cini               ; init routine in "C"
        add     sp,4
        pop     bx
        pop     es

; timer einh„ngen

        push    es
        push    bx
        mov     ax, offset timer_handler
        mov     bx, 91                        ; Anzahl der Ticks fuer 5 Sekunden eintragen
        mov     dl, DevHlp_TickCount          ; Befehl ins DL - Reg setzten
        call    devhlp_ptr                    ; Dev helper Aufruf

        pop     bx
        pop     es

  ; the next 2 lines set up the address of the end of the real
  ; device driver. the init cseg will be deleted by dos

        mov     word ptr rh_end_addr[bx],offset end_of_driver
        mov     word ptr rh_end_addr+2[bx],offset dsegend

ENDADDR:

        ret

initialize endp

_TEXT    ends
        end
