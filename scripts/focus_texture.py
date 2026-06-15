from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any


ROOT = Path(__file__).resolve().parents[1]
TEMPLATE_FILE = ROOT / "data" / "templates" / "teeworlds_textures.json"


def load_template(template_id: str, part_id: str) -> tuple[dict[str, Any], dict[str, Any]]:
    data = json.loads(TEMPLATE_FILE.read_text(encoding="utf-8"))
    for template in data["templates"]:
        if template["id"] != template_id:
            continue
        for part in template["parts"]:
            if part["id"] == part_id:
                return template, part
        raise SystemExit(f"Parte '{part_id}' nao existe no template '{template_id}'.")
    raise SystemExit(f"Template '{template_id}' nao existe.")


def focus_texture(
    template_id: str,
    part_id: str,
    input_path: Path,
    output_path: Path,
    dim_strength: float,
    focus_scale: float,
) -> None:
    try:
        from PIL import Image, ImageDraw
    except ImportError as exc:
        raise SystemExit(
            "Pillow nao esta instalado. Rode: py -m pip install -r requirements.txt"
        ) from exc

    template, part = load_template(template_id, part_id)
    image = Image.open(input_path).convert("RGBA")

    expected_size = (int(template["width"]), int(template["height"]))
    if image.size != expected_size:
        print(
            f"Aviso: imagem tem {image.size[0]}x{image.size[1]}, "
            f"mas o template espera {expected_size[0]}x{expected_size[1]}."
        )

    x = int(part["x"])
    y = int(part["y"])
    w = int(part["w"])
    h = int(part["h"])
    box = (x, y, x + w, y + h)

    overlay = Image.new("RGBA", image.size, (0, 0, 0, int(255 * dim_strength)))
    mask = Image.new("L", image.size, 255)
    draw = ImageDraw.Draw(mask)
    draw.rectangle(box, fill=0)

    result = image.copy()
    result.alpha_composite(Image.composite(overlay, Image.new("RGBA", image.size), mask))

    focus = image.crop(box)
    max_focus_w = max(1, int(image.width * focus_scale))
    max_focus_h = max(1, int(image.height * focus_scale))
    ratio = min(max_focus_w / max(1, focus.width), max_focus_h / max(1, focus.height))
    new_size = (max(1, int(focus.width * ratio)), max(1, int(focus.height * ratio)))
    focus = focus.resize(new_size, Image.Resampling.NEAREST)

    px = (image.width - focus.width) // 2
    py = (image.height - focus.height) // 2
    result.alpha_composite(focus, (px, py))

    border = ImageDraw.Draw(result)
    border.rectangle((px, py, px + focus.width - 1, py + focus.height - 1), outline=(255, 255, 255, 230), width=2)
    border.rectangle(box, outline=(90, 220, 255, 255), width=2)

    output_path.parent.mkdir(parents=True, exist_ok=True)
    result.save(output_path)
    print(f"Preview criado: {output_path}")


def main() -> None:
    parser = argparse.ArgumentParser(description="Cria preview com foco em uma parte da textura.")
    parser.add_argument("--template", required=True, help="ID do template, exemplo: gameskin")
    parser.add_argument("--part", required=True, help="ID da parte, exemplo: gun")
    parser.add_argument("--input", required=True, type=Path, help="Imagem de entrada")
    parser.add_argument("--output", required=True, type=Path, help="Imagem de saida")
    parser.add_argument("--dim", default=0.5, type=float, help="Escurecimento do resto da imagem")
    parser.add_argument("--focus-scale", default=0.7, type=float, help="Tamanho maximo do foco na tela")
    args = parser.parse_args()

    focus_texture(args.template, args.part, args.input, args.output, args.dim, args.focus_scale)


if __name__ == "__main__":
    main()
