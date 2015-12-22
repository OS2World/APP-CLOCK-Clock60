
#include "SETADDR.HH"
#include "DCFIOCTL.H"

#define BUFFLEN 1000
#define MSGFILE "OSO001.MSG"

#define WPDCFAPP "WPDCF77"
#define WPDCFKEY "Settings"
#define PORTSTRSIZE (sizeof(DCF77_PORT) + sizeof(PORT_PAIR))

HAB     hAB                  = NULLHANDLE;         /* Anchor Block        */
HMQ     hMQ                  = NULLHANDLE;         /* Message Queue       */
HMODULE hModSETADDR          = NULLHANDLE;         /* Module Handle       */
HWND    hWndDeskTop          = NULLHANDLE;         /* Desktop Handle      */
PHWND   phWndMain            = NULL;               /* Main Window Handle  */
INT     cArgcMain;                                 /* Arguments from main */
CHAR**  pArgvMain;                                 /* Arguments from main */
HFILE   hfDriver;                                  /* Handle to driver    */

CHAR    szAppName [20];                            /* Application class name     */
CHAR    szWorkBuf [80];                            /* Working storage character buffer  */
USHORT  nWorkValue;
BOOL    card_is_6038;

PDLLINITTYPE     pDllInit         = NULL;          /* Pointer to the Dll init struct   */
PDATAOBJECTTABLE pDataObjectTable = NULL;          /* Pointer to the Data Object table */


PSZ    pszMode6036[] = { "200","240","280","2C0","300","340","380","3C0" };
USHORT usItems6036   = 8;

PSZ    pszMode6030[] = { "200","210","220","230","240","250","260","270",
                         "280","290","2A0","2B0","2C0","2D0","2E0","2F0",
                         "300","310","320","330","340","350","360","370",
                         "380","390","3A0","3B0","3C0","3D0","3E0","3F0" };
USHORT usItems6030   = 32;

PSZ    pszMode6038[] = { "0200","0240","0280","02C0","0300","0340","0380","03C0",
                         "0400","0440","0480","04C0","0500","0540","0580","05C0",
                         "0600","0640","0680","06C0","0700","0740","0780","07C0",
                         "0800","0840","0880","08C0","0900","0940","0980","09C0",
                         "0A00","0A40","0A80","0AC0","0B00","0B40","0B80","0BC0",
                         "0C00","0C40","0C80","0CC0","0D00","0D40","0D80","0DC0",
                         "0E00","0E40","0E80","0EC0","0F00","0F40","0F80","0FC0",
                         "1000","1040","1080","10C0","1100","1140","1180","11C0",
                         "1200","1240","1280","12C0","1300","1340","1380","13C0",
                         "1400","1440","1480","14C0","1500","1540","1580","15C0",
                         "1600","1640","1680","16C0","1700","1740","1780","17C0",
                         "1800","1840","1880","18C0","1900","1940","1980","19C0",
                         "1A00","1A40","1A80","1AC0","1B00","1B40","1B80","1BC0",
                         "1C00","1C40","1C80","1CC0","1D00","1D40","1D80","1DC0",
                         "1E00","1E40","1E80","1EC0","1F00","1F40","1F80","1FC0" };
USHORT usItems6038   = 120;



/***************************************************************************/
/*                                                                         */
/* Presentation Manager Program Main Body                                  */
/*                                                                         */
/* The following routine is the Presentation Manager program Main Body.    */
/* The Main Body of a PM program is concerned with associating the         */
/* application with the Presentation Manager system, creating its          */
/* message queue, registering and displaying its main window, servicing    */
/* its message queue during the time that the application is active,       */
/* and disassociating the application from PM when the user is finished    */
/* with the application. The remaining parts of this source module that    */
/* are concerned with the Presentation Manager are the application's       */
/* window procedures (main window procedure, child window procedures,      */
/* and dialog window procedures) that process the messages associated      */
/* with the application's various windows.                                 */
/*                                                                         */
/***************************************************************************/

int main (int argc, CHAR *argv[])
{
  QMSG qMsg;           /* MSG structure to store your messages             */
  PID  idProcess;      /* Process identifier for adding name to switch list*/
  TID  idThread;       /* Thread identifier                                */
  CHAR szWindowSETADDRTitle [25];
  APIRET rc;
  ULONG  ulAction, ulParmLen, ulDataLen;
  USHORT usMilli;
  DCF77_DATETIME dcf_datetime;

  /* The WinInitialize routine initializes the Presentation Manager        */
  /* facilities for use by this application and returns a handle to the    */
  /* anchor block assigned to the application by PM.                       */

  hAB = WinInitialize(0);

  if (hAB == NULLHANDLE)
    return (-3);

  /* WinCreateMsgQueue call creates a message queue for this application    */
  /* Many of an application's Presentation Manager calls require a message  */
  /* queue parameter as a part of their calling sequence. This API call     */
  /* creates a message queue for the application and returns the message    */
  /* queue handle to the application. This call includes the anchor block   */
  /* handle to identify the application and the desired size of the message */
  /* queue. The size parameter can be specified explicitly or specified     */
  /* as zero. A zero value results in the default queue size. A NULL return */
  /* value indicates the message queue could not be created.                */

  hMQ = WinCreateMsgQueue(hAB, 0);

  if (hMQ == NULLHANDLE)
    return (-4);

  /* Query desktop handle                                                  */
  hWndDeskTop = WinQueryDesktopWindow(hAB, NULLHANDLE);

  if (hWndDeskTop == NULLHANDLE)
    return (-5);

  kwInitSETADDR (hWndDeskTop, argc, argv);

  WinLoadString (hAB,                     /* Load Program name string */
                 hModSETADDR,
                 IDS_APP_NAME,
                 sizeof(szAppName),
                 szAppName);

  if (kwRegisterClasses() == FALSE)
    return (-8);


  /* The CreateWindow function creates a frame window for this             */
  /* application's top window, and set the window's size and               */
  /* location as appropriate.                                              */


  WinLoadString (hAB,
                 hModSETADDR,
                 IDS_WINDOWSETADDR_TITLE,
                 sizeof(szWindowSETADDRTitle),
                 szWindowSETADDRTitle);


  hWndWindowSETADDR = kwCreateWindow (HWND_DESKTOP,
                                       HWND_DESKTOP,
                                       (PVOID) &hWndDeskTop,
                                        szAppName,
                                        szWindowSETADDRTitle,
                                        FCF_TITLEBAR     |
                                        FCF_SYSMENU      |
                                        FCF_MINBUTTON    |
                                        FCF_DLGBORDER    |
                                        FCF_SHELLPOSITION|
                                        FCF_TASKLIST,
                                        0L,
                                        116, 65,
                                        154, 133,
                                        ID_WINDOWSETADDR,
                                        SWP_SHOW );

  if (hWndWindowSETADDR == NULLHANDLE)
    return (FALSE);

  rc = DosOpen(DEVICENAME, &hfDriver, &ulAction, 0, FILE_NORMAL,
               OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,
               OPEN_SHARE_DENYNONE | OPEN_ACCESS_READWRITE, NULL);
  if (rc)
  {
     ErrBox (hWndWindowSETADDR, MPFROM2SHORT(1, IDS_ERROR_NODRIVER), MB_OK | MB_ERROR);
     return (FALSE);
  }

  ulParmLen = 0;
  ulDataLen = sizeof(USHORT);
  rc = DosDevIOCtl(hfDriver, IOCTL_DCF77, DCF77_GETMILLI,
                   NULL, 0, &ulParmLen,
                   &usMilli, sizeof(USHORT), &ulDataLen);
  if (rc)
  {
     ErrBox (hWndWindowSETADDR, MPFROM2SHORT(1, IDS_ERROR_WRONGDRIVER), MB_OK | MB_ERROR);
     DosClose(hfDriver);
     return (FALSE);
  }
  else if (usMilli == 0xFFFF)
  {
     ErrBox (hWndWindowSETADDR, MPFROM2SHORT(1, IDS_ERROR_NOCARD), MB_OK | MB_ERROR);
     DosClose(hfDriver);
     return (FALSE);
  }

  rc = DosDevIOCtl(hfDriver, IOCTL_DCF77, DCF77_GETDATETIME,
                   NULL, 0, &ulParmLen,
                   &dcf_datetime, sizeof(dcf_datetime), &ulDataLen);

  if ((rc == 0) && (dcf_datetime.timezone == TZ_GPS))
     card_is_6038 = TRUE;
  else
     card_is_6038 = FALSE;

  /* The following two lines of program code are where this application     */
  /* spends most of its active life. The WinGetMsg call to Presentation     */
  /* Manager identifies the application by anchor block handle and provides */
  /* PM a message structure in which to place the next message for the      */
  /* application. The last three parameters of the WinGetMsg call are set   */
  /* to NULL or 0 to request that all messages for all the application's    */
  /* windows (main window, child window, dialog window) be sent to this     */
  /* program. The Presentation Manager fills the message structure with the */
  /* next message in the application's message queue and returns control to */
  /* the program. The program then calls WinDispatchMsg to dispatch the     */
  /* message to PM where it will be sent to one of the application's window */
  /* procedures by PM. When WinDispatchMsg has dispatched the message and   */
  /* returns, the program again calls WinGetMsg for the next message. If    */
  /* there are no messages in the application's message queue, WinGetMsg    */
  /* waits until one is available. For all messages, except WM_QUIT,        */
  /* WinGetMsg returns a nonzero value. The WM_QUIT message is put in the   */
  /* application's message queue when the Close function is selected from   */
  /* the main window's system menu. This message returns a zero value which */
  /* causes the WHILE loop to terminate. The WHILE loop is followed by the  */
  /* program steps to disassociate the application from the Presentation    */
  /* Manager and to terminate the program.                                  */

  while (WinGetMsg (hAB, &qMsg, 0, 0, 0))
    WinDispatchMsg (hAB, &qMsg);

  /* Perform clean up before exiting application.                           */
  DosClose(hfDriver);

  /* The following routine destroys the application's frame window (which   */
  /* also destroys its child windows), destroys its message queue, and      */
  /* disassociates the application from the Presentation Manager system.    */

  WinDestroyWindow (*phWndMain); /* Destroy the frame window                  */
  hWndWindowSETADDR = NULLHANDLE;

  WinDestroyMsgQueue (hMQ);    /* Destroy this application's message queue  */
  WinTerminate       (hAB);    /* Terminate this application's use of the   */
                               /* Presentation Manager resources            */

  /* Free all the memory allocated by the application for its data objects  */
  /* and the data object table.                                             */
  kwFreeSETADDRDataObjects (pDataObjectTable);

  return (NO_ERROR);

} /* main */




