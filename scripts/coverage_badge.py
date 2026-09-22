"""Create an SVG badge from real gcovr JSON summary data (stdlib only)."""
import argparse
import json
from pathlib import Path


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("summary", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    data = json.loads(args.summary.read_text(encoding="utf-8"))
    total = data["line_total"]
    covered = data["line_covered"]
    if total <= 0 or not 0 <= covered <= total:
        raise ValueError("Coverage must contain a non-empty, valid line count")
    percent = 100.0 * covered / total
    color = "#4c1" if percent >= 95 else "#dfb317" if percent >= 80 else "#e05d44"
    label = f"{percent:.1f}%"
    svg = f"""<svg xmlns="http://www.w3.org/2000/svg" width="144" height="20" role="img" aria-label="line coverage: {label}">
<title>line coverage: {label}</title>
<rect width="94" height="20" fill="#555"/>
<rect x="94" width="50" height="20" fill="{color}"/>
<g fill="#fff" text-anchor="middle" font-family="Verdana,Arial,sans-serif" font-size="11">
<text x="47" y="14">line coverage</text><text x="119" y="14">{label}</text>
</g></svg>
"""
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(svg, encoding="utf-8")
    print(f"Line coverage: {covered}/{total} = {label}")


if __name__ == "__main__":
    main()
