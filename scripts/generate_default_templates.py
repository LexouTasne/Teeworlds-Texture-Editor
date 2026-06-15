from __future__ import annotations

import json
import os
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont


ROOT = Path(__file__).resolve().parents[1]
TEMPLATE_FILE = ROOT / "data" / "templates" / "teeworlds_textures.json"
OUT = ROOT / "data" / "defaults"


def font(size: int) -> ImageFont.ImageFont:
    for name in ("arial.ttf", "segoeui.ttf"):
        try:
            return ImageFont.truetype(name, size)
        except OSError:
            pass
    return ImageFont.load_default()


def draw_label(draw: ImageDraw.ImageDraw, box: tuple[int, int, int, int], text: str) -> None:
    x1, y1, x2, y2 = box
    draw.rectangle(box, outline=(70, 220, 255, 190), width=2)
    draw.text((x1 + 4, y1 + 4), text, fill=(255, 255, 255, 230), font=font(12))


def draw_gameskin(path: Path) -> None:
    img = Image.new("RGBA", (1024, 512), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)

    d.line((0, 24, 32, 56), fill=(245, 238, 218, 255), width=6)
    d.line((32, 24, 0, 56), fill=(245, 238, 218, 255), width=6)
    d.rectangle((72, 49, 130, 63), fill=(128, 92, 35, 255))
    d.rectangle((130, 24, 188, 128), fill=(112, 76, 28, 255), outline=(70, 45, 18, 255), width=3)
    d.ellipse((334, 78, 384, 126), fill=(255, 78, 70, 255))
    d.ellipse((676, 10, 727, 60), fill=(220, 26, 39, 255), outline=(255, 255, 255, 255), width=3)
    d.polygon((264, 162, 316, 120, 452, 130, 400, 182), fill=(255, 108, 26, 255))
    d.polygon((287, 160, 317, 138, 390, 144, 355, 170), fill=(255, 225, 25, 255))
    d.rectangle((684, 150, 740, 170), fill=(113, 205, 234, 255), outline=(255, 255, 255, 255), width=2)
    d.rectangle((402, 338, 450, 512), fill=(17, 89, 220, 255))
    d.rectangle((530, 338, 578, 512), fill=(225, 22, 34, 255))
    d.rectangle((0, 132, 46, 260), outline=(208, 158, 33, 255), width=5)
    d.ellipse((350, 252, 365, 267), fill=(90, 90, 90, 255))
    d.rectangle((74, 274, 262, 324), fill=(208, 218, 222, 255))
    d.polygon((218, 274, 292, 252, 264, 302), fill=(205, 36, 45, 255))
    d.rectangle((120, 410, 266, 466), fill=(172, 184, 188, 255))
    d.polygon((266, 420, 300, 438, 266, 456), fill=(100, 110, 112, 255))
    d.arc((307, 424, 354, 470), 180, 360, fill=(245, 245, 245, 255), width=5)

    # Laser trails on the right side.
    for y in (42, 162, 314):
        d.polygon((795, y, 910, y - 36, 1023, y + 18, 910, y + 78), fill=(245, 250, 255, 255))
        d.polygon((835, y + 8, 920, y - 12, 1005, y + 22, 920, y + 56), fill=(147, 220, 245, 220))
        d.line((860, y + 18, 980, y + 26), fill=(89, 190, 229, 255), width=4)

    for box, text in [
        ((0, 0, 96, 32), "hook"),
        ((2, 128, 130, 224), "hammer"),
        ((2, 224, 130, 288), "gun"),
        ((2, 288, 162, 352), "shotgun"),
        ((2, 352, 162, 416), "grenade"),
        ((2, 416, 162, 480), "laser"),
        ((320, 0, 384, 64), "heart"),
        ((192, 64, 256, 96), "eye"),
    ]:
        draw_label(d, box, text)
    img.save(path)


