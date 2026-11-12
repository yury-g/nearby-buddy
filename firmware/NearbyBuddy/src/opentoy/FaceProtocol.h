// SPDX-License-Identifier: MIT
#pragma once
#include "Protocol.h"
enum Expression {NEUTRAL,HAPPY,ANGRY,SURPRISED,SORRY,NERVOUS};
struct FaceState {int x=0,y=0,pupilX=0,pupilY=0;unsigned left=0,right=0;Expression expression=NEUTRAL;bool pupils=true,bottomLids=true;bool doodle=false;int eyeBend=0,browBend=0,mouthBend=0,mouthSkew=0;unsigned jaw=0;};
inline const char *expressionName(Expression e){return e==HAPPY?"happy":e==ANGRY?"angry":e==SURPRISED?"surprised":e==SORRY?"sorry":e==NERVOUS?"nervous":"neutral";}
inline bool faceAxis(const char *p,int &out){bool negative=*p=='-';if(negative)p++;unsigned n;if(!boundedNumber(p,0,100,n))return false;out=negative?-(int)n:(int)n;return true;}
inline bool parseFace(const char *p,FaceState &out){
 char token[5][16]={};
 for(unsigned t=0;t<5;t++){
  unsigned n=0;while(*p&&*p!=' '){if(n>=15)return false;token[t][n++]=*p++;}
  if(!n)return false;
  if(t<4){if(*p!=' ')return false;p++;}else if(*p)return false;
 }
 FaceState next;next.pupils=false;next.bottomLids=false;
 if(!faceAxis(token[0],next.x)||!faceAxis(token[1],next.y)||!boundedNumber(token[2],0,100,next.left)||!boundedNumber(token[3],0,100,next.right))return false;
 if(!strcmp(token[4],"neutral"))next.expression=NEUTRAL;
 else if(!strcmp(token[4],"happy"))next.expression=HAPPY;
 else if(!strcmp(token[4],"angry"))next.expression=ANGRY;
 else if(!strcmp(token[4],"surprised"))next.expression=SURPRISED;
 else return false;
 out=next;return true;
}
inline bool parseEyes(const char *p,FaceState &out){
 char token[8][16]={};
 for(unsigned t=0;t<8;t++){
  unsigned n=0;while(*p&&*p!=' '){if(n>=15)return false;token[t][n++]=*p++;}
  if(!n)return false;
  if(t<7){if(*p!=' ')return false;p++;}else if(*p)return false;
 }
 FaceState next;
 if(!faceAxis(token[0],next.x)||!faceAxis(token[1],next.y)||!faceAxis(token[2],next.pupilX)||!faceAxis(token[3],next.pupilY)||!boundedNumber(token[4],0,100,next.left)||!boundedNumber(token[5],0,100,next.right))return false;
 if(!strcmp(token[6],"bottom"))next.bottomLids=true;
 else if(!strcmp(token[6],"top"))next.bottomLids=false;
 else return false;
 if(!strcmp(token[7],"neutral"))next.expression=NEUTRAL;
 else if(!strcmp(token[7],"happy"))next.expression=HAPPY;
 else if(!strcmp(token[7],"angry"))next.expression=ANGRY;
 else if(!strcmp(token[7],"surprised"))next.expression=SURPRISED;
 else return false;
 out=next;return true;
}
