//
const TILE_WIDTH = 112
const TILE_HEIGHT = 64
const PATH_GAME_FOLDER = 'F:/Games/1'
const BACKGROUND_SPRITES_NRM_FILE_PATH = 'F:/Games/1/textur.paxx.nrm'
const ANIMATED_BACKGROUND_SPRITES_NRM_FILE_PATH = 'F:/Games/1/anim.paxx.nrm'
const PICTURES_NRM_FILE_PATH = 'F:/Games/1/packer.paxx.nrm'
const TEXTUR_DAT_FILE_PATH = 'F:/Games/1/Data/textur.dat'
const ANIM_DAT_FILE_PATH = 'F:/Games/1/Data/anim.dat'
const SPRITES_FILE_FORMAT = 'bmp'
const BOOL_IS_RLE_COMPRESSED_DATA = 0x69
const BOOL_IS_UNCOMPRESSED_DATA = 0x6A
const MAGIC = 'PakkaByRCL^DPL2000'
const BMP_HEADER_SIZE = 138
const BMP_HEADER_SIZE1 = 54
const COLOR_RGBA_MAGENTA_INT = 0x00FF00FF
const IS_LITTLE_ENDIAN = true
const BACKGROUND_SPRITE_OFFSET = true
const SPRITES_FROM_ONE_IMAGE = 0
const SPRITES_FROM_MULTIPLE_IMAGES = 1

function getFilenameFromPath(path) {
    return path.replace(/^.*[\\/]/, '');
}

function getFileExtension(fileName) {
    return fileName.substring(fileName.lastIndexOf('.') + 1);
}

function unpack8bitTo32Bit(value) {
    return (value | (value << 8) | (value << 16) | (value << 24));
}  

function splitLastOccurrence(str, substring) {
  const lastIndex = str.lastIndexOf(substring);
  
  let before = '';
  let after = '';
  
  if (lastIndex !== -1) {
    before = str.slice(0, lastIndex);
    after = str.slice(lastIndex + 1);
  }
  
  return [before, after];
}

function create2DArray(m, n) {
    return Array.from({ length: m }, () => Array.from({ length: n }, () => 0));
};

function getKey(val) {
  return [...map].find(([key, value]) => val === value)[0];
}

function setBMPImageHeader(bmpBuffer, width, height) {
    const dataView = new DataView(bmpBuffer);
       
    dataView.setUint16(0, 19778, IS_LITTLE_ENDIAN); // magic
    dataView.setUint32(2, BMP_HEADER_SIZE + width * height * 4, IS_LITTLE_ENDIAN); // bfSize
    dataView.setUint32(6, 0, IS_LITTLE_ENDIAN); // bfReserved
    dataView.setUint32(10, BMP_HEADER_SIZE, IS_LITTLE_ENDIAN); // bfOffBits
        
    dataView.setUint32(14, 124, IS_LITTLE_ENDIAN); // bfSize 
    dataView.setUint32(18, width, IS_LITTLE_ENDIAN); // bfWidth
    dataView.setUint32(22, height, IS_LITTLE_ENDIAN); // bfHeight
    dataView.setUint16(26, 1, IS_LITTLE_ENDIAN); // biPlanes
    dataView.setUint16(28, 32, IS_LITTLE_ENDIAN); // biBitCount
    dataView.setUint32(30, 3, IS_LITTLE_ENDIAN); // biCompression
    
    dataView.setUint32(34, width * height * 4); // biSizeImage
    dataView.setUint32(38, 2835, IS_LITTLE_ENDIAN); // biXPelsPerMeter
    dataView.setUint32(42, 2835, IS_LITTLE_ENDIAN); // biYPelsPerMeter
    dataView.setUint32(46, 0, IS_LITTLE_ENDIAN); // biCrlUsed
    dataView.setUint32(50, 0, IS_LITTLE_ENDIAN); // biClrImportant 

    //ColorTable
    dataView.setUint32(54, 0x0000ff00, IS_LITTLE_ENDIAN); // r     
    dataView.setUint32(58, 0x00ff0000, IS_LITTLE_ENDIAN); // g
    dataView.setUint32(62, 0xff000000, IS_LITTLE_ENDIAN); // b
    dataView.setUint32(66, 0x000000ff, IS_LITTLE_ENDIAN); // a  
}

