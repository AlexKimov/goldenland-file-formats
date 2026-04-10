# utils.gd
# Helper functions ported from the original JavaScript module.

static func load_json_from_file(path: String) -> Dictionary:
	var file = FileAccess.open(path, FileAccess.READ)
	if not file:
		printerr("Failed to open JSON file: ", path)
		return {}
	var content = file.get_as_text()
	file.close()
	var json = JSON.new()
	var error = json.parse(content)
	if error:
		printerr("JSON parse error: ", json.get_error_message())
		return {}
	return json.data

static func get_filename_from_path(path: String) -> String:
	return path.get_file()

static func get_file_extension(file_name: String) -> String:
	return file_name.get_extension()

static func unpack8bit_to_32bit(value: int) -> int:
	return value | (value << 8) | (value << 16) | (value << 24)

static func split_last_occurrence(str: String, substring: String) -> Array:
	var last_index = str.rfind(substring)
	var before = ""
	var after = ""
	if last_index != -1:
		before = str.substr(0, last_index)
		after = str.substr(last_index + 1)
	return [before, after]

static func create_2d_array(m: int, n: int) -> Array:
	var arr = []
	arr.resize(m)
	for i in range(m):
		var row = []
		row.resize(n)
		for j in range(n):
			row[j] = 0
		arr[i] = row
	return arr

# Original getKey() used an undefined variable 'map'. It cannot be ported without context.
# static func get_key(val):
#     return [...map].find(([key, value]) => val === value)[0]

static func get_folders_from_relative_paths(root_dir: String, file_paths: Array) -> Array:
	var folders = {}
	for file_path in file_paths:
		if file_path.begins_with(root_dir):
			var parts = file_path.split("\\")
			var folder = parts[1] if parts.size() > 1 else ""
			folders[folder] = true
	var result = []
	for f in folders:
		result.append(f)
	return result

static func hex_to_integer(str: String) -> int:
	return str.trim_prefix("#").hex_to_int()

static func concat_array_buffers(buffer1: PackedByteArray, buffer2: PackedByteArray) -> PackedByteArray:
	var tmp = PackedByteArray()
	tmp.append_array(buffer1)
	tmp.append_array(buffer2)
	return tmp

static func get_value_from_str(str: String, delimiter: String = ":", pos: int = 1) -> String:
	var parts = str.split(delimiter)
	if parts.size() > pos:
		return parts[pos].strip_edges()
	return ""

static func get_index_from_str(str: String) -> int:
	var parts = str.split(":")
	if parts.is_empty():
		return -1
	var name_part = parts[0].strip_edges()
	var underscore_parts = name_part.split("_")
	if underscore_parts.size() > 1:
		return underscore_parts[1].to_int()
	return -1
