"""Generate firmware-derived screen textures and material groups for vendor CAD."""
from pathlib import Path
import ast,re,subprocess,json
import numpy as np
import trimesh
from PIL import Image,ImageDraw
ROOT=Path(__file__).resolve().parents[1]
REPO=ROOT.parent
WORK=REPO/'build/studio'
WORK.mkdir(parents=True,exist_ok=True)
(ROOT/'textures').mkdir(exist_ok=True)
tree=ast.parse((REPO/'tools/render_visuals.py').read_text())
template=next(ast.literal_eval(n.value) for n in tree.body if isinstance(n,ast.Assign) and any(isinstance(t,ast.Name) and t.id=='TEMPLATE' for t in n.targets))
source=(REPO/'firmware/NearbyBuddy/Face.h').read_text()
expression=source[source.index('  FaceState expression('):source.index('  bool green_ = false;')]
(WORK/'face.cpp').write_text(template.replace('__EXPRESSION__',expression))
subprocess.run(['clang++','-std=c++17','-I'+str(REPO/'firmware/NearbyBuddy'),str(WORK/'face.cpp'),'-o',str(WORK/'face')],check=True)
data=subprocess.check_output([str(WORK/'face')])
fontsrc=(ROOT/'reference/glcdfont.c').read_text().split('font[] PROGMEM = {',1)[1].split('};',1)[0]
font=bytes(int(x,16) for x in re.findall(r'0x([0-9A-Fa-f]{2})',re.sub(r'//[^\n]*','',fontsrc)))
def rgb565(v):return (round(((v>>11)&31)*255/31),round(((v>>5)&63)*255/63),round((v&31)*255/31))
def text(im,s,x,y,scale=1,color=(255,255,255)):
    d=ImageDraw.Draw(im)
    for i,c in enumerate(s):
        for col in range(5):
            bits=font[ord(c)*5+col]
            for row in range(8):
                if (bits>>row)&1:
                    xx=x+(i*6+col)*scale;yy=y+row*scale
                    d.rectangle((xx,yy,xx+scale-1,yy+scale-1),fill=color)
for role,name in enumerate(['blue','green']):
    im=Image.new('RGB',(240,135),'black');packed=data[(90*2+role)*1024:(90*2+role+1)*1024]
    for y in range(88):
        for x in range(208):
            sy=y*64//88;sx=x*128//208
            if packed[(sy//8)*128+sx]&(1<<(sy%8)):im.putpixel((x+16,y),rgb565(0x47e9 if role else 0x353f))
    text(im,'BAT 12%' if role else 'BAT 100%',8,90,2)
    text(im,'~<1 ft',240-8-6*6*2,90,2)
    title='Nearby Buddy v0.1.3';text(im,title,(240-len(title)*12)//2,109,2)
    stamp='2026-11-11 18:45:33';text(im,stamp,(240-len(stamp)*6)//2,127,1,rgb565(0xc638))
    im.save(ROOT/f'textures/screen-{name}.png')

# Component colors are visualization choices. Vendor vertices remain unchanged.
palette={'pcb':[.035,.045,.058],'metal':[.57,.61,.66],'gold':[.78,.57,.23],
         'plastic':[.025,.03,.039],'cream':[.84,.83,.73],'glass':[.009,.014,.02],
         'chip':[.055,.06,.065],'tan':[.40,.29,.17]}
board=trimesh.load(ROOT/'stl/board.stl')
meshes=[];tints=[]
for c in board.split(only_watertight=False):
    lo,hi=c.bounds;center=c.bounds.mean(0)
    kind='metal'
    if len(c.faces)>18000:kind='pcb'
    elif len(c.faces)>3000 and lo[2]>=26.99:kind='glass'
    elif lo[2]>28.3:kind='plastic'
    elif lo[2]>=26.99:kind='metal'
    elif len(c.faces)>6000:kind='metal'
    elif lo[0]>6 and hi[0]<15 and lo[1]<1 and hi[1]<9 and lo[2]<21:kind='cream'
    elif lo[0]>21 and hi[0]<25 and lo[2]<22:kind='cream'
    elif hi[0]>43 and len(c.faces)>500:kind='metal'
    elif len(c.faces)>500:kind='chip'
    elif min(c.extents)<.15:kind='gold'
    else:kind='tan' if len(c.faces)>70 else 'metal'
    meshes.append({'vertices':c.vertices.round(6).tolist(),'faces':c.faces.tolist(),'material':kind})
    tints.extend([palette[kind]]*(len(c.faces)*3))
combined=trimesh.util.concatenate([trimesh.Trimesh(vertices=m['vertices'],faces=m['faces'],process=False) for m in meshes])
combined.export(WORK/'board-material-order.stl')
(WORK/'board-materials.json').write_text(json.dumps({'palette':palette,'components':meshes}))
(WORK/'board-tints.bin').write_bytes(np.asarray(tints,dtype=np.float32).tobytes())
print('Prepared 240×135 RGB565-derived screens and vendor geometry material groups.')
