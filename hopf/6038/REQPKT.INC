; static char *SCCSID = "@(#)reqpkt.h   6.4 92/05/08";  
;***********************************************************************  
;                                                                         
;  Driver Name: ADD/DM - Include Files                                    
;               ----------------------                                    
;                                                                         
;  Source File Name: REQPKT.H                                             
;                                                                         
;  Descriptive Name: OS/2 Request Packet structures                       
;                                                                         
;  Function: Defines OS/2 Request Packets for BLOCK type devices.         
;                                                                         
;                                                                         
; ----------------------------------------------------------------------  
;                                                                         
;  Copyright (C) 1992 IBM Corporation                                     
;                                                                         
;  DISCLAIMER OF WARRANTIES.  The following [enclosed] code is            
;  provided to you solely for the purpose of assisting you in             
;  the development of your applications. The code is provided             
;  "AS IS", without warranty of any kind. IBM shall not be liable         
;  for any damages arising out of your use of this code, even if          
;  they have been advised of the possibility of such damages.             
;                                                                         
; ----------------------------------------------------------------------  
;                                                                         
;  Change Log                                                             
;                                                                         
;  Mark    Date      Programmer  Comment                                  
;  ----    ----      ----------  -------                                  
;  0001    08.10.92  Breining    Umbau auf getrennte Assemblierbarkeit ohne weitere Module
;                                                                         
;                                                                         
;***********************************************************************  
; ---------------------------------  
;  Misc constants                    
; ---------------------------------  
MAX_DISKDD_CMD	EQU	29 
;******************************  
;  Device Driver Header          
;******************************  

DDHDR	STRUC
DDH_NextHeader	DD	?
DDH_DevAttr	DW	?
DDH_StrategyEP	DW	?
DDH_InterruptEP	DW	?
DDH_DevName	DB	'        '
DDH_ProtModeCS	DW	?
DDH_ProtModeDS	DW	?
DDH_RealModeCS	DW	?
DDH_RealModeDS	DW	?
DDH_SDevCaps	DD	?                   ;  bit map of DD /MM restrictions   
DDHDR	ENDS
;******************************  
;  BIOS Parameter Block          
;******************************  

BPB	STRUC
BPB_BytesPerSector	DW	?
BPB_SectorsPerCluster	DB	?
BPB_ReservedSectors	DW	?
BPB_NumFATs		DB	?
BPB_MaxDirEntries	DW	?
BPB_TotalSectors	DW	?
BPB_MediaType		DB	?
BPB_NumFATSectors	DW	?
BPB_SectorsPerTrack	DW	?
BPB_NumHeads		DW	?
BPB_HiddenSectors	DD	?
BPB_BigTotalSectors	DD	?
BPB_Reserved_1		DB	6 DUP (?)
BPB	ENDS
;**************************  
;   Request Packet Header    
;**************************  

RPH	STRUC
RPH_Len		DB	?
RPH_Unit	DB	?
RPH_Cmd		DB	?
RPH_Status	DW	?
RPH_Flags	DB	?
RPH_Reserved_1	DB	3 DUP (?)
RPH_Link	DD	?
RPH	ENDS
;  Status word in RPH   
STERR			EQU	8000H            ;  Bit 15 - Error
STINTER			EQU	0400H            ;  Bit 10 - Interim character
STBUI			EQU	0200H            ;  Bit  9 - Busy
STDON			EQU	0100H            ;  Bit  8 - Done
STECODE			EQU	00FFH            ;  Error code
WRECODE			EQU	0000H 
STATUS_DONE		EQU	0100H 
STATUS_ERR_UNKCMD	EQU	8003H 
;  Bit definitions for Flags field in RPH   
RPF_Int13RP		EQU	01H         ;  Int 13 Request Packet             
RPF_CallOutDone		EQU	02H         ;  Int 13 Callout completed          
RPF_PktDiskIOTchd	EQU	04H         ;  Disk_IO has touched this packet   
RPF_CHS_ADDRESSING	EQU	08H         ;  CHS Addressing used in RBA field  
RPF_Internal		EQU	10H         ;  Internal request packet command   
RPF_TraceComplete	EQU	20H         ;  Trace completion flag             
;***********************  
;   Init Request Packet   
;***********************  