/***************************************************************************/
/*                                                                         */
/* Window message Procedure for object: WindowSETADDR                      */
/*                                                                         */
/*                                                                         */
/* This procedure provides service routines for the general PM events      */
/* (messages) that PM sends to the window, as well as the user             */
/* initiated events (messages) that are generated when the user selects    */
/* the action bar and pulldown menu controls or the corresponding          */
/* keyboard accelerators.                                                  */
/*                                                                         */
/* The SWITCH statement shown below distributes the window messages to     */
/* the respective message service routines, which are set apart by the     */
/* CASE statements. The window procedures must provide an appropriate      */
/* service routine for its end user initiated messages, as well as the     */
/* general PM messages (like the WM_CLOSE message). If a message is        */
/* sent to this procedure for which there is no programmed CASE clause     */
/* (i.e., no service routine), the message is defaulted to the             */
/* WinDefWindowProc function, where it is disposed of by PM.               */
/*                                                                         */
/***************************************************************************/

MRESULT EXPENTRY WindowSETADDRWndProc (HWND   hWnd,
                                        ULONG  Message,
                                        MPARAM Param1,
                                        MPARAM Param2)
{

  MRESULT DefResult = 0;
  PWINDOWSETADDRTYPE pWindowSETADDRData = (PWINDOWSETADDRTYPE) WinQueryWindowPtr (hWnd, QWL_USER);

  switch (Message)
  {
    case WM_CREATE:
    {
      /* The WM_CREATE message is sent to a window when an application     */
      /* requests that the window be created.  The window procedure        */
      /* for the new window receives this message after the window is      */
      /* created, but before the window becomes visible.                   */
      /* The logic below examines the data pointed to by the window        */
      /* word and extracts the callers handle if valid.                    */

      /* Setup control structure for passing window word                   */
      WINDOWDATA *pWinData = (WINDOWDATA *) Param1 ;

      if (( pWinData->pData == NULL) || (WinIsWindow(hAB, * (PHWND) pWinData->pData)) )
      {
         kwAllocMemory ( (PPVOID) &pWindowSETADDRData ,sizeof(WINDOWSETADDRTYPE));

         if (pWindowSETADDRData != NULL)
         {
            if ( pWinData->pData == NULL)
               pWindowSETADDRData->hWndCaller = *phWndMain;
            else
               pWindowSETADDRData->hWndCaller = * (PHWND) pWinData->pData;

            pWindowSETADDRData->fTransferBuffer = FALSE;
         }
         else
         {
            WinDestroyWindow (hWnd);
            break;
         }
      }
      else
      {
         pWindowSETADDRData =  (PWINDOWSETADDRTYPE) pWinData->pData;
         pWindowSETADDRData->fTransferBuffer = TRUE;
      }

      /* Set main windows frame handle for global referencing              */
      *phWndMain = WinQueryWindow(hWnd, QW_PARENT);

      WinSetWindowPtr (hWnd, QWL_USER, pWindowSETADDRData);


      /* Set the caller's handle for the Client Area Template              */
      pWindowSETADDRData->PanelCLIENTData.hWndCaller = hWnd;

      /* Load the Client Area Template                                     */
      WinSendMsg(hWnd, KW_LOAD_CAT, 0L, 0L);


      WinSetFocus (HWND_DESKTOP, WinQueryWindow(hWnd, QW_PARENT));

    } /* WM_CREATE */
    break;


    case KW_LOAD_CAT:
    {
      HWND  hWndCAT = NULLHANDLE;
      ULONG FrameStyles;
      RECTL rectFrame;
      PVOID pData   = &pWindowSETADDRData->PanelCLIENTData;
      {
       DIALOGDATA  DialogCtrlData; /* Dialog CreateParams Control structure */

       DialogCtrlData.cb = sizeof(DIALOGDATA);
       DialogCtrlData.pData = pData;            /* save pointer to Template's data  */


       /* Load the Client Area Template "PanelCLIENT"                                */
       hWndCAT = WinLoadDlg (hWnd,                               /* Parent       */
                             hWnd,                               /* Owner        */
                             PanelCLIENTDlgProc,                 /* Message Proc */
                             hModSETADDR,
                             ID_PANELCLIENT,                     /* Resource ID  */
                             &DialogCtrlData);                   /* CreateParams */


      }


      FrameStyles = (ULONG)WinQueryWindowULong(hWndCAT, QWL_STYLE);

      /* Remove any type of border that may be present on the CAT Panel          */
      FrameStyles &= ~FS_DLGBORDER;
      FrameStyles &= ~FS_SIZEBORDER;
      FrameStyles &= ~FS_BORDER;

      (ULONG)WinSetWindowULong(hWndCAT,
                               QWL_STYLE,
                               (LONG)FrameStyles);

      /* Destroy any Frame Control windows that may be present on the CAT Panel  */
      if(WinIsWindow(hAB, WinWindowFromID(hWndCAT, FID_TITLEBAR)))
        WinDestroyWindow(WinWindowFromID(hWndCAT, FID_TITLEBAR));

      if(WinIsWindow(hAB, WinWindowFromID(hWndCAT, FID_SYSMENU)))
        WinDestroyWindow(WinWindowFromID(hWndCAT, FID_SYSMENU));

      /* Destroy any Frame Control windows that may be present on the CAT Panel  */
      if(WinIsWindow(hAB, WinWindowFromID(hWndCAT, FID_MINMAX)))
        WinDestroyWindow(WinWindowFromID(hWndCAT, FID_MINMAX));

      /* Update the CAT Panel                                                    */
      WinSendMsg(hWndCAT, WM_UPDATEFRAME, 0L, 0L);


      WinQueryWindowRect (hWnd, &rectFrame);

      WinSendMsg(hWnd, WM_SIZE, 0L, MPFROM2SHORT(rectFrame.xRight, rectFrame.yTop));

      pWindowSETADDRData->hWndCAT   = hWndCAT;
      pWindowSETADDRData->pCAT_Data = &pWindowSETADDRData->PanelCLIENTData;


    } /* KW_LOAD_CAT */
    break;


    case KW_SET_CONTROLS:
    {
      /* Set Client Area Template Control(s)                               */
      WinSendMsg(pWindowSETADDRData->hWndCAT, KW_SET_CONTROLS, Param1, Param2);

    } /* KW_SET_CONTROLS */
    break;


    case KW_QUERY_CONTROLS:
    {
      /* Query Client Area Template Control(s)                              */
      WinSendMsg(pWindowSETADDRData->hWndCAT, KW_QUERY_CONTROLS, Param1, Param2);

    } /* KW_QUERY_CONTROLS */
    break;


    case WM_COMMAND:
    {
      /* The responses to menu items and client window push buttons        */
      /* are processed in this routine.                                    */
      /* The WM_COMMAND message is sent when a control has a               */
      /* event to notify its owner about.                                  */
      /* Events handled here are responses to menu bar and menu items.     */
      /* as well as to the client area controls in the window.             */
      /* Param1 contains the ID of the control causing the message.        */
      /* The message procedure performs a SWITCH on the control ID, in     */
      /* order to correctly process the message.                           */

      switch (SHORT1FROMMP(Param1))
      {
      } /* switch SHORT1FROMMP(Param1) */
    } /* WM_COMMAND */
    break;


    case WM_PAINT:       /* code to paint the window's client area         */
    {
      HPS   hPS;         /* Handle for the Presentation Space              */
      RECTL rClient;     /* Handle to rectangle formed by client area      */


      /* Obtain a handle to a cache presentation space                     */
      hPS = WinBeginPaint(hWnd, 0, 0);

      /* Determine the size of the client area                             */
      WinQueryWindowRect(hWnd, &rClient);

      /* Fill the background with the default background color             */
      WinFillRect (hPS, &rClient, CLR_BACKGROUND);

      /* return presentation space to state before WinBeginPaint           */
      WinEndPaint(hPS);
    } /* WM_PAINT */
    break;


    case WM_SIZE:
    {
      /* Resize the Template to fill the client area                       */
      WinSetWindowPos (pWindowSETADDRData->hWndCAT,
                    HWND_TOP,
                    0,0,
                    (SHORT1FROMMP(Param2)),
                    (SHORT2FROMMP(Param2)),
                    SWP_ACTIVATE | SWP_MOVE | SWP_SIZE | SWP_SHOW);

    }  /* WM_SIZE */
    break;


    case WM_MOUSEMOVE:
    {

    } /* WM_MOUSEMOVE */
    break;


    case WM_CLOSE:  /* close the window                                    */
    {
      WinSendMsg(pWindowSETADDRData->hWndCAT, KW_CLOSE, 0L, 0L);


      /* Perform the default CLOSE processing, which terminates this app   */
      DefResult = WinDefWindowProc (hWnd, Message, Param1, Param2);
      return (DefResult);
    } /* WM_CLOSE */


    case WM_DESTROY:
    {
      if (pWindowSETADDRData != NULL)
      {
        if (pWindowSETADDRData->fTransferBuffer == FALSE)
          kwFreeMemory ( (PVOID) pWindowSETADDRData);
      }

      DefResult = WinDefWindowProc (hWnd, Message, Param1, Param2);
      return (DefResult);
    } /* WM_DESTROY */


    default:
    {
      /* For any message not explicitly handled above, perform the         */
      /*  default PM processing, and return that value.                    */
      DefResult = WinDefWindowProc (hWnd, Message, Param1, Param2);
      return (DefResult);
    }

  } /* switch (Message) */

  return ((MRESULT) FALSE);

} /* WindowSETADDRWndProc  */


