#ifdef E32TO16
    extern unsigned short _Far16 _Pascal NETBIOS (char * _Seg16);    /* added for 32 bit compiler */
#else
    extern unsigned pascal far NETBIOS (char far *);
#endif
