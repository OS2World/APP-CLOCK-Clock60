# makefile
# Erstellt von IBM WorkFrame/2 MakeMake um 23:53:51 am 13 Dez 1998
#
# Diese Make-Datei enth„lt folgende Aktionen:
#  Compile::C++ Compiler
#  Link::Linker

.SUFFIXES:

.SUFFIXES: \
    .c .cpp .obj

.c.obj:
    @echo " Compile::C++ Compiler "
    icc.exe $(CDB) /I..\.. /I.. /I. /Ss /Q /Weffextparprorearetuniuse /Gm /G5 /C %s

{.}.c.obj:
    @echo " Compile::C++ Compiler "
    icc.exe $(CDB) /I..\.. /I.. /I. /Ss /Q /Weffextparprorearetuniuse /Gm /G5 /C %s

{..}.c.obj:
    @echo " Compile::C++ Compiler "
    icc.exe $(CDB) /I..\.. /I.. /I. /Ss /Q /Weffextparprorearetuniuse /Gm /G5 /C %s

.cpp.obj:
    @echo " Compile::C++ Compiler "
    icc.exe $(CDB) /I..\.. /I.. /I. /Ss /Q /Weffextparprorearetuniuse /Gm /G5 /C %s

{.}.cpp.obj:
    @echo " Compile::C++ Compiler "
    icc.exe $(CDB) /I..\.. /I.. /I. /Ss /Q /Weffextparprorearetuniuse /Gm /G5 /C %s

{..}.cpp.obj:
    @echo " Compile::C++ Compiler "
    icc.exe $(CDB) /I..\.. /I.. /I. /Ss /Q /Weffextparprorearetuniuse /Gm /G5 /C %s

.\TimeClnt.exe: \
    .\TimeClnt.obj \
    .\NetBIOS.obj \
    .\ShareMem.obj \
    {$(LIB)}acsnetb.lib \
    {$(LIB)}timeclnt.def
    @echo " Link::Linker "
    icc.exe @<<
     /Q /B" $(LDB) /packd /m"
     /FeTimeClnt.exe
     acsnetb.lib
     timeclnt.def
     .\TimeClnt.obj
     .\NetBIOS.obj
     .\ShareMem.obj
<<

!include "makefile.DEP"

cleanall:
      @if exist *.obj del *.obj
      @if exist *.exe del *.exe

