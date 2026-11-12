# Visual assets

The README embeds an 18-second GIF and links a script-free animated SVG for inspection and editing. The PNG shows both stylized device illustrations and their screen layout. Screen values are illustrative.

Face pixels come from the included DoodlePixels.h renderer. The exporter extracts the expression method from Face.h and uses DoodleAnimator and drawDoodle for a synthetic sleep-to-dance-to-rest sequence. No RF data drives the visual. Board geometry and footer text are drawn by the exporter.

To regenerate on macOS, install Pillow and use a C++17 compiler. Run from the chapter directory. Intermediates go under ignored build/.

    python3 -m venv build/visual-venv
    build/visual-venv/bin/pip install Pillow==12.1.1
    build/visual-venv/bin/python tools/render_visuals.py

The raster labels use macOS Arial. SVG uses Inter/Arial/sans-serif fallbacks. Other platforms can supply local font paths in the script. Pillow's license is included here. New illustration code and output are MIT licensed.

## Accurate CAD studio and screens

The README now leads with Adafruit CAD geometry staged in Blender and native Nearby Buddy screen pixels. The earlier stylized animation remains a labeled behavior tour. See [enclosure/STUDIO.md](../enclosure/STUDIO.md) for sources, exact screen placement, reproducible scripts, the packed Blender scene and the offline viewer.
