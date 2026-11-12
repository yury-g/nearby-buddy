// SPDX-License-Identifier: MIT
#pragma once
#include "DoodleProtocol.h"
#include <math.h>
enum DoodleChannel {DEYE,DBROW,DMOUTH,DSKEW,DJAW,DLEFT,DRIGHT,DWIDTH,DEYEWIDTH,DEYEY,DASYM,DWAVE,DCOUNT};
struct DoodleFrame {float v[DCOUNT]={};};
inline float doodleClamp(float x,float lo,float hi){return fmaxf(lo,fminf(hi,x));}
inline DoodleFrame doodleTarget(const FaceState &s){
 DoodleFrame f;float *v=f.v;
 v[DEYE]=.4f;v[DMOUTH]=.15f;v[DWIDTH]=12;v[DEYEWIDTH]=9;v[DEYEY]=29;
 if(s.expression==HAPPY){v[DEYE]=.85f;v[DMOUTH]=.9f;v[DWIDTH]=16;v[DEYEWIDTH]=10;v[DASYM]=.08f;}
 if(s.expression==SURPRISED){v[DEYE]=1.35f;v[DBROW]=-.3f;v[DMOUTH]=0;v[DWIDTH]=7;v[DEYEWIDTH]=6;v[DEYEY]=32;}
 if(s.expression==ANGRY){v[DEYE]=-.5f;v[DBROW]=1;v[DMOUTH]=-.7f;v[DEYEWIDTH]=10;v[DEYEY]=27;v[DASYM]=.12f;v[DWAVE]=.15f;}
 if(s.expression==SORRY){v[DEYE]=-.42f;v[DBROW]=-1;v[DMOUTH]=-.4f;v[DWIDTH]=9;v[DEYEY]=30;v[DASYM]=.1f;}
 if(s.expression==NERVOUS){v[DEYE]=.55f;v[DBROW]=-.1f;v[DMOUTH]=0;v[DWIDTH]=14;v[DEYEWIDTH]=8;v[DEYEY]=30;v[DASYM]=.65f;v[DWAVE]=1;}
 v[DEYE]=doodleClamp(v[DEYE]+s.eyeBend*.013f,-1.1f,1.5f);
 v[DBROW]=doodleClamp(v[DBROW]+s.browBend*.014f,-1.4f,1.4f);
 v[DMOUTH]=doodleClamp(v[DMOUTH]+s.mouthBend*.015f,-1.2f,1.2f);
 v[DSKEW]=s.mouthSkew*.01f;v[DJAW]=s.jaw*.01f;v[DLEFT]=s.left*.01f;v[DRIGHT]=s.right*.01f;
 return f;
}
class DoodleAnimator {
 DoodleFrame goal,velocity;bool ready=false,active=false;uint32_t last=0,moved[3]={};float energy[3]={};
 static unsigned part(unsigned k){return k==DEYE||k==DLEFT||k==DRIGHT||k==DEYEWIDTH||k==DEYEY?0:k==DBROW?1:2;}
public:
 DoodleFrame current;
 void reset(const FaceState &s,uint32_t now){goal=current=doodleTarget(s);velocity=DoodleFrame{};ready=true;active=false;last=now;for(unsigned i=0;i<3;i++){moved[i]=now-2000;energy[i]=0;}}
 void set(const FaceState &s,uint32_t now){
  if(!ready){reset(s,now);return;}
  auto next=doodleTarget(s);float change[3]={};
  for(unsigned k=0;k<DCOUNT;k++)change[part(k)]+=fabsf(goal.v[k]-next.v[k]);
  for(unsigned i=0;i<3;i++)if(change[i]>.0001f){moved[i]=now;energy[i]=doodleClamp(change[i],.15f,1);active=true;}
  goal=next;
 }
 void step(uint32_t now){
  if(!active){last=now;return;}
  float dt=fminf((uint32_t)(now-last)*.001f,.05f);last=now;bool moving=false,recent=false;
  for(unsigned k=0;k<DCOUNT;k++){
   velocity.v[k]+=(82*(goal.v[k]-current.v[k])-15*velocity.v[k])*dt;current.v[k]+=velocity.v[k]*dt;
   if(fabsf(goal.v[k]-current.v[k])>.003f||fabsf(velocity.v[k])>.02f)moving=true;
  }
  for(unsigned i=0;i<3;i++)if((uint32_t)(now-moved[i])<1250)recent=true;
  if(!moving&&!recent){current=goal;velocity=DoodleFrame{};active=false;}
 }
 bool settled()const{return !active;}
 bool tick(const FaceState &s,uint32_t now,bool visible){set(s,now);bool hadMotion=active;step(now);return visible&&hadMotion;}
 float wiggle(unsigned i,uint32_t now)const{float age=(uint32_t)(now-moved[i])*.001f;return active&&age<2?energy[i]*sinf(age*25)*expf(-age*6)*1.4f:0;}
};
struct DoodlePoint {float x,y;};
inline void doodleSegment(uint8_t *buffer,DoodlePoint a,DoodlePoint b){
 int x0=(int)floorf(fminf(a.x,b.x)-1),x1=(int)ceilf(fmaxf(a.x,b.x)+1),y0=(int)floorf(fminf(a.y,b.y)-1),y1=(int)ceilf(fmaxf(a.y,b.y)+1);
 if(x0<0)x0=0;if(x1>127)x1=127;if(y0<0)y0=0;if(y1>63)y1=63;
 float dx=b.x-a.x,dy=b.y-a.y,len=dx*dx+dy*dy;
 for(int y=y0;y<=y1;y++)for(int x=x0;x<=x1;x++){
  float t=len>0?doodleClamp(((x+.5f-a.x)*dx+(y+.5f-a.y)*dy)/len,0,1):0;
  float ex=x+.5f-a.x-t*dx,ey=y+.5f-a.y-t*dy;
  if(ex*ex+ey*ey<=1)buffer[(y/8)*128+x]|=(uint8_t)(1<<(y%8));
 }
}
inline void doodleQuadratic(uint8_t *buffer,DoodlePoint a,DoodlePoint control,DoodlePoint b){
 auto old=a;for(unsigned i=1;i<=32;i++){float t=i/32.f,u=1-t;DoodlePoint p{u*u*a.x+2*u*t*control.x+t*t*b.x,u*u*a.y+2*u*t*control.y+t*t*b.y};doodleSegment(buffer,old,p);old=p;}
}
inline void drawDoodle(const DoodleFrame &frame,Expression expression,uint8_t *buffer,float ew,float bw,float mw,uint8_t mask=0){
 memset(buffer,0,1024);const float *v=frame.v;
 float offsetX=(mask&8?6:0)-(mask&1?6:0),offsetY=(mask&4?4:0)-(mask&2?4:0);
 auto point=[&](float x,float y){return DoodlePoint{x+offsetX,y+offsetY};};
 for(unsigned eye=0;eye<2;eye++){
  float x=eye?91:37,side=eye?1:-1,bend=v[DEYE]+(eye?-1:1)*v[DASYM]*.22f,y=v[DEYEY]+ew*side*.6f;
  float blink=doodleClamp(v[eye?DRIGHT:DLEFT],0,1);
  doodleQuadratic(buffer,point(x-v[DEYEWIDTH],y),point(x+ew*.65f,y-(1-blink)*bend*20),point(x+v[DEYEWIDTH],y+v[DASYM]*side));
  float b=v[DBROW]+v[DASYM]*side*.55f,outer=13-fabsf(b)*1.5f+bw*side,inner=13+b*5+bw*.6f;
  doodleQuadratic(buffer,point(x-9,eye?inner:outer),point(x+bw*.5f,11-fabsf(b)*2+bw),point(x+9,eye?outer:inner));
 }
 float center=64+v[DSKEW]*4,w=v[DWIDTH],left=center-w,right=center+w,open=doodleClamp(v[DJAW],0,1);
 auto mouthPoint=[&](float t,float sign){
  float u=1-t,cy=doodleClamp(46+v[DMOUTH]*13+sign*open*(sign>0?15:9),27,65);
  float x=u*u*u*left+3*u*u*t*(left+w*.6f)+3*u*t*t*(right-w*.6f)+t*t*t*right;
  float y=u*u*u*(46-v[DSKEW]*2)+3*u*u*t*(cy+mw)+3*u*t*t*(cy-mw*.5f)+t*t*t*(46+v[DSKEW]*2)+sinf(t*3.14159265358979323846f*4)*v[DWAVE]*1.45f;
  return point(x,y);
 };
 for(unsigned edge=0;edge<(open<.045f?1U:2U);edge++){
  float sign=open<.045f?0:edge?1:-1;auto old=mouthPoint(0,sign);
  for(unsigned i=1;i<=32;i++){auto p=mouthPoint(i/32.f,sign);doodleSegment(buffer,old,p);old=p;}
 }
 if(expression==HAPPY||expression==SURPRISED){doodleSegment(buffer,point(16,26),point(13,23));doodleSegment(buffer,point(15,32),point(12,31));doodleSegment(buffer,point(112,26),point(115,23));doodleSegment(buffer,point(113,32),point(116,31));}
 else if(expression==ANGRY||expression==SORRY){doodleSegment(buffer,point(17,36),point(14,39));doodleSegment(buffer,point(20,38),point(18,41));doodleSegment(buffer,point(111,36),point(114,39));doodleSegment(buffer,point(108,38),point(110,41));}
 else if(expression==NERVOUS){doodleSegment(buffer,point(111,26),point(110,30));doodleSegment(buffer,point(116,29),point(115,33));}
}
