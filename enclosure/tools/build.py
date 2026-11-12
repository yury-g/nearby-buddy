#!/usr/bin/env python3
"""Export print parts, assembly meshes and reproducible CAD renders."""
from pathlib import Path
import subprocess, shutil
ROOT = Path(__file__).resolve().parents[1]
OPENSCAD = shutil.which('openscad')
if not OPENSCAD:
    raise SystemExit('Install OpenSCAD, then put openscad on PATH.')
def export(part, output, *args):
    subprocess.run([OPENSCAD, '-o', str(ROOT/output), '-D', f'part="{part}"',
                    *args, str(ROOT/'cad/nearby-buddy.scad')],check=True)
for part in ['shell','carrier','lid','key','joint-coupon','shell_assembly','carrier_assembly',
             'lid_assembly','key_assembly','board','battery','bay','plug','wires']:
    export(part, f'stl/{part}.stl','--export-format','binstl')
export('battery','stl/larger-battery.stl','--export-format','binstl','-D','larger_battery=true')
views = [
    ('empty-front','assembly','95,-80,95,30,8,15',False,False),
    ('empty-rear','assembly','105,85,-70,30,8,15',False,False),
    ('assembled-front','assembly','95,-80,95,30,8,15',True,True),
    ('assembled-rear','assembly','105,85,-70,30,8,15',True,True),
    ('inside-front','cutaway','95,-80,95,30,8,15',True,True),
    ('inside-rear','cutaway','95,-95,-45,30,8,15',True,True),
    ('exploded','exploded','110,-100,95,30,8,8',True,True),
    ('larger-battery','envelope','95,-90,-45,30,8,15',True,True),
]
for name,part,camera,board,battery in views:
    extra=['--imgsize=1400,1050','--viewall','--autocenter',f'--camera={camera}',
           '--colorscheme=Tomorrow','-D',f'show_board={str(board).lower()}',
           '-D',f'show_battery={str(battery).lower()}']
    if name=='larger-battery':extra+=['-D','larger_battery=true']
    export(part,f'images/{name}.png',*extra)
