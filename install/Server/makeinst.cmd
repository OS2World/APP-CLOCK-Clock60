@echo off
if "%1" == "" goto error

del dcfdrvr dcf_%1s.zip
cd ..
pack server\packing.lst server\dcfdrvr /L
copy readme.os2 server
cd server
zip dcf_%1s.zip dcfinst.exe dcfinst.hlp dcfdrvr readme.os2
goto end

:error
echo Syntax: makeinst <bldlevel>
echo   example: makeinst 250
:end
