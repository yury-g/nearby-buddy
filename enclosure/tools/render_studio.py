"""Blender 4.5 LTS studio renders. Run prepare_studio.py first; geometry is in mm."""
from pathlib import Path
import bpy, json, math, sys
from mathutils import Vector
ROOT=Path(__file__).resolve().parents[1]
DATA=json.loads((ROOT.parent/'build/studio/board-materials.json').read_text())
OUT=ROOT/'images'; STUDIO=ROOT/'studio';STUDIO.mkdir(exist_ok=True)
S=.001

def material(name,c,metal=0,rough=.4):
    m=bpy.data.materials.new(name);m.diffuse_color=(*c,1);m.use_nodes=True
    p=m.node_tree.nodes.get('Principled BSDF');p.inputs['Base Color'].default_value=(*c,1)
    p.inputs['Metallic'].default_value=metal;p.inputs['Roughness'].default_value=rough
    return m

def mesh(name,verts,faces,mat,parent=None):
    d=bpy.data.meshes.new(name);d.from_pydata([tuple(v*S for v in p) for p in verts],[],faces);d.update()
    o=bpy.data.objects.new(name,d);bpy.context.collection.objects.link(o);o.data.materials.append(mat);o.parent=parent;return o

def stl(name,mat,parent=None):
    bpy.ops.wm.stl_import(filepath=str(ROOT/'stl'/f'{name}.stl'),global_scale=S)
    o=bpy.context.object;o.name=name;o.data.materials.append(mat);o.parent=parent
    return o

def screen(role,parent):
    m=material(role+' native 240x135 LCD',(1,1,1),rough=.32)
    n=m.node_tree.nodes.new('ShaderNodeTexImage');n.image=bpy.data.images.load(str(ROOT/f'textures/screen-{role}.png'),check_existing=True);n.interpolation='Closest'
    p=m.node_tree.nodes.get('Principled BSDF');m.node_tree.links.new(n.outputs['Color'],p.inputs['Base Color']);m.node_tree.links.new(n.outputs['Color'],p.inputs['Emission Color']);p.inputs['Emission Strength'].default_value=.65;p.inputs['Specular IOR Level'].default_value=.015;p.inputs['Roughness'].default_value=.65
    o=mesh('Firmware LCD · illustrative values',[(12.336,4.092,28.249),(37.246,4.092,28.249),(37.246,18.952,28.249),(12.336,18.952,28.249)],[(0,1,2,3)],m,parent)
    uv=o.data.uv_layers.new(name='Native framebuffer'); coords=[(0,0),(1,0),(1,1),(0,1)]
    for i,c in enumerate(coords):uv.data[i].uv=c

def ring(x,y,r,parent):
    n=40;verts=[]
    for rr in (r,r-.22):
        verts += [(x+rr*math.cos(i*math.tau/n),y+rr*math.sin(i*math.tau/n),27.012) for i in range(n)]
    faces=[(i,(i+1)%n,(i+1)%n+n,i+n) for i in range(n)]
    mesh('Gold plated through hole',verts,faces,M['gold'],parent)

def board(role,parent):
    for kind in DATA['palette']:
        verts=[];faces=[]
        for c in DATA['components']:
            if c['material']!=kind:continue
            start=len(verts);verts.extend(c['vertices']);faces.extend([tuple(i+start for i in f) for f in c['faces']])
        mesh('Feather '+kind,verts,faces,M[kind],parent)
    for i in range(16):ring(6.35+i*2.54,21.59,.7874,parent)
    for i in range(12):ring(16.51+i*2.54,1.27,.7874,parent)
    for y in (2.54,20.32):ring(2.54,y,1.6,parent)
    screen(role,parent)

def group(name,offset=(0,0,0),rot=0):
    o=bpy.data.objects.new(name,None);bpy.context.collection.objects.link(o);o.location=Vector(offset)*S;o.rotation_euler.z=math.radians(rot);return o

def assembly(role,offset=(0,0,0),rot=0,kind='full',large=False):
    p=group(role,offset,rot)
    if kind=='bare':board(role,p);return
    if kind not in ('inside','exploded'):
        stl('shell_assembly',M[role],p)
    else:
        shell=stl('shell_assembly',M[role],p)
        if kind=='inside':shell.hide_render=True
        else:shell.location.z=35*S
    stl('carrier_assembly',M['carrier'],p)
    lid=stl('lid_assembly',M['lid'],p);key=stl('key_assembly',M['key'],p)
    if kind in ('inside','exploded'):
        lid.location=Vector((0,0,-24))*S; key.location=Vector((0,-14,-24))*S
        if kind=='inside':lid.hide_render=True;key.hide_render=True
    bp=group('Board assembly');bp.parent=p
    if kind=='exploded':bp.location.z=64*S
    board(role,bp)
    b=stl('larger-battery' if large else 'battery',M['foil'],p)
    if kind=='exploded':b.location.z=-10*S
    if not large:
        stl('plug',M['cream'],bp)
        if kind=='full':stl('wires',M['wire'],p)
    # Thin amber label on the modeled pouch envelope. Visual only; no fit geometry changes.
    # A readable foil label helps distinguish reference battery from an exact product model.
    curve=bpy.data.curves.new('Battery label','FONT');curve.body='50 × 25 × 10 mm\nTARGET ENVELOPE' if large else 'LiPo · 3.7 V\n400 mAh / 3898';curve.size=2.0*S;curve.align_x='CENTER';curve.space_line=1.25
    ob=bpy.data.objects.new('Battery envelope label',curve);bpy.context.collection.objects.link(ob);ob.parent=p;ob.location=Vector((39 if large else 33,9,16.55))*S;ob.data.materials.append(M['ink'])
    if kind=='exploded':ob.location.z-=10*S
    if kind=='inside':ob.location=Vector((39,9,6.48))*S;ob.rotation_euler.x=math.pi
    return p