/***************************************************************************/
/*                                                                         */
/* Dialog message Procedure for object:PanelCLIENT                         */
/*                                                                         */
/* This procedure is associated with the dialog box that is included in    */
/* the function name of the procedure. It provides the service routines    */
/* for the events (messages) that occur because the end user operates      */
/* one of the dialog box's buttons, entry fields, or controls.             */
/*                                                                         */
/*                                                                         */
/*   CUSTOM EVENTS:                                                        */
/*                                                                         */
/*   KW_SET_DEFAULTS      - Sets defaults of each control                  */
/*   KW_SET_CONTROLS      - Sets controls of each control                  */
/*   KW_QUERY_CONTROLS    - Query contrents of each control                */
/*   KW_REQUIRED_CONTROLS - Checks for required controls                   */
/*   KW_VALIDATE_CONTROLS - Check for valiidity of data in each control    */
/*   KW_CLOSE             - Page is about to be destroyed                  */
/*                                                                         */
/***************************************************************************/

MRESULT EXPENTRY PanelCLIENTDlgProc (HWND   hWnd,
                                       ULONG  Message,
                                       MPARAM Param1,
                                       MPARAM Param2)
{
  MRESULT DefResult = 0;
  PPANELCLIENTTYPE pPanelCLIENTData = (PPANELCLIENTTYPE) WinQueryWindowPtr (hWnd, QWL_USER);
  static LONG lIndex;
  ULONG  ulParmLen, ulDataLen;
  DCF77_DATA dcf77_data;
  CHAR pszPortAddr[6];

  switch (Message)
  {

    case WM_INITDLG:
    {
      /* The WM_INITDLG message is sent to a dialog when an application        */
      /* requests that the dialog be created. The window procedure             */
      /* for the new dialog receives this message after the dialog is          */
      /* created, but before the dialog becomes visible.                       */
      /* The logic below examines the data pointed to by the window            */
      /* word and extracts the callers handle if valid.                        */

      DIALOGDATA *pDlgData = (DIALOGDATA *) Param2 ;

      if (( pDlgData->pData == NULL) || (WinIsWindow(hAB, * (PHWND) pDlgData->pData)) )
      {
        kwAllocMemory ( (PPVOID) &pPanelCLIENTData ,sizeof(PANELCLIENTTYPE));

        if (pPanelCLIENTData != NULL)
        {
          if (pDlgData->pData == NULL)
            pPanelCLIENTData->hWndCaller = *phWndMain;
          else
            pPanelCLIENTData->hWndCaller = * (PHWND) pDlgData->pData;

          pPanelCLIENTData->fTransferBuffer = FALSE;
        }
        else
        {
          WinDestroyWindow (hWnd);
          break;
        }
      }
      else
      {
        pPanelCLIENTData =  (PPANELCLIENTTYPE) pDlgData->pData;
        pPanelCLIENTData->fTransferBuffer = TRUE;
      }

      WinSetWindowPtr (hWnd, QWL_USER, pPanelCLIENTData);

      pPanelCLIENTData->sfValidate = TRUE;


      /* Initialize controls to defaults                                       */
      WinPostMsg(hWnd, KW_SET_DEFAULTS, 0L, 0L);

      /* Set controls to current values                                        */
      WinPostMsg(hWnd, KW_SET_CONTROLS, 0L, 0L);

      /* Save an instance copy of data the object values                       */
      WinPostMsg(hWnd, KW_SAVE_DATA, 0L, 0L);

    } /* WM_INITDLG */
    break;


    /* This event sets the contents of the controls                            */
    case KW_SET_DEFAULTS:
    {

      if ((SHORT1FROMMP(Param1) == DID_NEW_ADDR) || (SHORT1FROMMP(Param1) == 0))
      {
        LONG lIdx   = 0;
        LONG lValue = 0;



        /* Initialize spin button control:                                       */
        WinSendDlgItemMsg (hWnd,
                           DID_NEW_ADDR,
                           SPBM_SETCURRENTVALUE,
                           MPFROMLONG(lValue),
                           0L);
      }

      if ((SHORT1FROMMP(Param1) == DID_MODE_6036) || (SHORT1FROMMP(Param1) == 0))
      {
        LONG lIdx   = 0;
        LONG lValue = 0;

        if (card_is_6038)
        {
          /* Initialize radio button control:                                      */
          WinEnableControl  (hWnd,
                             DID_MODE_6030,
                             FALSE);
          WinEnableControl  (hWnd,
                             DID_MODE_6036,
                             FALSE);

          /* Initialize spin button control entries:                               */
          WinSendDlgItemMsg (hWnd,
                             DID_NEW_ADDR,
                             SPBM_SETARRAY,
                             pszMode6038,
                             MPFROMSHORT(usItems6038));
        }
        else
        {
          /* Initialize radio button control:                                      */
          WinSendDlgItemMsg (hWnd,
                             DID_MODE_6036,
                             BM_SETCHECK,
                             MPFROMLONG(1),
                             0L);

          /* Initialize spin button control entries:                               */
          WinSendDlgItemMsg (hWnd,
                             DID_NEW_ADDR,
                             SPBM_SETARRAY,
                             pszMode6036,
                             MPFROMSHORT(usItems6036));
        }
      }



      return((MRESULT) TRUE);

    } /* KW_SET_DEFAULTS */


    /* This event sets the contents of the controls                            */
    case KW_SET_CONTROLS:
    {

      ulParmLen = 0;
      ulDataLen = sizeof(DCF77_DATA);
      DosDevIOCtl(hfDriver, IOCTL_DCF77, DCF77_QUERYDATA,
                  NULL, 0, &ulParmLen,
                  &dcf77_data, sizeof(DCF77_DATA), &ulDataLen);

      if ((SHORT1FROMMP(Param1) == DID_NEW_ADDR) || (SHORT1FROMMP(Param1) == 0))
      {
        LONG lValue;

        lIndex = (dcf77_data.usIOAddr - 512) / 16;

        if (card_is_6038)
            lIndex %= 480;
        else
            lIndex %= 32;

        lValue = lIndex;
        if (!((SHORT) WinSendDlgItemMsg (hWnd, DID_MODE_6030, BM_QUERYCHECK, NULL, NULL)))
           lValue /= 4;

        /* Initialize spin button control:                                       */
        WinSendDlgItemMsg (hWnd,
                           DID_NEW_ADDR,
                           SPBM_SETCURRENTVALUE,
                           MPFROMLONG(lValue),
                           0L);
      }

      if ((SHORT1FROMMP(Param1) == DID_ADDRESS) || (SHORT1FROMMP(Param1) == 0))
      {
          _ltoa(dcf77_data.usIOAddr, pszPortAddr, 16);
          WinSetDlgItemText(hWnd, DID_ADDRESS, pszPortAddr);
      }

      return((MRESULT) TRUE);

    } /* KW_SET_CONTROLS */


    /* This event queries the contents of the controls                         */
    case KW_QUERY_CONTROLS:
    {


      return((MRESULT) TRUE);

    } /* KW_QUERY_CONTROLS */


    /* This event verifies the contents of child controls that require data */
    case KW_REQUIRED_CONTROLS:
    {

      return((MRESULT) TRUE);

    } /* KW_REQUIRED_CONTROLS */


    /* Validate contents of child controls                                     */
    case KW_VALIDATE_CONTROLS:
    {

      return((MRESULT) TRUE);

    } /* KW_VALIDATE_CONTROLS */


    case WM_CONTROL:
    {
      switch (SHORT1FROMMP(Param1))  /* Control id */
      {
        /* AutoRadiobutton text: "Mode 6030"                                          */
        /* AutoRadiobutton ID: DID_MODE_6030                                          */
        case DID_MODE_6030:
        {
            ErrBox (hWndWindowSETADDR, MPFROM2SHORT(1, IDS_INFO_NORETURN), MB_OK | MB_INFORMATION);
            WinSendDlgItemMsg (hWnd,
                               DID_NEW_ADDR,
                               SPBM_SETARRAY,
                               pszMode6030,
                               MPFROMSHORT(usItems6030));
            WinSendDlgItemMsg (hWnd,
                               DID_NEW_ADDR,
                               SPBM_SETCURRENTVALUE,
                               MPFROMLONG(lIndex),
                               0L);
        } /* DID_MODE_6030 */
        break;

        /* AutoRadiobutton text: "Mode 6035/6036"                                     */
        /* AutoRadiobutton ID: DID_MODE_6036                                          */
        case DID_MODE_6036:
        {
            WinSendDlgItemMsg (hWnd,
                               DID_NEW_ADDR,
                               SPBM_SETARRAY,
                               pszMode6036,
                               MPFROMSHORT(usItems6036));
            WinSendDlgItemMsg (hWnd,
                               DID_NEW_ADDR,
                               SPBM_SETCURRENTVALUE,
                               MPFROMLONG(lIndex / 4),
                               0L);
        } /* DID_MODE_6036 */
        break;

        /* Entry field DID_NEW_ADDR                                                */
        case DID_NEW_ADDR:
        {
          switch (SHORT2FROMMP(Param1)) /* switch on Notification Code  */
          {
            /* Entry field notification messages. */

            case SPBN_SETFOCUS:/* Entry field is receiving the focus */
            {
            } /* SPBN_SETFOCUS */
            break;

            case SPBN_KILLFOCUS: /* Entry field is losing the focus  */
            {
            } /* SPBN_KILLFOCUS */
            break;

            case SPBN_UPARROW:
            case SPBN_DOWNARROW: /* Arrow Buttons are pressed  */
            {
                WinSendDlgItemMsg (hWnd,
                                   DID_NEW_ADDR,
                                   SPBM_QUERYVALUE,
                                   &lIndex,
                                   MPFROM2SHORT(0,SPBQ_DONOTUPDATE));
                if ((card_is_6038)
                 || (((SHORT) WinSendDlgItemMsg (hWnd,
                                   DID_MODE_6036,
                                   BM_QUERYCHECK,
                                   NULL, NULL)) == TRUE))
                    lIndex *= 4;                          /* 40h steps in 6036 mode */

            } /* SPBN_KILLFOCUS */
            break;
          } /* switch on Notification Code  */
        } /* Entry field DID_NEW_ADDR                                              */
        break;

      } /* Control id */
    } /* WM_CONTROL */
    break;

    case WM_HELP:
    {
      ErrBox (hWndWindowSETADDR,
              MPFROM2SHORT(1, (card_is_6038) ? IDS_HELP_INFO6038 : IDS_HELP_INFO),
              MB_OK | MB_INFORMATION | MB_MOVEABLE);
      return ((MRESULT) FALSE);
    }

    case WM_COMMAND:
    {
      HWND hWndParent;
      HWND hWndGrandParent;
      CHAR szClassName   [256];

      hWndParent           = WinQueryWindow(hWnd, QW_PARENT);
      hWndGrandParent      = WinQueryWindow(hWndParent, QW_PARENT);

      WinQueryClassName (hWndGrandParent,
                         sizeof(szClassName), szClassName);

      /* The Classname of the grandparent will be WC_NOTEBOOK for Notebooks.*/
      /* This is not true for Panels. This is how we determine whether the  */
      /* "OK" button was depressed on a Notebook and client area template   */
      /* or a panel. "#40" is the string returned which is the functional   */
      /* equivalent of the defined constant WC_NOTEBOOK.                    */

      switch (SHORT1FROMMP(Param1)) /* Menu id, Push button id */
      {
        case DID_BTN_APPLY: /* Button text: "Setzen"                        */
        {
          UCHAR ucAddr;
          UCHAR buffer[PORTSTRSIZE];
          DCF77_PORT *pdcf;

          /* Calculate byte for address setting                             */
          if (card_is_6038)
            ucAddr = (lIndex + 32) / 4;
          else
          {
            ucAddr =  lIndex + 32;
            if (((SHORT) WinSendDlgItemMsg (hWnd,
                                            DID_MODE_6036,
                                            BM_QUERYCHECK,
                                            NULL, NULL)) == TRUE)
            {
                lIndex &= 0xFC;
                ucAddr &= 0xFC;
                ucAddr |= 0x80;
            }
          }
          /* Fill structure for address setting                             */
          pdcf = (PDCF77_PORT) buffer;
          pdcf->bNum = 2;
          pdcf->ppData[0].bPortOfs = 0x30;
          pdcf->ppData[0].bValue   = ucAddr;
          pdcf->ppData[1].bPortOfs = 0x37;
          pdcf->ppData[1].bValue   = 0x14;

          /* Send structure to device driver                                */
          ulParmLen = 0;
          ulDataLen = PORTSTRSIZE;
          DosDevIOCtl(hfDriver, IOCTL_DCF77, DCF77_SETPORT,
                      NULL, 0, &ulParmLen,
                      pdcf, PORTSTRSIZE, &ulDataLen);


          ulParmLen = 0;
          ulDataLen = sizeof(DCF77_DATA);
          DosDevIOCtl(hfDriver, IOCTL_DCF77, DCF77_QUERYDATA,
                      NULL, 0, &ulParmLen,
                      &dcf77_data, sizeof(DCF77_DATA), &ulDataLen);

          dcf77_data.usIOAddr = lIndex * 16 + 0x200;

          DosDevIOCtl(hfDriver, IOCTL_DCF77, DCF77_SETDATA,
                      NULL, 0, &ulParmLen,
                      &dcf77_data, sizeof(DCF77_DATA), &ulDataLen);

          PrfWriteProfileData(HINI_USERPROFILE, WPDCFAPP, WPDCFKEY,
                              &dcf77_data, sizeof(DCF77_DATA));

          /* Set controls to current values                                       */
          WinPostMsg(hWnd, KW_SET_CONTROLS, 0L, 0L);

          return ((MRESULT) FALSE);
        } /* Button text: "Setzen"                                                 */

        case DID_BTN_CANCEL: /* Button text: "Abbruch"                             */
        {
          WinSendMsg(WinWindowFromID(*phWndMain, FID_CLIENT), WM_CLOSE, NULL, NULL);

          return ((MRESULT) FALSE);
        } /* Button text: "Abbruch"                                                */
      } /* Menu id, Push button id */
    } /*WM_COMMAND */
    break;

    case WM_SYSCOMMAND:
        WinSendMsg (hWndWindowSETADDR, Message, Param1, Param2);
        break;


    case WM_CLOSE:
    {
      DefResult = WinDefDlgProc (hWnd, Message, Param1, Param2);
      return (DefResult);
    } /* WM_CLOSE */


    case WM_DESTROY:
    {
      USHORT usResult = 0;
      usResult = WinQueryWindowUShort (hWnd, QWS_FLAGS);

      if (usResult & FF_DLGDISMISSED)
        usResult = WinQueryWindowUShort (hWnd, QWS_RESULT);
      else
        usResult = FALSE;

      if (pPanelCLIENTData != NULL)
      {
        CHAR   szClassName [256];

        WinQueryClassName (pPanelCLIENTData->hWndCaller,
                         sizeof(szClassName), szClassName);

        if (strncmp (szClassName, "WC_PROCESS", 10) == 0)
        {
          if (usResult == TRUE)
            WinPostMsg (pPanelCLIENTData->hWndCaller, KW_NEXT_STEP, 0L, 0L);
          else
            WinPostMsg (pPanelCLIENTData->hWndCaller, KW_CANCEL_PROCESS, 0L, 0L);
        }

        if (pPanelCLIENTData->fTransferBuffer == FALSE)
          kwFreeMemory ( (PVOID) pPanelCLIENTData);
      }

      DefResult = WinDefDlgProc (hWnd, Message, Param1, Param2);
      return (DefResult);
    } /* WM_DESTROY */

    /* This message is sent by the owner notebook to each of its pages when the */
    /* notebook is closed.  This case is responsible for cleaning up and        */
    /* retrieving any information for this page.                                */

    case KW_CLOSE:
    {

      /* Invalidate window ptr since caller will destroy memory    */
      if (pPanelCLIENTData->fTransferBuffer == TRUE)
        WinSetWindowPtr (hWnd, QWL_USER, NULL);

        return ((MRESULT) TRUE);

    } /* KW_CLOSE */


    default:
    {
      DefResult = WinDefDlgProc (hWnd, Message, Param1, Param2);
      return (DefResult);
    }

  }
  return ((MRESULT) FALSE);

} /* PanelCLIENTDlgProc */





