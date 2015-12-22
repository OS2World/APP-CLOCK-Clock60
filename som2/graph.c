/***************************************************************************\
 *
 * PROGRAM NAME: WPDCF77
 * -------------
 *
 * VERSION: 6.0
 * --------
 *
 * MODULE NAME: GRAPH
 * ------------
 *
 * DESCRIPTION:
 * ------------
 *   Display the radio receiption quality
 *
 * NOTES:
 * ------
 *
 * FUNCTIONS:
 * -----------
 *   ClkGraph ()
 *   GraphFrameWndProc ()
 *   GraphWndProc ()
 *   GrphCreate ()
 *   GrphSize ()
 *   GrphMove ()
 *   GrphPaint ()
 *   GrphTimer ()
 *   GrphCommand ()
 *   GrphClose ()
 *   Loop ()
 *   thAdj6036 ()
 *
 *  Ver.    Date      Comment
 *  ----    --------  -------
 *  1.00    10-23-94  First release
 *  2.00    06-16-95  Extensions
 *  3.00    10-30-95  Client/Server support; SOM2
 *  4.00    03-15-96  Support for hopf
 *  4.20    10-15-96  Support for hopf GPS
 *  4.30    17-05-97  Support for HR-Timer
 *  4.40    01-31-00  Multimedia support, bug fixing
 *  5.10    02-10-01  Bug fixing position of icon view
 *  5.20    06-09-01  Bug fixing audio data
 *  5.30    01-16-02  Implementation of reminder function
 *  5.40    11-22-03  Bug fixing reminder function
 *  6.00    02-15-04  USB support
 *
 *  Copyright (C) noller & breining software 1995...2007
 *
\******************************************************************************/
#define INCL_DOSDATETIME
#define INCL_DOSDEVICES
#define INCL_DOSDEVIOCTL
#define INCL_DOSERRORS
#define INCL_DOSPROCESS
#define INCL_GPICONTROL
#define INCL_GPITRANSFORMS
#define INCL_GPIPRIMITIVES
#define INCL_GPISEGEDITING
#define INCL_GPISEGMENTS
#define INCL_GPILCIDS
#define INCL_WINFRAMEMGR
#define INCL_WINWINDOWMGR
#define INCL_WINMENUS
#define INCL_WININPUT
#define INCL_WINPOINTERS
#define INCL_WINTRACKRECT
#include <os2.h>
#include <stdlib.h>
#include <limits.h>

#include "dcfioctl.h"

#pragma info(noeff)         			// suppress SOM compiler informationals

#include "wpdcf77.ih"
#include "wpdcf.h"

#include "res.h"
#include "clock.h"
#include "clockpan.h"
#include "clkdata.h"
#include "dcf77.h"
#include "graph.h"
#include "debug.h"

#define TIMELOOP        1000000         // SchleifenzÑhler fÅr CPU-Geschwindigkeitsbestimmung
#define MAXLOOP         3               // Zahl der SchleifendurchlÑufe bei der Geschwindigkeitsbestimmung

#define GRAPH_FONT      "Tms Rmn"

#define BORDER_X        15              // Offset fÅr Viewport (x-Richtung)
#define BORDER_Y        30              // Offset fÅr Viewport (y-Richtung)
#define BORDER_XR       3               // rechter Rand fÅr Viewport
#define BORDER_YT       3               // oberer Rand fÅr Viewport
#define CHHEIGHT        14              // Zeichengrî·e fÅr Achsbeschriftung

#define CMAXGRSEGMENT   10              // maximale Zahl der Grafik-Segmente

/*--------------------------------------------------------------*\
 *  Globale Definitionen und Variable fÅr dieses Modul          *
\*--------------------------------------------------------------*/
/* Transformationsmatrix fÅr Me·kurve                     */
/* Matrix:  n      0      0                               */
/*          0      n      0                               */
/*         Dx     Dy      1                               */
/* Der Wert fÅr n mu· an die Fenstergrî·e angepa·t werden */
static MATRIXLF matlfGraph = {MAKEFIXED (1, 0), MAKEFIXED (0, 0), 0,
                              MAKEFIXED (0, 0), MAKEFIXED (1, 0), 0,
                              BORDER_X,         BORDER_Y,         1};

/* kann global bleiben, da diese Adresse im System konstant bleibt */
static PFNWP pfnwpFrameGraphWndProc;    // alte FrameWndProc fÅr WinSubclassWindow

static LONG lFirstSegId;
static LONG lNextSegId;

/*******************************************************************\
    calcRcvQualXLen: Berechnet die Zahl der Punkte auf der x-Achse
    im Modus "EmpfangsqualitÑt". Der Maximalwert hÑngt von der
    Zahl der Me·punkte im Puffer ab. Es wird zwischen folgenden
    Bereichen umgeschaltet:
      10h  20h  240h 1000h
    return:  Zahl der Me·punkte
\*******************************************************************/
ULONG calcRcvQualXLen (VOID)
    {
    if (ulIndexGrphData < 10 * 60 / CCHMINPERPIXEL)
        return 10 * 60 / CCHMINPERPIXEL;

    if (ulIndexGrphData < 20 * 60 / CCHMINPERPIXEL)
        return 20 * 60 / CCHMINPERPIXEL;

    if (ulIndexGrphData < 240 * 60 / CCHMINPERPIXEL)
        return 240 * 60 / CCHMINPERPIXEL;

    return CCHGRPHBUF / sizeof (USHORT);
    }

