/***************************************************************************\
 *
 * PROGRAM NAME: WPDCF77
 * -------------
 *
 * VERSION: 6.0
 * --------
 *
 * MODULE NAME: PAINT
 * ------------
 *
 * DESCRIPTION:
 * ------------
 *   Drawing of the analog presentation of the clock
 *
 * NOTES:
 * ------
 *
 * FUNCTIONS:
 * -----------
 *   ClkDrawClock ()
 *   ClkUpdateScreen ()
 *   ClkPaint ()
 *   ClkDrawHand ()
 *   ClkDrawFace ()
 *   ClkDrawRing ()
 *   ClkDrawWS ()
 *   ClkDrawLED ()
 *   ClkDrawBatt ()
 *   ClkDrawDate ()
 *   ClkDrawTicks ()
 *   ClkDrawTrapez ()
 *   ClkDrawFullRing ()
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
#define INCL_WIN
#define INCL_GPI
#define INCL_DOSSEMAPHORES
#define INCL_DOSDATETIME

#include <os2.h>
#include <string.h>
#include <stdlib.h>
#include <cutil.h>

#include "dcfioctl.h"

#pragma info(noeff)         // suppress SOM compiler informationals

#include "wpdcf77.ih"
#include "wpdcf.h"

#include "clock.h"
#include "res.h"
#include "clkdata.h"
#include "wndproc.h"
#include "paint.h"
#include "dcf77.h"
#include "debug.h"

/*--------------------------------------------------------------*\
 *  Globale Definitionen und Variable fÅr dieses Modul          *
\*--------------------------------------------------------------*/
#define CLK_MAJORTICKS  0x0001
#define CLK_MINORTICKS  0x0002

#define XDATE   52
#define YDATE   14
#define ZDATE    1

#define COLOR_LED_SHADE     0x404040
#define COLOR_LED_SURFACE   0x606060
#define COLOR_LED_RED       0xFF0000
#define COLOR_LED_GREEN     0x00FF00
#define COLOR_BATT_RED      0xFF0000
#define COLOR_BATT_YELLOW   0xFFFF00
#define COLOR_WHITE         0xFFFFFF
#define COLOR_YELLOW        0xFFFF00

/* Transformationsmatrix fÅr Koordinatenfestlegung Ziffernblatt */
/* Matrix:  1      0      0  */
/*          0      1      0  */
/*        100    100      1  */
static MATRIXLF matlfModel =
    {
    MAKEFIXED (1, 0), MAKEFIXED (0, 0), 0,
    MAKEFIXED (0, 0), MAKEFIXED (1, 0), 0,
    100,              100,              1
    };

/* Sinus-Array; der Abstand zwischen 2 Werten betrÑgt 6¯ */
static const FIXED fxSin[60] =
    {
    0x00000000, 0x00001ac2, 0x00003539, 0x00004f1b, 0x0000681f, 0x00007fff,
    0x00009679, 0x0000ab4c, 0x0000be3e, 0x0000cf1b, 0x0000ddb3, 0x0000e9de,
    0x0000f378, 0x0000fa67, 0x0000fe98, 0x0000ffff, 0x0000fe98, 0x0000fa67,
    0x0000f378, 0x0000e9de, 0x0000ddb3, 0x0000cf1b, 0x0000be3e, 0x0000ab4c,
    0x00009679, 0x00008000, 0x00006820, 0x00004f1b, 0x00003539, 0x00001ac2,
    0x00000000, 0xffffe53e, 0xffffcac7, 0xffffb0e5, 0xffff97e1, 0xffff8001,
    0xffff6988, 0xffff54b5, 0xffff41c2, 0xffff30e5, 0xffff224d, 0xffff1622,
    0xffff0c88, 0xffff0599, 0xffff0168, 0xffff0001, 0xffff0167, 0xffff0599,
    0xffff0c88, 0xffff1622, 0xffff224d, 0xffff30e5, 0xffff41c2, 0xffff54b4,
    0xffff6987, 0xffff8000, 0xffff97e0, 0xffffb0e4, 0xffffcac6, 0xffffe53e
    };

/*******************************************************************\
    ClkDrawClock: Analoge Uhr als Bitmap erzeugen
    Eingang: hps:          Ziel-PS
             ulMode:       aktueller Zeichenmodus DCLK_*
             somThis:      Zeiger auf Instanzdaten
             pdt:          DATETIME-Struktur mit aktueller Zeit
             flRcvrStatus: Statusbyte von DCF-Treiber
\*******************************************************************/
VOID ClkDrawClock (HPS hps, ULONG ulMode, WPDCF77Data *somThis,
                PDATETIME pdt, USHORT flRcvrStatus)
    {
    BYTE  bHourAngle;
    RECTL rcl;

    /* Die Hardware ist verschwunden => Uhr neu aufbauen */
    if (ulMode == DCLK_STATUS && !isAnySource ())
        ulMode = DCLK_ALL;

    if (ulMode == DCLK_ALL)
        {
        /* Hintergrund fÅllen */
        WinQueryWindowRect (_hwndClient, &rcl);
        WinFillRect (hps, &rcl, _sclrBG.ulSurface);

        /* Transparentfarbe definieren */
        GpiSetBackColor (hps, _ulclrTransparent);

        /* Ziffernblatt zeichnen */
        ClkDrawRing (hps, somThis);

        /* Tick-Marken zeichnen */
        if (_ulPresence & PRESENCE_MAJORTICKS)
            ClkDrawTicks (hps, somThis, PRESENCE_MAJORTICKS);
        if (_ulPresence & PRESENCE_MINORTICKS && !(_ulPresence & PRESENCE_DISP_MINIMIZE))
            ClkDrawTicks (hps, somThis, PRESENCE_MINORTICKS);
        }

    /* Ziffernblatt und Zeiger */
    if (ulMode == DCLK_HANDS || ulMode == DCLK_ALL)
        {
        ClkDrawFace (hps, somThis);
        ClkDrawDate (hps, somThis, pdt);

        /* Alarm-Markierung zeichnen, wenn Alarm aktiv */
         if (_dtAlarm.usMode & AM_ACTIVE)
            {
            /* Berechnung des Winkels der Alarmmarkierung */
            bHourAngle = (_dtAlarm.dtAlarm.hours * (UCHAR)5 ) % (UCHAR)60 +
                          _dtAlarm.dtAlarm.minutes / (UCHAR)12;

            ClkDrawHand (hps, somThis, HT_ALARM, bHourAngle);
            }

        /* Berechnung des Winkels des Stundenzeigers */
        bHourAngle = (pdt->hours * (UCHAR)5 ) % (UCHAR)60 +
                      pdt->minutes / (UCHAR)12;

        ClkDrawHand (hps, somThis, HT_HOUR_SHADE, bHourAngle);
        ClkDrawHand (hps, somThis, HT_MINUTE_SHADE, pdt->minutes);
        ClkDrawHand (hps, somThis, HT_HOUR, bHourAngle);
        ClkDrawHand (hps, somThis, HT_MINUTE, pdt->minutes);
        }

    /* DCF77-Status */
    if (ulMode == DCLK_STATUS || ulMode == DCLK_ALL)
        {
        if (!(_ulPresence & PRESENCE_DISP_MINIMIZE))
            {
            ClkDrawLED  (hps, somThis, flRcvrStatus);
            ClkDrawWS   (hps, flRcvrStatus);
            ClkDrawBatt (hps, flRcvrStatus);
            }
        }

    return;
    }