/***************************************************************************/
/*                                                                         */
/* kwRegisterClasses Function                                              */
/*                                                                         */
/* The following function registers the classes for all of the             */
/* components associated with this EXE.  The function                      */
/* returns TRUE if it is successful, otherwise it returns FALSE.           */
/*                                                                         */
/*                                                                         */
/* PARAMETERS: VOID                                                        */
/*                                                                         */
/* RETURN:     TRUE                                                        */
/*                                                                         */
/***************************************************************************/

APIRET kwRegisterClasses (VOID)
{
  APIRET rc = NO_ERROR;


  rc = WinRegisterClass (hAB,             /* Anchor block handle             */
                        (PCH)szAppName,   /* Name of class being registered  */
                         WindowSETADDRWndProc,      /* Window procedure for class */
                         CS_SIZEREDRAW   ,
                         8);            /* Extra bytes to reserve            */
  if (rc == FALSE)
    return (FALSE);



  return (TRUE); /* Registration of the classes was successful */
} /* kwRegisterClasses */


/***************************************************************************/
/*                                                                         */
/* kwCreateWindow Function                                                 */
/*                                                                         */
/* The following function is used to create a window (the main window,     */
/* a child window, an icon window, etc.) and set it's initial size         */
/* and position. It returns the handle to the frame window.                */
/*                                                                         */
/*                                                                         */
/* PARAMETERS:                                                             */
/*                                                                         */
/*  HWND   hWndParent,    Parent handle of the window to be created        */
/*  HWND   hWndOwner,     Owner handle of the window to be created         */
/*  PVOID  pWindowData,   Pointer to window data                           */
/*  PSZ    szClassName,   Class name of the window                         */
/*  PSZ    szTitle,       Title of the window                              */
/*  ULONG  FrameFlags,    Frame control flags for the window               */
/*  ULONG  flStyle,       Frame window style                               */
/*  INT    x,             Initial vertical origin                          */
/*  INT    y,             Initial horizontal origin                        */
/*  INT    cx,            Initial height of the window                     */
/*  INT    cy,            Initial width of the window                      */
/*  USHORT ResID,         Resource id value                                */
/*  USHORT uSizeStyle)    User defined size and location flags             */
/*                                                                         */
/*                                                                         */
/* RETURN: Window frame handle                                             */
/*         NULL if window creation error                                   */
/*                                                                         */
/***************************************************************************/

