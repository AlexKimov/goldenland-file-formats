class MAPTile {
    constructor(filename) {
        this.col = 0;          
        this.row = 0;
        this.tileIndex = 0;        
    }  
}

class MAPFileParser {
    constructor(filename, datFilename) {
        this.filename = filename;  
        this.datFilename = datFilename;  
        this.file = new BinaryFile(filename, BinaryFile.ReadOnly);  
        this.filebuffer = null;
        this.tiles = [];        
    }

    read() { 
        this.filebuffer = this.file.readAll();
        let dataView = new DataView(this.filebuffer);
        
        const mapCollSpriteNum = dataView.getUint16(0, true);
        const mapRowSpriteNum = dataView.getUint16(2, true);  
        
        const backgroundSpriteNum = dataView.getUint16(8, true);
        const pictureSpriteNum = dataView.getUint16(10, true);
        const envSpriteNum = dataView.getUint16(12, true);
        const tileNum = dataView.getUint32(16, true);
    
        let ofs = 36;
        let col = row = 0;
        
        for (let index = 0; index < backgroundSpriteNum; index++) {
            const groupIndex = dataView.getUint16(ofs, true);
            const groupSpriteIndex = dataView.getUint16(ofs + 2, true);
            ofs += 4;
            
            if (groupIndex >= 5000) {           
                if ((groupIndex % 5000) <= 36) { 
                    this.tiles.push(MAPTile(col, row, index)); 
                } else
                    tiled.log(groupIndex );                
            }                
            else 
                this.tiles.push(MAPTile(col, row, index));
                           
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
            
    }
}

export default MAPFileParser;
