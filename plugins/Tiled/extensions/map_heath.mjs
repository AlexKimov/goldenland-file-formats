import { Helpers } from 'utils.mjs';
import { BMP } from 'bmp.mjs';
import Settings from 'settings.mjs';
import MagicFileParser from 'magic.mjs';
import DatFileParser from 'dat.mjs';
import NRM8bitImage from 'nrm.mjs';
import NRMFileImageParser from 'nrm.mjs';
import MAPFileParser from 'map.mjs';
//
const SPRITES_FILE_FORMAT = 'bmp'
const IS_LITTLE_ENDIAN = true
const scriptPath = __filename
const TILE_WIDTH = 112 
const TILE_HEIGHT = 64
const GAME_DIR = "F:\\Games\\1\\"

function getAnimatedSpriteDimensions(dimensions, spriteNum = 0) {
    if (dimensions[0] > (dimensions[1] / 2)) {
        width = (dimensions[0] / spriteNum);
        height = (dimensions[1] / spriteNum);
    } else {
        width = dimensions[0];
        height = dimensions[1]/spriteNum;
    }
    
    return [width, height];
};  

function createTileset(filename, classname = "") {
    const tileset = new Tileset(filename);
    
    tileset.objectAlignment = Tileset.Center;
    tileset.tileRenderSize = Tileset.TileSize;
    tileset.orientation = Tileset.Orthogonal;
    tileset.className = classname;
    
    tileset.margin = 0;
    tileset.tileSpacing = 0;   

    return tileset;
}

function animateTiles(tiles, splitBySpriteNum = 0, fps) {
    let msec = Math.round(1000 / fps);  

    if (splitBySpriteNum > 0) {
        let animTile = tiles[splitBySpriteNum - 1]; 
        let indx = 0;
        let frames = [];  
        
        tiles.forEach((value, index) => {           
            frames.push({tileId: index, duration: msec});
            value.setProperty("Index", `${indx}` );
                        
            indx += 1;
            if ((index + 1) % splitBySpriteNum === 0) {
                animTile.frames = frames.reverse();
                animTile = tiles[index + splitBySpriteNum];
                indx = 0;
                frames = [];
            };
            
        });
    } else {
        let animTile = tiles[tiles.length - 1]; 
        
        const frames = [];
        
        tiles.forEach((value, index) => {          
            frames.push({tileId: (tiles.length - 1 - index), duration: msec});
            value.setProperty("Index", `${index}` );
        });
        
        animTile.frames = frames; 
    }
}

function addImageTileToTileset(data, tileset, name, transparency){
    let fileData = data;
    if (transparency) {
        fileData =  BMP.convert8BitImageTo32Bit(data);
    }
    
    const img = new Image();
    img.loadFromData(fileData, SPRITES_FILE_FORMAT);  
    
    const tile = tileset.addTile();
    
    tile.setImage(img);
    const parts = name.split("\\");
    
    if (parts.length > 0) {
        const filename = name.split("\\")[parts.length - 1];
        const objectName = name.split("\\")[parts.length - 2];
        tile.setProperty("Filename", `${filename}` );    
        tile.setProperty("ObjectName", `${objectName}` );    
    } else
        tile.setProperty("Filename", `${name}` );
    
    return tile;    
}

function addImageTilesToTileset(imageFile, tileset, name, spriteNum, transparency = true) {
    const files = BMP.splitImage(imageFile.data, spriteNum);
    
    if (files.length > 0) {
        for (const file of files) {
            const tile = addImageTileToTileset(file, tileset, name, transparency);                
        }     
    } else {     
        addImageTileToTileset(imageFile.data, tileset, name, transparency);
    }
        
    animateTiles(tileset.tiles, spriteNum, 24);
}

function createArchiveTileset(archive, settings) {
    const index = settings.currentImageFilesIndex;
    let imgFiles;
    if (index)
        imgFiles = [settings.imageFiles[index - 1]];
    else
        imgFiles = settings.imageFiles;
    let tileset;  
           
    for (const ifiles of imgFiles) {  
        let sets = settings.tilesets[ifiles.tileset];        
        tileset = createTileset(sets.name, sets.classname);
          
        let imageFiles; 
        if (ifiles.index) 
            imageFiles = [ifiles.files[ifiles.subIndex][ifiles.index - 1]];      
        else
            imageFiles = ifiles.files[ifiles.subIndex];
        for (const file of imageFiles) {
            const img = archive.getImageByIndex(file.index);

            if (!sets.animation)            
                addImageTileToTileset(img.data, tileset, img.filepath, sets.transparency);
            else { 
                let width = 0; 
                let height = 0;
                if (sets.sprite !== null) {
                    width = sets.sprite.width, 
                    height = sets.sprite.height
                } 
                let spriteNum = 0;
                let isPerson = false;
                let fps = 24;
                if (sets.sprite !== null) {
                    spriteNum = sets.sprite.count[file.spriteIndex];
                    isPerson = (sets.sprite.count.length > 1);
                    fps = sets.sprite.fps;
                }

                addImageTilesToTileset(img, tileset, width, height, spriteNum, isPerson, fps, sets.transparency);
            } 
        }       
    }
 
    return tileset;    
}