VOID calcTransformMatrix (HWND hwnd)
    {
    ULONG ulMaxIndex;
    RECTL rcl;

    WinQueryWindowRect (hwnd, &rcl);

    /* Bestimmen des maximalen Index fÅr die x-Achse ulMaxIndex */
    if (ulGraphView == OPEN_ANTADJ)
        ulMaxIndex = ulMaxPixel;
    else
        ulMaxIndex = calcRcvQualXLen ();

    /* Transformationsmatrix des Grafiksegmentes berechnen     */
    /* Die Werte reichen von (0,0) bis (ulMaxIndex,CCHGRPHMAX) */
    if (ulMaxIndex)
        {
        matlfGraph.fxM11 = 0x10000 * (rcl.xRight - rcl.xLeft - BORDER_X - BORDER_XR) / ulMaxIndex;
        matlfGraph.fxM22 = 0x10000 * (rcl.yTop - rcl.yBottom - BORDER_Y - BORDER_YT) / CCHGRPHMAX;
        }
    else
        {
        matlfGraph.fxM11 = MAKEFIXED (1, 0);
        matlfGraph.fxM22 = MAKEFIXED (1, 0);
        }

    return;
    }

/*******************************************************************\
    dtoa: Umwandlung einer double-Zahl in einen String. Es wird
    nur 1 Nachkommastelle berÅcksichtigt, bei ganzen Zahlen wird
    kein Dezimalpunkt in den String geschrieben. Der Zahlenbereich
    umfa·t 0...LONG_MAX
    Eingang: d:   double-Zahl
             psz: Zielpuffer
    Ausgang: psz: Ergebnis
    return:  Zeiger auf Ergebnisstring
\*******************************************************************/
PCHAR dtoa (double d, PCHAR psz)
    {
    ULONG uld, n;
    ULONG ul    = (ULONG)(10. * d + 0.5);
    PCHAR pNext = psz;

    /* Vorkommastellen abdividieren */
    for (uld = 1000000000; uld > 1; uld /= 10)
        {
        n = ul / uld;
        if (n || pNext != psz || uld == 10)
            *pNext++ = n + '0';
        ul %= uld;
        }

    /* Nachkommastelle behandeln, falls vorhanden */
    if (ul)
        {
        *pNext++ = '.';
        *pNext++ = ul + '0';
        }

    *pNext = '\0';
    return psz;
    }

/*******************************************************************\
    GraphDrawTicks: Ticks der x-Achse zeichnen und beschriften.
    Dazu wird die x-Achse in Intervalle unterteilt, deren Anzahl
    von der LÑnge der Achse und dem maximalen Wert der Beschriftung
    abhÑngt.
    Eingang: hps:     Presentation-space handle
             ulValue: maximaler Beschriftungswert (hîchster Tick)
             prcl:    Fenstergrî·e
\*******************************************************************/
VOID GraphDrawTicks (HPS hps, ULONG ulValue, PRECTL prcl)
    {
    ULONG  ulIntv, ulIntvA;
    ULONG  i, ul;
    LONG   cx, cxString;
    POINTL ptl;
    CHAR   sz[12];
    POINTL aptl[12];

    if (ulValue == 0)
        return;

    cx = prcl->xRight - prcl->xLeft - BORDER_X - BORDER_XR;

    /* maximale Zahl der Tick-Intervalle bezogen auf AchsenlÑnge */
    ulIntvA = cx / (CHHEIGHT * 4);

    /* maximale Zahl der Ticks bezogen auf Me·zeit */
    /* 1. ulValue auf den Bereich 10...99 einschrÑnken */
    ul = ulValue * 10;
    for (i = 1; ul / i > 99; i *= 10) {}
    ul /= i;

    /* 2. Durch die grî·te Primzahl abdividieren, so da· das Ergebnis <= ulIntvA ist */
    for (ulIntv = i = 1; i < 99; i++)
        {
        if ((ul % i == 0) && (ul / i <= ulIntvA))
            {
            ulIntv = ul / i;
            break;
            }
        }

    /* Ticks zeichnen und beschriften */
    for (i = 0; i <= ulIntv; i++)
        {
        /* Tick zeichnen */
        ptl.x = BORDER_X + i*cx/ulIntv;
        ptl.y = BORDER_Y;
        GpiMove (hps, &ptl);
        ptl.y -= BORDER_Y / 5;
        GpiLine (hps, &ptl);
        ptl.y = 4*BORDER_Y/5 - 12;

        /* Tickwert in String konvertieren */
        dtoa ((double)(ulValue*i)/(double)ulIntv, sz);
        ul = strlen (sz);

        /* StringlÑnge in Pixel berechnen */
        GpiQueryCharStringPos (hps, 0, ul, sz, NULL, aptl);
        cxString = aptl[ul].x - aptl[0].x;
        if (i == ulIntv)
            ptl.x -= cxString;          // letzter Tick rechtsbÅndig
        else
            ptl.x -= cxString / 2;      // Ticks mittig

        /* String darstellen */
        GpiCharStringAt (hps, &ptl, ul, sz);
        }

    return;
    }

