setlocal
set path=D:\programme\WarpIn;%path%
set beginlibpath=D:\programme\WarpIn
del *.wpi
wic DCF77.wpi -a 1 -c..\install *
endlocal
