# Default texture images

The editor loads the PNG files in this folder directly. It does not redraw or recreate the texture at runtime.

Use these exact filenames:

- `gameskin.png`
- `skin.png`
- `hud.png`
- `emoticons.png`
- `particles.png`
- `ddrace_logo.png`

If you want the shipped defaults to be the exact Teeworlds/DDRace images, replace the placeholder PNGs in this folder with the real PNG files using the same names.

The current app renders them pixel-perfect:

- no upscale when the image already fits;
- only downscale when the image is larger than the canvas;
- nearest-neighbor sampling for crisp pixels;
- integer zoom in the focus preview so parts are not stretched.
