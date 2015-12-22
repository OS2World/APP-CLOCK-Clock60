#ifndef _MGDEF_
#define _MGDEF_

#ifdef DEBUG
// Debug mit Ausgaben
#include <stdio.h>
#define show(a) {a;};
#endif

#ifndef DEBUG
#define show(a) ;
#endif

#ifndef NULL
#define NULL 0
#endif
#endif