def point(o,target):o.rotation_euler=(Vector(target)*S-o.location).to_track_quat('-Z','Y').to_euler()
def setup():
    global M
    bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False)
    M={k:material(k,c,metal=.75 if k in ('metal','gold') else 0,rough=.25 if k=='glass' else .43) for k,c in DATA['palette'].items()}
    M.update(blue=material('Blue satin printed shell',(.055,.29,.65)),green=material('Green satin printed shell',(.12,.46,.26)),carrier=material('Carrier',(.38,.48,.53)),lid=material('Rear lid',(.1,.14,.19)),key=material('Locking key',(1,.45,.045)),foil=material('Battery foil envelope',(.63,.68,.72),.75,.36),wire=material('Wire route',(.7,.025,.035)),ink=material('Battery ink',(.035,.04,.045)))
    scene=bpy.context.scene;scene.render.engine='CYCLES';scene.cycles.samples=32;scene.cycles.use_denoising=True
    scene.render.resolution_x=1600;scene.render.resolution_y=1050;scene.render.resolution_percentage=100
    scene.render.image_settings.file_format='PNG';scene.world.color=(.55,.55,.55)
    scene.view_settings.view_transform='AgX';scene.view_settings.look='AgX - Medium High Contrast';scene.view_settings.exposure=0
    scene.unit_settings.system='METRIC';scene.unit_settings.length_unit='MILLIMETERS'
    for name,loc,power,size in [('Key',(-80,-100,180),55,.15),('Fill',(150,-20,100),35,.12),('Rim',(10,140,160),65,.1),('Rear fill',(-80,-90,-140),35,.14),('Rear rim',(100,100,-100),25,.1)]:
        d=bpy.data.lights.new(name,'AREA');d.energy=power*.01;d.shape='DISK';d.size=size;o=bpy.data.objects.new(name,d);scene.collection.objects.link(o);o.location=Vector(loc)*S;point(o,(30,10,10))
    cam=bpy.data.cameras.new('Studio camera');o=bpy.data.objects.new('Studio camera',cam);scene.collection.objects.link(o);scene.camera=o;cam.type='ORTHO';cam.lens=50

def render(name,target=(30,10,15),direction=(90,-120,140),scale=125,floor=-3,save=False):
    scene=bpy.context.scene;cam=scene.camera;cam.location=(Vector(target)+Vector(direction))*S;point(cam,target);cam.data.ortho_scale=scale*S
    if floor is not None:
        bpy.ops.mesh.primitive_plane_add(size=2,location=(0,0,floor*S));bpy.context.object.name='Studio ground';bpy.context.object.data.materials.append(material('Warm gray background',(.63,.65,.67)))
    scene.render.filepath=str(OUT/f'{name}.png')
    if save:
        for im in bpy.data.images:
            if im.source=='FILE':im.pack()
        bpy.context.preferences.filepaths.save_version=0
        bpy.ops.wm.save_as_mainfile(filepath=str(STUDIO/'nearby-buddy.blend'),compress=True)
    bpy.ops.render.render(write_still=True)

selected=set(sys.argv[sys.argv.index('--')+1:]) if '--' in sys.argv else set()
if not selected or 'devices-studio' in selected:
    setup();assembly('blue',offset=(-35,-20,0),rot=-9,kind='bare');assembly('green',offset=(24,14,0),rot=8,kind='bare');render('devices-studio',target=(20,8,26),direction=(25,-85,150),scale=133,floor=17)
if not selected or 'enclosure-studio' in selected:
    setup();assembly('blue',offset=(-44,-23,0),rot=-8);assembly('green',offset=(40,25,0),rot=8,large=True);render('enclosure-studio',target=(30,18,12),direction=(55,-110,160),scale=210,floor=-3,save=True)
if not selected or 'front-studio' in selected:
    setup();assembly('blue');render('front-studio',scale=140)
if not selected or 'rear-studio' in selected:
    setup();assembly('green',large=True);render('rear-studio',direction=(75,105,-130),scale=145,floor=None)
if not selected or 'inside-studio' in selected:
    setup();assembly('blue',kind='inside',large=True);render('inside-studio',target=(30,10,6),direction=(70,-90,-105),scale=130,floor=None)
if not selected or 'exploded-studio' in selected:
    setup();assembly('green',kind='exploded',large=True);render('exploded-studio',target=(30,10,28),direction=(100,-145,100),scale=250,floor=-31)
print('Rendered selected CAD studio views. Full run includes the packed studio/nearby-buddy.blend scene.')
