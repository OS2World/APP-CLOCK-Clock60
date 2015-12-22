@echo off
if "%1" == "" goto error

del dcfdrvr dcf_%1c.zip
cd ..
pack client\packing.lst client\dcfdrvr /L
copy readme.os2 client
cd client
zip dcf_%1c.zip dcfinst.exe dcfinst.hlp dcfdrvr readme.os2
goto end

:error
echo Syntax: makeinst <bldlevel>
echo   example: makeinst 250
:end
