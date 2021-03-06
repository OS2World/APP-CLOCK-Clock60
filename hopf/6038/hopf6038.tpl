;***************************************************************************
;*
;*                      Physical Device Driver Sample
;*                 Copyright (c) IBM Corporation   1992
;*                           All Rights Reserved
;*
;***************************************************************************
PHYSICAL DEVICE
CODE    PRELOAD
DATA    PRELOAD

DESCRIPTION   '@#NB:X.XX.XX#@ Driver for hopf DCF 77 Receiver 6038 (c) 2001'

SEGMENTS
 _DATA          CLASS 'DATA'
 CONST          CLASS 'CONST'
 _BSS           CLASS 'BSS'
 _INIDATA       CLASS 'INIDATA'
 'Code'         CLASS 'CODE' IOPL
 _TEXT          CLASS 'CODE' IOPL

;
;pdd uses no stack but
;it keeps the linker from complaining
;
STACKSIZE   2048
