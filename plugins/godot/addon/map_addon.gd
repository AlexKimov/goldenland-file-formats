@tool
extends EditorImportPlugin
#@export_global_dir var default_dir: String = "F:/Games/1/"

# Preload separated modules (no runtime overhead)
const CONSTS   = preload("res://addons/nrm_tiles_import/constants.gd")
const UI  = preload("res://addons/nrm_tiles_import/map_import_options.gd")
const MapImporter = preload("res://addons/nrm_tiles_import/map_importer.gd")

# ── Metadata ─────────────────────────────────────────────────────────
func _get_importer_name() -> String: return CONSTS.PLUGIN_NAME

func _get_visible_name() -> String: return CONSTS.VISIBLE_NAME

func _get_recognized_extensions() -> PackedStringArray: return [CONSTS.RECOGNIZED_EXT]

func _get_resource_type() -> String: return CONSTS.RESOURCE_TYPE

func _get_save_extension() -> String: return CONSTS.SAVE_EXT

func _get_priority() -> float: return 1.0

func _get_import_order() -> int: return 0  # Standard import order

# ── UI & Presets ─────────────────────────────────────────────────────
func _get_import_options(path: String, preset_index: int) -> Array[Dictionary]:
	return UI.get_options()

func _get_preset_count() -> int:
	return UI.get_preset_count()

func _get_preset_name(preset_index: int) -> String:
	return UI.get_preset_name(preset_index)

func _get_import_options_for_preset(preset_index: int) -> Array[Dictionary]:
	var options := UI.get_options().duplicate(true)
	var overrides := UI.get_preset_overrides(preset_index)
	
	for opt in options:
		if overrides.has(opt.name):
			opt.default_value = overrides[opt.name]
	return options
	
func _get_option_visibility(path: String, option_name: StringName, options: Dictionary) -> bool:
	return true	

# ── Import Execution ─────────────────────────────────────────────────
func _import(
	source_file: String, 
	save_path: String, 
	options: Dictionary,
	platform_variants: Array[String], 
	gen_files: Array[String]
) -> Error:			
		
	var base_name = source_file.get_basename()
	var scene_path = "%s.tscn" % [base_name]
		
	if source_file.get_extension().to_lower() == "map":
		var tile_map = MapImporter.import_map(source_file, options)

		var packed = PackedScene.new()
		if packed.pack(tile_map) != OK: return ERR_CANT_CREATE
		tile_map.free() 
			
		var save_result = ResourceSaver.save(packed, scene_path)
		if save_result != OK:
			return save_result

	return OK
	
	

	