/*******************************************************************\
    ClkUpdateScreen: Bitmap der analogen Uhr in Fenster kopieren
    Eingang: hps:        Ziel-PS (Screen-PS)
             somThis:    Zeiger auf Instanzdaten
             prclUpdate: Fenster-Rechteck
\*******************************************************************/
VOID ClkUpdateScreen (HPS hps, WPDCF77Data *somThis, RECTL *prclUpdate)
    {
    POINTL aptl[3];

    if (prclUpdate != NULL)
        {
        aptl[0].x = prclUpdate->xLeft;
        aptl[0].y = prclUpdate->yBottom;
        aptl[1].x = prclUpdate->xRight;
        aptl[1].y = prclUpdate->yTop;
        aptl[2].x = prclUpdate->xLeft;
        aptl[2].y = prclUpdate->yBottom;
        }
    else
        {
        WinQueryWindowRect (_hwndClient, (PRECTL)aptl);
        aptl[2].x = aptl[2].y = 0;
        }

    GpiBitBlt (hps, _hpsBuffer, 3, aptl, ROP_SRCCOPY, 0);
    return;
    }

/*******************************************************************\
    ClkPaint: gesamte Uhr erzeugen, analog und digital
    Eingang: hwnd:        Window-handle
             pWindowData: PWINDOWDATA-Struktur der Uhr
\*******************************************************************/
VOID ClkPaint (HWND hwnd, PWINDOWDATA pWindowData)
    {
    CHAR        achFinalDate[9];
    WPDCF77Data *somThis;
    RECTL       rclUpdate;
    STR8        szFamilyname;

    /* Zugang zu Instanzdaten initialisieren */
    somThis = pWindowData->somThis;

    WinBeginPaint (hwnd, _hps, &rclUpdate);
    GpiCreateLogColorTable (_hps, 0, LCOLF_RGB, 0, 0, NULL);

    if (_ulPresence & PRESENCE_DISP_ANALOG)
        {
        /* Analoge Uhr */
        memcpy (szFamilyname, _szFontFamilyDate, sizeof (STR8));
        szFamilyname[sizeof (STR8)-1] = '\0';
        GpiCreateLogFont (_hpsBuffer, &szFamilyname, 0, &_fAttrDate);

        if (_bBufferDirty)
            {
            ClkDrawClock (_hpsBuffer, DCLK_ALL, somThis, &_dt, _flRcvrStatus);
            _bBufferDirty = FALSE;
            }

        ClkUpdateScreen (_hps, somThis, &rclUpdate);

        /* Als letztes den Sekundenzeiger zeichnen, damit XOR funktioniert */
        if (_ulPresence & PRESENCE_DISP_SEC_HAND && !(_ulPresence & PRESENCE_DISP_MINIMIZE))
            ClkDrawHand(_hps, somThis, HT_SECOND, _dt.seconds);
        }
    else
        {
        /* Digitale Uhr */
        memcpy (szFamilyname, _szFontFamilyDate, sizeof (STR8));
        szFamilyname[sizeof (STR8)-1] = '\0';
        GpiCreateLogFont (_hps, &szFamilyname, LCID_DATE, &_fAttrDate);

        memcpy (szFamilyname, _szFontFamilyTime, sizeof (STR8));
        szFamilyname[sizeof (STR8)-1] = '\0';
        GpiCreateLogFont (_hps, &szFamilyname, LCID_TIME, &_fAttrTime);

        ClkDrawDigitalTime (hwnd, &_dt, DCLK_ALL);
        }

    /* Datum wird Icontext, wenn Uhr ikonisiert ist und */
    /* Datum und Uhrzeit angezeigt werden soll          */
    if (_ulPresence & PRESENCE_DISP_MINIMIZE && _ulPresence & PRESENCE_DISP_DATE)
        {
        GetDateTime (&_dt, MODE_PM | MODE_YEAR_2D, NULL, achFinalDate);
        WinSetWindowText (_hwndFrame, achFinalDate);
        }
    else
        {
        if (_szTimezone[0] != '\0')
            WinSetWindowText (_hwndFrame, _szTimezone);
        else
            WinSetWindowText (_hwndFrame, _wpQueryTitle (pWindowData->somSelf));
        }

    WinEndPaint (_hps);

    return;
    }