const watchForStateChange = function (widget, stateKey, state) {
    if (widget.currentTextChanged) {
        widget.currentTextChanged.connect((newValue) => {
            state[stateKey] = newValue;
        });
    }
    if (widget.valueChanged) {
        widget.valueChanged.connect((newValue) => {
             state[stateKey] = newValue;
        });
    }    
};

function getFilepath(path) {
   return path.substring(0, scriptPath.lastIndexOf('/') + 1);
}

function loadJSONFromFile(path) {
    let file = new TextFile(path, TextFile.ReadOnly);
    let content = file.readAll();
    file.close();
    return JSON.parse(content);
}

let heathTilesetFormat = {
    name: "Heath: The Unchosen Path tileset format",
    extension: "nrm",
    read: function (filename) {
        const tilesetName = Helpers.getFilenameFromPath(filename);
        const nrmFile = new NRMFileImageParser(filename);
        nrmFile.read();
        
        const jsonFilename = getFilepath(scriptPath) + "settings.json";          
        let defaultData = loadJSONFromFile(jsonFilename);
        const importSettings = defaultData[tilesetName];
        
        if (importSettings === undefined) {
            tiled.error("File isn't supported");
            return 1;
        }
        
        const settings = new Settings(importSettings);
        
        let state = {"settings": settings}; 
        let dialog = new Dialog();
          
        dialog.addSeparator("Paths");
        
        dialog.addHeading("Game dir: ");
        let filePicker = dialog.addFilePicker("");
            filePicker.fileUrlChanged.connect((newUrl) => {      
        });  
      
        let datNameInput = dialog.addTextInput('Data filename: ', '');
        watchForStateChange(datNameInput, "datNameInput", state);
        state["datNameInput"] = "";   

        let settingsNameInput = dialog.addTextInput('Settings: ', '');
        watchForStateChange(settingsNameInput, "settingsNameInput", state);
        state["settingsNameInput"] = "";           
                
        dialog.addSeparator("Tileset"); 
        
         let dirsComboBox = dialog.addComboBox("Folders", "");          
        dirsComboBox.toolTip = "Choose tileset";       
        state["dirsComboBox"] = "";             
        watchForStateChange(dirsComboBox, "dirsComboBox", state);
          
        let filesComboBox = dialog.addComboBox("Files", "");          
        filesComboBox.toolTip = "Choose file"; 
        state["filesComboBox"] = "";             
        watchForStateChange(filesComboBox, "filesComboBox", state);
        
        dialog.addHeading("Name: ");        
        let tilesetNameInput = dialog.addTextInput('', tilesetName);
        watchForStateChange(tilesetNameInput, "tilesetNameInput", state);
        state["tilesetNameInput"] = "";
      
        dialog.addHeading("Classname: ");        
        let classnameNameInput = dialog.addTextInput('', '');
        watchForStateChange(classnameNameInput, "classnameNameInput", state);
        state["classnameNameInput"] = "";
      
        dialog.windowTitle = "Import tileset dialog";
  
        dialog.addSeparator("Tile");
        let animateCheckBox = dialog.addCheckBox("Enable tiles animation", true); 
        let transparencyCheckBox = dialog.addCheckBox("Enable tile transparency", true); 

        let fpsEdit = dialog.addNumberInput("FPS:");
        fpsEdit.decimals = 0;
        fpsEdit.maximum = 100;
        fpsEdit.value = 24;
        state["fpsEdit"] = fpsEdit.value;
        watchForStateChange(fpsEdit, "fpsEdit", state); 

        let countEdit = dialog.addNumberInput("Sprite number:");
        countEdit.decimals = 0;
        countEdit.maximum = 250;
        countEdit.value = 0;
        state["countEdit"] = countEdit.value;
        watchForStateChange(countEdit, "countEdit", state); 
        
        let tileWidthEdit = dialog.addNumberInput("Width:");
        tileWidthEdit.decimals = 0;
        tileWidthEdit.maximum = 250;
        tileWidthEdit.value = 0;
        state["tileWidthEdit"] =  tileWidthEdit.value;
        watchForStateChange(tileWidthEdit, "tileWidthEdit", state);
        
        let tileHeightEdit = dialog.addNumberInput("Height:");
        tileHeightEdit.decimals = 0;
        tileHeightEdit.maximum = 250;
        tileHeightEdit.value = 0;
        let height = state["tileHeightEdit"] = tileHeightEdit.value;
        watchForStateChange(tileHeightEdit, "tileHeightEdit", state);                 


        let folders = [];
               
        if (settings.imageFiles.length === 1) { 
            let rootDir = settings.imageFiles[0].root;        
            if (settings.imageFiles[0].subdirs === null) {
                folders.push(rootDir);
                       
                let dirFiles; 
                if (importSettings.settingsFiles.length) {
                    const magicScenesFiles = nrmFile.getFilenamesByFilePath(importSettings.settingsFiles[0]);
                    const imageFiles = nrmFile.getFilenamesByFilePath(rootDir);
                    
                    const values = imageFiles.reduce((acc, item) => {
                        acc[item.name] = item.index;
                        return acc;
                    }, {});
                    
                    dirFiles = [];
                    const counts = [];
                    magicScenesFiles.forEach((msFile, index) => {
                        const magic = new MagicFileParser();
                        magic.readFromBuffer(nrmFile.getImageByIndex(msFile.index).data);
                        
                        const name = Helpers.getFilenameFromPath(magic.imageFilename.toLowerCase());
                        const indx = values[name];
                        const spriteIndex = index;
                        counts.push(magic.spriteCount);

                        dirFiles.push({name: name, index: indx, spriteIndex: spriteIndex});
                    });

                    settings.tilesets[0].sprite.count.push(...counts);
                } else
                    dirFiles = nrmFile.getFilenamesByFilePath(rootDir);
                settings.imageFiles[0].files.push([...dirFiles]);
            }
            else {  
                let dirs; 
              
                if (settings.imageFiles[0].subdirs.length === 0) { 
                    dirs = nrmFile.getRootDirs(rootDir);

                    settings.imageFiles[0].subdirs.push([...dirs]);
                    folders.push(...dirs);
                } else
                    dirs = settings.imageFiles[0].subdirs;

                for (const folder of dirs) {
                    const dirFiles = nrmFile.getFilenamesByFilePath(folder);               
                    settings.imageFiles[0].files.push([...dirFiles]);
                };                 
            }
            classnameNameInput.text = settings.tilesets[0].classname;
            if (settings.datFiles.length) {
                datNameInput.text = settings.datFiles[0];
            }
            
            if (settings.settingsFiles.length) {
                settingsNameInput.text = settings.settingsFiles[0];
            }
            
            if (settings.tilesets[0].sprite !== null) {
                tileHeightEdit.value = settings.tilesets[0].sprite.height;
                tileWidthEdit.value = settings.tilesets[0].sprite.width;
                tileWidthEdit.fps = settings.tilesets[0].sprite.fps;
            } else {
                tileHeightEdit.enabled = false;
                tileWidthEdit.enabled = false;                
                countEdit.enabled = false;                
                fpsEdit.enabled = false;                
            }
            
        } else {
            for (let imageFile of settings.imageFiles) {
                const root = imageFile.root;
                folders.push(root);                
                const dirFiles = nrmFile.getFilenamesByFilePath(root);
                imageFile.files.push([...dirFiles]);
            }
        }
     
        dirsComboBox.currentIndexChanged.connect((value)=>{ 
        
            if (dirsComboBox.currentIndex && settings.imageFiles.length >= 1) {           
                let idx;
                let imgfiles;
                
                settings.currentImageFilesIndex = dirsComboBox.currentIndex;
                
                if (settings.imageFiles.length === 1) {
                    idx = 0;                    
                    if (settings.imageFiles[idx].subdirs !== null) {
                        settings.imageFiles[idx].subIndex = dirsComboBox.currentIndex - 1;
                        imgfiles = settings.imageFiles[idx].files[settings.imageFiles[idx].subIndex];
                        settings.currentImageFilesIndex = 1;     
                  } else {
                        
                        imgfiles = settings.imageFiles[0].files[0];

                   }
                } else {              
                    idx = dirsComboBox.currentIndex -  1;                    
                    
                    imgfiles = settings.imageFiles[idx].files[0];
                }                 
        

                const filenames = [""];
                for (const file of imgfiles) {;
                    filenames.push(file.name);                     
                }
                
                filesComboBox.clear();
                filesComboBox.addItems([...filenames]);        
                filesComboBox.currentIndex = 1;               
                                
                const index = settings.imageFiles[idx].tileset;           
             
                classnameNameInput.text = settings.tilesets[index].classname;
                
                if (settings.datFiles.length)
                    if (settings.imageFiles[idx].dat !== null) {
                        let dir = settings.datFiles[settings.imageFiles[idx].dat];
                        datNameInput.text = dir !== null ? dir : "";
                    } else
                        datNameInput.text = "";
                    
                if (settings.tilesets[0].sprite !== null) {    
                    tileHeightEdit.value = settings.tilesets[index].sprite.height;
                    tileWidthEdit.value = settings.tilesets[index].sprite.width;
                   
                } else {
                    tileHeightEdit.enabled = false;
                    tileWidthEdit.enabled = false;                
                }
                
                animateCheckBox.checked = settings.tilesets[index].animation;
                transparencyCheckBox.checked = settings.tilesets[index].transparency;
           }
        });
        
        filesComboBox.currentIndexChanged.connect((value)=>{
                const indx = settings.currentImageFilesIndex - 1;
                settings.imageFiles[indx].index = filesComboBox.currentIndex;
                
                if (settings.imageFiles[indx].index > 0) {
                    const index = settings.imageFiles[indx].tileset;
                    if (settings.tilesets[index].sprite !== null) { 
                        const imageFiles = settings.imageFiles[indx];
                        const tilesetIndex = imageFiles.tileset;
                        const findex = imageFiles.index - 1;
                        const subIndex = imageFiles.subIndex;
                        
                        const spriteIndex = imageFiles.files[subIndex][findex].spriteIndex;
                        countEdit.value =settings.tilesets[index].sprite.count[spriteIndex];
                    }
                }

       });

        dirsComboBox.addItems([...folders]);
        dirsComboBox.currentIndex = 1;         
        
        let importButton = dialog.addButton("Import");
        importButton.clicked.connect(() => {
            const index = settings.currentImageFilesIndex; 
            if (index) { 
                const imageFiles = settings.imageFiles[index - 1];            
                const idx = imageFiles.tileset;
                const findex = imageFiles.index;
                const subIndex = imageFiles.subIndex; 
                const spriteIndex = imageFiles.files[subIndex][findex].spriteIndex; 
                               
                settings.tilesets[idx].name = tilesetNameInput.text;
                settings.tilesets[idx].transparency = transparencyCheckBox.checked;
                settings.tilesets[idx].animation = animateCheckBox.checked;
                
                if (settings.tilesets[idx].sprite !== null) {
                    settings.tilesets[idx].sprite.count[spriteIndex] = countEdit.value;
                    // settings.tilesets[idx].sprite.height = tileHeightEdit.value;
                    // settings.tilesets[idx].sprite.width = tileWidthEdit.value;
                    settings.tilesets[idx].sprite.fps = fpsEdit.value;
                }
                
                if (settings.datFiles.length !== 0)
                    settings.datFiles[0] = [datNameInput.text]; 
                dialog.done(1);                
            }
        });
        
        let cancelButton = dialog.addButton("Cancel");
        cancelButton.clicked.connect(() => {
            dialog.done(0);
        });
            
        if (dialog.exec() === Dialog.Accepted) {
            return createArchiveTileset(nrmFile, settings);  
        } else {
            return createTileset("Empty");
        }      
    }
}

