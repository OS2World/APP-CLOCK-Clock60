# makefile
# Created by IBM WorkFrame/2 MakeMake at 11:54:16 on 13 April 2016
#
# The actions included in this make file are:
#  Compile::C++ Compiler
#  Link::Linker

.SUFFIXES: .cpp .obj

.all: \
    .\hopf.exe

.cpp.obj:
    @echo " Compile::C++ Compiler "
    icc.exe $(CDB) -I.. /Tdp /Fo"%|dpfF.obj" /C %s

.\hopf.exe: \
    .\Inout.obj \
    .\ShareMem.obj \
    .\HopfMain.obj \
    .\InitClass.obj \
    {$(LIB)}hopf.def
    @echo " Link::Linker "
    icc.exe @<<
     /B" $(LDB) /noe"
     /Fe.\hopf.exe
     hopf.def
     .\Inout.obj
     .\ShareMem.obj
     .\HopfMain.obj
     .\InitClass.obj
<<

!include "makefile.DEP"

cleanall:
      @if exist *.obj del *.obj
      @if exist *.exe del *.exe

