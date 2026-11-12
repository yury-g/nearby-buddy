#!/usr/bin/env python3
"""Digital fit checks. Does not substitute for a print or electrical testing."""
from pathlib import Path
import json,itertools,warnings
import numpy as np
import trimesh
warnings.filterwarnings('ignore',category=RuntimeWarning)
root=Path(__file__).resolve().parents[1]
def mesh(name):return trimesh.load(root/'stl'/f'{name}.stl')
report={'units':'mm','status':'DIGITAL CHECK ONLY — physical print untested','parts':{},'collisions':[]}
for n in ['shell','carrier','lid','key']:
    m=mesh(n)
    report['parts'][n]={'watertight':bool(m.is_watertight),'connected_bodies':len(m.split()),'bounds':m.bounds.round(3).tolist(),'volume_mm3':round(float(m.volume),3)}
    assert m.is_watertight and len(m.split())==1,n
    assert abs(m.bounds[0,2])<1e-4,(n,'not on print bed')
    steep=(m.face_normals[:,2]<-.708)&(m.triangles_center[:,2]>.05)
    area=float(m.area_faces[steep].sum())
    report['parts'][n]['steep_downward_area_mm2']=round(area,5)
    assert area<.05,(n,'steep unsupported downward surfaces',area)
parts={n:mesh(n) for n in ['shell_assembly','carrier_assembly','lid_assembly','key_assembly','bay','battery','larger-battery','plug','wires']}
for a,b in itertools.combinations(parts,2):
    if a in ['bay','battery','larger-battery','plug','wires'] and b in ['bay','battery','larger-battery','plug','wires']:continue
    v=float(trimesh.boolean.intersection([parts[a],parts[b]],engine='manifold').volume)
    if v>.001:report['collisions'].append({'a':a,'b':b,'volume_mm3':v})
# Vendor STL contains surfaces as well as solids. Avoid silently trusting a
# boolean on a non-watertight board. Conservatively test each component AABB.
boxes=0
for c in mesh('board').split(only_watertight=False):
    if min(c.extents)<.00001:continue
    boxes+=1
    box=c if c.is_volume else trimesh.creation.box(c.extents,transform=trimesh.transformations.translation_matrix(c.bounds.mean(axis=0)))
    for n in ['shell_assembly','carrier_assembly','lid_assembly','key_assembly']:
        v=float(trimesh.boolean.intersection([parts[n],box],engine='manifold').volume)
        if v>.001:report['collisions'].append({'a':n,'b':'board component envelope','volume_mm3':v,'bounds':c.bounds.tolist()})
report['board_components_checked']=boxes
report['board_method']='Valid component solids; conservative AABB for non-solid surfaces. Flat zero-thickness remnants omitted.'
report['battery_bay_mm']=[52,27,12]
report['target_larger_battery_mm']=[50,25,10]
report['adafruit_3898_max_mm']=[37,17.5,8.2]
report['not_verified']=['Physical tolerances and retention','Print bridging and overhang quality','Actual JST plug, lead bend and strain relief','Soldered headers or accessories','Thermal, impact, radio and battery performance']
(root/'FIT-CHECK.json').write_text(json.dumps(report,indent=2)+'\n')
assert not report['collisions'],report['collisions']
print(f'PASS: four connected watertight print parts; {boxes} board envelopes; zero solid interference; full battery bay clear.')