/*******************************************************************\
    ClkDrawHand: Zeiger zeichnen
    Eingang: hps:        Ziel-PS
             somThis:    Zeiger auf Instanzdaten
             sHandType:  Zeigertyp HT_*
             sAngle:     Darstellwinkel
\*******************************************************************/
VOID ClkDrawHand (HPS hps, WPDCF77Data *somThis, SHORT sHandType, SHORT sAngle)
    {
    static FIXED  fxWidth = MAKEFIXED (1, 0);
    static POINTL aptlHour [] =
        {{6, 0}, {0, 62}, {-6, 0}, {0, -14}, {6, 0}};
    static POINTL aptlHourLine1 [] =
        {{0L, -10L}, {0L, 56}};

    static POINTL aptlMinute [] =
        {{5, 0}, {0, 77}, {-5, 0}, {0, -14}, {5, 0}};
    static POINTL aptlMinuteLine1 [] =
        {{0L, -15L}, {0L, 72}};

    static POINTL aptlAlarm [] =
        {{0, 80}, {3, 75}, {-3, 75}, {0, 80}};

    static POINTL aptlSecond [] = {{0, -15}, {0, 74}};
    static POINTL ptlOrigin     = {0, 0};

    static LONG cptlHour   = sizeof (aptlHour)   / sizeof (POINTL);
    static LONG cptlMinute = sizeof (aptlMinute) / sizeof (POINTL);
    static LONG cptlSecond = sizeof (aptlSecond) / sizeof (POINTL);
    static LONG cptlAlarm  = sizeof (aptlAlarm)  / sizeof (POINTL);

    /* Matrix:  1      0      0  */
    /*          0      1      0  */
    /*          100    100    1  */
    MATRIXLF matlfHand =
        {
        MAKEFIXED (1, 0), MAKEFIXED (0, 0), 0,
        MAKEFIXED (0, 0), MAKEFIXED (1, 0), 0,
        100,              100,              1
        };

    /* Matrix:  1      0      0  */
    /*          0      1      0  */
    /*          3      3      1  */
    static MATRIXLF matlfShade =
        {
        MAKEFIXED (1, 0), MAKEFIXED (0, 0), 0,
        MAKEFIXED (0, 0), MAKEFIXED (1, 0), 0,
        3,               -3L,               1
        };

    /* Rotationsmatrix erzeugen und in den PS setzen */
    /* Matrix:  cos x -sin x  0 */
    /*          sin x  cos x  0 */
    /*          100    100    1 */
    matlfHand.fxM11 =
    matlfHand.fxM22 = fxSin[(sAngle + 15) % 60];
    if (_ulPresence & PRESENCE_BAVARIAN)
        {
        matlfHand.fxM21 = fxSin[(sAngle + 30) % 60];    // Uhr lÑuft rÅckwÑrts
        matlfHand.fxM12 = fxSin[sAngle];
        }
    else
        {
        matlfHand.fxM12 = fxSin[(sAngle + 30) % 60];    // Uhr lÑuft vorwÑrts
        matlfHand.fxM21 = fxSin[sAngle];
        }
    GpiSetModelTransformMatrix (hps, MATLF_SIZE, &matlfHand, TRANSFORM_REPLACE); //<<<???

    GpiSetLineWidth (hps, fxWidth);

    // angeforderten Zeiger zeichnen
    // falls der Zeiger ein Bitmap ist, gilt folgende Berechnungsgrundlage:
    // Die LÑnge der Zeiger soll von der Achse bis zur Spitze
    // unabhÑngig von der Bmp-Grî·e immer gleich gro· sein, das
    // VerhÑltnis cx/cy soll nach Streckung gleich bleiben.
    // Zeiger in Pixel: cx = hx + mx        cy = hy + my
    // in Window-Koord: Cx = Hx + Mx        Cy = Hy + My
    //
    // bekannt: cx/cy = CX/CY ; mx/hx = Mx/Hx ; my/hy = My/Hy
    //          cx, cy aus BitmapInfo
    //          hx, hy aus HotspotInfo
    //          My gegebene ZeigerlÑnge
    //
    // Hy = My*(hy/my)
    // Hx = My*(hx/my)
    // Mx = My*(mx/my)
    switch (sHandType)
        {
        case HT_HOUR:
            if (_pIData->hBmpHHand != NULLHANDLE)
                {
                double dMm;
                LONG   lColor;
                LONG   lMix;
                LONG   lMx, lHx, lHy;
                POINTL aptl[4];
                BITMAPINFOHEADER2 bmpInfo;

                lMix = GpiQueryBackMix (hps);
                GpiSetBackMix (hps, BM_SRCTRANSPARENT);

                bmpInfo.cbFix = sizeof (BITMAPINFOHEADER2);
                GpiQueryBitmapInfoHeader (_pIData->hBmpHHand, &bmpInfo);
                lHy = bmpInfo.cy - _pIData->ptlHHandHotspot.y;
                if (lHy > 0)
                    {
                    dMm = 62.0 / (double)lHy;
                    lMx = (LONG)(dMm * (double)(bmpInfo.cx - _pIData->ptlHHandHotspot.x));
                    lHx = (LONG)(dMm * (double)_pIData->ptlHHandHotspot.x);
                    lHy = (LONG)(dMm * (double)_pIData->ptlHHandHotspot.y);

                    lColor = GpiQueryColor (hps);
                    GpiSetColor (hps, 0x0);

                    aptl[0].x = -lHx;       // Tx1 inclusive
                    aptl[0].y = -lHy;       // Ty1
                    aptl[1].x = lMx;        // Tx2
                    aptl[1].y = 62;         // Ty2
                    aptl[2].x = 0;          // Sx1 non inclusive
                    aptl[2].y = 0;          // Sy1
                    aptl[3].x = bmpInfo.cx; // Sx2
                    aptl[3].y = bmpInfo.cy; // Sy2
                    GpiWCBitBlt (hps, _pIData->hBmpHHand, 4, aptl, ROP_SRCCOPY, BBO_IGNORE);
                    }
                GpiSetBackMix (hps, lMix);
                GpiSetColor (hps, lColor);
                }
            else
                {
                /* Stundenzeigerposition berechnen */
                GpiSetColor (hps, _sclrHHand.ulSurface);
                GpiBeginPath (hps, 1);
                GpiSetCurrentPosition (hps, aptlHour);
                GpiPolyLine (hps, cptlHour-1, &aptlHour[1]);
                GpiEndPath (hps);
                GpiFillPath (hps, 1, FPATH_ALTERNATE);
                GpiSetColor (hps, _sclrHHand.ulShade);
                GpiSetCurrentPosition (hps, aptlHour);
                GpiPolyLine (hps, cptlHour-1, &aptlHour[1]);
                GpiSetColor (hps, _sclrHHand.ulShade);
                GpiSetCurrentPosition (hps, aptlHourLine1);
                GpiPolyLine (hps, 1, &aptlHourLine1[1]);
                }
            break;

        case HT_HOUR_SHADE:
            if (_pIData->hBmpHHand == NULLHANDLE)
                {
                GpiSetModelTransformMatrix (hps, MATLF_SIZE, &matlfShade, TRANSFORM_ADD);  //<<<???
                GpiSetColor (hps, _sclrFace.ulShade);
                GpiBeginPath (hps, 1);
                GpiSetCurrentPosition (hps, aptlHour);
                GpiPolyLine (hps, cptlHour-1, &aptlHour[1]);
                GpiEndPath (hps);
                GpiFillPath (hps, 1, FPATH_ALTERNATE);
                }
            break;

        case HT_MINUTE:
            if (_pIData->hBmpMHand != NULLHANDLE)
                {
                double dMm;
                LONG   lColor;
                LONG   lMix;
                LONG   lMx, lHx, lHy;
                POINTL aptl[4];
                BITMAPINFOHEADER2 bmpInfo;

                lMix = GpiQueryBackMix (hps);
                GpiSetBackMix (hps, BM_SRCTRANSPARENT);

                bmpInfo.cbFix = sizeof (BITMAPINFOHEADER2);
                GpiQueryBitmapInfoHeader (_pIData->hBmpMHand, &bmpInfo);
                lHy = bmpInfo.cy - _pIData->ptlMHandHotspot.y;
                if (lHy > 0)
                    {
                    dMm = 77.0 / (double)lHy;
                    lMx = (LONG)(dMm * (double)(bmpInfo.cx - _pIData->ptlMHandHotspot.x));
                    lHx = (LONG)(dMm * (double)_pIData->ptlMHandHotspot.x);
                    lHy = (LONG)(dMm * (double)_pIData->ptlMHandHotspot.y);

                    lColor = GpiQueryColor (hps);
                    GpiSetColor (hps, 0x0);

                    aptl[0].x = -lHx;       // Tx1 inclusive
                    aptl[0].y = -lHy;       // Ty1
                    aptl[1].x = lMx;        // Tx2
                    aptl[1].y = 77;         // Ty2
                    aptl[2].x = 0;          // Sx1 non inclusive
                    aptl[2].y = 0;          // Sy1
                    aptl[3].x = bmpInfo.cx; // Sx2
                    aptl[3].y = bmpInfo.cy; // Sy2
                    GpiWCBitBlt (hps, _pIData->hBmpMHand, 4, aptl, ROP_SRCCOPY, BBO_IGNORE);
                    }
                GpiSetBackMix (hps, lMix);
                GpiSetColor (hps, lColor);
                }
            else
                {
                GpiSetColor (hps, _sclrMHand.ulSurface);
                GpiBeginPath (hps, 1);
                GpiSetCurrentPosition (hps, aptlMinute);
                GpiPolyLine (hps, cptlMinute-1, &aptlMinute[1]);
                GpiEndPath (hps);
                GpiFillPath (hps, 1, FPATH_ALTERNATE);
                GpiSetColor (hps, _sclrMHand.ulShade);
                GpiSetCurrentPosition (hps, aptlMinute);
                GpiPolyLine (hps, cptlMinute-1, &aptlMinute[1]);
                GpiSetColor (hps, _sclrMHand.ulShade);
                GpiSetCurrentPosition (hps, aptlMinuteLine1);
                GpiPolyLine (hps, 1, &aptlMinuteLine1[1]);
                }
            GpiSetColor (hps, _sclrMHand.ulShade);
            GpiSetCurrentPosition (hps, &ptlOrigin);
            GpiFullArc (hps, DRO_OUTLINEFILL, MAKEFIXED (2, 0));
            break;

        case HT_MINUTE_SHADE:
            if (_pIData->hBmpMHand == NULLHANDLE)
                {
                GpiSetModelTransformMatrix (hps, MATLF_SIZE, &matlfShade, TRANSFORM_ADD); //<<<???
                GpiSetColor (hps, _sclrFace.ulShade);
                GpiBeginPath (hps, 1);
                GpiSetCurrentPosition (hps, aptlMinute);
                GpiPolyLine (hps, cptlMinute-1, &aptlMinute[1]);
                GpiEndPath (hps);
                GpiFillPath (hps, 1, FPATH_ALTERNATE);
                }
            break;

        case HT_SECOND:
            /* im XOR-Mixmode zeichnen, um Zeiger wieder entfernen zu kînnen */
            GpiSetMix (hps, FM_INVERT);
            GpiSetCurrentPosition (hps, aptlSecond);
            GpiPolyLine (hps, cptlSecond-1, aptlSecond+1);
            GpiSetMix (hps, FM_OVERPAINT);
            break;

        case HT_ALARM:
            GpiSetColor (hps, 0xFFFFFF);
            GpiBeginPath (hps, 1);
            GpiSetCurrentPosition (hps, aptlAlarm);
            GpiPolyLine (hps, cptlAlarm-1, &aptlAlarm[1]);
            GpiEndPath (hps);
            GpiFillPath (hps, 1, FPATH_ALTERNATE);
            GpiSetColor (hps, 0x000000);
            GpiSetCurrentPosition (hps, aptlAlarm);
            GpiPolyLine (hps, cptlAlarm-1, &aptlAlarm[1]);
            break;
        }

    return;
    }