function getTileDataFromMapFileTiles(mapFile, datFile, isAnimated, startIndex) {
    let groupIndex = 0;
    let index = -1;
    let filename = "";
    let tilesData = {};
    const endIndex = startIndex + datFile.groups.length  + 1;
    
    for (const tile of mapFile.map.backgroundTiles) { 
        groupIndex = tile.groupIndex;
 
        if ((groupIndex >= startIndex) && (groupIndex < endIndex)) {
            if (startIndex > 0)        
                groupIndex = groupIndex % startIndex - 1; 
        
            const name = datFile.groups[groupIndex]["name"];
            filename = `${name}_${tile.spriteIndex + 1}.bmp`;
            if (isAnimated)
                filename = name.toLowerCase(); 
            
            if (!tilesData.hasOwnProperty(filename)) {
                const size = parseInt(datFile.groups[groupIndex]["size"]);
                if (isAnimated)
                    index += size;
                else
                    index += 1;

                tilesData[filename] = {size: size, index: index};
                if (datFile.groups[groupIndex].length > 2) {
                    tilesData[filename]["snd"] = parseInt(datFile.groups[groupIndex]["snd"])            
                }         
            }  
        }
    }
  
    return tilesData;
}


function createTilesetFromImages(nrmFile, tilesData, isAnimated) {       
    const tileset = createTileset(nrmFile.filename, "background");

    for (const name in tilesData) {
        const img = nrmFile.getImageByName(name, isAnimated); 
         
        if (!isAnimated) {
            addImageTileToTileset(img.data, tileset, name, true);
        } else {        
            addImageTilesToTileset(img, tileset, name, tilesData[name].size);
        };
    } 
                
    return tileset;
}

