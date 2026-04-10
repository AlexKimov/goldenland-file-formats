class_name MAPParser extends RefCounted

# ------------------------------------------------------------------------------
# Data classes (dataclass style)
# ------------------------------------------------------------------------------
class MAPTile:
	var coll: int = 0
	var row: int = 0
	var x_offset: int = 0
	var y_offset: int = 0
	var group_index: int = 0
	var sprite_index: int = 0
	
	func _init(
		group_idx: int = 0,
		tile_idx: int = 0,		
		cell_coll: int = 0, 
		cell_row: int = 0, 
		x_offs: int = 0, 
		y_offs: int = 0) -> void:
			
		coll = cell_coll
		row = cell_row
		x_offset = x_offs
		y_offset = y_offs
		group_index = group_idx
		sprite_index = tile_idx


class MAPObject:
	var coll_sprite_num: int = 0
	var row_sprite_num: int = 0
	var background_sprite_num: int = 0
	var objects_sprite_num: int = 0
	var trees_sprite_num: int = 0
	var constraints_objects_num: int = 0
	
	var background_tiles: Array = [] 
	var objects_tiles: Array = []      
	var trees_tiles: Array = []       
	
	func _init() :
		background_tiles = []
		objects_tiles = []
		trees_tiles = []
		
	func add_background_tile(tile):	
		var index = background_tiles.size()	
		tile.row = index / row_sprite_num		
		tile.coll = index - (tile.row * row_sprite_num)
			 
		background_tiles.append(tile)


# ------------------------------------------------------------------------------
# Parser
# ------------------------------------------------------------------------------
var _file: FileAccess
var map: MAPObject


func _init():
	map = MAPObject.new()


func close() -> void:
	if _file:
		_file.close()


func parse(filename: String) -> void:
	_file = FileAccess.open(filename, FileAccess.READ)
	if not _file:
		printerr("MAPFileParser: Failed to open ", filename)
		return
	
	# Header (little-endian)
	_file.seek(0)
	map.row_sprite_num = _file.get_16()
	_file.seek(2)
	map.coll_sprite_num = _file.get_16()
	
	_file.seek(8)
	map.background_sprite_num = _file.get_16()
	map.objects_sprite_num = _file.get_16()
	map.trees_sprite_num = _file.get_16()
	
	_file.seek(16)
	map.constraints_objects_num = _file.get_32()
	
	# Background tiles
	_file.seek(36)
	
	var pos = 36
	map.background_tiles.clear()
	for i in range(map.background_sprite_num):
		var group_idx = _file.get_16()
		_file.seek(pos + 2)
		var sprite_idx = _file.get_16()
		map.add_background_tile(MAPTile.new(group_idx, sprite_idx))
		pos += 4;
		
	# Objects tiles (commented out in original – kept as comment)
	# var ofs = 36 + map.backgroundSpriteNum * 4
	# _file.seek(ofs)
	# for i in range(map.objectsSpriteNum):
	#     var groupIndex = _file.get_8()
	#     var spriteIndex = _file.get_8()
	#     var xOffset = _file.get_16()
	#     var yOffset = _file.get_16()
	#     var xTileIndex = _file.get_8()
	#     var yTileIndex = _file.get_8()
	#     map.objectsTiles.append({...})
	
	# Trees tiles (commented out)
	# similar loop
	
	close()