VOID GrphCreate (HWND hwnd, PWINDOWDATA pWindowData, PHPS phps, PHDC phdc)
    {
    WPDCF77Data  *somThis;
    CHAR          szText[CCHMAXPGNAME];
    SIZEL         sizl;
    ULONG         idString;

    /* Zugang zu Instanzdaten initialisieren */
    somThis = pWindowData->somThis;

    /* Class view/usage details und Window spezifische Daten */
    pWindowData->UseItem.type    = USAGE_OPENVIEW;
    pWindowData->ViewItem.view   = ulGraphView;
    pWindowData->ViewItem.handle = _hwndFrameGraph;

    /* Window-Ptr mit pWindowData fÅllen und View registrieren */
    WinSetWindowPtr (hwnd, QWL_USER, pWindowData);
    _wpAddToObjUseList (pWindowData->somSelf, &pWindowData->UseItem);
    idString = (ulGraphView == OPEN_ANTADJ ? IDS_GRAPHVIEWANT : IDS_GRAPHVIEW);
    WinLoadString (hab, hmod, idString, CCHMAXPGNAME, szText);
    _wpRegisterView (pWindowData->somSelf, _hwndFrameGraph, szText);
    WinSetFocus (HWND_DESKTOP, _hwndFrameGraph);

    *phdc = WinOpenWindowDC (hwnd);
    sizl.cx = sizl.cy = 0;
    *phps = GpiCreatePS (hab, *phdc, (PSIZEL)&sizl,
                         PU_PELS | GPIT_NORMAL | GPIA_ASSOC);

    /* Voreinstellungen fÅr das Grafik-Segment */
    lFirstSegId = lNextSegId = 1;
    GpiSetDrawingMode (*phps, DM_RETAIN);
    GpiSetInitialSegmentAttrs (*phps, ATTR_CHAINED, ATTR_ON);
    GpiSetInitialSegmentAttrs (*phps, ATTR_FASTCHAIN, ATTR_ON);
    GpiSetEditMode (*phps, SEGEM_INSERT);

    WinSetWindowPos (_hwndFrameGraph, NULLHANDLE,
        _lxGraphPos, _lyGraphPos, _lcxGraph, _lcyGraph,
        SWP_ACTIVATE | SWP_SIZE | SWP_MOVE);

    WinShowWindow (_hwndFrameGraph, TRUE);

    /* Timer starten */
    _wpAdd1sTimer (pWindowData->somSelf, hwnd);

    return;
    }

VOID GrphSize (HWND hwnd)
    {
    WPDCF77Data  *somThis;
    PWINDOWDATA   pWindowData;
    SWP           swp;

    /* Zugang zu Instanzdaten initialisieren */
    pWindowData = (PWINDOWDATA)WinQueryWindowPtr (hwnd, QWL_USER);
    somThis = pWindowData->somThis;

    /* Ist das Fenster ikonisiert? Nein => Position sichern */
    WinQueryWindowPos (_hwndFrameGraph, &swp);
    if (!(swp.fl & SWP_MINIMIZE))
        {
        _lcxGraph   = swp.cx;
        _lcyGraph   = swp.cy;
        _wpSaveDeferred (pWindowData->somSelf);
        }

    /* Berechnen der Transformationsmatrix fÅr das Grafiksegment */
    calcTransformMatrix (hwnd);

    return;
    }

VOID GrphMove (HWND hwnd)
    {
    WPDCF77Data  *somThis;
    PWINDOWDATA   pWindowData;
    SWP           swp;

    /* Zugang zu Instanzdaten initialisieren */
    pWindowData = (PWINDOWDATA)WinQueryWindowPtr (hwnd, QWL_USER);
    somThis = pWindowData->somThis;

    /* Ist das Fenster ikonisiert? */
    WinQueryWindowPos (_hwndFrameGraph, &swp);
    if (!(swp.fl & SWP_MINIMIZE))
        {
        _lxGraphPos = swp.x;
        _lyGraphPos = swp.y;
        }

    _wpSaveDeferred (pWindowData->somSelf);

    return;
    }

VOID GrphRepaint (HWND hwnd)
    {
    WPDCF77Data  *somThis;
    PWINDOWDATA   pWindowData;
    PUSHORT       pBuffer;
    ULONG         ulMaxIndex;
    ULONG         i;
    POINTL        ptl;

    /* Zugang zu Instanzdaten initialisieren */
    pWindowData = (PWINDOWDATA)WinQueryWindowPtr (hwnd, QWL_USER);
    somThis = pWindowData->somThis;

    /* Statistik oder Oszillogramm anzeigen?                 */
    /* => Bestimmen des Zeigers auf den Datenbereich pBuffer */
    /*    und den die Zahl der Daten ulMaxIndex              */
    if (ulGraphView == OPEN_ANTADJ)
        {
        /* Antenne einrichten */
        pBuffer    = pusAdjBuffer;
        ulMaxIndex = ulMaxPixel;
        if (isHopf603x ())
            GpiDeleteSegment (_hpsGraph, lFirstSegId);
        else
            {
            /* Wenn der Standardtimer aktiviert wurde, wird die  */
            /* Anzeige abgebrochen                               */
            DebugULd (D_STAT, "Repaint", "TimerMode", DataDCF.usTimerMode);
            if (DataDCF.usTimerMode != TIMER_MODE_HR)
                {
                WinPostMsg (hwnd, WM_CLOSE, 0L, 0L);
                return;
                }

            /* Die Grafiksegment werden in einem Ring verwaltet, */
            /* d. h. die Seg-Id lÑuft von 1 bis CMAXGRSEGMENT    */
            /* Hier wird die lNextSegId inkrementiert            */
            lNextSegId = (lNextSegId % CMAXGRSEGMENT) + 1;
            if (lNextSegId == lFirstSegId)
                {
                /* 1. Segment aus Chain lîschen, da Ring voll */
                GpiDeleteSegment (_hpsGraph, lFirstSegId);
                lFirstSegId = (lFirstSegId % CMAXGRSEGMENT) + 1;
                }
            }
        }
    else
        {
        /* QualitÑtsanzeige */
        pBuffer    = pGraphData;
        ulMaxIndex = ulIndexGrphData;
        calcTransformMatrix (hwnd);
        GpiDeleteSegment (_hpsGraph, lFirstSegId);
        }

    DebugULd (D_WND, "GrphRepaint", "ulMaxIndex", ulMaxIndex);

    /* Die Daten werden in ein retain-Grafiksegment ausgegeben */
    GpiOpenSegment (_hpsGraph, lNextSegId);
    GpiSetElementPointer (_hpsGraph, 0x7FFFFFFF);

    /* Daten ausgeben */
    GpiSetColor (_hpsGraph, CLR_DEFAULT);
    ptl.x = 0;
    ptl.y = (LONG)pBuffer[0];
    GpiMove (_hpsGraph, &ptl);
    GpiSetLineWidth (_hpsGraph, LINEWIDTH_THICK);
    for (i=1; i<ulMaxIndex && pBuffer[i]!=USHRT_MAX; i++)
        {
        ptl.x++;
        ptl.y = (LONG)pBuffer[i];
        GpiLine (_hpsGraph, &ptl);
        }

    GpiCloseSegment (_hpsGraph);

    WinInvalidateRect (_hwndClientGraph, NULL, FALSE);
    return;
    }

