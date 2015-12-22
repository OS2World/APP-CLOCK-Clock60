/* Build complete DCF77 package */
/* Syntax: buildall <buildlevel> */

/* Check arguments */
parse arg bldlvl

if bldlvl='' then do
  say 'incomplete arguments; Syntax:'
  say 'buildall <buildlevel>'
  say '  buildlevel := X.XX.XX'
  exit
end

/* Language independent part */
say
say '[33mÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿[37m'
say '[33m³ Device Driver ³[37m'
say '[33mÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ[37m'
say '[33m- zeit.sys -[37m'
'cd zeit'
'setlocal'
'call iml.cmd'
'call ic600.cmd'
'sed -e "s/X.XX.XX/'bldlvl'/" zeit.tpl > zeit.def'
'nmake cleanall'
'nmake DEBUG=NO MODE=SHAREWARE'
if RC > 0 then do
  EXIT
end
'copy zeit.sys ..\install\zeit.sys.shareware'
'nmake cleanall'
'nmake DEBUG=NO'
if RC > 0 then do
  EXIT
end
'endlocal'
'copy zeit.sys ..\install'
'cd ..'

say
say '[33m- hopf6036.sys -[37m'
'cd hopf\6036'
'setlocal'
'call iml.cmd'
'call ic600.cmd'
'sed -e "s/X.XX.XX/'bldlvl'/" hopf6036.tpl > hopf6036.def'
'nmake cleanall'
'nmake DEBUG=NO'
if RC > 0 then do
  EXIT
end
'endlocal'
'copy hopf6036.sys  ..\install'
'cd ..\..'

say
say '[33m- hopf6038.sys -[37m'
'cd hopf\6038'
'setlocal'
'call iml.cmd'
'call ic600.cmd'
'sed -e "s/X.XX.XX/'bldlvl'/" hopf6038.tpl > hopf6038.def'
'nmake cleanall'
'nmake DEBUG=NO'
if RC > 0 then do
  EXIT
end
'endlocal'
'copy hopf6038.sys  ..\install'
'cd ..\..'

say
say '[33m- hopf.exe -[37m'
'cd hopfext'
'sed -e "s/X.XX.XX/'bldlvl'/" hopf.tpl > hopf.def'
'nmake cleanall'
'nmake DEBUG=NO'
if RC > 0 then do
  EXIT
end
'copy hopf.exe ..\install'
'cd ..'

say
say '[33mÚÄÄÄÄÄÄÄÄÄÄÄ¿[37m'
say '[33m³ Utilities ³[37m'
say '[33mÀÄÄÄÄÄÄÄÄÄÄÄÙ[37m'
say '[33m- hotspot.exe -[37m'
'cd hotspot'
'sed -e "s/X.XX.XX/'bldlvl'/" hotspot.tpl > hotspot.def'
'nmake cleanall'
'nmake DEBUG=NO'
if RC > 0 then do
  EXIT
end
'copy hotspot.exe ..\install'
'cd ..'

say
say '[33m- setaddr.exe -[37m'
'cd setaddr'
'sed -e "s/X.XX.XX/'bldlvl'/" setaddr.tpl > setaddr.def'
'nmake cleanall'
'nmake DEBUG=NO'
if RC > 0 then do
  EXIT
end
'copy setaddr.exe ..\install'
'cd ..'

say
say '[33mÚÄÄÄÄÄÄÄÄÄÄÄ¿[37m'
say '[33m³ WPS class ³[37m'
say '[33mÀÄÄÄÄÄÄÄÄÄÄÄÙ[37m'
say '[33m- wpdcf77.dll -[37m'
'cd som2'
'sed -e "s/X.XX.XX/'bldlvl'/" clock.tpl > clock.def'
'nmake cleanall'
'nmake DEBUG=NO'
if RC > 0 then do
  EXIT
end
'copy wpdcf77.dll ..\..\install'
'cd ..'

say
say '[33mÚÄÄÄÄÄÄÄÄÄÄÄ¿[37m'
say '[33m³ LAN::Time ³[37m'
say '[33mÀÄÄÄÄÄÄÄÄÄÄÄÙ[37m'
say '[33m- TimeClnt.exe -[37m'
'cd time_cs50\timeclnt'
'sed -e "s/X.XX.XX/'bldlvl'/" timeclnt.tpl > timeclnt.def'
'nmake cleanall'
'nmake DEBUG=NO'
if RC > 0 then do
  EXIT
end
'copy timeclnt.exe ..\..\install'
'cd ..\..'

say
say '[33m- TimeServ.exe -[37m'
'cd time_cs50\timeserv'
'sed -e "s/X.XX.XX/'bldlvl'/" timeserv.tpl > timeserv.def'
'nmake cleanall'
'nmake DEBUG=NO'
if RC > 0 then do
  EXIT
end
'copy timeserv.exe ..\..\install'
'cd ..\..'

/* Generate installer packages */
say
say '[33mÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿[37m'
say '[33m³ Installer packages ³[37m'
say '[33mÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ[37m'

bldnum = substr(bldlvl, 1, 3)
bldnum = delstr(bldnum, 2, 1)

'cd install\Client'
'call makeinst.cmd ' bldnum
'cd ..\Server'
'call makeinst.cmd ' bldnum
'cd ..\Shareware'
'call makeinst.cmd ' bldnum

'cd ..\..'
say '** Finished **'

