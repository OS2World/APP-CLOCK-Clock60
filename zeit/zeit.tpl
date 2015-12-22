;***************************************************************************
;*
;*            Physical Device Driver for external DCF 77 Receiver
;*                 Copyright (c) noller & breining software
;*
;***************************************************************************
PHYSICAL DEVICE
CODE    PRELOAD
DATA    PRELOAD

DESCRIPTION    '@#NB:X.XX.XX#@ Driver for DCF 77 Receiver (c) 2001'

SEGMENTS
 _DATA         CLASS 'DATA'
 _BSS          CLASS 'BSS'
 C_COMMON      CLASS 'BSS'
 CONST         CLASS 'CONST'
 _INIDATA      CLASS 'INIDATA'
 _TEXT         CLASS 'CODE' IOPL
 _INITEXT      CLASS 'CODE' IOPL

STACKSIZE      2048

