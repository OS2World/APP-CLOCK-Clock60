@echo off
if "%1" == "" goto error

del dcfdrvr dcf_%1sw.zip
cd ..
pack shareware\packing.lst shareware\dcfdrvr /L
copy readme.os2 shareware
cd shareware
zip dcf_%1sw.zip dcfinst.exe dcfinst.hlp dcfdrvr readme.os2
goto end

:error
echo Syntax: makeinst <bldlevel>
echo   example: makeinst 250
:end
