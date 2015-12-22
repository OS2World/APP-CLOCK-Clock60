#===================================================================
#
#
#   Copyright 1990,1992 IBM Corporation
#
#===================================================================

#
# Comment out or delete the .inc file you don't want
#
#===================================================================
#
#   A list of all of the object files
#
#===================================================================

AOBJ = hopf6038.OBJ zeitrout.obj zeitini.obj init.obj sz_wz.obj common.obj

#-------------------------------------------------------------------
#   This section lists all files to be built by the make.
#-------------------------------------------------------------------
all: $(AOBJ) hopf6038.lnk hopf6038.sys



#-------------------------------------------------------------------
#   This section creates the command file used by the linker.  This
#   command file is recreated automatically every time you change
#   the object file list, linker flags, or library list.
#-------------------------------------------------------------------


#===================================================================
#
# Dependencies
#
#   This section lists all object files needed to be built for the
#   application, along with the files it is dependent upon (e.g. its
#   source and any header files).
#
#===================================================================
hopf6038.obj: hopf6038.asm

zeitini.obj:  zeitini.asm

zeitrout.obj: zeitrout.c hopf6038.h ..\..\dcfioctl.h ..\tloc.h ..\common.h

init.obj:     init.c ..\common.h ..\tloc.h

sz_wz.obj:    sz_wz.c ..\tloc.h

common.obj:   ..\common.c

hopf6038.lnk: makefile
        echo $(AOBJ) /nod /EXEPACK /MAP /ALIGN:16 >  hopf6038.lnk
        echo hopf6038.sys   >> hopf6038.lnk
        echo hopf6038.map,os2286, >> hopf6038.lnk
        echo hopf6038.def;  >> hopf6038.lnk

hopf6038.sys:  $(AOBJ) hopf6038.def hopf6038.lnk
          link386  @hopf6038.lnk

{..}.c.obj:
        cl -c -Zi -I.. -I..\.. -Os -nologo %s

.c.obj:
        cl -c -Zi -I.. -I..\.. -Os -nologo %s

.asm.obj:
        ml -c -Zi $*.asm

cleanall:
      @if exist *.obj del *.obj
      @if exist *.sys del *.sys