HWND kwCreateWindow (
       HWND   hWndParent,   /* Parent handle of the window to be created   */
       HWND   hWndOwner,    /* Owner handle of the window to be created    */
       PVOID  pWindowData,  /* Pointer to window data                      */
       PSZ    szClassName,  /* Class name of the window                    */
       PSZ    szTitle,      /* Title of the window                         */
       ULONG  FrameFlags,   /* Frame control flags for the window          */
       ULONG  flStyle,      /* Frame window style                          */
       INT    x,            /* Initial horizontal and vertical location    */
       INT    y,
       INT    cx,           /* Initial width and height of the window      */
       INT    cy,
       USHORT ResID,        /* Resource id value                           */
       USHORT uSizeStyle)   /* User defined size and location flags        */
{
  HPS         hPS;          /* handle to a presentation space              */
  SWP         swp;
  HWND        hWndFrame;    /* Frame window handle                         */
  HWND        hWndClient;   /* Client window handle                        */
  USHORT      rc;           /* accepts return codes from function calls    */
  USHORT      SizeStyle;    /* local window positioning options            */
  FRAMECDATA  CtlData;      /* Frame-control data                          */
  FONTMETRICS FontMetrics;  /* Font metrics data                           */
  float       Xmod, Ymod;
  WINDOWDATA  WindowCtrlData; /* Client Window Control data structure      */

  WindowCtrlData.cb = sizeof(WINDOWDATA);
  WindowCtrlData.pData = pWindowData;

  CtlData.cb            = sizeof(FRAMECDATA);
  CtlData.flCreateFlags = FrameFlags;
  CtlData.hmodResources = hModSETADDR;
  CtlData.idResources   = ResID;

  hPS = WinGetPS (HWND_DESKTOP);
  GpiQueryFontMetrics (hPS, (LONG)sizeof(FONTMETRICS), &FontMetrics);
  Xmod = (float) FontMetrics.lAveCharWidth   / (float) 4;
  Ymod = (float) FontMetrics.lMaxBaselineExt / (float) 8;
  WinReleasePS (hPS);

  /* Create the frame window                                               */
  hWndFrame = WinCreateWindow (hWndParent,  /* parent window               */
                               WC_FRAME,    /* class name                  */
                               szTitle,     /* window text                 */
                               0L,          /* window style                */
                               0, 0,        /* position (x,y)              */
                               0, 0,        /* size (width,height)         */
                               hWndOwner,   /* owner window                */
                               HWND_TOP,    /* sibling window              */
                               ResID,       /* resource id                 */
                               &CtlData,    /* control data                */
                               NULL);       /* presentation parms          */

  if (hWndFrame == NULLHANDLE)
  {
    CHAR     szErrorText   [128]; /* Buffer for error messages             */
    CHAR     szErrorFormat [128]; /* Buffer for error format string        */
    ERRORID  eidErrorCode;        /* Error code                            */

    /* Retrieve the latest error code                                      */
    eidErrorCode = WinGetLastError (hAB);

    WinLoadString (hAB,
                    hModSETADDR,
                    IDS_ERR_WINDOW_CREATE_FORMAT,
                    sizeof(szErrorFormat),
                    szErrorFormat);

    /* Format the string to include the latest error code.                 */
    sprintf (szErrorText, szErrorFormat, (ULONG)eidErrorCode);

    WinMessageBox (HWND_DESKTOP,
                   hWndParent,
                   szErrorText,
                   0,
                   0,
                   MB_OK | MB_ICONEXCLAMATION);

    return (NULLHANDLE);
  }

  /* Create the frame window                                               */
  hWndClient = WinCreateWindow (hWndFrame,      /* parent window           */
                                szClassName,    /* class name              */
                                NULL,           /* window text             */
                                0L,             /* window style            */
                                0, 0,           /* position (x,y)          */
                                0, 0,           /* size (width,height)     */
                                hWndFrame,      /* owner window            */
                                HWND_TOP,       /* sibling window          */
                                FID_CLIENT,     /* resource id             */
                                &WindowCtrlData, /* window Control Data    */
                                NULL);           /* presentation parms     */

  if (hWndClient == NULLHANDLE)
  {
    CHAR     szErrorText   [128]; /* Buffer for error messages             */
    CHAR     szErrorFormat [128]; /* Buffer for error format string        */
    ERRORID  eidErrorCode;        /* Error code                            */

    /* Retrieve the latest error code */
    eidErrorCode = WinGetLastError (hAB);

    WinLoadString (hAB,
                   hModSETADDR,
                   IDS_ERR_WINDOW_CREATE_FORMAT,
                   sizeof(szErrorFormat),
                   szErrorFormat);

    /* Format the string to include the latest error code.                 */
    sprintf (szErrorText, szErrorFormat, (ULONG) eidErrorCode);

    WinMessageBox (HWND_DESKTOP,
                   hWndParent,
                   szErrorText,
                   0,
                   0,
                   MB_OK | MB_ICONEXCLAMATION);

    return (NULLHANDLE);
  }

  /* Set size options                                                      */
  if ( (x == 0) && (y == 0) && (cx == 0) && (cy == 0) )
  {
    SizeStyle = uSizeStyle | SWP_ACTIVATE | SWP_ZORDER;
  }
  else
  {
    if ( (x == 0) && (y == 0) )
    {
      SizeStyle = uSizeStyle | SWP_ACTIVATE | SWP_SIZE;
    }
    else
    {
      if ( (cx == 0) && (cy == 0) )
      {
        SizeStyle = uSizeStyle | SWP_ACTIVATE | SWP_MOVE;
      }
      else
      {
        SizeStyle = uSizeStyle | SWP_ACTIVATE | SWP_MOVE | SWP_SIZE;
      }
    }
  }


  rc = WinSetWindowPos (hWndFrame, HWND_TOP,
                        (SHORT)(x  * Xmod),
                        (SHORT)(y  * Ymod),
                        (SHORT)(cx * Xmod),
                        (SHORT)(cy * Ymod),
                        SizeStyle);

  if (rc != TRUE)
  {
    CHAR szErrorText [80];

    WinLoadString (hAB,
                   hModSETADDR,
                   IDS_ERR_WINDOW_POS,
                   sizeof(szErrorText),
                   szErrorText);

    WinMessageBox (HWND_DESKTOP,
                   hWndParent,
                   szErrorText,
                   0,
                   0,
                   MB_OK | MB_ICONEXCLAMATION);

    return (NULLHANDLE);
  }

  /* return the handle of the newly created window to the caller.          */
  return (hWndFrame);

} /* kwCreateWindow */


