/***************************************************************************\
 *
 * PROGRAMMNAME: ZEIT, WPDCF77
 * -------------
 *
 * VERSION: 6.0
 * --------
 *
 * MODULNAME: DCFIOCTL.H
 * ----------
 *
 * BESCHREIBUNG:
 * -------------
 *   Header-Datei IOCtls an Device-Driver
 *   Hinweis zu Timern:
 *   TIMER_MODE_SYS ist der OS/2-Systemtimer (32ms); usTicklen mu· 0 sein
 *   TIMER_MODE_HR  ist der OS/2 Warp 4.0 High Resolution-Treiber
 *
 *  Ver.    Date      Comment
 *  ----    --------  -------
 *  1.00    10-23-94  First release
 *  4.00    03-15-96  UnterstÅtzung fÅr hopf
 *  4.10    06-19-96  UnterstÅtzung fÅr hopf 6036
 *  4.20    09-12-96  UnterstÅtzung fÅr hopf 6038
 *  4.40    01-31-00  Multimedia-UnterstÅtzung, Bugfixing
 *  5.00    06-26-00  Zusammenstellung neuer Release
 *  5.10    02-10-01  Fehlerkorrektur Positionsspeicherung Iconview
 *  5.20    06-09-01  Fehlerkorrektur Audiodaten
 *  5.30    01-16-02  Implementierung der Erinnerungsfunktion
 *  5.40    11-22-03  Fehlerkorrektur Erinnerungsfunktion
 *  6.00    02-15-04  USB UnterstÅtzung
 *
 *  Copyright (C) noller & breining software 1995...2004
 *
\******************************************************************************/
#ifndef DCFIOCTL_H
#define DCFIOCTL_H

#ifdef __32BIT__
#ifndef FAR
#define FAR
#endif /* FAR */
#endif /* __32BIT__ */

#define DEVICENAME          "DCF77$"    // Device-Name des Treibers

#define IOCTL_DCF77             0x90    // DCF77 Kategorie

#define DCF77_SETOFFSET         0x40    // Offseteinstellung setzen
#define DCF77_SETDATA           0x41    // Daten setzen
#define DCF77_SETPORT           0x42    // Daten an Portadresse ausgeben
#define DCF77_SETWZSZ           0x45    // Winter-/Sommer-Umschaltzeitpunkt setzen (hopf 6038)
#define DCF77_SETCOORD          0x46    // GPS-Koordinaten setzen
#define DCF77_QUERYOFFSET       0x60    // Offseteinstellung holen
#define DCF77_QUERYDATA         0x61    // Daten holen
#define DCF77_GETSTATUS         0x62    // Status holen
#define DCF77_GETDATETIME       0x63    // Datum / Uhrzeit holen
#define DCF77_GETMILLI          0x64    // Millisekunden / Analogwerte abholen
#define DCF77_QUERYCOORD        0x66    // GPS-Koordinaten abholen
#define DCF77_GETCFGFILE        0x67    // Namen der Konfigurationsdatei holen
#define DCF77_GETLVLDATA        0x68    // Puffer mit BitlÑngen holen
#define DCF77_GETDEVICEDESCR    0x69    // read device descriptor
#pragma pack(1)

/********************* DCF77_SETOFFSET / DCF77_QUERYOFFSET ********************/
/* Parameterpaket */
// BYTE     reserved

/* Datenpaket */
typedef struct _DCF77_OFFSET
    {
    BYTE    flUTC;                      // 0 = MEZ/MESZ; 1 = UTC
    SHORT   sOffset;                    // Zeitoffset -2...+3 Stunden
    } DCF77_OFFSET;
typedef DCF77_OFFSET FAR *PDCF77_OFFSET;

/********************* DCF77_SETDATA /DCF77_QUERYDATA *************************/
/* Parameterpaket */
// BYTE     CommandData
#define CMD_DATA_ACTUAL     0           // Aktuelle Einstellungen
#define CMD_DATA_DEFAULT    1           // Default-Parameter (nur DCF77_QUERYDATA)

/* Datenpaket */
typedef struct _DCF77_DATA
    {
    BYTE    bSupply;                    // Versorgung, siehe SUPPLY_*-Werte
    BYTE    bSignal;                    // Steuerleitung 0...7 | SIGNAL_*-Werte
    BYTE    bPort;                      // Eingangsport, siehe PORT_*-Werte
    BYTE    bcRepeat;                   // WiederholprÅfung 1...10
    USHORT  usIOAddr;                   // I/O-Adresse fÅr Nicht-Standard-Adapter
    USHORT  usSetInterval;              // Intervallzeit (Setzen der Systemuhr) in min
    USHORT  usThreshold;                // 10*Schwellwert ImpulslÑnge
    USHORT  usTicklen;                  // TicklÑnge in ms
    USHORT  usTimerMode;                // Timertyp, siehe TIMER_MODE_*-Werte
    } DCF77_DATA;
