all: $(AOBJ) zeit.lnk zeit.sys

.suffixes:

.suffixes: .c

LFLAGS = $(LDB) /nod /exepack /packd /a:16 /map
MFLAGS = -aln

zeit.obj:       zeit.asm

zeitini.obj:    zeitini.asm

zeitrout.obj:   zeitrout.c zeit_c.h ..\dcfioctl.h

ini_c.obj:      ini_c.c zeit_c.h

timer.obj:      timer.c ..\dcfioctl.h

common.obj:     common.c

trace.obj:      trace.asm

.asm.obj:
        @echo " Compile::Assembler "
#        alp -Mb -Sme -Od $*.asm -Fl:$*.lst +Lm +Ld
        ml $(MDB_MS) -c -Fl$*.lst -Sa -nologo $*.asm

.c.obj:
        @echo " Compile::Compiler "
        cl $(SWM) $(CDB_MS) -I.. -c -G2 -Zd -Fa$*.lst -Os -nologo $*.c

.\zeit.sys: \
    .\zeit.OBJ \
    .\zeitrout.obj \
    .\zeitini.obj \
    .\ini_c.obj \
    .\trace.obj \
    .\timer.obj \
    .\common.obj \
    .\zeit.def \
    makefile
        @echo " Link::Linker "
        link386 $(LFLAGS) @<<$(@B).lnk
zeit.OBJ +
zeitrout.obj +
zeitini.obj +
ini_c.obj +
trace.obj +
timer.obj +
common.obj
zeit.sys
zeit.map
os2286.lib
zeit.def
<<keep
        mapsym $(MFLAGS) $*.map

cleanall:
      @if exist *.obj del *.obj
      @if exist *.sys del *.sys