VOID GrphPaint (HWND hwnd)
    {
    WPDCF77Data  *somThis;
    PWINDOWDATA   pWindowData;
    ULONG         ulMaxTime;
    ULONG         ulStringId;
    POINTL        ptl;
    POINTL        ptlVwPort;
    GRADIENTL     grd;
    SIZEF         sizef;
    RECTL         rcl;
    FATTRS        fattrs;
    CHAR          sz[CCHMAXMSG];

    /* Zugang zu Instanzdaten initialisieren */
    pWindowData = (PWINDOWDATA)WinQueryWindowPtr (hwnd, QWL_USER);
    somThis = pWindowData->somThis;

    WinBeginPaint (hwnd, _hpsGraph, NULL);
    WinQueryWindowRect (hwnd, &rcl);
    WinFillRect (_hpsGraph, &rcl, CLR_WHITE);

    /* Model-Transform zurÅcksetzen */
    GpiSetModelTransformMatrix (_hpsGraph, 0, NULL, TRANSFORM_REPLACE);

    /* Rahmen zeichnen */
    GpiSetColor (_hpsGraph, CLR_RED);
    GpiSetLineWidth (_hpsGraph, LINEWIDTH_NORMAL);
    ptl.x = BORDER_X;
    ptl.y = BORDER_Y;
    GpiMove (_hpsGraph, &ptl);
    ptl.x = rcl.xRight - BORDER_XR;
    ptl.y = rcl.yTop   - BORDER_YT;
    GpiBox (_hpsGraph, DRO_OUTLINE, &ptl, 0, 0);

    /* Statistik oder Oszillogramm anzeigen? */
    if (ulGraphView == OPEN_ANTADJ)
        {
        /* Antenne einrichten */
        ulStringId = IDS_YAXESA;
        if (isHopf603x ())
            {
            ulMaxTime = 2;

            /* y-Skalierung: Differenz zwischen min/max Empfangspegel */
            ptl.x = 5;
            ptl.y = CCHGRPHMAX*9/10;
            _itoa (ulDCFDelta, sz + WinLoadString (hab, hmod, IDS_DCFDELTA, CCHMAXMSG, sz), 10);
            GpiCharStringAt (_hpsGraph, &ptl, strlen (sz), sz);
            }
        else
            ulMaxTime = 1;
        }
    else
        {
        /* QualitÑtsanzeige */
        ulStringId = IDS_YAXESQ;
        ulMaxTime  = 60 * CCHMINPERPIXEL * calcRcvQualXLen ();
        }

    /* Achsenbeschriftung */
    memset (&fattrs, 0, sizeof (FATTRS));
    fattrs.usRecordLength = sizeof (FATTRS);
    fattrs.fsFontUse = FATTR_FONTUSE_OUTLINE;
    strcpy (fattrs.szFacename, GRAPH_FONT);
    if (GpiCreateLogFont (_hpsGraph, NULL, 0, &fattrs) == FONT_MATCH)
        {
        ptlVwPort.x = rcl.xRight - rcl.xLeft - BORDER_X - BORDER_XR;
        ptlVwPort.y = rcl.yTop - rcl.yBottom - BORDER_Y - BORDER_YT;
        if (ptlVwPort.x > 0 && ptlVwPort.y > 0)
            {
            GpiSetCharMode (_hpsGraph, CM_MODE3);

            /* Beschriftung der y-Achse */
            sizef.cx = MAKEFIXED (CHHEIGHT, 0);
            sizef.cy = MAKEFIXED (CHHEIGHT, 0);
            GpiSetCharBox (_hpsGraph, &sizef);          // Zeichengrî·e
            grd.x = 0;
            grd.y = 1;
            GpiSetCharAngle (_hpsGraph, &grd);          // Zeichenorientierung
            ptl.x = 2 * BORDER_X / 3;
            ptl.y = rcl.yTop / 2;
            GpiCharStringAt (_hpsGraph, &ptl, WinLoadString (hab, hmod, ulStringId, CCHMAXMSG, sz), sz);
            ptl.y = BORDER_Y;
            GpiCharStringAt (_hpsGraph, &ptl, 1, "0");

            /* Beschriftung der x-Achse */
            sizef.cx = MAKEFIXED (CHHEIGHT, 0);
            sizef.cy = MAKEFIXED (CHHEIGHT, 0);
            GpiSetCharBox (_hpsGraph, &sizef);          // Zeichengrî·e
            grd.x = 0;
            grd.y = 0;
            GpiSetCharAngle (_hpsGraph, &grd);          // Zeichenorientierung
            ptl.x = rcl.xRight / 2;
            ptl.y = 4;
            WinLoadString (hab, hmod, IDS_XAXES, CCHMAXMSG, sz);
            if (ulMaxTime > 10)
                {
                ulMaxTime /= 3600;
                strcat (sz, "h");
                }
            else
                strcat (sz, "s");
            GpiCharStringAt (_hpsGraph, &ptl, strlen (sz), sz);
            GraphDrawTicks (_hpsGraph, ulMaxTime, &rcl);
            }
        }

    /* Transformationsmatrix fÅr das Grafiksegment setzen */
    GpiSetModelTransformMatrix (_hpsGraph, 9, &matlfGraph, TRANSFORM_REPLACE);

    /* Inhalt der Grafiksegmentes ausgeben */
    GpiDrawChain (_hpsGraph);

    WinEndPaint (_hpsGraph);
    return;
    }