typedef DCF77_DATA FAR *PDCF77_DATA;

#define SUPPLY_L1_POS   0x01            // Leitung 1: positive Spannung
#define SUPPLY_L2_POS   0x02            // Leitung 2: positive Spannung
#define SUPPLY_OFF      0x04            // Leitungen  inaktiv

#define SIGNAL_MASK     0x07            // Maske fÅr Bitnummer
#define SIGNAL_SERIAL   0x10            // öbertragung Åber RxD
#define SIGNAL_SCAN     0x20            // Automatisches scannen aktiv

#define PORT_SER1       0x01            // Serielle Schnittstelle 1
#define PORT_SER2       0x02            // Serielle Schnittstelle 2
#define PORT_SER3       0x03            // Serielle Schnittstelle 3
#define PORT_SER4       0x04            // Serielle Schnittstelle 4
#define PORT_SERX       0x11            // Serielle Schnittstelle, I/O-Adresse in usIOAddr
#define PORT_GAME       0x05            // Standard-Gameport
#define PORT_GAMEX      0x15            // Gameport, I/O-Adresse in usIOAddr
#define PORT_PORTX      0x10            // Port mit I/O-Adresse in usIOAddr
#define PORT_HOPF6036   0x20            // DCF77-Einsteckkarte 603x von hopf
#define PORT_USB        0x40            // USB-Treiber
#define PORT_SHMEM      0x80            // Daten stammen aus Shared memory

#define TIMER_MODE_UNK     0            // Typ noch nicht bekannt (Treiber-Init)
#define TIMER_MODE_SYSI    1            // Typ vorlÑufig System-Timer (Treiber-InitComplete)
#define TIMER_MODE_SYS     2            // Typ ist System-Timer
#define TIMER_MODE_HR      3            // Typ ist High Resolution-Timer

/********************* DCF77_SETPORT ******************************************/
typedef struct _PORT_PAIR
    {
    BYTE    bPortOfs;                   // Port-Offset (Port = Basis + Offset)
    BYTE    bValue;                     // Wert fÅr Port
    } PORT_PAIR;

/* Parameterpaket */
// BYTE     reserved

/* Datenpaket */
typedef struct _DCF77_PORT
    {
    BYTE         bNum;                  // Anzahl der folgenden Datenpaare
    PORT_PAIR    ppData[1];             // Datenpaare
    } DCF77_PORT;
typedef DCF77_PORT FAR *PDCF77_PORT;

/********************* DCF77_GETSTATUS ****************************************/
/* Parameterpaket */
// BYTE     reserved

/* Datenpaket */
typedef struct _DCF77_STATUS
    {
    UCHAR   cDrvType;                   // Treibertyp, siehe DRVTYPE_*-Werte
    UCHAR   cVerMajor;                  // Hauptversionsnummer
    UCHAR   cVerMinor;                  // Unterversionsnummer
    BYTE    bPolarity;                  // PolaritÑt 0=negativ, 1=positiv
    BYTE    bIsHRTimerAvail;            // 1 = HR-Timer verfÅgbar
    USHORT  flStatus;                   // Status, siehe STATUS_*-Werte
    USHORT  uscBadSeconds;              // empfangene falsche Zeiten
    USHORT  uscGoodSeconds;             // empfangene korrekte Zeiten
    USHORT  uscTimeSet;                 // Zahl der SetzvorgÑnge
    USHORT  flMaskST;                   // Maskierung, siehe MASK_ST_*-Werte
    USHORT  flMaskSS;                   // Maskierung, siehe MASK_SS_*-Werte
    USHORT  flMaskTO;                   // Maskierung, siehe MASK_TO_*-Werte
    USHORT  flMaskIV;                   // Maskierung, siehe MASK_IV_*-Werte
    USHORT  flMaskLN;                   // Maskierung, siehe MASK_LN_*-Werte
    } DCF77_STATUS;
typedef DCF77_STATUS FAR *PDCF77_STATUS;

#define DRVTYPE_EXTMOD      1           // Treiber ZEIT.SYS
#define DRVTYPE_HOPF6036    2           // Treiber HOPF6036.SYS
#define DRVTYPE_HOPF6038    3           // Treiber HOPF6038.SYS
#define DRVTYPE_USB         4           // Treiber USBDCF.SYS

