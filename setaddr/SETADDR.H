#if !defined (SETADDR_H)
  #define     SETADDR_H

/***************************************************************************/
/*                                                                         */
/*  Data Object Table types and function prototypes.                       */
/*                                                                         */
/***************************************************************************/

#if !defined (DATAOBJECTTABLE_TYPE)
  #define     DATAOBJECTTABLE_TYPE

typedef _Packed struct 
{
  CHAR  StructName [64];
  PVOID pDataObject;
} DATATABLEELEMENT, * PDATAOBJECTTABLE;

#endif

/* DLL Initialization structure                                            */
#if !defined (DLLINIT_TYPE)
  #define     DLLINIT_TYPE

typedef _Packed struct 
{
  HAB              hAB;
  PHWND            phWndMain;
  INT              cArgcMain;
  CHAR**           pArgvMain;                      
  PDATAOBJECTTABLE pDataObjectTable;
} DLLINITTYPE, * PDLLINITTYPE;

typedef MRESULT (*INITPROC) (PDLLINITTYPE);

#endif

APIRET kwAllocateSETADDRDataObjects (PDLLINITTYPE pDllInit);
 
/* Standard Dialog control data structure  */
#if !defined (DIALOGCONTROLDATA_TYPE)
#define       DIALOGCONTROLDATA_TYPE

typedef _Packed struct 
{
 USHORT  cb;
 PVOID   pData;
} DIALOGDATA;
 
#endif
      
#if !defined (PANELCLIENT_TYPE)
  #define     PANELCLIENT_TYPE

typedef _Packed struct 
{
  BOOL   fTransferBuffer;      /* Flag indicating use of hWndOwner buffer   */
  HWND   hWndCaller;
  BOOL   sfValidate;           /* Init validation flag to TRUE */
} PANELCLIENTTYPE, * PPANELCLIENTTYPE;

#endif
      
#if !defined (WINDOWSETADDR_TYPE)
  #define     WINDOWSETADDR_TYPE

typedef _Packed struct 
{
  BOOL      fTransferBuffer; /* Flag indicating use of hWndOwner buffer      */
  HWND      hWndCaller;      /* Callers handle                               */
  HWND      hWndCAT;         /* Handle of client area template               */
  PVOID     pCAT_Data;
  PANELCLIENTTYPE PanelCLIENTData;
} WINDOWSETADDRTYPE, * PWINDOWSETADDRTYPE;
#endif 
    

#endif /* SETADDR_H */

