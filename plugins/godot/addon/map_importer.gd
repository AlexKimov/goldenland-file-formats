# Converts a Heath: The Unchosen Path .map file into a Node2D with TileMapLayer children.
@tool
class_name HeathMapImporter extends RefCounted

# --- Imports (you must provide these scripts) ---
const Constants = preload("res://addons/nrm_tiles_import/constants.gd")
const Helpers = preload("res://addons/nrm_tiles_import/utils.gd")
const MAP = preload("res://addons/nrm_tiles_import/map.gd")
const DAT = preload("res://addons/nrm_tiles_import/dat.gd")
const NRM = preload("res://addons/nrm_tiles_import/nrm.gd")

# ------------------------------------------------------------------------------
# Public API
# ------------------------------------------------------------------------------
static func import_map(map_path: String, options: Dictionary) -> Node2D:
	var importer = HeathMapImporter.new()
	return importer._read(map_path, options)

func _read(filename: String, options: Dictionary) -> Node2D:
	# Parse .map file
	var map_parser = MAP.new()
	map_parser.parse(filename)
	# Root node to hold all layers
	var root = Node2D.new()
	root.name = "Heath Map"
	var layer = TileMapLayer.new()

	# Create TileMapLayers (each layer gets its own TileMapLayer node)
	var width = options[Constants.OptionKeys.BACKGROUND_TILE_WIDTH]
	var height = options[Constants.OptionKeys.BACKGROUND_TILE_HEIGHT]

	var static_bg_sprites = _create_tile_map_layer(root, "Background static tilemaps")
	var game_dir = options[Constants.OptionKeys.GAME_DATA_DIRECTORY]

	# Static background sprites (textur.paxx.nrm + textur.dat)
	var textur_nrm = NRM.NRMParser.new("%s/%s" % [game_dir, Constants.NRMFiles.TEXTUR_NRM_FILENAME])
	textur_nrm.parse()
	
	var textur_dat = DAT.new("%s/Data/%s" % [game_dir, Constants.DATFiles.TEXTUR_DAT_FILENAME])
	textur_dat.parse()
	
	_create_tileset_from_map_tiles(
		static_bg_sprites, map_parser.map.background_tiles, map_parser.map.row_sprite_num,
		textur_nrm, textur_dat, false
	)
	#
	## Animated background sprites (anim.paxx.nrm + anim.dat)
	#var anim_nrm = NRMFileImageParser.new()
	#anim_nrm.read(Constants.GAME_DIR + "anim.paxx.nrm")
	#var anim_dat = DatFileParser.new()
	#anim_dat.read(Constants.GAME_DIR + "Data/anim.dat", properties)
	#_create_tileset_from_map_tiles(
		#animated_layer_node, map_data.background_tiles, map_data.row_sprite_num,
		#anim_nrm, anim_dat, true, 5000
	#)
	#
	## TODO: sprites with offset (not natively supported by TileMapLayer)
	
	return root


func _create_tile_map_layer(parent: Node2D, name: String) -> TileMapLayer:
	var layer = TileMapLayer.new()
	layer.name = name
	
	parent.add_child(layer)
	layer.owner = parent
	
	return layer

func _get_tile_data_from_map_tiles(map_file_tiles, dat_file, is_animated, start_index):
	var tiles_data = {}
	var index = -1
	var end_index = start_index + dat_file.groups.size() + 1

	for tile in map_file_tiles:
		var group_index = tile.group_index
		
		if group_index >= start_index and group_index < end_index:
			if start_index > 0:
				group_index = group_index % start_index - 1
			
			var name = dat_file.groups[group_index]["name"]
			var filename = name + "_" + str(tile.sprite_index + 1) + ".bmp"

			if is_animated:
				filename = name.to_lower()
			
			if not tiles_data.has(filename):
				var size = int(dat_file.groups[group_index]["size"])
				if is_animated:
					index += size
				else:
					index += 1
				tiles_data[filename] = {"size": size, "index": index}
				if dat_file.groups[group_index].size() > 2:
					tiles_data[filename]["snd"] = int(dat_file.groups[group_index]["snd"])
			
	return tiles_data


func _create_tileset_from_images(nrm_file, tiles_data, is_animated) -> TileSet:
	var tile_set = TileSet.new()
	var bmp_images: Array[Image] = []
	
	for name in tiles_data:
		var img = nrm_file.find_by_subpath(name)
		if img == null:
			continue		
		
		var bmp_image = Image.new()
		bmp_image.load_bmp_from_buffer(img.get_data())
		bmp_images.append(bmp_image)
	
	var atlas_source = TileSetAtlasSource.new()
	atlas_source.texture = combine_images_to_grid_atlas(bmp_images, 16)

	atlas_source.texture_region_size = Vector2i(112, 64)

	var coll: int = 0
	var row: int = -1
	for i in range(tiles_data.size()):
		coll = i % 16
		if (coll == 0):
			row += 1
			
		var tile_coords = Vector2i(coll, row)
		atlas_source.create_tile(tile_coords)
		
	tile_set.add_source(atlas_source)			
	tile_set.tile_size = Vector2i(112, 64)
		
	tile_set.tile_shape = TileSet.TILE_SHAPE_ISOMETRIC
	tile_set.tile_layout = TileSet.TILE_LAYOUT_STACKED
	tile_set.resource_name = nrm_file.filepath.get_file()
	
	return tile_set

func combine_images_to_grid_atlas(images: Array[Image], columns: int, max_texture_size: int = 16384) -> ImageTexture:
	if images.is_empty() or columns <= 0:
		push_error("Invalid input: images empty or columns <= 0")
		return null

	# Calculate cell size (max width and height among all images)
	var cell_width = 0
	var cell_height = 0
	for img in images:
		cell_width = max(cell_width, img.get_width())
		cell_height = max(cell_height, img.get_height())

	var rows = ceili(float(images.size()) / columns)
	var atlas_width = columns * cell_width
	var atlas_height = rows * cell_height

	# Ensure texture size doesn't exceed GPU limit
	if atlas_width > max_texture_size or atlas_height > max_texture_size:
		push_error("Atlas size (%dx%d) exceeds max texture size %d" % [atlas_width, atlas_height, max_texture_size])
		return null

	# Create atlas image (use a format that supports transparency if any image has alpha)
	var atlas_img = Image.create(atlas_width, atlas_height, false, Image.FORMAT_RGBA8)
	atlas_img.fill(Color(0, 0, 0, 0))

	# Blit each image into its grid cell (centered at the top-left of the cell)
	for i in images.size():
		var img = images[i]
		var col = i % columns
		var row = i / columns
		var dest_x = col * cell_width
		var dest_y = row * cell_height
		var dest_pos = Vector2i(dest_x, dest_y)
		var src_rect = Rect2i(Vector2i.ZERO, img.get_size())
		atlas_img.blit_rect(img, src_rect, dest_pos)

	return ImageTexture.create_from_image(atlas_img)

func _create_tileset_from_map_tiles(layer: TileMapLayer, map_file_tiles, row_sprite_num, nrm_file, dat_file, is_animated, start_index = 0):
	var tiles_data = _get_tile_data_from_map_tiles(map_file_tiles, dat_file, is_animated, start_index)
	## Assign the TileSet to the layer
	layer.tile_set = _create_tileset_from_images(nrm_file, tiles_data, is_animated)
	
	for tile in map_file_tiles:
		layer.set_cell(Vector2i(tile.coll, tile.row), 0, Vector2i(0, 0))
