/* Register class WPDCF77 */
echo off

Call RxFuncadd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
Call SysLoadFuncs

say ""
say "Try to Register class"
say ""

rc = SysRegisterObjectClass("WPDCF77", wpdcf77)
beep(1890,100)

say ""
say 'WPDCF77 class registered'

rc = SysCreateObject("WPDCF77", "New Clock", "<WP_DESKTOP>")

exit
