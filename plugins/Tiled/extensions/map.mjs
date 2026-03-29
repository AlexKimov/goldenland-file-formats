class MAPTile {
    constructor(filename) {
        this.col = 0;          
        this.row = 0;
        this.tileIndex = 0;        
    }  
}

class MAPObject {
    constructor(filename) {
        this.collSpriteNum = 0;          
        this.rowSpriteNum = 0;
        this.backgroundSpriteNum = 0;        
        this.objectsSpriteNum = 0;        
        this.environmentSpriteNum = 0;        
        this.constraintsObjectsNum = 0;

        this.backgroundTiles = [];        
        this.objectsTiles = [];        
    }  
}

class MAPFileParser {
    constructor(filename) {
        this.filename = filename;  
        this.file = new BinaryFile(filename, BinaryFile.ReadOnly);  
        this.filebuffer = null;
        this.map = new MAPObject();        
    }
    
    close() {
        this.file.close();
    }
    
    read() { 
        this.filebuffer = this.file.readAll();
        let dataView = new DataView(this.filebuffer);
           
        this.map.rowSpriteNum = dataView.getUint16(0, true);
        this.map.collSpriteNum = dataView.getUint16(2, true);  
        
        this.map.backgroundSpriteNum = dataView.getUint16(8, true);
        this.map.objectsSpriteNum = dataView.getUint16(10, true);
        this.map.environmentSpriteNum = dataView.getUint16(12, true);
        this.map.constraintsObjectsNum = dataView.getUint32(16, true);
  
        let ofs = 36;
        // let col = row = 0;
        
        for (let index = 0; index < this.map.backgroundSpriteNum; index++) {
            const groupIndex = dataView.getUint16(ofs, true);
            const groupSpriteIndex = dataView.getUint16(ofs + 2, true);
            
            this.map.backgroundTiles.push({
                groupIndex: groupIndex, 
                spriteIndex: groupSpriteIndex
            });
            ofs += 4;               
        };
        
        ofs = 7708;
        for (let index = 0; index < this.map.environmentSpriteNum; index++) {
            const groupIndex = dataView.getUint8(ofs, true);
            const groupSpriteIndex = dataView.getUint8(ofs + 1, true);
            const xOffset = dataView.getUint16(ofs + 2, true);
            const yOffset = dataView.getUint16(ofs + 4, true);
            const xTileIndex = dataView.getUint8(ofs + 6, true);
            const yTileIndex = dataView.getUint8(ofs + 7, true);
            
            this.map.objectsTiles.push({
                groupIndex: groupIndex, 
                spriteIndex: groupSpriteIndex,
                xOffset: xOffset,
                yOffset: yOffset,
                xTileIndex: xTileIndex,
                yTileIndex: yTileIndex
            });
            
            ofs += 8;
        };       
                   
    }
}

export default MAPFileParser;
