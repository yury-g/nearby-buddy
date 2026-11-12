from pathlib import Path
import json,base64
root=Path(__file__).resolve().parents[1]
files={'shell':'shell_assembly','carrier':'carrier_assembly','lid':'lid_assembly','key':'key_assembly','board':'board','battery':'battery','larger':'larger-battery','bay':'bay','plug':'plug','wires':'wires'}
data={k:base64.b64encode((root/'stl'/f'{v}.stl').read_bytes()).decode() for k,v in files.items()}
studio=root.parent/'build/studio'
data['board']=base64.b64encode((studio/'board-material-order.stl').read_bytes()).decode()
screens={k:base64.b64encode((root/f'textures/screen-{k}.png').read_bytes()).decode() for k in ['blue','green']}
tints=base64.b64encode((studio/'board-tints.bin').read_bytes()).decode()
html=(root/'viewer/template.html').read_text().replace('__MESH_DATA__',json.dumps(data)).replace('__SCREEN_DATA__',json.dumps(screens)).replace('__BOARD_TINTS__',json.dumps(tints))
(root/'viewer/index.html').write_text(html)
print('Wrote self-contained offline 3D viewer.')