/*******************************************************************\
    ClkDrawFace: Zeichnen des Ziffernblattes
    Eingang: hps:        Ziel-PS
             somThis:    Zeiger auf Instanzdaten
\*******************************************************************/
VOID ClkDrawFace (HPS hps, WPDCF77Data *somThis)
    {
    static POINTL ptlFace = { 0,  0L};

    /* Zentrum bei (100, 100); Ziffernblatt zeichnen */
    GpiSetModelTransformMatrix (hps, MATLF_SIZE, &matlfModel, TRANSFORM_REPLACE);
    GpiSetColor (hps, _sclrFace.ulSurface);
    DebugULx (D_WND, "DrawFace", "bmp-Handle", _pIData->hBmpFace);

    if (_pIData->hBmpFace != NULLHANDLE)
        {
        LONG   lBackColor;
        POINTL aptl[4];
        BITMAPINFOHEADER2 bmpInfo;

        GpiBeginPath (hps, 1L);
        GpiSetCurrentPosition (hps, &ptlFace);
        GpiFullArc (hps, DRO_OUTLINE, MAKEFIXED (80, 0));
        GpiEndPath (hps);
        GpiSetClipPath (hps, 1, SCP_ALTERNATE | SCP_AND);

        bmpInfo.cbFix = sizeof (BITMAPINFOHEADER2);
        GpiQueryBitmapInfoHeader (_pIData->hBmpFace, &bmpInfo);
        aptl[0].x = -80;                // Tx1 inclusive
        aptl[0].y = -80;                // Ty1
        aptl[1].x = 80;                 // Tx2
        aptl[1].y = 80;                 // Ty2
        aptl[2].x = 0;                  // Sx1 non inclusive
        aptl[2].y = 0;                  // Sy1
        aptl[3].x = bmpInfo.cx;         // Sx2
        aptl[3].y = bmpInfo.cy;         // Sy2

        lBackColor = GpiQueryBackColor (hps);
        GpiSetBackColor (hps, 0x0);
        GpiWCBitBlt (hps, _pIData->hBmpFace, 4, aptl, ROP_SRCCOPY, BBO_IGNORE);
        GpiSetBackColor (hps, lBackColor);
        GpiSetClipPath (hps, 0, SCP_ALTERNATE);
        }
    else
        {
        GpiSetCurrentPosition (hps, &ptlFace);
        GpiFullArc (hps, DRO_OUTLINEFILL, MAKEFIXED (80, 0));
        }

    return;
    }

