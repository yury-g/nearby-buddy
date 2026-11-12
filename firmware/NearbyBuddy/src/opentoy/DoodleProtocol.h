// SPDX-License-Identifier: MIT
#pragma once
#include "FaceProtocol.h"
inline bool parseDoodle(const char *p,FaceState &out){
 char token[8][16]={};
 for(unsigned t=0;t<8;t++){
  unsigned n=0;while(*p&&*p!=' '){if(n>=15)return false;token[t][n++]=*p++;}
  if(!n)return false;
  if(t<7){if(*p!=' ')return false;p++;}else if(*p)return false;
 }
 FaceState next;next.doodle=true;
 if(!faceAxis(token[0],next.eyeBend)||!faceAxis(token[1],next.browBend)||!faceAxis(token[2],next.mouthBend)||!faceAxis(token[3],next.mouthSkew)||!boundedNumber(token[4],0,100,next.jaw)||!boundedNumber(token[5],0,100,next.left)||!boundedNumber(token[6],0,100,next.right))return false;
 bool known=false;
 for(unsigned i=NEUTRAL;i<=NERVOUS;i++){Expression e=(Expression)i;if(!strcmp(token[7],expressionName(e))){next.expression=e;known=true;break;}}
 if(!known)return false;
 out=next;return true;
}
inline FaceState restingDoodle(){FaceState s;s.doodle=true;return s;}