function createTilesetFromMapTiles(map, backgroundmapLayer, mapFile, nrmFile, datFile, isAnimated, startIndex = 0) {
    // get tiles which will be loaded for current map
    const tilesData = getTileDataFromMapFileTiles(mapFile, datFile, isAnimated, startIndex);
    let tileset = createTilesetFromImages(nrmFile, tilesData, isAnimated);
    
    map.addTileset(tileset);
    
    let layerEdit = backgroundmapLayer.edit();

    let coll = 1, row = 0;
    
    let groupIndex = 0;
    const endIndex = startIndex + datFile.groups.length + 1;
    for (const tile of mapFile.map.backgroundTiles) {  
        groupIndex = tile.groupIndex;

        if (groupIndex >= startIndex && groupIndex < endIndex) { 
            if (startIndex > 0)        
                groupIndex = groupIndex % startIndex - 1;  
        
            const name = datFile.groups[groupIndex]["name"];
            let filename = `${name}_${tile.spriteIndex + 1}.bmp`;
            if (isAnimated)
                filename = name.toLowerCase();            

            layerEdit.setTile(coll - 1, row, tileset.tiles[tilesData[filename].index]);
        } 
        
        if (coll % (mapFile.map.rowSpriteNum) === 0) {
            coll = 0;
            row++;
        }    
        
        coll++;        
    }
     
    layerEdit.apply();     
}

