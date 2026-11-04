"""Read-only inventory checks; no compiler or third-party Python packages required."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import sys
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--original', type=Path, help='Optional original Cpp_Files folder')
    parser.add_argument('--manifest', type=Path, help='Write original selected-file SHA256 manifest')
    args = parser.parse_args()
    errors = []
    sources = sorted((ROOT / 'src').glob('*'))
    for source in sources:
        if source.suffix not in ('.h', '.cpp'):
            continue
        text = source.read_text(encoding='utf-8-sig')
        for include in re.findall(r'^\s*#include\s*"([^"]+)"', text, re.M):
            if not (source.parent / include).is_file():
                errors.append(f'{source.name}: missing local include {include}')
        if re.search(r'[A-Za-z]:[/\\]', text):
            errors.append(f'{source.name}: contains an absolute drive path')
    runtime = (ROOT / 'src/runtimeAssets.cpp').read_text(encoding='utf-8')
    assets = sorted(set(re.findall(r'"((?:texture|font)/[^"]+|map\.txt|base\.txt)"', runtime)))
    for name in assets:
        path = ROOT / 'assets' / name
        if not path.is_file() or path.stat().st_size == 0:
            errors.append(f'Missing or empty asset: {name}')
        elif path.suffix == '.png' and not path.read_bytes().startswith(b'\x89PNG\r\n\x1a\n'):
            errors.append(f'Invalid PNG signature: {name}')
        elif path.suffix == '.ttf' and path.read_bytes()[:4] not in (b'\0\1\0\0', b'OTTO'):
            errors.append(f'Invalid font signature: {name}')
    if not (ROOT / 'assets/font/OFL.txt').is_file():
        errors.append('Missing bundled font license')
    lines = (ROOT / 'assets/map.txt').read_text().splitlines()
    size, rows = int(lines[0]), lines[1:]
    if not rows or any(len(row) != len(rows[0]) or not re.fullmatch(r'[0-9]+', row) for row in rows):
        errors.append('Invalid map rows')
    elif len(rows) * len(rows[0]) != size:
        errors.append('Declared map size does not match rows')
    else:
        width, height = len(rows[0]), len(rows)
        values = list(map(int, (ROOT / 'assets/base.txt').read_text().split()))
        occupied = set()
        if len(values) != 6:
            errors.append('base.txt requires six integers')
        else:
            for i in range(2):
                team, center, spawn = values[3*i:3*i+3]
                x, y = center % width, center // width
                if team != i+1 or not 0 <= spawn < size or not (1 <= x < width-1 and 1 <= y < height-1):
                    errors.append('Invalid base team/center/spawn')
                    continue
                footprint = {center + dy*width + dx for dy in range(-1, 2) for dx in range(-1, 2)}
                if occupied & footprint or any(rows[p//width][p%width] != '0' for p in footprint):
                    errors.append('Base footprint is obstructed or overlaps')
                occupied |= footprint
            for spawn in values[2::3]:
                if 0 <= spawn < size and (spawn in occupied or rows[spawn//width][spawn%width] != '0'):
                    errors.append('Obstructed spawn point')
    cmake = (ROOT / 'CMakeLists.txt').read_text(encoding='utf-8')
    for source in sources:
        if source.suffix == '.cpp' and f'src/{source.name}' not in cmake:
            errors.append(f'{source.name}: omitted from CMake build')
    print(f'Local source/header files: {len(sources)}; runtime assets: {len(assets)}')
    print(f'Map: {len(rows[0])} x {len(rows)} = {size} cells')
    if args.original:
        original = args.original.resolve()
        for project in sorted(original.glob('**/*.vcxproj')):
            missing = []
            for element in ET.parse(project).getroot().iter():
                if element.tag.rsplit('}', 1)[-1] in ('ClCompile', 'ClInclude', 'Text', 'ResourceCompile', 'None'):
                    name = element.get('Include')
                    if name and '$(' not in name and not (project.parent / name).is_file():
                        missing.append(name)
            print(f'Original {project.relative_to(original)}: missing entries = {missing}')
            if project == original / 'game/game/game.vcxproj' and missing:
                errors.append('Selected original VS project references missing files')
        selected = [*sorted((original / 'game/game').glob('*.cpp')),
                    *sorted((original / 'game/game').glob('*.h')),
                    original / 'game/game/map.txt', original / 'game/game/base.txt',
                    *sorted((original / 'game/texture').glob('*.png')),
                    original / 'game/ClassDiagram.png']
        for name in ('map.txt', 'base.txt'):
            if (ROOT / 'assets' / name).read_bytes() != (original / 'game/game' / name).read_bytes():
                errors.append(f'Original data changed: {name}')
        for image in (original / 'game/texture').glob('*.png'):
            if image.read_bytes() != (ROOT / 'assets/texture' / image.name).read_bytes():
                errors.append(f'Original image changed: {image.name}')
        if args.manifest:
            args.manifest.write_text(json.dumps({
                'selected_version': 'game',
                'files': {p.relative_to(original).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest()
                          for p in selected}}, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    for error in errors:
        print('FAIL:', error, file=sys.stderr)
    print('PASS' if not errors else f'{len(errors)} issue(s)')
    return bool(errors)


if __name__ == '__main__':
    sys.exit(main())
