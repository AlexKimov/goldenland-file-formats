const SPRITE_COUNT_OFFSET = 3020 
const IMAGE_FILENAME_OFFSET = 9196 

class MagicFileParser {
    constructor() {
        this.spriteCount = 0;
        this.imageFilename = "";     
    }   
    
    readFromBuffer(buffer) { 
        let dataView = new DataView(buffer);
        
        this.spriteCount = dataView.getUint32(SPRITE_COUNT_OFFSET, true);
        let imageFilenameLength = dataView.getUint32(SPRITE_COUNT_OFFSET + 4, true);
              
        this.imageFilename = buffer.slice(IMAGE_FILENAME_OFFSET, IMAGE_FILENAME_OFFSET + 256).toString().toLowerCase();         
    }
}

export default MagicFileParser;