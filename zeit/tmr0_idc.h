/* TMR0_IDC.H

   MODIFICATION HISTORY
   DATE       PROGRAMMER   COMMENT
   01-Jul-95  Timur Tabi   Creation
   04-Aug-96  Timur Tabi   Corrected comment
*/

#ifndef TIMER0_INCLUDED
#define TIMER0_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#define TMR0_REG        0
#define TMR0_DEREG      1
#define TMR0_GETTMRPTR  2

/* This function will will register a new time handler, modify a current
** time handler, and deregister a current one.  A registered one will be
** called every n milliseconds. Modifying a handler means to change the
** repeat time by sending a function hook that already exists.  Note that
** a handler can not be added or removed with an interrupt thread.  A current
** time handler can be modified with an interrupt thread.
*/

/* The actual rate is once every .999849142 milliseconds, which is faster than
** 1ms by 0.015%.  It will be ahead by one millisecond every 6.628 seconds.
*/

/* This is the IDC parameter convention for calling Timer0.  The first parm,
**   command, is what action needs to be done.  ulParm1 is used as a
**   Ptr to a function, the function being the timer handler.  ulParm2
**   is used for any other parms that might be needed.  Currently it is
**   being used as a ULONG with the repeat timer in number of milliseconds.
*/

/* To call this function use DevHelp_AttachDD, to get the IdcHandler location.
**   Then declare a PTMRFN, Pointer to Timer Function which is defined below,
**   and put the correct data from DevHelpAttachDD into the PTMRFN.  Then
**   just call the PTMRFN with the right parameters.
*/

/*   EX  To register a device that will have function FOO called every 10ms
**        one would type.
**
**       PMTMRFN ptmrfn;
**       ptmrfn(TMR0_REG, FOO, 10);
*/

/*  NOTE: THE UPPER 16 BITS (MSW) OF THE THIRD PARAMETER MUST BE ZERO!!!!!
**  That is, even though it's defined as a ULONG, it can only contain values
**  in the range 0 to 65535.
*/

/* Return Codes:   0  -  Worked Correctly
**                 1  -  Interrupt Thread trying to modify size of array
**                 2  -  Too many devices
**                 3  -  Hook not found
**                 4  -  Illegal parameter
**                 5  -  Problem getting IRQ0
**                 6  -  Trying to register during or before INIT COMPLETE
*/

typedef void (_far _loadds *PFN_TMR0REG) (void);
// the type of the function that TIMER0 calls every n milliseconds

typedef int (_far _loadds _cdecl *PTMRFN) (int iCmd, ULONG ulParm1, ULONG ulParm2);
// the type of the TIMER0$ IDC entry point function.

typedef struct {
   USHORT      ausReserved[3];     // 3 reserved words
   PTMRFN      pfn;                // far pointer to IDC entry
   USHORT      ds;                 // data segment of IDC
} TIMER0_ATTACH_DD;

#ifdef __cplusplus
}
#endif

#endif