/*******************************************************************\
    ClkDrawRing: Zeichnen des Ziffernblatt-Ringes
    Eingang: hps:        Ziel-PS
             somThis:    Zeiger auf Instanzdaten
\*******************************************************************/
VOID ClkDrawRing (HPS hps, WPDCF77Data *somThis)
    {
    static POINTL ptlLight=   {-2L,  2L};
    static POINTL ptlShade=   { 1L, -1L};
    static POINTL ptlFace =   { 0L,  0L};
    static POINTL ptlShadeIn= {-3L,  3L} ;
    static POINTL ptlLightIn= { 1L, -1L} ;

    /* Zeichnung auf (100, 100) zentrieren */
    GpiSetModelTransformMatrix (hps, MATLF_SIZE, &matlfModel, TRANSFORM_REPLACE);

    ClkDrawFullRing (hps,
                  &ptlShade,
                  MAKEFIXED (95, 0),
                  MAKEFIXED (99, 0),
                  _sclrRing.ulShade);

    ClkDrawFullRing (hps,
                  &ptlLight,
                  MAKEFIXED (95, 0),
                  MAKEFIXED (98, 0),
                  _sclrRing.ulLight);

    ClkDrawFullRing (hps,
                  &ptlLightIn,
                  MAKEFIXED (88, 0),
                  MAKEFIXED (94, 0),
                  _sclrRing.ulLight);

    ClkDrawFullRing (hps,
                  &ptlShadeIn,
                  MAKEFIXED (86, 0),
                  MAKEFIXED (93, 0),
                  _sclrRing.ulShade);

    ClkDrawFullRing (hps,
                  &ptlFace,
                  MAKEFIXED (94, 0),
                  MAKEFIXED (98, 0),
                  _sclrRing.ulSurface);

    GpiSetColor (hps, _sclrFace.ulSurface);
    GpiSetCurrentPosition (hps, &ptlFace);
    GpiFullArc (hps, DRO_OUTLINEFILL, MAKEFIXED (91, 0));

    return;
    }

