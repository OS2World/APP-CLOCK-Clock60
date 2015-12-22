/***************************************************************************\
 *
 * PROGRAMMNAME: WPDCF77
 * -------------
 *
 * VERSION: 6.0
 * --------
 *
 * MODULNAME: RES.H
 * ----------
 *
 * BESCHREIBUNG:
 * -------------
 *   Header-Datei fÅr CLOCK.RC
 *
 *  Ver.    Date      Comment
 *  ----    --------  -------
 *  1.00    10-23-94  First release
 *  3.20    03-15-96  UnterstÅtzung fÅr hopf
 *  5.10    02-10-01  Fehlerkorrektur Positionsspeicherung Iconview
 *  5.20    06-09-01  Fehlerkorrektur Audiodaten
 *  5.30    01-16-02  Implementierung der Erinnerungsfunktion
 *  5.40    11-22-03  Fehlerkorrektur Erinnerungsfunktion
 *  6.00    02-15-04  USB UnterstÅtzung
 *
 *  Copyright (C) noller & breining software 1995...2004
 *
\******************************************************************************/
#include "dlg.h"

/* KontextmenÅ */
#define IDM_OPENVIEW                    6000
#define IDM_OPENSTAT                    6001
#define IDM_MAINMENU                    6002
#define IDM_OPENGRPH                    6003
#define IDM_OPENANT                     6004
#define IDM_REMINDER                    6005
/* Die folgenden MenÅ-IDs mÅssen mit den entsprechenden OPEN_*-Werten Åbereinstimmen */
#define IDM_OPENANALOG                  6500
#define IDM_OPENDIGITAL                 6501
#define IDM_ABOUT                       6502
#define IDM_OPENSTATUS                  6503
#define IDM_OPENGRAPH                   6504
#define IDM_OPENANTENNA                 6505
#define IDM_REMIND                      6506
#define IDM_OPENDEBUG                   6507

/* MenÅ Grafikfenster */

/*--------------------------------------------------------------*\
 *  Stringtable ids
\*--------------------------------------------------------------*/
#define IDS_APPNAME                     1000
#define IDS_TIMEOK_DCF                  1001
#define IDS_DATEOK_DCF                  1002
#define IDS_TIMEOK_GPS                  1003
#define IDS_DATEOK_GPS                  1004
#define IDS_TIMENOCARRIER               1005
#define IDS_DATENOCARRIER               1006
#define IDS_TIMEFAIL                    1007
#define IDS_DATEFAIL                    1008
#define IDS_DRIVERFAIL                  1009
#define IDS_SERVERVERSION               1010
#define IDS_STATUSVIEW                  1011
#define IDS_PAGENAME_ALARM              1012
#define IDS_PAGENAME_DATETIME           1013
#define IDS_PAGENAME_VIEW               1014
#define IDS_PAGENAME_DCF                1015
#define IDS_PAGENAME_GPS                1016
#define IDS_PAGENAME_OFFSET             1017
#define IDS_PAGENAME_INTERVAL           1018
#define IDS_PAGENAME_TIMEZONE           1019
#define IDS_PAGENAME_AUDIO              1020
#define IDS_ALRM_DATE                   1021
#define IDS_ALRM_DAILY                  1022
#define IDS_ALRM_SEARCH_PROG            1023
#define IDS_ALRM_SEARCH_SOUNDFILE       1024
#define IDS_ABOUT                       1025
#define IDS_POSITIVE                    1026
#define IDS_NEGATIVE                    1027
#define IDS_PORT_INDEX                  1028
#define IDS_PORT_NUMBER                 1029
#define IDS_PORT_USB                    1030
#define IDS_GRAPHCLASS                  1031
#define IDS_GRAPHVIEW                   1032
#define IDS_GRAPHVIEWANT                1033
#define IDS_XAXES                       1034
#define IDS_YAXESQ                      1035
#define IDS_YAXESA                      1036
#define IDS_DCFDELTA                    1037
#define IDS_GPSPOSITION                 1038
#define IDS_GPSINVPOSITION              1039
#define IDS_WEEKDAY_SUN                 1040
#define IDS_WEEKDAY_MON                 1041
#define IDS_WEEKDAY_TUE                 1042
#define IDS_WEEKDAY_WED                 1043
#define IDS_WEEKDAY_THU                 1044
#define IDS_WEEKDAY_FRI                 1045
#define IDS_WEEKDAY_SAT                 1046
#define IDS_SEARCH_TZTAB                1047
#define IDS_SWITCHDATETIME              1048
#define IDS_INVALIDDATE                 1049
#define IDS_BITMAP_SEARCH_FILE          1050
#define IDS_GONG_TEXT                   1051
#define IDS_ALRM_TIME                   1052
#define IDS_ALRM_REMIND                 1053

/*--------------------------------------------------------------*\
 *  Messagetable ids
\*--------------------------------------------------------------*/
#define IDMSG_ERRORSTARTALARM           1
#define IDMSG_ALARMEXPIRED              2
#define IDMSG_NODIGIT                   3
#define IDMSG_ALARMTITLE                4
#define IDMSG_ERRHWHOPF                 5
#define IDMSG_ERRLOCFILE                6
#define IDMSG_ERRLOCENTRY               7
#define IDMSG_ERRBMPFILE                8
#define IDMSG_ERRFILENOTFOUND           9
#define IDMSG_ERRALARMTHREAD           10
