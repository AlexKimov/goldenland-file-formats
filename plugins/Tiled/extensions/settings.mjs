class Sprite {
  constructor(data = {}) {
        this.width = data.width;
        this.height = data.height;
        this.count = data.count;
        this.fps = data.fps;
  }
}

class ImageFile {
  constructor(data = {}) {
    this.index = data.index !== null ? data.index : 0;
    this.subIndex = data.subIndex !== null ? data.subIndex : 0;
    this.description = data.description !== null ? data.description : "Animated background tiles and some other objects.";
    this.root = data.root !== null ? data.root : "anim";
    this.subdirs = data.subdirs;
    this.files = data.files !== null ? data.files : [];
    this.settings = data.settings; 
    this.tileset = data.tileset !== null ? data.tileset : 0;
    this.dat = data.dat !== null ? data.dat : 0;
    this.parent = data.parent; 
  }
}

class Tileset {
  constructor(data = {}) {
    this.name = data.name !== null ? data.name : "";
    this.classname = data.classname !== null ? data.classname : "background_animated";
    let sprite = null;
    if (data.sprite !== null)
        sprite = new Sprite(data.sprite);   
    this.sprite = sprite;
    this.transparency = data.transparency !== null ? data.transparency : true;
    this.animation = data.animation !== null ? data.animation : true;
    this.naming = data.naming !== null ? data.naming : 2;
  }
}

class Settings {
  constructor(sourceData) {
    this.currentImageFilesIndex = sourceData.currentImageFilesIndex;
    this.settingsFiles = sourceData.settingsFiles;
    this.imageFiles = (sourceData.imageFiles || []).map(file => new ImageFile(file));
    this.datFiles = sourceData.datFiles;
    this.tilesets = (sourceData.tilesets || []).map(tileset => new Tileset(tileset));
  }
}

export default Settings;