def draw_skin(path: Path) -> None:
    img = Image.new("RGBA", (256, 128), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    d.ellipse((10, 10, 90, 90), fill=(255, 170, 0, 255), outline=(169, 102, 0, 255), width=3)
    d.ellipse((108, 10, 188, 90), fill=(210, 210, 210, 210), outline=(150, 150, 150, 255), width=3)
    d.ellipse((194, 10, 224, 36), fill=(70, 150, 255, 255))
    d.ellipse((226, 10, 256, 36), fill=(70, 150, 255, 255))
    d.ellipse((54, 35, 66, 58), fill=(0, 0, 0, 255))
    d.ellipse((75, 35, 87, 58), fill=(0, 0, 0, 255))
    d.ellipse((95, 72, 130, 100), fill=(165, 120, 55, 255), outline=(0, 0, 0, 255), width=2)
    for box, text in [
        ((0, 0, 96, 96), "body"),
        ((96, 0, 192, 96), "shadow"),
        ((192, 0, 224, 32), "foot L"),
        ((224, 0, 256, 32), "foot R"),
        ((192, 32, 224, 48), "eye"),
    ]:
        draw_label(d, box, text)
    img.save(path)


def draw_hud(path: Path) -> None:
    img = Image.new("RGBA", (512, 512), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    d.ellipse((0, 0, 64, 64), fill=(220, 28, 45, 255), outline=(255, 255, 255, 255), width=3)
    d.polygon((96, 5, 126, 24, 118, 60, 74, 60, 66, 24), fill=(170, 180, 188, 255), outline=(255, 255, 255, 255))
    d.rectangle((132, 16, 188, 48), fill=(100, 120, 150, 255), outline=(255, 255, 255, 255), width=2)
    d.line((208, 32, 254, 32), fill=(240, 240, 240, 255), width=4)
    d.line((232, 8, 232, 56), fill=(240, 240, 240, 255), width=4)
    d.ellipse((292, 18, 310, 36), fill=(255, 235, 70, 255))
    d.ellipse((326, 18, 344, 36), fill=(255, 235, 70, 255))
    for box, text in [
        ((0, 0, 64, 64), "health"),
        ((64, 0, 128, 64), "armor"),
        ((128, 0, 192, 64), "ammo"),
        ((192, 0, 256, 64), "cursor"),
    ]:
        draw_label(d, box, text)
    img.save(path)


def draw_emoticons(path: Path) -> None:
    img = Image.new("RGBA", (512, 512), (255, 255, 255, 0))
    d = ImageDraw.Draw(img)
    labels = ["OOP!", "!", "...", "SORRY!", "!!", "ZZZ", "WTF", "??"]
    positions = [(22, 46), (188, 24), (36, 190), (272, 178), (448, 170), (34, 392), (132, 420), (408, 404)]
    for text, pos in zip(labels, positions):
        d.text(pos, text, fill=(78, 80, 83, 255), font=font(42 if len(text) < 4 else 34))
    d.ellipse((288, 24, 360, 96), fill=(201, 34, 48, 255))
    d.polygon((170, 304, 210, 282, 238, 318, 210, 350), fill=(40, 45, 48, 255))
    d.ellipse((188, 302, 198, 318), fill=(255, 255, 255, 255))
    d.ellipse((206, 302, 216, 318), fill=(255, 255, 255, 255))
    d.arc((336, 396, 366, 430), 180, 360, fill=(78, 80, 83, 255), width=4)
    d.arc((370, 396, 400, 430), 180, 360, fill=(78, 80, 83, 255), width=4)
    img.save(path)


def draw_particles(path: Path) -> None:
    img = Image.new("RGBA", (512, 512), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    d.ellipse((20, 20, 82, 82), fill=(255, 255, 255, 255))
    d.ellipse((82, 20, 144, 82), fill=(248, 248, 248, 230))
    d.ellipse((145, 18, 212, 86), fill=(240, 240, 240, 215))
    d.regular_polygon((260, 58, 42), 8, fill=(255, 255, 255, 245))
    d.ellipse((0, 104, 118, 222), fill=(250, 250, 250, 235), outline=(170, 170, 170, 255), width=4)
    d.arc((30, 132, 92, 194), 300, 250, fill=(130, 130, 130, 255), width=5)
    d.regular_polygon((164, 160, 74), 5, fill=(96, 205, 235, 245), outline=(0, 110, 155, 255))
    d.ellipse((178, 144, 220, 186), fill=(77, 188, 224, 255))
    d.regular_polygon((120, 368, 130), 18, fill=(255, 94, 20, 245))
    d.ellipse((66, 300, 180, 428), fill=(255, 230, 18, 255))
    img.save(path)


def draw_logo(path: Path) -> None:
    img = Image.new("RGBA", (1024, 384), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    for x, y, s in [(30, 94, 110), (160, 112, 150), (300, 76, 92)]:
        d.ellipse((x, y, x + s, y + s), fill=(255, 166, 0, 255), outline=(154, 92, 0, 255), width=4)
        d.ellipse((x + s * 0.38, y + s * 0.32, x + s * 0.46, y + s * 0.52), fill=(0, 0, 0, 255))
        d.ellipse((x + s * 0.55, y + s * 0.32, x + s * 0.63, y + s * 0.52), fill=(0, 0, 0, 255))
    d.text((430, 18), "DDRace", fill=(255, 160, 0, 255), font=font(120))
    d.text((430, 150), "Network", fill=(88, 190, 255, 255), font=font(110))
    img.save(path)


def main() -> None:
    OUT.mkdir(parents=True, exist_ok=True)
    data = json.loads(TEMPLATE_FILE.read_text(encoding="utf-8"))
    drawers = {
        "gameskin": draw_gameskin,
        "skin": draw_skin,
        "hud": draw_hud,
        "emoticons": draw_emoticons,
        "particles": draw_particles,
        "ddrace_logo": draw_logo,
    }
    for template in data["templates"]:
        drawer = drawers.get(template["id"])
        if drawer is None:
            continue
        output = OUT / f"{template['id']}.png"
        if output.exists() and os.environ.get("TTE_OVERWRITE_DEFAULTS") != "1":
            print(f"skipped existing {output}")
            continue
        drawer(output)
        print(f"created {output}")


if __name__ == "__main__":
    main()
