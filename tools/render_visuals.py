# SPDX-License-Identifier: MIT
"""Render the illustrative README tour using the current firmware face code."""
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont
import math
import html

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / 'docs/images'
OUT.mkdir(parents=True, exist_ok=True)
TEMPLATE = r"""#include <cstdio>
#include <cstring>
#include <cstdlib>
#include "Proximity.h"
#include "src/opentoy/DoodlePixels.h"
struct Preview {bool green_;
__EXPRESSION__};
int main(){
 Preview faces[2]={{false},{true}}; DoodleAnimator motion[2];
 Mood last[2]={Mood::Sleep,Mood::Sleep}; unsigned changed[2]={0,0};
 for(unsigned frame=0;frame<180;frame++){
 unsigned now=frame*100;
 unsigned stage=frame<20?0:frame<40?1:frame<60?2:frame<80?3:frame<100?4:frame<130?5:frame<145?3:frame<160?1:0;
 Mood mood=static_cast<Mood>(stage);
 for(unsigned role=0;role<2;role++){
 if(mood!=last[role]){changed[role]=now;last[role]=mood;}
 auto pose=faces[role].expression(mood,now-changed[role],now);
 if(frame==0)motion[role].reset(pose,now);else motion[role].tick(pose,now,true);
 unsigned char pixels[1024];unsigned char offset=0;
 if(mood==Mood::Dance){unsigned beat=((now-changed[role])/(role?280:215))%4;const unsigned char steps[]={1,2,8,4};offset=steps[role?(beat+1)%4:beat];}
 drawDoodle(motion[role].current,pose.expression,pixels,motion[role].wiggle(0,now),motion[role].wiggle(1,now),motion[role].wiggle(2,now),offset);
 fwrite(pixels,1,1024,stdout);
 }
 }
}
"""
import subprocess
WORK = ROOT / 'build/visuals'
WORK.mkdir(parents=True, exist_ok=True)
face_source = (ROOT / 'firmware/NearbyBuddy/Face.h').read_text()
expression = face_source[face_source.index('  FaceState expression('):face_source.index('  bool green_ = false;')]
harness = TEMPLATE.replace('__EXPRESSION__', expression)
(WORK / 'render.cpp').write_text(harness)
subprocess.run(['clang++', '-std=c++17', '-I' + str(ROOT / 'firmware/NearbyBuddy'),
                str(WORK / 'render.cpp'), '-o', str(WORK / 'render')], check=True)
DATA = subprocess.check_output([str(WORK / 'render')])

FONT = '/System/Library/Fonts/Supplemental/Arial.ttf'
BOLD = '/System/Library/Fonts/Supplemental/Arial Bold.ttf'
BG = '#f3f2ed'
INK = '#17262b'
COLORS = ['#31a5ff', '#42e34a']
LABELS = ['Apart · visual sleep', 'Noticing a friend', 'A small smile', 'Getting happier',
          'Elated', 'Very close · dance']
def font(size, bold=False):
    return ImageFont.truetype(BOLD if bold else FONT, size)
