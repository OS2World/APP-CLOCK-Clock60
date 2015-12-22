#ifndef __SHAREMEM__
#define __SHAREMEM__

#include "dcfioctl.h"

typedef SHMEM DCF77SharedMem;

class DCF77Communication
    {
        DCF77SharedMem *pdcf77ShMem;
        ULONG           flThisProg;

    public:
                        DCF77Communication ();
                        ~DCF77Communication ();
        VOID            CreateShMem (ULONG flProg = 0);
        BOOL            isClient ();
        BOOL            isDriver ();
        BOOL            isServer ();
        BOOL            isClock ();
        BOOL            isVersion ();
        DCF77SharedMem *getDCFShMemPtr () {return pdcf77ShMem;};
    };

#endif /* __SHAREMEM__ */
