# makefile
# Erstellt von IBM WorkFrame/2 MakeMake um 0:12:42 am 14 Dez 1998
#
# Diese Make-Datei enth„lt folgende Aktionen:
#  Compile::C++ Compiler
#  Link::Linker

.SUFFIXES:

.SUFFIXES: \
    .cpp .obj

.cpp.obj:
    @echo " Compile::C++ Compiler "
    icc.exe $(CDB) /I..\.. /I.. /I. /Ss /Q /Weffextparprorearetuniuse /Gm /G5 /C %s

{..\timeserv}.cpp.obj:
    @echo " Compile::C++ Compiler "
    icc.exe $(CDB) /I..\.. /I.. /I. /Ss /Q /Weffextparprorearetuniuse /Gm /G5 /C %s

{..}.cpp.obj:
    @echo " Compile::C++ Compiler "
    icc.exe $(CDB) /I..\.. /I.. /I. /Ss /Q /Weffextparprorearetuniuse /Gm /G5 /C %s

.\TimeServ.exe: \
    .\ShareMem.obj \
    .\TimeServ.obj \
    .\DCF77.obj \
    .\NetBIOS.obj \
    {$(LIB)}acsnetb.lib \
    {$(LIB)}timeserv.def
    @echo " Link::Linker "
    icc.exe @<<
     /Q /B" $(LDB) /packd /m"
     /FeTimeServ.exe
     acsnetb.lib
     timeserv.def
     .\ShareMem.obj
     .\TimeServ.obj
     .\DCF77.obj
     .\NetBIOS.obj
<<

!include "makefile.DEP"

cleanall:
      @if exist *.obj del *.obj
      @if exist *.exe del *.exe