function convertBMP8BitImageTo32Bit(imageData) {
    const dataView = new DataView(imageData);
    const biCrlUsed = dataView.getUint32(46, IS_LITTLE_ENDIAN); 
    const bfWidth = dataView.getUint32(18, IS_LITTLE_ENDIAN); 
    const bfHeight = dataView.getUint32(22, IS_LITTLE_ENDIAN); 

    const size = bfWidth * bfHeight;    
    
    let data = new ArrayBuffer(size * 4 + BMP_HEADER_SIZE);
    // tiled.log(bfWidth);         
    // tiled.log(bfHeight);         
    const dataView2 = new DataView(data);
    
    setBMPImageHeader(data, bfWidth, bfHeight); 
    
    let colors = [];
    let offset = BMP_HEADER_SIZE1;
    let offset2 = BMP_HEADER_SIZE;

    for (let i = 0; i < biCrlUsed; i++) {
        color = dataView.getUint32(offset, true);
        colors.push(color);
        offset += 4;
    }
    
    a = 0;
    b = true;

    for (let i = 0; i < size; i++) {
        index = dataView.getUint8(offset, true);
         
        let alpha = colors[index] === COLOR_RGBA_MAGENTA_INT ? alpha = 0x00 : alpha = 0xFF; // 0xFF00FF magenta color used for transparency
        let pixel = (alpha) | (((colors[index] >> 16) & 0xFF) << 8) | (((colors[index] >> 8) & 0xFF) << 16) | (((colors[index]) & 0xFF) << 24)        
        dataView2.setUint32(offset2, pixel, IS_LITTLE_ENDIAN);

        offset2 += 4;        
        offset += 1;        
    }
    
    return data;
}

function unpackImageDataFromBuffer(buffer, size) {
    let packedBufferPos = 0;
    let outDataBufferPos = 0;
    const dataView = new DataView(buffer);
    const data = new ArrayBuffer(size);
    const dataView1 = new DataView(data);
                       
    while (packedBufferPos < buffer.byteLength) {
        const controlByte = dataView.getUint8(packedBufferPos, true);
        packedBufferPos += 1;

        // first byte is control one, where 105 means data is rle compressed
        switch (controlByte) {
            case BOOL_IS_RLE_COMPRESSED_DATA:  
           
                let dataByte = dataView.getUint8(packedBufferPos, true); 
                const length = dataView.getUint16(packedBufferPos + 1, true);

                if (length) {
                    // copy byte 4 times to out buffer for length times

                    for (let i = 0; i < (length >> 2); ++i) { 
                        dataView1.setUint32(outDataBufferPos, unpack8bitTo32Bit(dataByte) );
                        outDataBufferPos += 4;
                    }
                    
                    // padding bytes for bmp format                          
                    padding = length & 3; 
                                                                
                    for (let i = 0; i < padding; ++i) {
                        dataView1.setUint8(outDataBufferPos, dataByte);
                         
                       outDataBufferPos += 1;
                    }                              
                };

                packedBufferPos += 3; 
                break;                
            case BOOL_IS_UNCOMPRESSED_DATA:
                const db = dataView.getUint8(packedBufferPos);            
                dataView1.setUint8(outDataBufferPos, db);
                outDataBufferPos += 1;
                packedBufferPos += 1;
                break;
            default:              
                dataView1.setUint8(outDataBufferPos, controlByte);
                outDataBufferPos += 1;
        }
    }    
    
    return data;
}

function createTileset(filename) {
    const tileset = new Tileset(filename);
    tileset.objectAlignment = Tileset.Center;
    tileset.tileRenderSize = Tileset.TileSize;
    tileset.orientation = Tileset.Orthogonal;
    return tileset;
}

