#include "mgdef.h"
#include "InitClass.hpp"
#ifdef DEBUG
#include <stdio.h>
#endif

InitClass::InitClass(int cnt,Initc_Pro *initc_pronew)
{
  initc_cnt = cnt;
  initc_pro= initc_pronew;
}


void InitClass::analyse(int arc,char * arcv[])
{
  int *validvalue = NULL;
  for (int i = 0;i < arc;i++){
     if ((arcv[i][0] == '-' || arcv[i][0] == '/' )&&
         (arcv[i][1] != 0)){
        for (int z = 0; z <initc_cnt;z++)
            if (arcv[i][1] == initc_pro[z].index){
                if (initc_pro[z].mode == GETINT){
                    validvalue = initc_pro[z].value;
                    if (arcv[i][2] >= '0' && arcv[i][2] <= '9' && validvalue != NULL){
                        *validvalue = arcv[i][2]-'0';
                    validvalue = NULL;
                    }
                }
                if (initc_pro[z].mode == SETNUL){
                    validvalue = initc_pro[z].value;
                    *validvalue = 0;
                }
            }
        }else{
            if (arcv[i][0] >= '0' && arcv[i][0] <= '9'
                && validvalue != NULL)
                *validvalue = arcv[i][0]-'0';
            validvalue = NULL;
        }
    }
}
