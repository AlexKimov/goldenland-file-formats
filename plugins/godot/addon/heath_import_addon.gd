@tool
extends EditorPlugin

const ImportPlugin = preload("res://addons/nrm_tiles_import/map_addon.gd")
var _import_plugin: EditorImportPlugin

func _enter_tree() -> void:
	_import_plugin = ImportPlugin.new()
	add_import_plugin(_import_plugin)
	pass

func _exit_tree() -> void:
	if _import_plugin:
		remove_import_plugin(_import_plugin)
		_import_plugin = null
	pass
