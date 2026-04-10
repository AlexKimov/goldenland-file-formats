extends RefCounted

const CONSTS = preload("res://addons/nrm_tiles_import/constants.gd")

## Returns the exact array Godot uses to build the Import Dock UI
static func get_options() -> Array[Dictionary]:
	return [
		{
			"name": CONSTS.OptionKeys.GAME_DATA_DIRECTORY,
			"default_value": "F:/Games/1/",
			"property_hint": PROPERTY_HINT_GLOBAL_DIR,
			"hint_string": ""
		},
		{
			"name": CONSTS.OptionKeys.BACKGROUND_TILE_WIDTH,
			"default_value": 112,
			"property_hint": PROPERTY_HINT_RANGE,
			"hint_string": "0,300,1" 
		},
		{
			"name": CONSTS.OptionKeys.BACKGROUND_TILE_HEIGHT,
			"default_value": 64,
			"property_hint": PROPERTY_HINT_RANGE,
			"hint_string": "0,300,1"  
		},
		{
			"name": CONSTS.OptionKeys.ANIMATION,
			"default_value": true,
			"property_hint": PROPERTY_HINT_NONE
		},
		{
			"name": CONSTS.OptionKeys.TILE_COLOR_TRANSPARENCY,
			"default_value": true,
			"property_hint": PROPERTY_HINT_NONE
		},
	]

static func get_preset_count() -> int:
	return 1

static func get_preset_name(preset_index: int) -> String:
	return ["Default"][preset_index]

## Returns only the values that differ from defaults for this preset
static func get_preset_overrides(preset_index: int) -> Dictionary:
	match preset_index:
		0: return {
			CONSTS.OptionKeys.BACKGROUND_TILE_WIDTH: 112,
			CONSTS.OptionKeys.BACKGROUND_TILE_HEIGHT: 64,
			CONSTS.OptionKeys.ANIMATION: true,
			CONSTS.OptionKeys.TILE_COLOR_TRANSPARENCY: true
		}
	return {}
