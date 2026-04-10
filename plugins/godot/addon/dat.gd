class_name DatParser extends RefCounted

const Helpers = preload("res://addons/nrm_tiles_import/utils.gd")
const Constants = preload("res://addons/nrm_tiles_import/constants.gd")

var groups: Array[Dictionary] = []
var _properties: Dictionary
var _file_path: String

var _json_properties = {
	Constants.DATFiles.ANIM_DAT_FILENAME: {
		"names": ["name", "size"]
	},
	Constants.DATFiles.TEXTUR_DAT_FILENAME: {
		"names": ["name", "size", "snd"]
	},
	Constants.DATFiles.LOWPICTURE_DAT_FILENAME: {
		"names": ["name", "size"]
	},
	Constants.DATFiles.PICTURE_DAT_FILENAME: {
		"names": ["name", "size"]
	},
	Constants.DATFiles.TREES_DAT_FILENAME: {
		"names": ["name", "size"]
	},
	Constants.DATFiles.PEOPLE_DAT_FILENAME: {
		"names": ["name", "snd", "id"]
	},
	Constants.DATFiles.LOC_DAT_FILENAME: {
		"names": ["loc", "num"]
	}
}

func _init(file_path: String) -> void:
	_file_path = file_path	
	var filename: String = file_path.get_file()
	_properties = _json_properties[filename]
	
func parse() -> Error:

	var file := FileAccess.open(_file_path, FileAccess.READ)
	if not file:
		push_error("DatFileParser: Failed to open file '%s'" % _file_path)
		return FileAccess.get_open_error()

	var lines := file.get_as_text().split("\n")
	file.close()

	var i: int = 0
	var line_count: int = lines.size()
	
	while i < line_count:
		var line: String = lines[i].strip_edges()
		if line.is_empty():
			i += 1
			continue

		if line.contains("nGroup"):
			var group_count: int = int(Helpers.get_value_from_str(line))
			groups.resize(group_count)
			for g in range(group_count):
				groups[g] = {}
			i += 1
			continue

		for prop_name in _properties.names as Array[String]:
			
			if i >= line_count:
				break

			line = lines[i].strip_edges()

			var value: String = Helpers.get_value_from_str(line)
			var index: int = Helpers.get_index_from_str(line)

			if index >= 0 and index < groups.size():
				groups[index][prop_name] = value
			i += 1

	return OK