/***************************************************************************/
/*                                                                         */
/*  kwCenter Function                                                      */
/*                                                                         */
/*  Centers a window/dialog on top of another window/dialog.               */
/*                                                                         */
/*                                                                         */
/*  PARAMETERS:  HWND hWnd        Handle of the window to be centered      */
/*               HWND hWndCenter  Handle of the window to be centered on   */
/*                                                                         */
/*  RETURN    :  BOOL TRUE        Successful positioning                   */
/*                    FALSE       Unsuccessful positioning                 */
/*                                                                         */
/***************************************************************************/

BOOL kwCenter (HWND hWnd,         /* handle of the window to be centered    */
               HWND hWndCenter)   /* handle of the window to center on      */
{
   LONG   X, Y;                   /* Destination points                    */
   SWP    DlgSwp, CenterSwp;      /* Set window position structures        */
   LONG   lScreenWidth, lScreenHeight;  /* Dimensions of the DESKTOP       */

   /* Query width and depth of dialog box                                  */
   WinQueryWindowPos (hWnd, (PSWP) &DlgSwp);

   /* Query width and depth of caller                                      */
   WinQueryWindowPos (hWndCenter, (PSWP) &CenterSwp);

   /* Determine the difference in center points from owner to child        */
   X = (CenterSwp.x + (CenterSwp.cx / 2)) - (DlgSwp.x + (DlgSwp.cx / 2));
   Y = (CenterSwp.y + (CenterSwp.cy / 2)) - (DlgSwp.y + (DlgSwp.cy / 2));

   /* Ajust x,y origin coordinates to new position of child                */
   X = DlgSwp.x + X;
   Y = DlgSwp.y + Y;

   /* If either x,y origin point is less than zero, then set that point to */
   /* zero so that the entire dialog box will be positioned on the desktop.*/
   /* First check the bottom and left sides.                               */
   X = (X < 0) ? 0 : X;
   Y = (Y < 0) ? 0 : Y;

   /* Query dimensions of the DESKTOP screen.                              */
   lScreenWidth = WinQuerySysValue (HWND_DESKTOP, SV_CXSCREEN);
   lScreenHeight = WinQuerySysValue (HWND_DESKTOP, SV_CYSCREEN);

   /* Second check the top and right sides against the desktop screen width
      and height.                                                          */
   if ((X + DlgSwp.cx) > lScreenWidth)
     X = lScreenWidth - DlgSwp.cx;

   X = (X < 0) ? 0 : X;

   if ((Y + DlgSwp.cy) > lScreenHeight)
     Y = lScreenHeight - DlgSwp.cy;

   /* move the dialog box to the center                                    */
   return (WinSetWindowPos (hWnd, HWND_TOP, X, Y, 0, 0, SWP_MOVE));

} /* kwCenter */


