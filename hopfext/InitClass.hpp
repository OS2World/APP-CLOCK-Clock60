#ifndef INITCLASS
#define INITCLASS

const int GETINT  = 0;
const int SETNUL  = 1;
//const int GETCHAR = 0;

struct Initc_Pro{
   int  mode;      // siehe oben
   char index;     // KÅrzel zum scanen
   int  *value;    // gescanter value
};

class InitClass{
   Initc_Pro *initc_pro;
   int initc_cnt;
public:
   InitClass(int cnt,Initc_Pro *initc_pro);
   void analyse(int arc,char * arcv[]);
   ~InitClass(){};
};
#endif
