# NRMImageParser.gd
# Godot 4.x | Typed GDScript | Value-type correct | 4-space indent

const _RLE_MARKER := 0x69
const _RAW_MARKER := 0x6A
const _HEADER_SKIP := 18
const _ENTRY_HEADER_SIZE := 12


# ─────────────────────────────────────────────────────────────
# Decompression Engine
# ─────────────────────────────────────────────────────────────
class Decompressor:
	
	static func _expand_byte(byte: int) -> int:
		return (byte << 24) | (byte << 16) | (byte << 8) | byte
	
	static func process(src: PackedByteArray, target_size: int) -> PackedByteArray:
		var out := PackedByteArray()
		out.resize(target_size)
		
		var src_pos := 0
		var dst_pos := 0
		var src_len := src.size()
		
		while src_pos < src_len and dst_pos < target_size:
			var ctrl := src[src_pos]
			src_pos += 1
			
			if ctrl == _RLE_MARKER:
				var byte := src[src_pos]
				var count := src.decode_u16(src_pos + 1)
				src_pos += 3
				
				if count == 0: continue
				
				var expanded := _expand_byte(byte)
				var quad_count := count >> 2
				var remainder := count & 3
				
				for i in quad_count:
					out.encode_u32(dst_pos, expanded)
					dst_pos += 4
				
				for i in remainder:
					out[dst_pos] = byte
					dst_pos += 1
			
			elif ctrl == _RAW_MARKER:
				out[dst_pos] = src[src_pos]
				dst_pos += 1
				src_pos += 1
			else:
				out[dst_pos] = ctrl
				dst_pos += 1
		
		return out


# ─────────────────────────────────────────────────────────────
# Image Data Container
# ─────────────────────────────────────────────────────────────
class NRMImage:
	var size: int
	var unpacked_size: int
	var filepath: String
	var offset: int
	var _data: PackedByteArray
	var _header_end: int = -1
	var _loaded: bool = false
	
	func _init(p_size: int, unpacked_sz: int, p_path: String, data_offset: int):
		size = p_size
		unpacked_size = unpacked_sz
		filepath = p_path
		offset = data_offset
		_data = PackedByteArray()
	
	func load_data(src: PackedByteArray) -> void:
		if _loaded: return
		var chunk := src.slice(offset, offset + size)
		_data = Decompressor.process(chunk, unpacked_size)
		_loaded = true
	
	func get_data() -> PackedByteArray:
		return _data
	
	func has_data() -> bool:
		return _loaded and not _data.is_empty()
	
	func get_header() -> PackedByteArray:
		if not has_data() or _data.size() < 14:
			return PackedByteArray()
		if _header_end < 0:
			_header_end = _data.decode_u32(10)
		return _data.slice(0, _header_end)
	
	func get_dimensions() -> Vector2i:
		if not has_data() or _data.size() < 26:
			return Vector2i.ZERO
		return Vector2i(_data.decode_u32(18), _data.decode_u32(22))
	
	func unload_data() -> void:
		_data = PackedByteArray()
		_header_end = -1
		_loaded = false


# ─────────────────────────────────────────────────────────────
# Archive Parser
# ─────────────────────────────────────────────────────────────
class NRMParser:
	var _file: FileAccess
	var _buffer: PackedByteArray
	var _entries: Array[NRMImage]
	var _filepath_index: Dictionary
	var filepath: String
	
	func _init(path: String):
		filepath = path
		_file = FileAccess.open(path, FileAccess.READ)
		_entries = []
		_filepath_index = {}
	
	func _notification(what: int):
		if what == NOTIFICATION_PREDELETE and _file:
			_file.close()
	
	func parse() -> bool:
		if not _file or not _file.is_open():
			return false
		
		_buffer = _file.get_buffer(_file.get_length())
		var pos := _HEADER_SKIP
		var limit := _buffer.size()
		
		while pos + _ENTRY_HEADER_SIZE <= limit:
			pos += 5
			var entry_size := _buffer.decode_u32(pos)
			var unpacked_size := _buffer.decode_u32(pos + 4)
			var name_len := _buffer.decode_u32(pos + 8)
			
			if entry_size == 0:
				pos += _ENTRY_HEADER_SIZE
				continue
			
			var name_start := pos + _ENTRY_HEADER_SIZE
			var name_end := name_start + name_len
			if name_end > limit:
				break
			
			var path := _buffer.slice(name_start, name_end).get_string_from_ascii()
			var data_start := name_end
			
			if path.contains("bmp") or path.contains("msc"):
				var size = entry_size - name_len
				var img := NRMImage.new(size, unpacked_size, path, data_start)
				_filepath_index[path] = _entries.size()
				_entries.append(img)
			
			pos += _ENTRY_HEADER_SIZE + entry_size
		
		return true
	
	func count() -> int:
		return _entries.size()
	
	func has_path(path: String) -> bool:
		return _filepath_index.has(path)
	
	func get_by_index(idx: int) -> NRMImage:
		return _entries[idx] if idx < _entries.size() else null
	
	func get_by_path(path: String) -> NRMImage:
		var idx := _filepath_index.get(path, -1)
		return _entries[idx] if idx >= 0 else null
	
	func find_by_subpath(sub: String, exact: bool = false) -> NRMImage:
		if exact:
			var idx := _filepath_index.get(sub, -1)
			if idx >= 0:
				var entry = _entries[idx]
				entry.load_data(_buffer)
				return entry
		else:
			for entry in _entries:
				if entry.filepath.contains(sub):
					entry.load_data(_buffer)
					return entry
		return null
	
	func list_paths(prefix: String = "") -> PackedStringArray:
		var out := PackedStringArray()
		if prefix.is_empty():
			for entry in _entries:
				out.append(entry.filepath)
		else:
			for entry in _entries:
				if entry.filepath.begins_with(prefix):
					out.append(entry.filepath)
		return out
	
	func get_root_folders(base: String) -> PackedStringArray:
		var folders: Dictionary = {}
		for entry in _entries:
			if not entry.filepath.begins_with(base):
				continue
			var rel := entry.filepath.substr(base.length()).lstrip("/\\")
			if rel.is_empty():
				continue
			var sep := "/" if "/" in rel else ("\\" if "\\" in rel else "")
			var first := rel.split(sep)[0] if sep else rel
			if first.is_empty():
				continue
			var key := base + "/" + first if base and not base.ends_with("/") else base + first
			folders[key] = true
		var result := PackedStringArray()
		for key in folders:
			result.append(key)
		return result
	
	func load_all() -> void:
		for entry in _entries:
			entry.load_data(_buffer)
	
	func unload_all() -> void:
		for entry in _entries:
			entry.unload_data()