function createbackgroundmapLayer(name, map) {
    let layer = new TileLayer();
    layer.width = map.width;
    layer.height = map.height;
    layer.name = name;        
    map.addLayer(layer);
    
    return layer;    
}
  
let heathMapFormat = {
    name: "Heath: The Unchosen Path map format",
    extension: "map",
    read: function(filename) {
        const mapFile = new MAPFileParser(filename);
        mapFile.read();
        mapFile.close();
        
        let map = new TileMap();  
        map.setSize(mapFile.map.rowSpriteNum, mapFile.map.collSpriteNum);         
        map.setTileSize(TILE_WIDTH, TILE_HEIGHT);
        map.orientation = TileMap.Staggered;         
        
        const backgroundmapLayer = createbackgroundmapLayer("Background", map);
        
        const jsonFilename = getFilepath(scriptPath) + "dat.json";         
        let properties = loadJSONFromFile(jsonFilename);
        // static background sprites
        const backgroundSpritesFile = new NRMFileImageParser(GAME_DIR + "textur.paxx.nrm");
        backgroundSpritesFile.read();        
        const backgroundSpritesDatFile = new DatFileParser(GAME_DIR + "Data\\textur.dat", properties);
        backgroundSpritesDatFile.read();     
        createTilesetFromMapTiles(
            map, backgroundmapLayer, mapFile, backgroundSpritesFile, 
            backgroundSpritesDatFile, false);
             
        // animated background sprites
        const backgroundAnimatedSpritesFile = new NRMFileImageParser(GAME_DIR + "anim.paxx.nrm");
        backgroundAnimatedSpritesFile.read();
        const backgroundAnimatedSpritesDatFile = new DatFileParser(GAME_DIR + "Data\\anim.dat", properties);
        backgroundAnimatedSpritesDatFile.read();  
        createTilesetFromMapTiles(
            map, backgroundmapLayer, mapFile, backgroundAnimatedSpritesFile, 
            backgroundAnimatedSpritesDatFile, true, 5000);

        // const backgroundmapLayer = createbackgroundmapLayer("Pictures", map);
        
        return map;        
    }
}             

tiled.registerMapFormat("Heath (2001) map", heathMapFormat)

 // function onAssetChanged(asset) 
 // {
     // if (asset)
	// {
       // if (asset.isTileset) {

       // }
    // }
 // }
     
tiled.registerTilesetFormat("Heath tileset", heathTilesetFormat);
// tiled.activeAssetChanged.connect(onAssetChanged)