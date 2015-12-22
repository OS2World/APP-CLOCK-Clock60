#ifndef _TRACE_
#define _TRACE_

#ifdef DEBUG
void trace (short minor, char near *buffer, short size);

#define TRACE(m,b,s)            trace(m,b,s)
#define DCF77_DATASET           1
#define DCF77_MINUTE            2
#define DCF77_60S               3
#define DCF77_CHGPOL            4
#define DCF77_THR               5
#define DCF77_THRSET            6
#define DCF77_THREXCEEDED       7
#define DCF77_0TO1              8
#define DCF77_1TO0              9
#define DCF77_RESULT            10
#define DCF77_EVAL_START        11
#define DCF77_EVAL_DATA         12
#define DCF77_EVAL_ERR          13
#define DCF77_EVAL_OFFSET       14
#define DCF77_EVAL_OK           15
#define DCF77_EVAL_ERR_PARITY   16
#else
#define TRACE(m,b,s)
#endif /* DEBUG */

#endif /* _TRACE_ */
