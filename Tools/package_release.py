"""Archive the clean Windows Shipping package and write its SHA-256 checksum."""
import hashlib
import pathlib
import zipfile

root = pathlib.Path(__file__).resolve().parents[1]
package = root / 'Builds/Distribution/Windows'
output = root / 'Dist'
output.mkdir(exist_ok=True)
archive = output / 'Lonemoore-0.2.0-Windows-x64.zip'
launcher = package / 'play-lonemoore.exe'
legacy_launcher = package / 'DungeonCrawler.exe'
if legacy_launcher.is_file():
    legacy_launcher.replace(launcher)
if not launcher.is_file():
    raise SystemExit('Build the Shipping distribution first.')

with zipfile.ZipFile(archive, 'w', zipfile.ZIP_DEFLATED, compresslevel=6) as z:
    for path in sorted(package.rglob('*')):
        relative = path.relative_to(package)
        if not path.is_file() or any(p.lower() in ('saved', 'intermediate') for p in relative.parts):
            continue
        if path.suffix.lower() in ('.pdb', '.log', '.dmp', '.sav', '.debug'):
            continue
        if path.name.startswith('Manifest_'):
            continue
        z.write(path, relative.as_posix())
    z.writestr('README.txt', '''Lonemoore 0.2.0 - Windows x64 Shipping build

Extract the entire ZIP into a writable folder and run play-lonemoore.exe.
Keep all supplied folders together. Unreal Editor is not required.
If needed, install Engine/Extras/Redist/en-us/vc_redist.x64.exe.

W/S move, A/D turn, E interact, M map, I inventory, C character,
J journal, T town, Esc menu, F5 quicksave, F9 quickload confirmation.

Start a fresh New Game. Back up existing saves before upgrading.
Shipping saves normally live in %LOCALAPPDATA%/DungeonCrawler/Saved/SaveGames.
This early release still needs sustained player testing for balance and pacing.

Source, full player guide, and known issues:
https://github.com/nbwillcox/lonemoore
''')
with archive.open('rb') as handle:
    digest = hashlib.file_digest(handle, 'sha256').hexdigest()
(output / 'SHA256SUMS.txt').write_text(f'{digest}  {archive.name}\n', encoding='utf-8')
print(f'{archive}: {archive.stat().st_size:,} bytes\nSHA-256: {digest}')
