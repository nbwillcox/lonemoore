import unreal as u,json
from pathlib import Path
R=Path(r'J:\Lonemoore_Regional_Identity');assert Path(u.Paths.project_dir()).resolve()==(R/'UnrealTest').resolve();E=u.MaterialEditingLibrary;rows=[]
for name in ['Hell_CooledSlag','Hell_HangingBasalt']:
 for prefix in ['MI_','MI_UV_']:
  mi=u.load_asset('/Game/RegionalIdentity/Materials/'+prefix+name);assert mi
  before=E.get_material_instance_scalar_parameter_value(mi,'EmissionStrength');t=E.get_material_instance_texture_parameter_value(mi,'emission');assert t
  E.set_material_instance_scalar_parameter_value(mi,'EmissionStrength',80);E.update_material_instance(mi);u.EditorAssetLibrary.save_loaded_asset(mi)
  rows.append(dict(material=mi.get_path_name(),before=before,after=E.get_material_instance_scalar_parameter_value(mi,'EmissionStrength'),texture=t.get_path_name()))
(R/'reports/emission_calibration.json').write_text(json.dumps(rows,indent=2));print('EMISSION_CALIBRATED',rows)
