#ifndef _ZEIT_C_
#define _ZEIT_C_

#include "dcfioctl.h"

#define VERSION_MAJOR       5
#define VERSION_MINOR       4

#define GAME                0
#define GAMEX               1
#define SER                 2
#define SERX                3
#define COM                 4
#define COMX                5

#define AUTOSTART           0   // Erkennung Signalleitung gestartet
#define AUTOON              1   // Signalleitungserkennung lÑuft
#define AUTOOFF             2   // GerÑtetyp benîtigt keine Erkennung
#define AUTOSCANNED         3   // Bit erkannt

/* dcffield_pol */
#define POSITIVE            0
#define NEGATIVE            0xFF

/* Local / greenw. Zeitauswertung */
#define TIME_LOCAL          0
#define TIME_UTC            1

/* Schwellwertberechnung */
#define THR_10MS_MIN        5               // Minimalwert Impulsdauer: 50ms
#define THR_10MS_MAX        25              // Maximalwert Impulsdauer: 250ms
#define THR_PEAK_DIST       5               // Mindestabstand 2er Maxima
#define THR_ARRAYSIZE       THR_10MS_MAX-THR_10MS_MIN+1

/* Schwellwerte fÅr Impulserkennung */
#define THR_TICKLEN         10L             // HR-Timertick in ms
#define THR_HR_CARRIER      320             // Erkennung TrÑgerausfall; 3.2s
#define THR_HR_POLARITY     150             // Erkennung PolaritÑt;     1.5s
#define THR_HR_MINUTE       150             // Erkennung Minutenimpuls; 1.5s
#define THR_HR_BITLEN       120/THR_TICKLEN // Erkennung BitpolaritÑt;  Startwert: 120ms
#define THR_SYS_CARRIER     100             // Erkennung TrÑgerausfall; 3.2s
#define THR_SYS_POLARITY    45              // Erkennung PolaritÑt;     1.5s
#define THR_SYS_MINUTE      45              // Erkennung Minutenimpuls; 1.5s
#define THR_SYS_BITLEN      4               // Erkennung BitpolaritÑt;  Startwert: 128ms

/* Definitionen fÅr Status-Wort im Request-Paket */
#define STERR               0x8000          // Bit 15 - Error
#define STINTER             0x0400          // Bit 10 - Interim character
#define STBUI               0x0200          // Bit  9 - Busy
#define STDON               0x0100          // Bit  8 - Done
#define STECODE             0x00FF          // Error code

#define STATUS_DONE         STDON
#define STATUS_BAD_CMD      STERR+STDON+ERROR_I24_BAD_COMMAND
#define STATUS_GEN_FAIL     STERR+STDON+ERROR_I24_GEN_FAILURE
#define STATUS_INV_PARM     STERR+STDON+ERROR_I24_INVALID_PARAMETER
#define STATUS_IN_USE       STERR+STDON+ERROR_I24_DEVICE_IN_USE

/***********************************************************************/
/*  Funktionsprototypen                                                */
/***********************************************************************/
PSZ far          f_strcpy (PSZ pszDest, const PSZ pszSource);
USHORT far       f_strlen (PSZ pszSource);
PVOID far        f_memcpy (PVOID pDest, const PVOID pSource, USHORT cLen);

void             start (void);
unsigned char    getbit (unsigned short);
void far _loadds timerHandler (void);
void far _loadds iTimerHandler (void);
BOOL             registerTimer (USHORT newTimerMode, USHORT tickLen);

BOOL             insertThrValue (USHORT bitlen);
VOID             calcThreshold (VOID);
VOID             insertLvl (UCHAR c);

void             outbyte (short port, short value);
short            inbyte (short port);
USHORT           TestSeg (PUSHORT pSeg, USHORT usLen);
USHORT           LvlWait (PBYTE pBuffer, USHORT uscBuffer);
VOID             LvlInsert (BYTE chr);
VOID             LvlComIns (BYTE chr);
BOOL             LvlIsReady (VOID);

VOID             fillDataDCF (PDCF77_DATA pDataDCF);

#endif /*_ZEIT_C_*/
