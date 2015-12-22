/* Deregister class WPDCR77 */
echo off

Call RxFuncadd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
Call SysLoadFuncs

result = SysDeregisterObjectClass("WPDCF77")

say result

exit