VOID GrphTimer (HWND hwnd)
    {
    WPDCF77Data  *somThis;
    PWINDOWDATA pWindowData;

    pWindowData = (PWINDOWDATA)WinQueryWindowPtr (hwnd, QWL_USER);
    somThis = pWindowData->somThis;
    GetDCFStatus ();                                    // Aufrufen wegen Status-Puffer

    if (ulGraphView == OPEN_GRAPH)
        {
        if (ulTimerCntr - _ulMinuteCntr >= 60)
            {
            DebugE (D_STAT, "GrphTimer", "Fenster neu zeichnen");
            _ulMinuteCntr = ulTimerCntr;
            WinSendMsg (_hwndClientGraph, WM_REPAINT, MPVOID, MPVOID);
            }
        }
    else
        _ulMinuteCntr = 0;

    return;
    }

VOID GrphCommand (HWND hwnd, ULONG ulSource)
    {
    switch (ulSource)
        {
        case DID_CANCEL + (CMDSRC_PUSHBUTTON<<16):
            WinSendMsg (hwnd, WM_CLOSE, MPVOID, MPVOID);
            return;
        }
    return;
    }

VOID GrphClose (HWND hwnd)
    {
    WPDCF77Data *somThis;
    PWINDOWDATA  pWindowData;

    pWindowData = (PWINDOWDATA)WinQueryWindowPtr (hwnd, QWL_USER);
    somThis = pWindowData->somThis;
    ulGraphView = 0;
    somSelfGraphView = NULL;
    _wpRemove1sTimer (pWindowData->somSelf, hwnd);
    _wpDeleteFromObjUseList (pWindowData->somSelf, &pWindowData->UseItem);
    _wpFreeMem (pWindowData->somSelf, (PBYTE)pWindowData);
    WinDestroyWindow (_hwndFrameGraph);
    _hwndFrameGraph = NULLHANDLE;
    return;
    }

/*******************************************************************\
    Loop: Verzîgerungsschleife; wird als Unterprogramm ausgefÅhrt,
    damit der Optimierer an den zwei Stellen keine unterschiedliche
    Lîsungen verursacht.
    Eingang: l: Startwert fÅr Schleife
\*******************************************************************/
VOID Loop (LONG l)
    {
    do {} while (l--);
    return;
    }

