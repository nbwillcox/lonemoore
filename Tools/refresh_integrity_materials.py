import unreal as u, pathlib
root=pathlib.Path(u.Paths.project_dir());tools=u.AssetToolsHelpers.get_asset_tools()
source=(root/'Tools/import_integrity_kit.py').read_text()
exec(source[source.index('def material('):source.index("(root/'Saved/IntegrityUpdate/import_bounds.json')")])
exec((root/'Tools/import_integrity_ambience.py').read_text())
print('INTEGRITY_MATERIALS_REFRESHED')