/*******************************************************************\
    ClkDrawWS: Zeichnen der Winter-/Sommerzeit Statusanzeige
    Eingang: hps:          Ziel-PS
             flRcvrStatus: Status von DCF-Treiber
\*******************************************************************/
VOID ClkDrawWS (HPS hps, USHORT flRcvrStatus)
    {
    LONG   i;
    USHORT usAngle;
    static POINTL ptlCenter = {84, -84};
    static FIXED  fxRad   = MAKEFIXED (5, 0);
    static FIXED  fxWidth = MAKEFIXED (2, 0);
    static POINTL aptlS[8] = {{69,-84}, {99,-84},   /* Daten fÅr Sonne */
                              {73,-73}, {95,-95},
                              {84,-69}, {84,-99},
                              {95,-73}, {73,-95}};
    static POINTL aptlW[5] = {{ 0,  0}, { 0, 10},   /* Daten fÅr Flocke */
                              {-3,  8}, { 0,  6}, { 3,  8}};
    /* Transformationsmatrix fÅr die Rotation der Flocke */
    /* Matrix:  cos x  -sin x  0 */
    /*          sin x   cos x  0 */
    /*          100     100    1 */
    MATRIXLF matlfFlake =
        {
        MAKEFIXED (1, 0), MAKEFIXED (0, 0), 0,
        MAKEFIXED (0, 0), MAKEFIXED (1, 0), 0,
        184,              16,               1
        };

    /* Nur zeichnen, wenn TrÑger und Hardware vorhanden sind */
    if (flRcvrStatus & STATUS_CARRIER   &&
        flRcvrStatus & STATUS_TIMEVALID &&
        flRcvrStatus & (STATUS_TZONE1 | STATUS_TZONE2) &&
        StatDCF.flMaskST & MASK_ST_TZONE1 &&
        isAnySource ())
        {
        if (flRcvrStatus & STATUS_TZONE2)
            {
            /* Es ist Winterzeit */
            GpiSetColor (hps, COLOR_WHITE);
            for (usAngle = 0; usAngle < 60; usAngle += 10)
                {
                /* Rotationsmatrix setzen */
                matlfFlake.fxM11 =
                matlfFlake.fxM22 = fxSin[(usAngle + 15) % 60];
                matlfFlake.fxM12 = fxSin[(usAngle + 30) % 60];
                matlfFlake.fxM21 = fxSin[usAngle];
                GpiSetModelTransformMatrix (hps, MATLF_SIZE, &matlfFlake, TRANSFORM_REPLACE); //<<<???

                GpiSetLineWidth (hps, fxWidth);
                GpiMove (hps, aptlW);
                GpiLine (hps, aptlW+1);
                GpiMove (hps, aptlW+2);
                GpiPolyLine (hps, 2, aptlW+3);
                GpiSetLineWidth (hps, LINEWIDTH_NORMAL);
                }
            }
        else
            {
            /* Es ist Sommerzeit */
            GpiSetModelTransformMatrix (hps, MATLF_SIZE, &matlfModel, TRANSFORM_REPLACE);

            GpiSetColor (hps, COLOR_YELLOW);
            GpiMove (hps, &ptlCenter);
            GpiFullArc (hps, DRO_OUTLINEFILL, fxRad);
            for (i=0; i<4; i++)
                {
                GpiMove (hps, &aptlS[2*i]);
                GpiLine (hps, &aptlS[2*i+1]);
                }
            }
        }

    return;
    }

/*******************************************************************\
    ClkDrawBatt: Zeichnen der Batterie-Statusanzeige
    Eingang: hps:          Ziel-PS
             flRcvrStatus: Status von DCF-Treiber
\*******************************************************************/
VOID ClkDrawBatt (HPS hps, USHORT flRcvrStatus)
    {
    static FIXED  fxWidth = MAKEFIXED (2, 0);
    static POINTL aptlB[4] = {{-75,84}, {-75,93}, {-93,93}, {-93,84}};
    static POINTL aptlC[4] = {{-75,87}, {-72,87}, {-72,90}, {-75,90}};
    static POINTL aptlD[2] = {{-93,81}, {-75,96}};

    /* Nur zeichnen, wenn Hardware vorhanden ist */
    if (isAnySource () && StatDCF.flMaskST & MASK_ST_BATT &&
        flRcvrStatus & STATUS_BATT)
        {
        /* Batterie zeichnen */
        GpiSetModelTransformMatrix (hps, MATLF_SIZE, &matlfModel, TRANSFORM_REPLACE);
        GpiSetLineWidth (hps, fxWidth);
        GpiSetColor (hps, COLOR_BATT_RED);
        GpiMove (hps, aptlB+3);
        GpiPolyLine (hps, 4, aptlB);
        GpiMove (hps, aptlC);
        GpiPolyLine (hps, 3, aptlC+1);
        GpiSetColor (hps, COLOR_BATT_YELLOW);
        GpiMove (hps, aptlD);
        GpiLine (hps, aptlD+1);
        }

    return;
    }