/*******************************************************************\
    thAdj6036: Thread zur Aufnahme des Oszillogramms beim Antenne
    einrichten. Der Thread lÑuft in der Timecritical-PrioritÑt.
    Eingang: pIData: Zeiger auf Instanzdaten-Struktur
\*******************************************************************/
VOID _Optlink thAdj6036 (WPDCF77Data *somThis)
    {
    ULONG       ulcData, ulcParm;
    ULONG       ulc;
    ULONG       ulMin, ulMax;
    LONG        lTestLoop;
    LONG        lTimeLoop;
    LONG        lTimeFakt;
    BYTE        bParmPacket;
    DCF77_MILLI strucMilli1, strucMilli2;
    DCF77_PORT  strucPort;

    DosSetPriority (PRTYS_THREAD, PRTYC_TIMECRITICAL, 0, 0);    /* PrioritÑt hochsetzen */
    lTestLoop = 0;                                              /* Zahl der TestdurchlÑufe */
    do
        {
        /* Bestimmen der CPU-Geschwindigkeit: Es wird eine geringe Geschwindigkeit angenommen. */
        /* Ist das Resultat zu klein, wird die Schleifenvariable schrittweise erhîht           */
        lTimeLoop = TIMELOOP;
        lTimeFakt = LONG_MAX;
        do
            {
            ulcData = ulcParm = bParmPacket = 0;
            if (DosDevIOCtl (hDrvr, IOCTL_DCF77, DCF77_GETMILLI,
                             &bParmPacket, sizeof (bParmPacket), &ulcParm,
                             &strucMilli1, sizeof (DCF77_MILLI), &ulcData))
                break;
            Loop (lTimeLoop);
            ulcData = ulcParm = bParmPacket = 0;
            if (DosDevIOCtl (hDrvr, IOCTL_DCF77, DCF77_GETMILLI,
                             &bParmPacket, sizeof (bParmPacket), &ulcParm,
                             &strucMilli2, sizeof (DCF77_MILLI), &ulcData))
                break;

            lTimeFakt = strucMilli2.usValue - strucMilli1.usValue;
            DebugULd (D_STAT, "thAdj6036", "SchleifenzÑhler", lTimeLoop);
            DebugULd (D_STAT, "thAdj6036", "Differenz", lTimeFakt);
            lTimeFakt += lTimeFakt < 0 ? 999 : 0;
            DebugULd (D_STAT, "thAdj6036", "Differenz, korrigiert", lTimeFakt);
            if (lTimeFakt < 50)
                {
                DebugULd (D_STAT, "thAdj6036", "SchleifenzÑhler wird angepa·t", lTimeLoop);
                lTimeLoop <<= 2;
                if (lTimeLoop > TIMELOOP << 7)
                    lTimeFakt = LONG_MAX;                       /* Ausstieg: Zeit ist zu lang */
                }
            } while (lTimeFakt < 50);

        DosSetPriority (PRTYS_THREAD, PRTYC_REGULAR, 0, 0);     /* PrioritÑt zurÅcksetzen */

        if (lTimeLoop > TIMELOOP << 7 || lTimeFakt > 5000)
            {
            DebugE (D_STAT, "thAdj6036", "Ausstieg: Hardwareproblem");
            lTestLoop = MAXLOOP;
            break;
            }

        lTimeFakt = lTimeLoop / lTimeFakt;
        DebugULd (D_STAT, "thAdj6036", "N fÅr 1ms", lTimeFakt);

        /* 1 ms warten */
        Loop (lTimeFakt);

        /* 1 ms warten und prÅfen, ob auch 1 ms vorbei ist */
        ulcData = ulcParm = bParmPacket = 0;
        DosDevIOCtl (hDrvr, IOCTL_DCF77, DCF77_GETMILLI,
                     &bParmPacket, sizeof (bParmPacket), &ulcParm,
                     &strucMilli1, sizeof (DCF77_MILLI), &ulcData);

        Loop (lTimeFakt);
        ulcData = ulcParm = bParmPacket = 0;
        DosDevIOCtl (hDrvr, IOCTL_DCF77, DCF77_GETMILLI,
                     &bParmPacket, sizeof (bParmPacket), &ulcParm,
                     &strucMilli2, sizeof (DCF77_MILLI), &ulcData);

        /* Thread beenden, falls mittlerweile Fenster geschlossen ist */
        if (ulGraphView != OPEN_ANTADJ)
            {
            tidGraphThread = 0;
            goto Exit;
            }
        lTestLoop++;
        DebugULd (D_STAT, "thAdj6036", "SchleifendurchlÑufe", lTestLoop);
        } while ((strucMilli2.usValue - strucMilli1.usValue != 1) && (lTestLoop < MAXLOOP));

    if (lTestLoop >= MAXLOOP)
        {
        /* Entweder ist der Rechner zu schnell, zu langsam oder die Hardware nicht ok */
        DosSetPriority (PRTYS_THREAD, PRTYC_REGULAR, 0, 0);
        ulGraphView = OPEN_GRAPH;
        tidGraphThread = 0;
        WinPostMsg (_hwndClientGraph, WM_ERRHWHOPF, MPVOID, MPVOID);
        goto Exit;
        }

    /* 1ms warten */
    Loop (lTimeFakt);

    /* Start: Antenne ausrichten */
    ulcData = ulcParm = bParmPacket = 0;
    strucPort.bNum = 1;
    strucPort.ppData[0].bPortOfs = 0x37;
    strucPort.ppData[0].bValue   = 0x80;
    DosDevIOCtl (hDrvr, IOCTL_DCF77, DCF77_SETPORT,
        &bParmPacket, sizeof (bParmPacket), &ulcParm,
        &strucPort,   sizeof (DCF77_PORT),  &ulcData);

    /* Warten: CPU auf EmpfÑnger mu· Kommando verarbeiten */
    DosSleep (2000);

    while (ulGraphView == OPEN_ANTADJ)
        {
        /* Schleife: Auslesen der Daten im 1ms-Takt */
        for (ulMin=LONG_MAX, ulMax=0, ulc=0; ulc<ulMaxPixel; ulc++)
            {
            /* Werte in Puffer lesen */
            ulcData = ulcParm = bParmPacket = 0;
            DosDevIOCtl (hDrvr, IOCTL_DCF77, DCF77_GETMILLI,
                &bParmPacket, sizeof (bParmPacket), &ulcParm,
                &strucMilli1, sizeof (DCF77_MILLI), &ulcData);
            pusAdjBuffer[ulc] = CCHGRPHMAX - min (strucMilli1.usValue, CCHGRPHMAX);
            if (strucMilli1.usValue > ulMax)
                ulMax = strucMilli1.usValue;
            if (strucMilli1.usValue < ulMin)
                ulMin = strucMilli1.usValue;

            /* 1ms warten */
            Loop (lTimeFakt);
            }
        ulDCFDelta = ulMax - ulMin;

        DebugULd (D_STAT, "thAdj6036", "DCF77-Absenkung", ulMax - ulMin);
        DosSetPriority (PRTYS_THREAD, PRTYC_REGULAR, 0, 0);
        WinPostMsg (_hwndClientGraph, WM_REPAINT, MPVOID, MPVOID);
        DosSleep (400);                             // CPU fÅr andere Prozesse freigeben
        DosSetPriority (PRTYS_THREAD, PRTYC_TIMECRITICAL, 0, 0);
        }

    /* Stop: Antenne ausrichten */
    strucPort.ppData[0].bValue = 0x20;
    ulcData = ulcParm = bParmPacket = 0;
    DosDevIOCtl (hDrvr, IOCTL_DCF77, DCF77_SETPORT,
        &bParmPacket, sizeof (bParmPacket), &ulcParm,
        &strucPort,   sizeof (DCF77_PORT),  &ulcData);

Exit:
    /* Puffer wieder freigeben */
    DosFreeMem (pusAdjBuffer);
    pusAdjBuffer = NULL;

    tidGraphThread = 0;
    return;
    }

