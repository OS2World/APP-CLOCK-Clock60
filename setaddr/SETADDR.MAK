#
#
# Make file for SETADDR
#
#
.SUFFIXES : .EXE .DLL .OBJ .ASM .C .H .HH .RCH \
            .RES .RC                           \
            .HLP .IPF 
.RC.RES   :
  rc -r $*.RC

.C.OBJ    :
  icc /C+ /Ge+ /G3 /Gd- /Gm+ /Gn- /Gs- /N1 /Kb+ /W3 /Se /Ss+ /J+      $*.C

.OBJ.EXE :
  link386 /NOE /NOI /ALIGN:4 /BASE:0X10000  @$*.LNK
  rc $*.RES $*.EXE

ALL: SETADDR.EXE

SETADDR.RES :      SETADDR.RC SETADDR.RCH

SETADDR.OBJ : SETADDR.C \
           SETADDR.H   \
           SETADDR.HH  \
           SETADDR.RCH

SETADDR.EXE : SETADDR.OBJ SETADDR.RES \
      SETADDR.DEF SETADDR.LNK \
      SETADDR.MAK