/***************************************************************************/
/*                                                                         */
/*  kwAllocMemory (PPVOID pBaseAddress, ULONG ulSize)                      */
/*                                                                         */
/*  The common function handles memory allocation for entire               */
/*  application.                                                           */
/*                                                                         */
/*  PARAMETERS: PPVOID  Pointer to variable                                */
/*              ULONG   Number of bytes to allocate                        */
/*                                                                         */
/*  RETURN:      DosAllocMem return code                                   */
/*                                                                         */
/*                 0  - No error                                           */
/*                 8  - Not enough memory                                  */
/*                 87 - Invalid parameter                                  */
/*                 93 - Interrupt                                          */
/*                                                                         */
/***************************************************************************/

APIRET kwAllocMemory (PPVOID pBaseAddress, ULONG ulObjectSize)
{
  APIRET rc;

  rc = DosAllocMem (pBaseAddress, ulObjectSize, PAG_READ | PAG_WRITE | PAG_COMMIT);

  return (rc);

} /* kwAllocMemory */


/***************************************************************************/
/*                                                                         */
/*  kwFreeMemory (PVOID pBaseAddress)                                      */
/*                                                                         */
/*  PARAMETERS: PVOID  Pointer to variable where space was                 */
/*                     allocated                                           */
/*                                                                         */
/*  RETURN:     DosAllocMem return code                                    */
/*                                                                         */
/*                 0   - No error                                          */
/*                 5   - Access denied                                     */
/*                 95  - Interrupt                                         */
/*                 487 - Invalid address                                   */
/*                                                                         */
/*  The common function handles memory de-allocation for entire            */
/*  application.                                                           */
/*                                                                         */
/***************************************************************************/

APIRET kwFreeMemory (PVOID pBaseAddress)
{
  APIRET   rc;

  rc = DosFreeMem (pBaseAddress);

  return (rc);

} /* kwFreeMemory */



/***************************************************************************/
/*                                                                         */
/* kwSearchDataObjectTable                                                 */
/*                                                                         */
/* This function is called to locate a particular data object in the       */
/* data object table. It searches the global data object table for the     */
/* pointer to the data type identified by StructureName. This data object  */
/* Will have been allocated and added to the table by either this EXE,     */
/* or by a DLL's allocation function.                                      */
/*                                                                         */
/*                                                                         */
/* PARAMETERS:  PSZ StructureName                                          */
/*                                                                         */
/* RETURN:      PVOID  rc   If sucessful then Pointer to the object        */
/*                          if failed then NULL                            */
/*                                                                         */
/***************************************************************************/

PVOID kwSearchDataObjectTable (PSZ StructureName)
{
  PDATAOBJECTTABLE pElement;
  BOOL             fFound;

  /* Check global pointer pDataObjectTable for validity. */
  if (pDataObjectTable == NULL)
    return (NULL);

  /* The Data Object table exists.  Begin traversing the table, and */
  /* stop when either the structure is found, or the terminating entry */
  /* (with an empty StructName field) is reached.                      */

  pElement = pDataObjectTable;
  fFound   = FALSE;

  while ((!fFound) && (pElement->StructName[0] != '\0'))
  {
    if (strcmp(StructureName, pElement->StructName) == 0)
      fFound = TRUE;

    /* If the object has not been located, then advance to the next     */
    /* element in the table.                                           */
    if (!fFound)
      pElement++;
  }

  /* Return either a pointer to the object, or NULL if it was not located */
  if (fFound)
    return (pElement->pDataObject);
  else
    return (NULL);
} /* kwSearchDataObjectTable */


/***************************************************************************/
/*                                                                         */
/*  kwInitSETADDR                                                          */
/*                                                                         */
/*  Initialize DataObject Table and DLL initialization structure (pDllInit)*/
/*                                                                         */
/*                                                                         */
/*  PARAMETERS:  HWND   hWnd   - Window handle                             */
/*               int    argc   - Command line argument count               */
/*               CHAR*  argv[] - Array of command line argument strings    */
/*                                                                         */
/*  RETURN:      APIRET  rc                                                */
/*                                                                         */
/***************************************************************************/