/*******************************************************************\
    thAdjExtModule: Thread zur Aufnahme des Augendiagramms beim
    Antenne einrichten.
    Eingang: pIData: Zeiger auf Instanzdaten-Struktur
\*******************************************************************/
VOID _Optlink thAdjExtModule (WPDCF77Data *somThis)
    {
    ULONG        ulcParm, ulcData, ulc;
    ULONG        ulBytePerSec;
    BYTE         bParmPacket;
    PDCF77_LEVEL pstrucLvlData;

    /* Puffer fÅr Treiberdaten allokieren */
    DosAllocMem ((PPVOID)&pstrucLvlData, CCHLVLBUF, PAG_COMMIT | PAG_READ | PAG_WRITE);

    while (ulGraphView == OPEN_ANTADJ)
        {
        /* Timer-TicklÑnge bestimmen */
        GetDCFData ();

        /* Daten vom Treiber lesen */
        pstrucLvlData->usSize = ulAdjBufferSize;
        ulcParm = bParmPacket = 0;
        ulcData = ulAdjBufferSize + sizeof (DCF77_LEVEL) - 1;
        DosDevIOCtl (hDrvr, IOCTL_DCF77, DCF77_GETLVLDATA,
                     &bParmPacket,  sizeof (bParmPacket), &ulcParm,
                     pstrucLvlData, ulcData,              &ulcData);

        /* Signal:  ƒƒƒƒø⁄ƒƒƒƒƒƒø ⁄ƒƒƒƒ                                     */
        /*              ¿Ÿ      ¿ƒŸ                                         */
        /* Falls Puffer mit negativem Pegel beginnt, positive Flanke suchen */
        ulcData = 0;
        if (pstrucLvlData->bLevel[ulcData] == 0)
            for (; ulcData < ulAdjBufferSize; ulcData++)
                if (pstrucLvlData->bLevel[ulcData])
                    break;

        /* negative Flanke im Puffer suchen, falls positive Flanke gefunden */
        if (ulcData == ulAdjBufferSize)
            continue;

        for (; ulcData < ulAdjBufferSize; ulcData++)
            if (pstrucLvlData->bLevel[ulcData] == 0)
                break;

        /* 1s aus restlichem Puffer umkopieren */
        if (DataDCF.usTicklen)
            {
            ulBytePerSec = 1000 / DataDCF.usTicklen;
            for (ulc = 0; ulc < ulBytePerSec; ulc++, ulcData++)
                {
                if (ulcData < ulAdjBufferSize)
                    pusAdjBuffer[ulc] = pstrucLvlData->bLevel[ulcData] ? 0 : (CCHGRPHMAX * 9 / 10);
                else
                    pusAdjBuffer[ulc] = USHRT_MAX;
                }
            }

        WinPostMsg (_hwndClientGraph, WM_REPAINT, MPVOID, MPVOID);
        }

    /* Puffer wieder freigeben */
    DosFreeMem (pstrucLvlData);
    DosFreeMem (pusAdjBuffer);
    pusAdjBuffer = NULL;

    tidGraphThread = 0;
    }

MRESULT EXPENTRY GraphFrameWndProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
    {
    WPDCF77 *somSelf;

    switch (msg)
        {
        case WM_HELP:
            if (SHORT1FROMMP (mp2) == CMDSRC_PUSHBUTTON && SHORT1FROMMP (mp1) == DID_HELP ||
                SHORT1FROMMP (mp2) == CMDSRC_ACCELERATOR)
                {
                somSelf = (WPDCF77 *)WinQueryWindowPtr (hwnd, QWL_USER);
                _wpDisplayHelp (somSelf, PANEL_GRAPH, HELPFILENAME);
                return 0;
                }
            break;
        }

    return (*pfnwpFrameGraphWndProc)(hwnd, msg, mp1, mp2);
    }

/*******************************************************************\
    StatusDlgProc: Window-Prozedur fÅr den Graphen
    Eingang: hwnd: Window-Handle
             msg:  Message-Typ
             mp1:  1. Parameter
             mp2:  2. Parameter
\*******************************************************************/
MRESULT EXPENTRY GraphWndProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
    {
    PWINDOWDATA   pWindowData;
    WPDCF77Data  *somThis;

    switch (msg)
        {
        case WM_CREATE:
            pWindowData = (PWINDOWDATA) mp1;
            somThis = pWindowData->somThis;
            GrphCreate (hwnd, pWindowData, &_hpsGraph, &_hdcGraph);
            return FALSE;

        case WM_SIZE:
            GrphSize (hwnd);
            return 0;

        case WM_MOVE:
            GrphMove (hwnd);
            return 0;

        case WM_REPAINT:
            GrphRepaint (hwnd);
            return 0;

        case WM_PAINT:
            GrphPaint (hwnd);
            return 0;

        case WM_1STIMER:
            GrphTimer (hwnd);
            return 0;

        case WM_COMMAND:
            GrphCommand (hwnd, SHORT1FROMMP (mp1) + (SHORT1FROMMP (mp2)<<16));
            return 0;

        case WM_ERRHWHOPF:
            MessageBox (hwnd, IDMSG_ERRHWHOPF, MBTITLE_ERROR, MB_OK | MB_ERROR | MB_MOVEABLE, 0, NULL);
            return 0;

        case WM_CLOSE:
            GrphClose (hwnd);
            break;
        }

    return WinDefWindowProc (hwnd, msg, mp1, mp2);
    }