RPINIT	STRUC
RPINI_rph	DB	SIZE RPH DUP (?)
RPINI_Unit	DB	?
RPINI_DevHlpEP	DB	SIZE DWORD DUP (?)
RPINI_InitArgs	DB	SIZE DWORD DUP (?)
RPINI_DriveNum	DB	?
RPINIT	ENDS

RPINITOUT	STRUC
RPINO_rph	DB	SIZE RPH DUP (?)
RPINO_Unit	DB	?
RPINO_CodeEnd	DW	?
RPINO_DataEnd	DW	?
RPINO_BPBArray	DD	?
RPINO_Status	DW	?
RPINITOUT	ENDS
;********************************************  
;  struct DDD_Parm_List moved to H\DSKINIT.H   
;********************************************  
ifndef INCL_INITRP_ONLY
;******************************  
;   Media Check Request Packet   
;******************************  

RP_MEDIACHECK	STRUC
RPMC_rph	DB	SIZE RPH DUP (?)
RPMC_MediaDescr	DB	?
RPMC_rc		DB	?
RPMC_PrevVolID	DB	SIZE DWORD DUP (?)
RP_MEDIACHECK	ENDS
;******************************  
;   Build BPB                    
;******************************  

RP_BUILDBPB	STRUC
RPBPB_rph		DB	SIZE RPH DUP (?)
RPBPB_MediaDescr	DB	?
RPBPB_XferAddr		DD	?
RPBPB_bpb		DB	SIZE DWORD DUP (?)
RPBPB_DriveNum		DB	?
RP_BUILDBPB	ENDS
;******************************  
;   Read, Write, Write Verify    
;******************************  

RP_RWV	STRUC
RPRWV_rph		DB	SIZE RPH DUP (?)
RPRWV_MediaDescr	DB	?
RPRWV_XferAddr		DD	?
RPRWV_NumSectors	DW	?
RPRWV_rba		DD	?
RPRWV_sfn		DW	?
RP_RWV	ENDS
;******************************  
;   Nondestructive Read          
;******************************  

RP_NONDESTRUCREAD	STRUC
RPNDR_rph		DB	SIZE RPH DUP (?)
RPNDR_character		DB	?
RP_NONDESTRUCREAD	ENDS
;******************************  
;   Open/Close Device            
;******************************  

RP_OPENCLOSE	STRUC
RPOC_rph	DB	SIZE RPH DUP (?)
RPOC_sfn	DW	?
RP_OPENCLOSE	ENDS
;************************  
;   IOCTL Request Packet   
;************************  

RP_GENIOCTL	STRUC
RPGIO_rph		DB	SIZE RPH DUP (?)
RPGIO_Category		DB	?
RPGIO_Function		DB	?
RPGIO_ParmPacket	DB	SIZE DWORD DUP (?)
RPGIO_DataPacket	DB	SIZE DWORD DUP (?)
RPGIO_sfn		DW	?
RP_GENIOCTL	ENDS
;******************************  
;   Partitionable Fixed Disks    
;******************************  

RP_PARTFIXEDDISKS	STRUC
RPFD_rph		DB	SIZE RPH DUP (?)
RPFD_NumFixedDisks	DB	?
RP_PARTFIXEDDISKS	ENDS
;******************************  
;   Get Unit Map                 
;******************************  

RP_GETUNITMAP	STRUC
RPUM_rph	DB	SIZE RPH DUP (?)
RPUM_UnitMap	DD	?
RP_GETUNITMAP	ENDS
;*********************************  
;   Get Driver Capabilities  0x1D   
;*********************************  

RP_GETDRIVERCAPS	STRUC
RPDC_rph	DB	SIZE RPH DUP (?)
RPDC_Reserved	DB	3 DUP (?)
RPDC_pDCS	DB	SIZE DWORD DUP (?)
RPDC_pVCS	DB	SIZE DWORD DUP (?)
RP_GETDRIVERCAPS	ENDS
endif