function loadTilesetFromArchive(filename, datFile, spriteType, tilsetNames = []) {
    var nrmFile = new BinaryFile(filename, BinaryFile.ReadOnly);
    let filebuffer2 = nrmFile.readAll()
    let dataView1 = new DataView(filebuffer2);
    
    let nrmFilename = getFilenameFromPath(filename);
    
    let path = nrmFilename === "anim.paxx.nrm" ? ANIM_DAT_FILE_PATH : TEXTUR_DAT_FILE_PATH;
    let datFile = new TextFile(path); 
       
    const groupsMap = new Map();
    let groupNum = 0;
    let sprGroups = {};
    
    while (!datFile.atEof) {
      line = datFile.readLine();

      if (line.includes("nGroup")) {
            groupNum = parseInt(line.split(":")[1].trim());
            sprGroups = Array.from({ length: groupNum }, (_, index) => {
                const newMap = new Map();
                return newMap;
            });
      }
      else if (line.includes("name")) {
          splitedStr = line.split(":");
          name = splitedStr[1].trim();
          substr = splitedStr[0].trim();
          index = parseInt(substr.split("_")[1]);
          groupsMap.set(name.toLowerCase(), index);
      }              
    }  
    
    if (spriteType === SPRITES_FROM_MULTIPLE_IMAGES)
        tileset = createTileset(nrmFilename);

    let tilesets = [];    
    
    let pos = 18;
    while (pos < filebuffer2.byteLength) {
        pos += 5; 
        const size = dataView1.getUint32(pos, true);
        const unpackedSize = dataView1.getUint32(pos + 4, true);
        const strlen = dataView1.getUint32(pos + 8, true);  
        
        if (!size) {
            pos += 12;
           continue;
        }
        
        headerEndPos = pos + 12 + strlen;
        
        const filepath = filebuffer2.slice(pos + 12, headerEndPos).toString();

        const imageDataBuffer = filebuffer2.slice(headerEndPos, headerEndPos + size - strlen);         
        let bmpImage8bit = unpackImageDataFromBuffer(imageDataBuffer, unpackedSize);
     
        const img = new Image();  
                
        
        if (spriteType === SPRITES_FROM_MULTIPLE_IMAGES) {
            img.loadFromData(convertBMP8BitImageTo32Bit(bmpImage8bit), SPRITES_FILE_FORMAT);
            const filename = getFilenameFromPath(filepath);            
            const tile = tileset.addTile(filename);        
            tile.setImage(img);
            const substr = splitLastOccurrence(filename, "_");
            const id = groupsMap.get(substr[0].trim());
            if (id !== undefined) {
                let spriteId  = parseInt(substr[1].trim()) - 1;
                tile.setProperty("GroupId", id);
                tile.setProperty("SpriteId", spriteId);
                tile.setProperty("Filename", filename);
                tile.setProperty("Filename2", substr[0].trim());
            
                sprGroups[id].set(spriteId, tile.id);
            }
           
        } else {
            let name = filepath.split("\\")[1];
            let tileset = createTileset(name);            
            img.loadFromData(bmpImage8bit, SPRITES_FILE_FORMAT);        
            tileset.tileSize.height = TILE_HEIGHT;
            tileset.tileSize.width = TILE_WIDTH;
            tileset.margin = 0;
            tileset.tileSpacing = 0;
            tileset.transparentColor = "#ff00ff";
            tileset.loadFromImage(img);
            
            let animTile = tileset.tiles[0]; 
            frames = [];            
            tileset.tiles.forEach((value, index) => {             
                frames.push({tileId: index, duration: 100})
            });
            animTile.frames = frames; 

            let id = groupsMap.get(name);
            tileset.setProperty("GroupId", id);
            tilesets.push(tileset);           
        }
        
        pos += 12 + size;             
    };   
    
    if (spriteType === SPRITES_FROM_MULTIPLE_IMAGES)
        tilesets.push(tileset);
        
    nrmFile.close();
    return [tilesets, sprGroups];    
}

var heathTilesetFormat = {
    name = "Heath: The Unchosen Path tileset format",
    extension = "nrm",
    read: function (filename) {
        if (getFilenameFromPath(filename) === "anim.paxx.nrm")
            return loadTilesetFromArchive(filename, SPRITES_FROM_ONE_IMAGE)[0][0];            
        else        
            return loadTilesetFromArchive(filename, SPRITES_FROM_MULTIPLE_IMAGES)[0][0];        
    }
}  