/*******************************************************************\
 *                       Exportierte Funktionen                    *
\*******************************************************************/
/*******************************************************************\
    ClkStartThread: initialisiert den Grafik-Puffer und
    startet den Thread
    Eingang: somSelf: Zeiger auf Instanz
\*******************************************************************/
VOID ClkStartThread (WPDCF77 *somSelf)
    {
    WPDCF77Data *somThis = WPDCF77GetData (somSelf);

    /* Puffer fÅr Zeichenroutine allokieren */
    if (isHopf603x ())
        ulAdjBufferSize = CCHGRPHBUF;
    else
        {
        GetDCFData ();
        if (DataDCF.usTicklen == 0)
            return;     // DataDCF-Struktur wurde nicht gefÅllt
        ulAdjBufferSize = sizeof (USHORT) * 1000 / DataDCF.usTicklen;
        }

    ulMaxPixel = ulAdjBufferSize / sizeof (USHORT);
    DosAllocMem ((PPVOID)&pusAdjBuffer, ulAdjBufferSize, PAG_COMMIT | PAG_READ | PAG_WRITE);

    tidGraphThread = _beginthread ((PFNTHD)(isHopf603x () ? thAdj6036 : thAdjExtModule), NULL, STACKLEN, somThis);

    return;
    }

/*******************************************************************\
    ClkGraph: Frame-Window des Graph-Fensters erzeugen
    Eingang: somSelf: Zeiger auf Instanz
    return:  Frame Window handle
\*******************************************************************/
HWND ClkGraph (WPDCF77 *somSelf)
    {
    CHAR         szClkName[CCHMAXMSG];
    PWINDOWDATA  pWindowData;
    FRAMECDATA   flFrameCtlData;             /* Frame Ctl Data */
    WPDCF77Data *somThis = WPDCF77GetData (somSelf);

    WinLoadString (hab, hmod, IDS_GRAPHCLASS, CCHMAXMSG, szClkName);

    if (!WinRegisterClass (hab, szClkName, GraphWndProc,
                           CS_SIZEREDRAW | CS_MOVENOTIFY, sizeof (pWindowData)))
        return (NULLHANDLE);

    pWindowData = (PWINDOWDATA) _wpAllocMem (somSelf, sizeof (WINDOWDATA), NULL);

    if (!pWindowData)
        {
        DebugE (D_DIAG, "ClkGraph", "Fehler beim Allokieren von pWindowData");
        return NULLHANDLE;
        }

    memset ((PVOID) pWindowData, 0, sizeof (WINDOWDATA));
    pWindowData->cb      = sizeof (WINDOWDATA);
    pWindowData->somSelf = somSelf;
    pWindowData->somThis = somThis;

    /* Frame Window erzeugen */
    flFrameCtlData.cb            = sizeof (flFrameCtlData);
    flFrameCtlData.flCreateFlags = FCF_SIZEBORDER | FCF_NOBYTEALIGN | FCF_SYSMENU |
                                   FCF_MINMAX     | FCF_TITLEBAR;
    flFrameCtlData.hmodResources = hmod;
    flFrameCtlData.idResources   = IDD_GRAPH;

    _hwndFrameGraph = WinCreateWindow (
        HWND_DESKTOP,               /* Parent */
        WC_FRAME,
        NULL,                       /* Zeiger auf Window Text */
        0,                          /* Window Style */
        0, 0, 0, 0,
        NULLHANDLE,
        HWND_TOP,
        (USHORT) IDD_GRAPH,         /* Window Identifier */
        (PVOID) &flFrameCtlData,    /* pointer to buffer */
        NULL);                      /* pointer to structure with pres. params. */

    if (!_hwndFrameGraph)
        return NULLHANDLE;

    pfnwpFrameGraphWndProc = WinSubclassWindow (_hwndFrameGraph, GraphFrameWndProc);

    WinSetWindowPtr (_hwndFrameGraph, QWL_USER, (PVOID)somSelf);

    /* Client Window erzeugen */
    _hwndClientGraph = WinCreateWindow (  /* WinCreateWindow, um CtlData zu Åbergeben */
        _hwndFrameGraph,    /* Parent */
        szClkName,
        NULL,                       /* Zeiger auf Window Text */
        0,                          /* Window Style */
        0, 0, 0, 0,
        _hwndFrameGraph,
        HWND_TOP,
        (USHORT) FID_CLIENT,        /* Window Identifier */
        pWindowData,                /* CtlData */
        NULL);                      /* PresParms */

    if (!_hwndClientGraph)
        {
        WinDestroyWindow (_hwndFrameGraph);
        return NULLHANDLE;
        }

    return _hwndFrameGraph;
    }

