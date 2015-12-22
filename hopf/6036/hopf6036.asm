;************************************************************
;               DCF77 Treiber
;               Version 6.0
;               Copyright  MG 1993...2004
;************************************************************
.286P
        page    60,80
        title   _DD   DCF Device Driver
      .MODEL  small;,OS_OS2
;        INCLUDE  REQPKT.INC     ; DEv Header Definitionen
        INCLUDE  devhlp.inc     ; DEFINITION OF DEVICE HELP CALLS
        INCLUDE  devsym.inc     ; Definitionen Error etc.
        PUBLIC  devhlp_ptr      ; @VDD
        PUBLIC  IniArgSeg       ; öbergabeparmeter
        PUBLIC  IniArgOfs       ; öbergabeparmeter
page


;       Request Header
rh          segment at 0
rh_len      db  ?               ; length of request header
rh_unit     db  ?               ; unit CSEG (not used)
rh_command  db  ?               ; command CSEG
rh_status   dw  ?               ; status
rh_rsvd     dw  2 dup(?)        ; reserved for DOS
rh_q_link   dw  2 dup(?)        ; request queue linkage
rh_ioctl_categ db  ?            ; category CSEG
rh_ioctl_funct db  ?            ; function CSEG
rh_ioctl_parm  dd  ?            ; addr of parms (offset and selector)
rh_ioctl_buff  dd  ?            ; addr of buffer (offset and selector)
rh          ends

CONST           segment dword public 'CONST'
CONST           ends

_BSS            segment dword public 'BSS'
_BSS            ends

_INIDATA        segment dword public 'INIDATA'
_INIDATA        ends

Code            segment dword public 'CODE'
Code            ends

DGROUP          group   CONST, _BSS, _DATA, _INIDATA
StaticGroup     group   Code, _TEXT

public  __acrtused             ; trick to force in startup
        __acrtused = 9876h     ; funny value not easily
                               ; matched in SYMDEB

;*************************************************************
; DSEG segment
;*************************************************************

_DATA    SEGMENT   PUBLIC  'DATA'
;       Device Header

        dd     -1               ; address of next device header
        dw     8880h            ; device attribute
        dw     dev_strategy     ; address of device strategy entry point
        dw     ?                ; reserved
        db     'DCF77$  '       ; name
        dw     4 dup(?)         ; reserved

; TCA Device Driver Work Area

devhlp_ptr      dd         ?    ; device help interface pointer
IniArgSeg       dw         ?    ; öbergabeparmeter
IniArgOfs       dw         ?    ; öbergabeparameter

_DATA            ends

;*************************************************************
; CSEG segment
;*************************************************************

_TEXT    SEGMENT  PUBLIC 'CODE'

        page
        assume  cs:StaticGroup,ds:DGROUP,es:rh   ; cs Register -> CSEG .....

        EXTRN    DosWrite:FAR
        EXTRN    initialize:FAR
        EXTRN    _timer_fctn:near
        EXTRN    _ioctl_fctn:near
;Device Strategy Entry Point

dev_strategy proc far
        ;INT     3
        mov     rh_status[bx],0100h     ; store successful return CSEG and
                                        ;    request complete status in
                                        ;    request header status word
        mov     al,rh_command[bx]       ; route control based on
        cmp     al,10h                  ;      command CSEG
        ja      invalid_command         ; invalid if above generic ioctl
        sal     al,1                    ; set up to
        cbw                             ;   take the
        mov     si,ax                   ;      correct jump into the table
        jmp     short route
        page
function_table label word
        dw      initialize               ; init
        dw      media_check
        dw      build_bpb
        dw      reserved_3
        dw      read
        dw      non_destruc_read
        dw      input_status
        dw      input_flush
        dw      write
        dw      write_verify
        dw      output_status
        dw      output_flush
        dw      reserved_c
        dw      dev_open                 ; open
        dw      dev_close                ; close
        dw      remov_media
        dw      ioctl                    ; generic ioctl
route:
        jmp     function_table[si]
;       Unsupported command CSEGs
write:
read:
        ;int 3
invalid_command:
media_check:
build_bpb:
reserved_3:
non_destruc_read:
input_status:
input_flush:
write_verify:
output_status:
output_flush:
reserved_c:
remov_media:
;ioctl:
        mov     rh_status[bx],8103h     ; store unknown command error and
                                        ;    request complete status in
                                        ;    request header status word
dev_open:
dev_close:
        ret
dev_strategy endp

timer_handler proc far public uses DS
        mov     ax, _DATA               ; Datensegmentzeiger setzen
        mov     DS, ax
        pusha
        call    _timer_fctn
        popa
        ret
timer_handler endp

_outbyte proc near
public _outbyte
        push    BP
        mov     BP,SP
        mov     DX,SS:[BP+4]
        mov     AX,SS:[BP+6]
        out     DX,AL
        pop     BP
        ret
_outbyte endp

_inbyte proc near
public _inbyte
        push    BP
        mov     BP,SP
        mov     DX,SS:[BP+4]
        in      AL,DX
        pop     BP
        jmp     $+2
        ret
_inbyte endp

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
; include devsym.inc ; header datei einziehen wegen Struktur
;_TEXT    SEGMENT  PUBLIC 'CODE'
;          public    IOCTL
        .386P
         stdin   equ  0
         stdout  equ  1
         stderr  equ  2

IOCTL     proc      far uses SI DI FS BX
        assume  cs:StaticGroup,ds:DGROUP,es:rh   ; cs Register -> CSEG .....
;-----    Kategorie ok?
          NOP ; int 3
          MOV       rh_status[bx], 0
;-----    Fill Port Funct
          PUSH      BX
          PUSH      ES
          MOV       AL, rh_ioctl_funct[bx]
          MOV       AH, rh_ioctl_categ[bx]
          PUSH      AX
          MOV       EAX, dword ptr rh_ioctl_buff[bx]
                                        ; far pointer in ES:DI
          PUSH      EAX                 ; Addresse auf Stack
          CALL      _ioctl_fctn
         ; lade Daten in Buffer
          ADD       SP,6
          CLC                           ; Clear CARRY = kein Fehler
@@:       POP       ES
          POP       BX
          MOV       rh_status[bx], AX
          OR        rh_status[bx], STDON
          ret
IOCTL     endp



;-----          PrÅft, ob ein Parameter/Daten-Segment fÅr einen IOCtl existiert
;         Eingang: 1. Parameter: FAR PTR: Zeiger auf Segment,
;                  2. Parameter: USHORT : MindestsegmentlÑnge
;         Ausgang: Ret-Value  : 1 = Fehler, 0 = OK
_TestSeg  proc public

          push      BP
          mov       BP, SP
          mov       CX, SS:[BP+8]
          mov       EAX, SS:[BP+4]
          mov       DX, 0

          ADD       CX, AX
          SHR       EAX, 16
          VERR      AX
          JNZ       Error
          LSL       AX, AX
          CMP       AX, CX
          JB        Error
          MOV       AX, 0
          JMP       short NoError

Error:    MOV       AX, 1
NoError:  pop       BP
          ret
_TestSeg  endp

_INT_3    proc public
          int       3
          ret
_INT_3    endp

_TEXT     ends

          end dev_strategy
