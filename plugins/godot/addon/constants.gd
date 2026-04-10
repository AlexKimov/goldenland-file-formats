extends RefCounted

# ── Plugin Identity ──────────────────────────────────────────────────
const PLUGIN_NAME: StringName = "Heath import adddon"
const VISIBLE_NAME: String = "Heath map (.map)"
const RECOGNIZED_EXT: String = "map"
const RESOURCE_TYPE: String = "PackedScene"
const SAVE_EXT: String = "tscn"

# ── NRM files  ──────────────
class NRMFiles:
	const TEXTUR_NRM_FILENAME: String = "textur.paxx.nrm"
	const ANIM_NRM_FILENAME: String = "anim.paxx.nrm"
	
# ── DAT files  ──────────────
class DATFiles:
	const TEXTUR_DAT_FILENAME: String = "textur.dat"
	const ANIM_DAT_FILENAME: String = "amim.dat"
	const LOWPICTURE_DAT_FILENAME: String = "lowpicture.dat"
	const PICTURE_DAT_FILENAME: String = "picture.dat"
	const TREES_DAT_FILENAME: String = "trees.dat"
	const PEOPLE_DAT_FILENAME: String = "people.dat"
	const LOC_DAT_FILENAME: String = "loc.dat"
	
# ── Option Keys ──────────────
class OptionKeys:
	const GAME_DATA_DIRECTORY: String = "game_data_directory"
	const BACKGROUND_TILE_WIDTH: String = "background_tile_width"
	const BACKGROUND_TILE_HEIGHT: String = "background_tile_height"
	const ANIMATION: String = "animation"
	const TILE_COLOR_TRANSPARENCY: String = "tile_color_transparency"

# ── Preset Configuration ─────────────────────────────────────────────
class Presets:
	const COUNT: int = 1
	const DEFAULT: int = 0
	const NAMES: Array[String] = ["Default"]

	static func get_overrides(preset_index: int) -> Dictionary:
		match preset_index:
			Presets.DEFAULT: return {}
		return {}
