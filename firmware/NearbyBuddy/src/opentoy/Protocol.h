// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
#include <string.h>
enum Mode { BLINK, ON, OFF };
struct ToyState { unsigned period=1000, duty=50; Mode mode=BLINK; };
enum Result { INVALID, ACCEPTED, STATUS_QUERY };
enum FrameKind { TEXT, BUTTON, BAD_FRAME };
struct Frame {FrameKind kind=BAD_FRAME;char text[64]={};unsigned button=0;bool pressed=false;};
inline const char *modeName(Mode m) {return m==ON?"on":m==OFF?"off":"blink";}
inline bool boundedNumber(const char *p,unsigned lo,unsigned hi,unsigned &value) {
 if(!*p)return false;
 unsigned n=0;
 for(;*p;++p){if(*p<'0'||*p>'9')return false;unsigned digit=*p-'0';if(n>(hi-digit)/10)return false;n=n*10+digit;if(n>hi)return false;}
 if(n<lo)return false;value=n;return true;
}
inline Result applyCommand(const char *cmd,ToyState &s) {
 if(!strcmp(cmd,"status"))return STATUS_QUERY;
 if(!strcmp(cmd,"reset")){s=ToyState{};return ACCEPTED;}
 if(!strncmp(cmd,"period ",7))return boundedNumber(cmd+7,100,10000,s.period)?ACCEPTED:INVALID;
 if(!strncmp(cmd,"duty ",5))return boundedNumber(cmd+5,1,99,s.duty)?ACCEPTED:INVALID;
 if(!strcmp(cmd,"mode blink")){s.mode=BLINK;return ACCEPTED;}
 if(!strcmp(cmd,"mode on")){s.mode=ON;return ACCEPTED;}
 if(!strcmp(cmd,"mode off")){s.mode=OFF;return ACCEPTED;}
 return INVALID;
}
inline void applyButton(unsigned b,bool pressed,ToyState &s) {
 if(!pressed)return;
 switch(b){case 1:s.mode=BLINK;break;case 2:s.mode=ON;break;case 3:s.mode=OFF;break;case 4:s=ToyState{};break;
 case 5:s.period=s.period>300?s.period-200:100;break;case 6:s.period=s.period<9800?s.period+200:10000;break;
 case 7:s.duty=s.duty>11?s.duty-10:1;break;case 8:s.duty=s.duty<89?s.duty+10:99;break;}
}
class FrameParser {
 char buffer[64]={};unsigned count=0;bool bad=false,binary=false;
public:
 void reset(){count=0;bad=false;binary=false;}
 bool pending() const {return count||bad;}
 bool feed(uint8_t c,Frame &f){
  if(binary){
   buffer[count++]=c;
   if(count==2 && c!='B'){reset();f.kind=BAD_FRAME;return true;}
   if(count<5)return false;
   uint8_t sum=0;for(unsigned i=0;i<4;++i)sum+=static_cast<uint8_t>(buffer[i]);
   bool valid=static_cast<uint8_t>(~sum)==c && buffer[2]>='1'&&buffer[2]<='8'&&(buffer[3]=='0'||buffer[3]=='1');
   f.kind=valid?BUTTON:BAD_FRAME;f.button=buffer[2]-'0';f.pressed=buffer[3]=='1';reset();return true;
  }
  if(c=='\r'||c=='\n'||c==';'){
   if(!count&&!bad)return false;
   f.kind=bad?BAD_FRAME:TEXT;buffer[count]=0;memcpy(f.text,buffer,count+1);reset();return true;
  }
  if(c=='!'&&!count&&!bad){binary=true;buffer[count++]=c;return false;}
  if(c<32||c>126){bad=true;return false;}
  if(count>=63){bad=true;return false;}
  buffer[count++]=c;return false;
 }
};
