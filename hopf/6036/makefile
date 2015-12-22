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
#   Attention: init-modules (zeitini, init) have to
#              be listed at the end
#
#===================================================================

AOBJ = hopf6036.OBJ zeitrout.obj common.obj zeitini.obj init.obj

#-------------------------------------------------------------------
#   This section lists all files to be built by the make.
#-------------------------------------------------------------------
all: $(AOBJ) hopf6036.lnk hopf6036.sys



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

hopf6036.obj: hopf6036.asm

zeitini.obj:  zeitini.asm

zeitrout.obj: zeitrout.c hopf6036.h ../../dcfioctl.h

init.obj:     init.c

common.obj:   ..\common.c

hopf6036.lnk: makefile
        echo $(AOBJ) /nod /EXEPACK /MAP /ALIGN:16 >  hopf6036.lnk
        echo hopf6036.sys   >> hopf6036.lnk
        echo hopf6036.map,os2286, >> hopf6036.lnk
        echo hopf6036.def;  >> hopf6036.lnk

hopf6036.sys:  $(AOBJ) hopf6036.def hopf6036.lnk
          link386  @hopf6036.lnk

{..}.c.obj:
        cl -c -I.. -I..\.. -Os -nologo %s

.c.obj:
        cl -c -I.. -I..\.. -Os -nologo %s

cleanall:
      @if exist *.obj del *.obj
      @if exist *.sys del *.sys