/*******************************************************************\
    ClkDrawLED: Zeichnen der Status-Zweifarb-LED
    Eingang: hps:          Ziel-PS
             somThis:      Zeiger auf Instanzdaten
             flRcvrStatus: Status von DCF-Treiber
\*******************************************************************/
VOID ClkDrawLED (HPS hps, WPDCF77Data *somThis, USHORT flRcvrStatus)
    {
    LONG clrLED;
    static FIXED  fxRadIn    = MAKEFIXED (4, 0);
    static FIXED  fxRadOut   = MAKEFIXED (5, 0);
    static FIXED  fxRadPt    = MAKEFIXED (0, 0x8000);
    static FIXED  fxRadLight = MAKEFIXED (3, 0);
    static POINTL ptlCenter = { 0, 0};      /* Mitte der LED */
    static POINTL ptl       = {-2, 0};      /* Verschiebung durch Perspektive */
    static POINTL ptlLight  = {-2, 0};      /* Mitte des hellen "Viertels" */
    /* Matrix:  0.707 -0.707 0  */
    /*          0.707  0.707 0  */
    /*         -90    -90    1  */
    static MATRIXLF matlfShade =
        {
        MAKEFIXED ( 0, 0xB504), MAKEFIXED (-1, 0x4AFC), 0,
        MAKEFIXED ( 0, 0xB504), MAKEFIXED ( 0, 0xB504), 0,
        -84,                   -84,                     1
        };

    /* LED nicht zeichnen, wenn kein DCF-Treiber vorhanden */
    if (!isAnySource ())
        return;

    GpiSetModelTransformMatrix (hps, MATLF_SIZE, &matlfModel, TRANSFORM_REPLACE);
    GpiSetModelTransformMatrix (hps, MATLF_SIZE, &matlfShade, TRANSFORM_PREEMPT); //<<<???

    if (!(flRcvrStatus & STATUS_CARRIER))
        clrLED = COLOR_LED_SHADE;
    else if (flRcvrStatus & STATUS_TIMEVALID)
        clrLED = COLOR_LED_GREEN;
    else
        clrLED = COLOR_LED_RED;

    /* Au·enring wird Clip Path */
    GpiBeginPath (hps, 1);
    GpiMove (hps, &ptlCenter);
    GpiFullArc (hps, DRO_OUTLINE, fxRadOut);
    GpiFullArc (hps, DRO_OUTLINE, fxRadIn);
    GpiEndPath (hps);
    GpiSetClipPath (hps, 1, SCP_ALTERNATE | SCP_AND);
    GpiSetColor (hps, _sclrBG.ulLight);
    GpiFullArc (hps, DRO_FILL, fxRadOut);
    GpiMove (hps, &ptl);
    GpiSetColor (hps, _sclrBG.ulShade);
    GpiFullArc (hps, DRO_FILL, fxRadOut);
    GpiSetClipPath (hps, 0, 0);

    /* LED zeichnen */
    GpiBeginPath (hps, 1);
    GpiMove (hps, &ptlCenter);
    GpiFullArc (hps, DRO_OUTLINE, fxRadIn);
    GpiEndPath (hps);
    GpiSetClipPath (hps, 1, SCP_ALTERNATE | SCP_AND);
    GpiSetColor (hps, clrLED);
    GpiFullArc (hps, DRO_FILL, fxRadIn);

    /* helle Seite der LED zeichnen, falls kein TrÑger */
    /* vorhanden oder die Uhrzeit falsch ist           */
    if (!(flRcvrStatus & STATUS_CARRIER))
        {
        GpiSetColor (hps, COLOR_LED_SURFACE);
        GpiMove (hps, &ptlLight);
        GpiFullArc (hps, DRO_FILL, fxRadLight);
        }

    /* heller Punkt */
    GpiMove (hps, &ptl);
    GpiSetColor (hps, COLOR_WHITE);
    GpiFullArc (hps, DRO_FILL, fxRadPt);

    GpiSetClipPath (hps, 0, 0);

    return;
    }

/*******************************************************************\
    ClkDrawDate: Darstellung des Datums
    Eingang: hps:          Ziel-PS
             somThis:      Zeiger auf Instanzdaten
             pdt:          DATETIME-Struktur mit aktuellem Datum
\*******************************************************************/
VOID ClkDrawDate (HPS hps, WPDCF77Data *somThis, PDATETIME pdt)
    {
    CHAR    achFinalDate[9];

    static RECTL  rcl = {ZDATE, 3*ZDATE, XDATE-ZDATE, YDATE-2*ZDATE};   // Textposition
    static POINTL aptlDateShade[] = {{0L,          0L},                 // Dunkle Kante
                                     {0L,          YDATE },
                                     {XDATE,       YDATE },
                                     {XDATE-ZDATE, YDATE-ZDATE},
                                     {ZDATE,       YDATE-ZDATE},
                                     {ZDATE,       ZDATE}};
    static POINTL aptlDateLight[] = {{0L,          0L},                 // Helle Kante
                                     {XDATE,       0L},
                                     {XDATE,       YDATE},
                                     {XDATE-ZDATE, YDATE-ZDATE},
                                     {XDATE-ZDATE, ZDATE},
                                     {ZDATE,       ZDATE}};
    static POINTL aptlDateBG[]    = {{0L ,         0L},                 // Hintergrund
                                     {XDATE,       YDATE}};

    if (_ulPresence & PRESENCE_DISP_DATE)
        {
        if (!(_ulPresence & PRESENCE_DISP_MINIMIZE))
            {
            GetDateTime (pdt, MODE_PM | MODE_YEAR_2D, NULL, achFinalDate);

            GpiSetModelTransformMatrix (hps, MATLF_SIZE, &_vmatlfDateTrans, TRANSFORM_REPLACE); //<<<???
            GpiSetModelTransformMatrix (hps, MATLF_SIZE, &_vmatlfDateScale, TRANSFORM_ADD); //<<<???

            /* Hintergrund vorbereiten */
            GpiSetColor (hps, _sclrDate.ulSurface);
            GpiSetCurrentPosition (hps, &aptlDateBG[0]);
            GpiBox (hps, DRO_FILL, &aptlDateBG[1], 0L, 0L);

            /* Datum darstellen */
            GetDateTime (pdt, MODE_PM | MODE_YEAR_2D, NULL, achFinalDate);
            ClkDrawString (hps, _ulclrDateText, achFinalDate, &rcl);

            /* Rahmen darstellen */
            GpiSetColor (hps, _sclrDate.ulShade);
            GpiSetCurrentPosition (hps, aptlDateShade);
            GpiBeginPath (hps, 1L);
            GpiPolyLine (hps, 5L, &(aptlDateShade[1]));
            GpiCloseFigure (hps);
            GpiEndPath (hps);
            GpiFillPath (hps, 1L, FPATH_ALTERNATE);

            GpiSetColor (hps, _sclrDate.ulLight);
            GpiSetCurrentPosition (hps, aptlDateLight);
            GpiBeginPath (hps, 1L);
            GpiPolyLine (hps, 5L, &(aptlDateLight[1]));
            GpiCloseFigure (hps);
            GpiEndPath (hps);
            GpiFillPath (hps, 1L, FPATH_ALTERNATE);
            }
        }

    return;
    }

