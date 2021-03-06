#ifndef _DCF77_
#define _DCF77_

#include "dcfioctl.h"

class dcf77QueryData
    {
        DCF77Communication *pDCFShMem;
        DCF77_DATETIME  dtOld;                      // tempor�rer Zwischenspeicher f�r alte Zeit
        DCF77_ALLDATA   dataDCF;                    // aktuelle Treiberdaten Zeit/Status/Offset
        HFILE           hDrvr;

    public:
                        dcf77QueryData ();
                        ~dcf77QueryData ();
        BOOL            getDCFData ();
        BOOL            isOK ();
        DCF77_ALLDATA  *getDCFDataPtr ();
    };

#endif /* _DCF77_ */