def face_image(frame, role):
    packed = DATA[(frame*2+role)*1024:(frame*2+role+1)*1024]
    image = Image.new('RGB',(128,64),'black')
    pixels = image.load()
    for y in range(64):
        for x in range(128):
            if packed[(y//8)*128+x] & (1<<(y%8)):
                pixels[x,y] = (49,165,255) if role==0 else (66,227,74)
    return image.resize((208,88),Image.Resampling.NEAREST)
def board(frame,role):
    image=Image.new('RGBA',(306,186))
    d=ImageDraw.Draw(image)
    d.rounded_rectangle((3,7,303,185),radius=18,fill='#d0d5d2')
    d.rounded_rectangle((0,0,300,178),radius=16,fill='#363746',outline='#626578',width=2)
    for x in (12,288):
        for y in (14,164):
            d.ellipse((x-5,y-5,x+5,y+5),fill='#c5aa6b')
            d.ellipse((x-2,y-2,x+2,y+2),fill='#202434')
    for x in range(40,270,19):
        d.ellipse((x,7,x+5,12),fill='#bbaa79')
        d.ellipse((x,165,x+5,170),fill='#bbaa79')
    d.rounded_rectangle((22,17,278,159),radius=5,fill='#101617')
    d.rectangle((30,21,269,155),fill='black')
    image.paste(face_image(frame,role),(46,21))
    d.text((38,112),'BAT 100%' if role==0 else 'BAT 12%',font=font(13,True),fill='white')
    d.text((202,112),['-- ft','~8.0 ft','~5.0 ft','~3.0 ft','~1.0 ft','~<1 ft'][stage(frame)],font=font(13,True),fill='white')
    d.text((67,133),'Nearby Buddy v0.1.3',font=font(13,True),fill='white')
    d.text((90,148),'2026-11-11 18:45:33',font=font(8),fill='#afb4b7')
    d.rounded_rectangle((0,74,15,105),radius=4,fill='#babdc3',outline='#737d85')
    d.rectangle((2,81,7,98),fill='#414652')
    return image
def stage(frame):
    return 0 if frame<20 else 1 if frame<40 else 2 if frame<60 else 3 if frame<80 else 4 if frame<100 else 5 if frame<130 else 3 if frame<145 else 1 if frame<160 else 0
def position(frame):
    if frame<20: return 70
    if frame<100:
        t=(frame-20)/80
        return round(70+100*(.5-.5*math.cos(t*math.pi)))
    if frame<130: return 170
    if frame<165:
        t=(frame-130)/35
        return round(170-100*(.5-.5*math.cos(t*math.pi)))
    return 70
frames=[]
for n in range(180):
    im=Image.new('RGB',(960,510),BG)
    d=ImageDraw.Draw(im)
    d.text((42,28),'Nearby Buddy',font=font(36,True),fill=INK)
    d.text((42,78),'Two faces. A shared sense of proximity.',font=font(19),fill='#58686c')
    d.text((42,125),LABELS[stage(n)] if n<130 else 'Moving apart · returning to rest',font=font(23,True),fill=INK)
    x=position(n)
    for role,bx in enumerate((x,960-x-300)):
        im.paste(board(n,role),(bx,192),board(n,role))
        d.text((bx+104,390),'Blueface' if role==0 else 'Greenface',font=font(19,True),fill=INK)
    d.line((100,443,860,443),fill='#c7cfca',width=1)
    d.text((42,466),'Illustrated behavior tour · firmware-rendered faces · illustrative screen values',font=font(16),fill='#58686c')
    frames.append(im)
palette=frames[115].quantize(colors=64)
frames[0].quantize(palette=palette).save(OUT/'nearby-buddy-motion.gif',save_all=True,
    append_images=[f.quantize(palette=palette,dither=Image.Dither.NONE) for f in frames[1:]],
    duration=100,loop=0,optimize=True,disposal=1)
frames[95].save(OUT/'nearby-buddy-devices.png')

def pixel_path(frame,role):
    packed=DATA[(frame*2+role)*1024:(frame*2+role+1)*1024]
    pieces=[]
    for y in range(64):
        x=0
        while x<128:
            if not packed[(y//8)*128+x] & (1<<(y%8)):
                x+=1
                continue
            start=x
            while x<128 and packed[(y//8)*128+x] & (1<<(y%8)): x+=1
            pieces.append(f'M{start} {y}h{x-start}v1h-{x-start}z')
    return ''.join(pieces)

times='0;0.111;0.222;0.333;0.444;0.555;0.722;0.805;0.889;1'
sample=[15,35,55,75,95,115,140,155,175]
parts=['<svg xmlns="http://www.w3.org/2000/svg" width="960" height="510" viewBox="0 0 960 510" role="img" aria-labelledby="title desc">',
'<title id="title">Nearby Buddy approach and separation</title>',
'<desc id="desc">Illustrated blue and green Feather boards move together, smile and dance, then move apart and close their eyes. Firmware-rendered face shapes. Synthetic behavior demonstration with illustrative screen values.</desc>',
f'<rect width="960" height="510" rx="20" fill="{BG}"/>',
'<g font-family="Inter,Arial,sans-serif" fill="#17262b">',
'<text x="42" y="61" font-size="36" font-weight="700">Nearby Buddy</text>',
'<text x="42" y="95" font-size="19" fill="#58686c">Two faces. A shared sense of proximity.</text>',
'<text x="42" y="149" font-size="22">Approach → smile → dance → separate → rest</text>']
for role in range(2):
    start=70 if role==0 else 590
    moves='0 0;0 0;100 0;100 0;0 0;0 0' if role==0 else '0 0;0 0;-100 0;-100 0;0 0;0 0'
    parts.append(f'<g transform="translate({start} 192)"><g><animateTransform attributeName="transform" type="translate" values="{moves}" keyTimes="0;0.111;0.555;0.722;0.917;1" dur="18s" repeatCount="indefinite"/>')
    parts.append('<rect x="3" y="7" width="300" height="178" rx="18" fill="#d0d5d2"/><rect width="300" height="178" rx="16" fill="#363746" stroke="#626578" stroke-width="2"/>')
    for x in (12,288):
        for y in (14,164):parts.append(f'<circle cx="{x}" cy="{y}" r="5" fill="#c5aa6b"/><circle cx="{x}" cy="{y}" r="2" fill="#202434"/>')
    for x in range(40,270,19):
        for y in (9,167):parts.append(f'<circle cx="{x}" cy="{y}" r="2.5" fill="#bbaa79"/>')
    parts.append('<rect x="22" y="17" width="256" height="142" rx="5" fill="#101617"/><rect x="30" y="21" width="240" height="135" fill="#000"/>')
    for i,frame in enumerate(sample):
        values=['1' if k==i or (k==9 and i==0) else '0' for k in range(10)]
        parts.append(f'<g opacity="{1 if i==0 else 0}"><animate attributeName="opacity" values="{";".join(values)}" keyTimes="{times}" calcMode="discrete" dur="18s" repeatCount="indefinite"/><path d="{pixel_path(frame,role)}" transform="translate(46 21) scale(1.625 1.375)" fill="{COLORS[role]}"/></g>')
    parts.append(f'<g fill="white"><text x="38" y="124" font-size="13" font-weight="700">BAT {100 if role==0 else 12}%</text><text x="150" y="144" text-anchor="middle" font-size="13">Nearby Buddy v0.1.3</text><text x="150" y="155" text-anchor="middle" font-size="8" fill="#afb4b7">2026-11-11 18:45:33</text></g>')
    for i,frame in enumerate(sample):
        values=['1' if k==i or (k==9 and i==0) else '0' for k in range(10)]
        distance=html.escape(['-- ft','~8.0 ft','~5.0 ft','~3.0 ft','~1.0 ft','~<1 ft'][stage(frame)])
        parts.append(f'<text x="202" y="124" font-size="13" fill="white" opacity="{1 if i==0 else 0}">{distance}<animate attributeName="opacity" values="{";".join(values)}" keyTimes="{times}" calcMode="discrete" dur="18s" repeatCount="indefinite"/></text>')
    parts.append('<rect x="0" y="74" width="15" height="31" rx="4" fill="#babdc3"/><rect x="2" y="81" width="5" height="17" fill="#414652"/>')
    parts.append(f'<text x="150" y="212" text-anchor="middle" font-size="19" font-weight="700">{"Blueface" if role==0 else "Greenface"}</text></g></g>')
parts.append('<path d="M100 443H860" stroke="#c7cfca"/><text x="42" y="480" font-size="16" fill="#58686c">Illustrated behavior tour · firmware-rendered faces · illustrative screen values</text></g></svg>')
(OUT/'nearby-buddy-motion.svg').write_text('\n'.join(parts))
print([(p.name,p.stat().st_size) for p in OUT.iterdir()])