#define STATUS_ANT          0x0001      // Ersatzantenne
#define STATUS_HLEAP        0x0002      // Stundensprung
#define STATUS_SLEAP        0x0004      // Schaltsekunde
#define STATUS_TZONE1       0x0008      // Zeitzone 1
#define STATUS_TZONE2       0x0010      // Zeitzone 2
#define STATUS_CARRIER      0x0100      // TrÑger vorhanden
#define STATUS_TIMEVALID    0x0200      // Zeit gÅltig
#define STATUS_SCAN         0x0400      // Scan-Modus aktiv
#define STATUS_BATT         0x0800      // Batterie leer
#define STATUS_ISDRIVER     0x8000      // Treiber/Shared memory vorhanden

/* Maskierung Status-Panel */
#define MASK_ST_ANT         0x0001      // Info Åber Ersatzantenne
#define MASK_ST_HLEAP       0x0002      // Info Åber Stundensprung
#define MASK_ST_SLEAP       0x0004      // Info Åber Schaltsekunde
#define MASK_ST_TZONE1      0x0008      // Info Åber Zeitzone 1
#define MASK_ST_TZONE2      0x0010      // Info Åber Zeitzone 2
#define MASK_ST_COORDINATES 0x0020      // Info Åber GPS-Koordinaten
#define MASK_ST_CARRIER     0x0100      // Info Åber TrÑger vorhanden
#define MASK_ST_TIMEVALID   0x0200      // Info Åber Zeit gÅltig
#define MASK_ST_SCAN        0x0400      // Info Åber Scan-Modus aktiv
#define MASK_ST_BATT        0x0800      // Info Åber Batterie leer
#define MASK_ST_POLARITY    0x1000      // Info Åber PolaritÑt
#define MASK_ST_BAD         0x2000      // Info Åber ZÑhler schlechter Empfang
#define MASK_ST_GOOD        0x4000      // Info Åber ZÑhler guter Empfang
#define MASK_ST_SET         0x8000      // Info Åber Zahl der SetzvorgÑnge
#define MASK_ST_ALL         0xFF3F

/* Maskierung Zeitoffset-Panel */
#define MASK_TO_SYSWIDE     0x0001      // Systemweite Einstellung mîglich
#define MASK_TO_ALL         0x0001

/* Maskierung Schnittstellen-Panel */
#define MASK_SS_SER1        0x0001      // Treiber unterstÅtzt COM1
#define MASK_SS_SER2        0x0002      // Treiber unterstÅtzt COM2
#define MASK_SS_SER3        0x0004      // Treiber unterstÅtzt COM3
#define MASK_SS_SER4        0x0008      // Treiber unterstÅtzt COM4
#define MASK_SS_SERX        0x0010      // Treiber unterstÅtzt COMX
#define MASK_SS_GAME        0x0020      // Treiber unterstÅtzt Game
#define MASK_SS_GAMEX       0x0040      // Treiber unterstÅtzt GameX
#define MASK_SS_HOPF        0x0080      // Treiber unterstÅtzt hopf603x
#define MASK_SS_USB         0x0100      // Treiber unterstÅtzt USB GerÑte
#define MASK_SS_ALL         0x01FF

/* Maskierung Intervall-Panel */
#define MASK_IV_UPDATE      0x0001      // Treiber unterstÅtzt Updateintevall
#define MASK_IV_THRESH      0x0002      // Treiber unterstÅtzt Schwellwert
#define MASK_IV_CHECK       0x0004      // Treiber unterstÅtzt WiederholprÅfung
#define MASK_IV_HRTIMER     0x0008      // Treiber unterstÅtzt HR-Timer
#define MASK_IV_ALL         0x000F

/* Maskierung Leitungs-Panel */
#define MASK_LN_POWER       0x0001      // Treiber braucht Signalleitung
#define MASK_LN_CONTROL     0x0002      // Treiber unterstÅtzt Steuerleitungs-Mode
#define MASK_LN_SIGNAL      0x0004      // Treiber unterstÅtzt Datenleitungs-Mode
#define MASK_LN_ALL         0x0007

/********************* DCF77_GETDATETIME **************************************/
/* Parameterpaket */
// BYTE     reserved

/* Datenpaket */
typedef struct _DCF77_DATETIME
    {
    UCHAR   hours;
    UCHAR   minutes;
    UCHAR   seconds;
    UCHAR   day;
    UCHAR   month;
    USHORT  year;
    SHORT   timezone;                   // Zeitzone, siehe TZ_*-Werte
    } DCF77_DATETIME;
typedef DCF77_DATETIME FAR *PDCF77_DATETIME;