APIRET kwInitSETADDR (HWND hWnd, int argc, CHAR *argv[])
{

  /* Allocate memory for the main window's handle                          */
  if( kwAllocMemory((PVOID) &phWndMain,
                    sizeof(PHWND)) )
  {
  /* Return with error if memory allocation failed                         */
   return (-1);
  }

  /* Initialize command line argument values                               */
  cArgcMain = argc;
  pArgvMain = argv;

  /* Initialize main window handle                                         */
  *phWndMain = hWnd;


  /* Allocate memory for the initial data object table.                    */
  if ( kwAllocMemory ((PVOID) &pDataObjectTable,
                      (MAX_TABLE_ELEMENTS * sizeof(DATATABLEELEMENT))))
  {
    return (-1);
  }

  /* Allocate memory for any DLL initialization                            */
  if ( kwAllocMemory ((PVOID) &pDllInit,
                      sizeof(DLLINITTYPE)))
  {
    return (-5);
  }
  else
  {
    pDllInit->hAB              = hAB;
    pDllInit->phWndMain        = phWndMain;
    pDllInit->cArgcMain        = cArgcMain;
    pDllInit->pArgvMain        = pArgvMain;
    pDllInit->pDataObjectTable = pDataObjectTable;
  }

  if ( kwAllocateSETADDRDataObjects (pDllInit) != NO_ERROR )
    return (-6);

  return (NO_ERROR);

} /* kwInitSETADDR */


/***************************************************************************/
/*                                                                         */
/*  kwAllocateSETADDRDataObjects                                           */
/*                                                                         */
/*  Initialize Data Objects and insert structure addresses into the Data   */
/*  Object table. Also initialize linked DLLs with pDllInit.               */
/*                                                                         */
/*                                                                         */
/*  PARAMETERS:  PDLLINITTYPE pDllInit                                     */
/*                                                                         */
/*  RETURN:      APIRET  rc                                                */
/*                                                                         */
/***************************************************************************/

APIRET kwAllocateSETADDRDataObjects (PDLLINITTYPE pDllInit)
{
  CHAR              szLoadError [256];
  APIRET            rc            = NO_ERROR;
  HMODULE           hModDll       = NULLHANDLE;
  PVOID*            pElement      = NULL;
  INITPROC          InitProc      = NULL;
  PDATAOBJECTTABLE  pCurrentEntry = pDataObjectTable;

  /* For each DLL that is used by this .EXE, call the                      */
  /* associated DLL memory allocation function. Pass each function a       */
  /* pointer to the application's Data Object Table.                       */
  return (NO_ERROR);

} /* kwAllocateSETADDRDataObjects */


/***************************************************************************/
/*                                                                         */
/* kwFreeSETADDRDataObjects                                                */
/*                                                                         */
/* This function is responsible for freeing all of the memory allocated by */
/* the EXE and its associated DLLS for Data Objects.                       */
/*                                                                         */
/* Pointers to all the allocated data objects are stored in the Data Table.*/
/* Traverse the data table, and free each of the data objects.             */
/* Then, free the memory allocated for the data table itself.              */
/*                                                                         */
/*                                                                         */
/* PARAMETERS:  PDATAOBJECTTABLE pDataObjectTable                          */
/*                                                                         */
/* RETURN:      APIRET  rc                                                 */
/*                                                                         */
/***************************************************************************/

APIRET kwFreeSETADDRDataObjects (PDATAOBJECTTABLE pDataObjectTable)
{
  PDATAOBJECTTABLE  pCurrentEntry;
  SHORT             nElementCount;
  APIRET            rcTemp;
  APIRET            rc     = NO_ERROR;

  /* Start at the beginning of the table. */
  pCurrentEntry = pDataObjectTable;
  nElementCount = 0;

  /* Free each data object structure element in the data object table      */
  while ( (nElementCount < MAX_TABLE_ELEMENTS) &&
          (pCurrentEntry->StructName[0] != '\0') )
  {
    rcTemp = kwFreeMemory( (PVOID) pCurrentEntry->pDataObject);
    if (rcTemp != 0)
      rc = rcTemp;                               /* store error code but continue */
    nElementCount++;
    pCurrentEntry++;
  }

  rc = kwFreeMemory ( (PVOID) pDataObjectTable); /* free data object table  */
    if (rcTemp != 0)
      rc = rcTemp;                               /* store error code but continue */

  rc = kwFreeMemory ( (PVOID) pDllInit);         /* free dll initialization info */
    if (rcTemp != 0)
      rc = rcTemp;                               /* store error code       */

  return(rc);

} /* kwFreeSETADDRDataObjects */


/***************************************************************************/
/*                                                                         */
/*  kwDisplayMemoryAllocationError                                         */
/*                                                                         */
/*  Error handling routine for memory allocation errors.                   */
/*                                                                         */
/*  PARAMETERS:  PSZ StructName                                            */
/*                                                                         */
/*  RETURN: VOID                                                           */
/*                                                                         */
/***************************************************************************/

VOID kwDisplayMemoryAllocationError (PSZ StructName)
{
  CHAR szMsgFormat [128];
  CHAR szMessage   [256];

  WinLoadString (hAB,                     /* Load Program name string */
                 hModSETADDR,
                 IDS_ERR_MEMORY_ALLOC,
                 sizeof(szMsgFormat),
                 szMsgFormat);

  sprintf (szMessage, szMsgFormat, StructName);

  WinMessageBox (HWND_DESKTOP,
                 HWND_DESKTOP,
                 szMessage,
                 NULL,
                 0,
                 MB_OK | MB_ICONEXCLAMATION);

  return;

} /* kwDisplayMemoryAllocationError */

/***********************************************************************
    Erzeugt eine Message-Box mit einer Fehlermeldung.
    Eingang: hwnd : Owner window handle
             mpErrCode : SHORT1 : 0 => SHORT2 : DOS-Error
                         SHORT1 : 1 => SHORT2 : Fehlerstring aus .RES
 ***********************************************************************/
USHORT ErrBox (HWND hwnd, MPARAM mpErrCode, USHORT usDialogStyle)
{
    USHORT usReturnValue, usErrorClass, usErrorNum;
    ULONG ulMsgLen;
    CHAR *pszPoi, *pszSrcPtr, *pszDstPtr;

    pszPoi = pszDstPtr = pszSrcPtr = malloc(BUFFLEN);

    usErrorNum      = SHORT2FROMMP (mpErrCode);
    if (usErrorNum >= ERROR_USER_DEFINED_BASE)
    {
        usErrorClass = 1;
        usErrorNum -= ERROR_USER_DEFINED_BASE;
    }
    else
        usErrorClass = SHORT1FROMMP (mpErrCode);

    if (usErrorClass)
        WinLoadString (hAB, 0, usErrorNum, BUFFLEN, pszSrcPtr);
    else
        {
        /* Message aus OS/2-Fehlerdatei laden */
        DosGetMessage (NULL, 0, pszSrcPtr, BUFFLEN, usErrorNum, MSGFILE, &ulMsgLen);

        /* CR-LF-Sequenzen entfernen */
        do
            {
            if (*pszSrcPtr != '\n' && *pszSrcPtr != '\r')
                *(pszDstPtr++) = *pszSrcPtr;
            pszSrcPtr++;
            } while (--ulMsgLen);
        *pszDstPtr = '\0';
        }

    /* Ausgabe eines passenden Alarms */
    if (usDialogStyle & MB_INFORMATION)
        WinAlarm(HWND_DESKTOP, WA_NOTE);
    if (usDialogStyle & MB_ERROR)
        WinAlarm(HWND_DESKTOP, WA_ERROR);

    /* Ausgabe der Message */
    usReturnValue = WinMessageBox (HWND_DESKTOP, hwnd, pszPoi,
                                   (PSZ) szAppName, 0L, usDialogStyle);
    free (pszPoi);
    return usReturnValue;
}



/***************************** UNLINKED **************************************/


/*****************************************************************************/