var heathMapFormat = {
    name = "Heath: The Unchosen Path map format",
    extension = "map",
    read: function(fileName) {
        var mapFile = new BinaryFile(fileName, BinaryFile.ReadOnly);
        let filebuffer1 = mapFile.readAll()
        let dataView = new DataView(filebuffer1);
        
        const mapCollSpriteNum = dataView.getUint16(0, true);
        const mapRowSpriteNum = dataView.getUint16(2, true);  
        
        const backgroundSpriteNum = dataView.getUint16(8, true);
        const pictureSpriteNum = dataView.getUint16(10, true);
        const envSpriteNum = dataView.getUint16(12, true);
        const tileNum = dataView.getUint32(16, true);

        var map = new TileMap();  
        map.setSize(mapCollSpriteNum, mapRowSpriteNum);         
        map.setTileSize(TILE_WIDTH, TILE_HEIGHT);
        map.orientation = TileMap.Staggered;    

        let ofs = 36;
        
        let layer = new TileLayer();
        layer.width = map.width;
        layer.height = map.height;
        layer.name = "Background";        
        var layerEdit = layer.edit(); 
        
        let backGroundTilesetData = loadTilesetFromArchive(BACKGROUND_SPRITES_NRM_FILE_PATH, SPRITES_FROM_MULTIPLE_IMAGES);
        let bgTileset = backGroundTilesetData[0][0];
        let backGroundSpriteGrArray = backGroundTilesetData[1];

        const animatedBackGroundTilesetData = loadTilesetFromArchive(ANIMATED_BACKGROUND_SPRITES_NRM_FILE_PATH, SPRITES_FROM_ONE_IMAGE);
        const anTilesets = animatedBackGroundTilesetData[0];

        const picturesTilesetData = loadTilesetFromArchive(PICTURES_NRM_FILE_PATH, SPRITES_FROM_MULTIPLE_IMAGES, {'picture', 'lowpicture'});
        const pictureTilesets = picturesTilesetData[0];        
        let pictureGrArray = backGroundTilesetData[1];        

        map.addTileset(bgTileset); 
        animatedBackGroundTilesetData[0].forEach((value, index) => {                       
            map.addTileset(value); 
        });           
   
        let col = row = 0;
                      
        for (let index = 0; index < backgroundSpriteNum; index++) {
            const groupIndex = dataView.getUint16(ofs, true);
            const groupSpriteIndex = dataView.getUint16(ofs + 2, true);
            ofs += 4;
            
            if (groupIndex >= 5000) {
                // 
                if ((groupIndex % 5000) <= 36) {                      
                    layerEdit.setTile(col, row, map.tilesets[(groupIndex % 5000)].tiles[0]);
                }
                
                else
                    tiled.log(groupIndex );                
            }                
            else 
                layerEdit.setTile(col, row, bgTileset.tiles[backGroundSpriteGrArray[groupIndex].get(groupSpriteIndex)]);
                           
            col += 1;  
            if (col == mapCollSpriteNum) {
                row += 1;
                col = 0;
            };                
        };
        
        ofs = 7708;
        for (let index = 0; index < envSpriteNum; index++) {
            const groupIndex = dataView.getUint8(ofs, true);
            const groupSpriteIndex = dataView.getUint8(ofs + 1, true);
            const xOffset = dataView.getUint16(ofs + 2, true);
            const yOffset = dataView.getUint16(ofs + 4, true);
            const xTileIndex = dataView.getUint8(ofs + 6, true);
            const yTileIndex = dataView.getUint8(ofs + 7, true);
            
            ofs += 8;

            layerEdit.setTile(col, row, envTileset.tiles[envSpriteGrArray[groupIndex].get(groupSpriteIndex)]);
        };       
    
        layerEdit.apply();
        map.addLayer(layer);

        mapFile.close();       
        return map;        
    }
}             

tiled.registerMapFormat("Heath (2001) map", heathMapFormat)
tiled.registerTilesetFormat("Heath tileset", heathTilesetFormat)