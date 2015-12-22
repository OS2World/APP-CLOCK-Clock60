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
  icc $(CDB) /I.. /C+ /Ss+     $*.C

.OBJ.EXE :
  ilink $(LDB) /NOE /NOI /ALIGN:4 /BASE:0X10000 setaddr.obj /PMTYPE:PM
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

cleanall:
      @if exist *.obj del *.obj
      @if exist *.res del *.res
      @if exist *.exe del *.exe

