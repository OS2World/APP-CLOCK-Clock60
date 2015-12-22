#if !defined (SETADDR_HH)
  #define     SETADDR_HH

#define FILENAME_SIZE                                        256


#ifndef EXTERN
#define  EXTERN
#endif 
#define  INCL_PM
#define  INCL_BASE

#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* SETADDR includes...                                                      */
#include "SETADDR.RCH"

#include "SETADDR.H"
 

/* Maximum number of elements for pDataObjectTable. */
#define MAX_TABLE_ELEMENTS                                  1000    

#define KW_CLOSE                                           (WM_USER + 0)
#define KW_NOTEBOOK_CLOSE                                  (KW_CLOSE)
#define KW_NEXT_STEP                                       (WM_USER + 1)
#define KW_CANCEL_PROCESS                                  (WM_USER + 2)
#define KW_SET_CONTROLS                                    (WM_USER + 3)
#define KW_SET_DEFAULTS                                    (WM_USER + 4)
#define KW_SAVE_DATA                                       (WM_USER + 5)
#define KW_RESTORE_DATA                                    (WM_USER + 6)
#define KW_QUERY_CONTROLS                                  (WM_USER + 7)
#define KW_LOAD_CAT                                        (WM_USER + 8)
#define KW_REQUIRED_CONTROLS                               (WM_USER + 9)
#define KW_VALIDATE_CONTROLS                               (WM_USER + 10)

/* User-defined messages */

  
EXTERN HAB     hAB;                               /* Anchor Block           */
EXTERN HMQ     hMQ;                               /* Message Queue          */
EXTERN HMODULE hModSETADDR;
EXTERN HWND    hWndDeskTop;                       /* Queried Desktop handle */
EXTERN PHWND   phWndMain;                         /* Main Window handle     */
EXTERN INT     cArgcMain;                         /* Arguments from main */
EXTERN CHAR**  pArgvMain;                         /* Arguments from main */
 
EXTERN CHAR    szAppName [20];                    /* Application class name           */
EXTERN CHAR    szWorkBuf [80];                    /* Working storage character buffer */
EXTERN USHORT  nWorkValue;          
 
EXTERN PDLLINITTYPE     pDllInit;                 /* Pointer to the Dll init struct   */
EXTERN PDATAOBJECTTABLE pDataObjectTable;         /* Pointer to the data object table */

/* Standard window control data structure  */
#if !defined (WINDOWCONTROLDATA_TYPE)
#define       WINDOWCONTROLDATA_TYPE

/* Standard Window Control data structure                                  */
typedef _Packed struct 
{
 USHORT  cb;
 PVOID   pData;
} WINDOWDATA;
 
#endif

/* Standard Dialog control data structure                                  */
#if !defined (DIALOGCONTROLDATA_TYPE)
#define       DIALOGCONTROLDATA_TYPE

typedef _Packed struct 
{
 USHORT  cb;
 PVOID   pData;
} DIALOGDATA;
 
#endif
      
EXTERN HWND hWndWindowSETADDR;   /* Handle to the Window Frame    */
MRESULT EXPENTRY PanelCLIENTDlgProc (HWND, ULONG, MPARAM, MPARAM);


MRESULT EXPENTRY WindowSETADDRWndProc  (HWND, ULONG, MPARAM, MPARAM);


 
APIRET kwInitSETADDR            (HWND hWnd, int argc, CHAR *argv[]);
APIRET kwRegisterClasses              (VOID);
HWND   kwCreateWindow                 (HWND,   HWND, PVOID,
                                       PSZ,    PSZ, 
                                       ULONG,  ULONG, 
                                       INT,    INT, 
                                       INT,    INT, 
                                       USHORT, USHORT);

BOOL   kwCenter                       (HWND hWnd, HWND hWndParent);
APIRET kwAllocMemory                  (PPVOID pBaseAddress, ULONG ulSize);
APIRET kwFreeMemory                   (PVOID pBaseAddress);
VOID   kwDisplayMemoryAllocationError (PSZ pStructName);
APIRET kwFreeSETADDRDataObjects (PDATAOBJECTTABLE pDataObjectTable);
PVOID  kwSearchDataObjectTable        (PSZ pStructureName);
USHORT ErrBox                         (HWND, MPARAM, USHORT);

#endif /* SETADDR_HH */