/*******************************************************************\
    ClkDrawTicks: Darstellung der Ticks
    Eingang: hps:     Ziel-PS
             somThis: Zeiger auf Instanzdaten
             usTicks: PRESENCE_*TICKS-Flags
\*******************************************************************/
VOID ClkDrawTicks (HPS hps, WPDCF77Data *somThis, USHORT usTicks)
    {
    USHORT usAngle, usTrapez;

    /* Transformationsmatrix fÅr die Rotation der Ticks */
    /* Matrix:  cos x  -sin x  0 */
    /*          sin x   cos x  0 */
    /*          100     100    1 */
    MATRIXLF matlfTicks =
        {
        MAKEFIXED (1, 0), MAKEFIXED (0, 0), 0,
        MAKEFIXED (0, 0), MAKEFIXED (1, 0), 0,
        100,              100,              1
        };

    /* Aussehen der Ticks definieren */
    static BYTE   aclr[12] =
        {
        0x06,   /* 0110   12*/      /* 1 Bit pro Seite, 1=hell, 0=dunkel */
        0x07,   /* 0111    1*/
        0x03,   /* 0011    2*/
        0x03,   /* 0011    3*/
        0x0B,   /* 1011    4*/
        0x09,   /* 1001    5*/
        0x09,   /* 1001    6*/
        0x09,   /* 1001    7*/
        0x0D,   /* 1101    8*/
        0x0C,   /* 1100    9*/
        0x0C,   /* 1100    9*/
        0x06    /* 0110   11*/
        };
    static  POINTL aptlMT[4][4] =
        {
        {{-3, 81}, {-1, 83}, { 1, 83}, { 3, 81}},       /* unten  */
        {{-3, 81}, {-1, 83}, {-1, 87}, {-3, 89}},       /* links  */
        {{-3, 89}, {-1, 87}, { 1, 87}, { 3, 89}},       /* oben   */
        {{ 3, 89}, { 1, 87}, { 1, 83}, { 3, 81}}        /* rechts */
        };
    static POINTL aptlMajorTickShadow[] = {{-1, 83}, {1, 87}};
    static POINTL aptlMinorTick[]       = {{ 0, 83}, {0, 85}};
    static FIXED  fxWidth = MAKEFIXED (1, 0);

    GpiSetLineWidth (hps, fxWidth);

    /* Zeichnen der Stunden-Ticks */
    if (usTicks & PRESENCE_MAJORTICKS)
        for (usAngle = 0; usAngle < 60; usAngle += 5)
            {
            /* Rotationsmatrix setzen */
            matlfTicks.fxM11 =
            matlfTicks.fxM22 = fxSin[(usAngle + 15) % 60];
            matlfTicks.fxM12 = fxSin[(usAngle + 30) % 60];
            matlfTicks.fxM21 = fxSin[usAngle];
            GpiSetModelTransformMatrix (hps, MATLF_SIZE, &matlfTicks, TRANSFORM_REPLACE); //<<<???

            /* Tick zeichnen */
            for (usTrapez = 0; usTrapez < 4; usTrapez++)
                ClkDrawTrapez (hps, aptlMT[usTrapez],
                    aclr[usAngle/5] & (1<<usTrapez) ?
                        _sclrMajorTicks.ulLight : _sclrMajorTicks.ulShade);
            GpiSetColor (hps, _sclrMajorTicks.ulSurface);
            GpiSetCurrentPosition (hps, &aptlMajorTickShadow[0]);
            GpiBox (hps, DRO_FILL, &aptlMajorTickShadow[1], 0, 0);
            }

    /* Zeichnen der Minutenticks */
    if (usTicks & PRESENCE_MINORTICKS)
        {
        GpiSetColor (hps, _sclrMinorTicks.ulShade);
        for (usAngle = 0; usAngle < 60; usAngle++)
            {
            if ((usAngle % 5) != 0)
                {
                /* Rotationsmatrix setzen */
                matlfTicks.fxM11 =
                matlfTicks.fxM22 = fxSin[(usAngle + 15) % 60];
                matlfTicks.fxM12 = fxSin[(usAngle + 30) % 60];
                matlfTicks.fxM21 = fxSin[usAngle];
                GpiSetModelTransformMatrix (hps, MATLF_SIZE, &matlfTicks, TRANSFORM_REPLACE); //<<<???

                /* Tick zeichnen */
                GpiSetCurrentPosition (hps, &aptlMinorTick[0]);
                GpiBox (hps, DRO_OUTLINEFILL, &aptlMinorTick[1], 0, 0);
                }
            }
        }
    }

/*******************************************************************\
    ClkDrawTrapez: Trapez fÅr Stundentick zeichnen
    Eingang: hps:   Ziel-PS
             aptl:  Zeiger auf Polygonzug
             color: RGB-Wert
\*******************************************************************/
VOID ClkDrawTrapez (HPS hps, POINTL *aptl, LONG color)
    {
    GpiSetColor (hps, color);
    GpiBeginPath (hps, 1L);                 /* Pfad beginnen */
    GpiSetCurrentPosition (hps, aptl);
    GpiPolyLine (hps, 3, &aptl[1]);         /* Drei Seiten zeichnen */
    GpiCloseFigure (hps);                   /* Dreieck schlie·en */
    GpiEndPath (hps);                       /* Pfad beenden */
    GpiFillPath (hps, 1L, FPATH_ALTERNATE); /* Pfad zeichnen */

    return;
    }

/*******************************************************************\
    ClkDrawFullRing: Ring zeichnen
    Eingang: hps:        Ziel-PS
             pptlCenter: Koordinaten des Mittelpunktes
             fxRadIn:    Innenradius
             fxRadOut:   Au·enradius
\*******************************************************************/
VOID ClkDrawFullRing (HPS hps, PPOINTL pptlCenter, FIXED fxRadIn, FIXED fxRadOut,
                      LONG lColor)
    {
    GpiSetColor (hps,lColor);
    GpiSetCurrentPosition (hps, pptlCenter);
    GpiBeginPath (hps, 1);
    GpiFullArc (hps, DRO_OUTLINE, fxRadIn);
    GpiFullArc (hps, DRO_OUTLINE, fxRadOut);
    GpiCloseFigure (hps);
    GpiEndPath (hps);
    GpiFillPath (hps, 1, FPATH_ALTERNATE);

    return;
    }

