/* */
      rc = '001'
      call RxFuncAdd 'SysLoadFuncs', 'REXXUTIL', 'SysLoadFuncs'
      call SysLoadFuncs

      irc = SysIni('USER','PM_National','iCountry')
      pos = VERIFY(irc,'0123456789','N')
      IF pos = 0 THEN
        return rc

      cc = SUBSTR(irc,1,pos-1)

      SELECT
        WHEN cc = '33' THEN
          rc = '033'
        WHEN cc = '39' THEN
          rc = '039'
        WHEN cc = '46' THEN
          rc = '046'
        WHEN cc = '49' THEN
          rc = '049'
      END

      return rc