/* Werte fÅr DCF77_DATETIME.timezone */
#define TZ_UNK             -1           // Unbekannt
#define TZ_PAC              0           // Pacific Time
#define TZ_MOU              1           // Mountain Time
#define TZ_CEN              2           // Central Time
#define TZ_EAS              3           // Eastern Time
#define TZ_MSF              4           // MSF (GB)
#define TZ_DCF              5           // DCF
#define TZ_GPS              6           // GPS Time

/********************* DCF77_GETMILLI *****************************************/
/* Parameterpaket */
// BYTE     reserved

/* Datenpaket */
typedef struct _DCF77_MILLI
    {
    USHORT          usValue;            // Millisekunden-Wert
    } DCF77_MILLI;
typedef DCF77_MILLI FAR *PDCF77_MILLI;

/********************* DCF77_SETWZSZ ******************************************/
/* Parameterpaket */
// BYTE     reserved

/* Datenpaket */
#ifdef _TLOC_
typedef struct _DCF77_WZSZ
    {
    TLOC_DEF    sDtSwitch1;             // 1. Umschaltzeitpunkt
    TLOC_DEF    sDtSwitch2;             // 2. Umschaltzeitpunkt
    } DCF77_WZSZ;
typedef DCF77_WZSZ FAR *PDCF77_WZSZ;
#endif /* _TLOC_ */

/********************* DCF77_SETCOORD / DCF77_GETCOORD ************************/
/* Parameterpaket */
// BYTE     reserved

/* Datenpaket */
typedef struct _DCF77_COORD
    {
    LONG        Breite;                 // Breite in GradMsec
    LONG        Laenge;                 // Laenge in GradMsec
    } DCF77_COORD;
typedef DCF77_COORD FAR *PDCF77_COORD;

// GradMsec = (Grad * 100 + Min) * 36000;

/****************************** DCF77_GETCFGFILE ******************************/
/* Parameterpaket */
// BYTE     reserved

/* Datenpaket */
// CHAR     szCfgFileName[CCHMAXPATH]   // Name des Konfigurationsfile

/**************************** DCF77_GETLVLDATA ********************************/
/* Parameterpaket */
// BYTE     reserved

/* Datenpaket */
typedef struct _DCF77_LEVEL
    {
    USHORT      usSize;                 // Puffergrî·e
    BYTE        bLevel[1];              // Puffer fÅr Bit-Pegel
    } DCF77_LEVEL;
typedef DCF77_LEVEL FAR *PDCF77_LEVEL;

/********************* DCF77_GETDEVICEDESCR ***********************************/
/* Parameterpaket */
// USHORT:  DESC_STR_*-value
#define DESC_STR_MANU       1
#define DESC_STR_PRODUCT    2
#define DESC_STR_SERIAL     3

/* Datenpaket */
typedef struct _DCF77_STRING
    {
    USHORT          usSize;             // Puffergrî·e
    USHORT          uscStrlen;          // UNICODE-StringlÑnge
    CHAR            cDesc[1];
    } DCF77_STRING;
typedef DCF77_STRING FAR *PDCF77_STRING;

#pragma pack()

/**************** Definitionen fÅr den Shared memory Bereich ******************/
#define SHMEMNAME           "\\SHAREMEM\\DCF77.DAT"
#define SEMNAME             "\\SEM32\\DCF77.SEM"

#define PROG_VER_MAJOR      6
#define PROG_VER_MINOR      0

#define PROG_SERVER         0x0001
#define PROG_CLIENT         0x0002
#define PROG_DRIVER         0x0004
#define PROG_CLOCK          0x0008

#define DEFAULT_INTERVAL    60          // Default-Intervall: 1min

#define TERM_VERSION        1           // falsche Serverversion

typedef struct _DCF77_ALLDATA
    {
    BYTE            bSize;
    BYTE            bVerMajor;
    BYTE            bVerMinor;
    ULONG           ulInterval;         // Sendeintervall in Sekunden
    BOOL            bSendSrvrTime;      // Bei Empfangsstîrung Serverzeit senden
    DCF77_OFFSET    dcf77Offset;        // Info von NB-Client/hopf -> Uhr
    DCF77_STATUS    dcf77Status;
    DCF77_DATETIME  dcf77DateTime;
    DCF77_DATA      dcf77Data;
    } DCF77_ALLDATA, *PDCF77_ALLDATA;

typedef struct _SHMEM
    {
    ULONG           flProgs;
    ULONG           ulTermReason;       // Grund fÅr Beenden des Clients TERM_*

    DCF77_ALLDATA   dcf77Data;
    } SHMEM, *PSHMEM;

#endif /* DCFIOCTL_H */
