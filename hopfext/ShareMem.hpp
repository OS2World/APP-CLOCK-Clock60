#include "dcfioctl.h"

typedef SHMEM DCF77SharedMem;

class DCF77Communication
    {
    DCF77SharedMem *pdcf77ShMem;
    DCF77_ALLDATA  dcf77Data;               // Datenbereich fÅr Server
    ULONG flThisProg;

    public:
        DCF77Communication (ULONG flProg = 0);
        ~DCF77Communication ();
        BOOL isClient ();
        BOOL isDriver ();
        BOOL isServer ();
        BOOL isClock ();
        BOOL isVersion ();
        DCF77SharedMem *getDCFShMemPtr ();
        DCF77_ALLDATA  *getDCFDataPtr ();
    };

